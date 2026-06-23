#include <thread>
#include <chrono>

#include "test_helper.h"

#include "qdmq/vision/vbuf.h"
#include "qdmq/vision/vipc_server.h"
#include "qdmq/vision/vipc_client.h"

static void zmq_sleep(int milliseconds=1000)
{
  if (messaging_use_zmq()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  }
}

void test_mq_vision_vbuf(void)
{
  const size_t len = 4096;
  shm_t fd;

  void *ptr = malloc_with_fd(len, &fd);
  TEST_ASSERT(ptr != NULL);

  char *byte_ptr = (char *)ptr;
  const char *msg = "VisionTestOO";
  size_t msglen = strlen(msg) + 1;
  assert(msglen <= len);

  memcpy(byte_ptr, msg, msglen);
  TEST_ASSERT(strcmp(byte_ptr, msg) == 0);

#ifdef _WIN32
  UnmapViewOfFile(ptr);
  CloseHandle(fd);
#else
  munmap(ptr, len);
  close(fd);
#endif
}

void test_mq_vision_connecting(void)
{
  VisionIpcServer server("camerad");
  server.create_buffers(VISION_SENSOR_MONO16, 1, 100, 100);
  server.start_listener();

  VisionIpcClient client = VisionIpcClient("camerad", VISION_SENSOR_MONO16, false);
  TEST_ASSERT(client.connect());
  TEST_ASSERT(client.connected);
}

void test_mq_vision_check_buffers(void)
{
  size_t width = 100, height = 200, num_buffers = 5;

  VisionIpcServer server("camerad");
  server.create_buffers(VISION_SENSOR_MONO16, num_buffers, width, height);
  server.start_listener();

  VisionIpcClient client = VisionIpcClient("camerad", VISION_SENSOR_MONO16, false);

  TEST_ASSERT(client.connect());
  TEST_ASSERT(client.buffers[0].width == width);
  TEST_ASSERT(client.buffers[0].height == height);
  TEST_ASSERT(client.buffers[0].len);
  TEST_ASSERT(client.num_buffers == num_buffers);
}

void test_mq_vision_send_single_buffer(void)
{
  VisionIpcServer server("camerad");
  server.create_buffers(VISION_SENSOR_MONO16, 1, 100, 100);
  server.start_listener();

  VisionIpcClient client = VisionIpcClient("camerad", VISION_SENSOR_MONO16, false);
  TEST_ASSERT(client.connect());
  zmq_sleep();

  VisionBuf * buf = server.get_buffer(VISION_SENSOR_MONO16);
  TEST_ASSERT(buf != nullptr);

  *((uint64_t*)buf->addr) = 1234;

  VisionIpcBufExtra extra = {0};
  extra.frame_id = 1337;
  buf->set_frame_id(extra.frame_id);

  server.send(buf, &extra);

  VisionIpcBufExtra extra_recv = {0};
  VisionBuf * recv_buf = client.recv(&extra_recv);

  TEST_ASSERT(recv_buf != nullptr);
  TEST_ASSERT(*(uint64_t*)recv_buf->addr == 1234);
  TEST_ASSERT(extra_recv.frame_id == extra.frame_id);
  TEST_ASSERT(recv_buf->get_frame_id() == extra.frame_id);
}
