#include "vipc.h"

#ifdef _WIN32
HANDLE ipc_connect(const char* pipe_name)
{
  HANDLE hpipe = CreateFileA(
    pipe_name,
    GENERIC_READ | GENERIC_WRITE,
    0,
    NULL,
    OPEN_EXISTING,
    0,
    NULL
  );

  if (hpipe == INVALID_HANDLE_VALUE) {
    return INVALID_HANDLE_VALUE;
  }
  return hpipe;
}

HANDLE ipc_bind(const char* pipe_name)
{
  HANDLE hpipe = CreateNamedPipeA(
    pipe_name,
    PIPE_ACCESS_DUPLEX,
    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
    PIPE_UNLIMITED_INSTANCES,
    4096,
    4096,
    0,
    NULL
  );

  if (hpipe == INVALID_HANDLE_VALUE) {
    return INVALID_HANDLE_VALUE;
  }
  bool connected = ConnectNamedPipe(hpipe, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED);
  if (!connected) {
    CloseHandle(hpipe);
    return INVALID_HANDLE_VALUE;
  }
  return hpipe;
}

int ipc_sendrecv_with_fds(bool   send, 
                          HANDLE h, 
                          void  *buf, 
                          size_t buf_size, 
                          int   *hs, 
                          int    num_h,
                          int   *out_num_hs) 
{
  if (send) {
    DWORD bytesw;

    bool ok = WriteFile(h, buf, buf_size, &bytesw, NULL);
    if (!ok) {
      return -1;
    }
    return (int)bytesw;

    // if (num_fds) {
    //   struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    //   assert(cmsg);
    //   cmsg->cmsg_level = SOL_SOCKET;
    //   cmsg->cmsg_type = SCM_RIGHTS;
    //   cmsg->cmsg_len = CMSG_LEN(sizeof(int) * num_fds);
    //   memcpy(CMSG_DATA(cmsg), fds, sizeof(int) * num_fds);
    // }
    // return sendmsg(fd, &msg, 0);
  } else {
    return -1;
  }
}
#else
#define getsocket() socket(AF_UNIX, SOCK_SEQPACKET, 0)

int ipc_connect(const char* socket_path) 
{
  int err;
  int sock = getsocket();

  if (sock < 0) return -1;
  struct sockaddr_un addr = {
    .sun_family = AF_UNIX,
  };
  snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", socket_path);

  err = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
  if (err != 0) {
    close(sock);
    return -1;
  }

  return sock;
}

int ipc_bind(const char* socket_path) 
{
  int err;
  unlink(socket_path);
  int sock = getsocket();

  struct sockaddr_un addr = {
    .sun_family = AF_UNIX,
  };
  snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", socket_path);
  err = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  assert(err == 0);

  err = listen(sock, 3);
  assert(err == 0);

  return sock;
}


int ipc_sendrecv_with_fds(bool send, 
                          int fd, 
                          void *buf, 
                          size_t buf_size, 
                          int* fds, 
                          int num_fds,
                          int *out_num_fds) 
{
  char control_buf[CMSG_SPACE(sizeof(int) * num_fds)];
  memset(control_buf, 0, CMSG_SPACE(sizeof(int) * num_fds));

  struct iovec iov = {
    .iov_base = buf,
    .iov_len = buf_size,
  };
  struct msghdr msg = {
    .msg_iov = &iov,
    .msg_iovlen = 1,
  };

  if (num_fds > 0) {
    assert(fds);

    msg.msg_control = control_buf;
    msg.msg_controllen = CMSG_SPACE(sizeof(int) * num_fds);
  }

  if (send) {
    if (num_fds) {
      struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
      assert(cmsg);
      cmsg->cmsg_level = SOL_SOCKET;
      cmsg->cmsg_type = SCM_RIGHTS;
      cmsg->cmsg_len = CMSG_LEN(sizeof(int) * num_fds);
      memcpy(CMSG_DATA(cmsg), fds, sizeof(int) * num_fds);
    }
    return sendmsg(fd, &msg, 0);
  } else {
    int r = recvmsg(fd, &msg, 0);
    if (r < 0) return r;

    int recv_fds = 0;
    if (msg.msg_controllen > 0) {
      struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
      assert(cmsg);
      assert(cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS);
      recv_fds = (cmsg->cmsg_len - CMSG_LEN(0));
      assert(recv_fds > 0 && (recv_fds % sizeof(int)) == 0);
      recv_fds /= sizeof(int);

      assert(fds && recv_fds <= num_fds);
      memcpy(fds, CMSG_DATA(cmsg), sizeof(int) * recv_fds);
    }

    if (msg.msg_flags) {
      for (int i=0; i<recv_fds; i++) {
        close(fds[i]);
      }
      return -1;
    }

    if (fds) {
      assert(out_num_fds);
      *out_num_fds = recv_fds;
    }
    return r;
  }
}
#endif
