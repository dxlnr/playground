#include "vipc.h"
#include "vipc_server.h"

std::string get_endpoint_name(std::string name, VisionType type)
{
  if (messaging_use_zmq()) {
    assert(name == "camerad" || name == "lived");
    return std::to_string(9000 + static_cast<int>(type));
  } else {
    return "visionipc_" + name + "_" + std::to_string(type);
  }
}

std::string get_ipc_path(const std::string &name) 
{
#ifdef _WIN32
  std::string path = R"(\\.\pipe\)";
  if (char *prefix = std::getenv("QDM_PREFIX")) {
    path += std::string(prefix) + "_";
  }
  return path + "visionipc_" + name;
#else
  std::string path = "/tmp/";
  if (char *prefix = std::getenv("QDM_PREFIX")) {
    path += std::string(prefix) + "_";
  }
  return path + "visionipc_" + name;
#endif
}

VisionIpcServer::VisionIpcServer(std::string name) : name(std::move(name)) 
{
  msg_ctx = Context::create();

#ifdef _WIN32
  std::random_device rd;
#else
  std::random_device rd("/dev/urandom");
#endif
  std::uniform_int_distribution<uint64_t> distribution(
    0, std::numeric_limits<uint64_t>::max()
  );
  server_id = distribution(rd);
}

void VisionIpcServer::create_buffers(VisionType type, size_t num_buffers,
                                                      size_t width,
                                                      size_t height)
{
  assert(num_buffers < VISIONIPC_MAX_FDS);
  size_t size;

  if      (type == VisionType::VISION_SENSOR_MONO8)  { size = width * height; }
  else if (type == VisionType::VISION_SENSOR_MONO16) { size = width * height * 2; }
  else                                               { size = 0; }
  create_buffers_with_sizes(type, num_buffers, width, height, size);
}

void VisionIpcServer::create_buffers_with_sizes(VisionType type, 
                                                size_t num_buffers, 
                                                size_t width, 
                                                size_t height, 
                                                size_t size) 
{
  for (size_t i = 0; i < num_buffers; i++)
  {
    VisionBuf* buf = new VisionBuf();
    buf->allocate(size);
    buf->idx = i;
    buf->type = type;
    buf->init_mono(width, height);
    buffers[type].push_back(buf);
  }
  cur_idx[type] = 0;
  // Create msgq publisher for each of the `name` + type combos
  //
  // TODO: compute port number directly if using zmq
  sockets[type] = PubSocket::create(msg_ctx, get_endpoint_name(name, type), false);
}

void VisionIpcServer::start_listener()
{
  listener_thread = std::thread(&VisionIpcServer::listener, this);
}

#ifdef _WIN32
void VisionIpcServer::listener()
{
  const std::string ipc_path = get_ipc_path(name);
  HANDLE pipe = ipc_bind(ipc_path.c_str());
  assert(pipe != INVALID_HANDLE_VALUE);
}
#else
void VisionIpcServer::listener()
{
  const std::string ipc_path = get_ipc_path(name);
  int sock = ipc_bind(ipc_path.c_str());
  assert(sock >= 0);

  while (!should_exit) {
    // Wait for incoming connection
    struct pollfd polls[1] = {{0}};
    polls[0].fd = sock;
    polls[0].events = POLLIN;

    int ret = poll(polls, 1, 100);
    if (ret < 0) {
      if (errno == EINTR || errno == EAGAIN) continue;
      std::cout << "poll failed, stopping listener" << std::endl;
      break;
    }

    if (should_exit) break;
    if (!polls[0].revents) {
      continue;
    }
    // Handle incoming request
    int fd = accept(sock, NULL, NULL);
    assert(fd >= 0);

    VisionType type = VisionType::VISION_STREAM_MAX;
    int r = ipc_sendrecv_with_fds(false, fd, &type, sizeof(type), nullptr, 0, nullptr);
    assert(r == sizeof(type));

    // send available stream types
    if (type == VisionType::VISION_STREAM_MAX) {
      std::vector<VisionType> available_stream_types;
      for (auto& [stream_type, _] : buffers) {
        available_stream_types.push_back(stream_type);
      }
      r = ipc_sendrecv_with_fds(true, fd, available_stream_types.data(), available_stream_types.size() * sizeof(VisionType), nullptr, 0, nullptr);
      assert(r == available_stream_types.size() * sizeof(VisionType));
      close(fd);
      continue;
    }
    if (buffers.count(type) <= 0) {
      std::cerr<< "got request for invalid buffer type: " << type << std::endl;
      close(fd);
      continue;
    }

    int fds[VISIONIPC_MAX_FDS];
    int num_fds = buffers[type].size();
    VisionBuf bufs[VISIONIPC_MAX_FDS];

    for (int i = 0; i < num_fds; i++)
    {
      fds[i] = buffers[type][i]->fd;
      bufs[i] = *buffers[type][i];
      bufs[i].server_id = server_id;
    }
    r = ipc_sendrecv_with_fds(true, fd, &bufs, sizeof(VisionBuf) * num_fds, fds, num_fds, nullptr);
    connected = true;

    close(fd);
  }
  close(sock);
  unlink(ipc_path.c_str());
}
#endif

VisionBuf * VisionIpcServer::get_buffer(VisionType type)
{
  // Do we want to keep track if the buffer has been sent out yet and warn user?
  assert(buffers.count(type));
  auto b = buffers[type];
  return b[cur_idx[type]++ % b.size()];
}

void VisionIpcServer::send(VisionBuf * buf, VisionIpcBufExtra * extra, bool sync)
{
  assert(buffers.count(buf->type));
  assert(buf->idx < buffers[buf->type].size());

  VisionIpcPacket packet = {0};
  packet.server_id = server_id;
  packet.idx = buf->idx;
  packet.extra = *extra; 

  sockets[buf->type]->send((char*)&packet, sizeof(packet));
}

VisionIpcServer::~VisionIpcServer()
{
  should_exit = true;
  // clear up thread if `listener` was called
  if (listener_thread.joinable())
    listener_thread.join();
  // VisionBuf cleanup
  for (auto const& [type, buf] : buffers) {
    for (VisionBuf* b : buf) {
      if (b->free() != 0)
        std::cerr << "Failed to free buffer\n";
      delete b;
    }
  }
  // Messaging cleanup
  for (auto const& [type, sock] : sockets)
    delete sock;
  // ctx cleanup
  delete msg_ctx;
}
