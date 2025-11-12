// server.c
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

long custom_poll(struct pollfd *fds,
                 nfds_t nfds,
                 const struct timespec *tmo,
                 const void *sigmask,
                 size_t sigsetsize);


int main()
{
  int listen_fd, conn_fd;
  struct sockaddr_in addr;
  char buf[1024];

  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) return 1;

  int opt = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
  addr.sin_port        = htons(5555);

  if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(listen_fd); return 1; }
  if (listen(listen_fd, 1) < 0)                                   { close(listen_fd); return 1; }

  printf("(Server) Listening on 127.0.0.1:5555\n");
  struct pollfd pfd;
  pfd.fd      = listen_fd;
  pfd.events  = POLLIN;
  pfd.revents = 0;

#if CUSTOM
  int timeout_ms = 5000;

  struct timespec ts;
  ts.tv_sec  = timeout_ms / 1000;
  ts.tv_nsec = (timeout_ms % 1000) * 1000000L;

  long n = custom_poll(&pfd, 1, &ts, NULL, 0);;
#else
  int n = poll(&pfd, 1, -1);
#endif
  if (n < 0)                   { close(listen_fd); return 1; }
  if (!(pfd.revents & POLLIN)) { close(listen_fd); return 1; }

  conn_fd = accept(listen_fd, NULL, NULL);
  if (conn_fd < 0) { close(listen_fd); return 1; }

  ssize_t r = read(conn_fd, buf, sizeof(buf) - 1);
  if (r < 0) { perror("read"); } else {
    buf[r] = '\0';
    printf("(Server) Received from client: %s\n", buf);
  }

  close(conn_fd);
  close(listen_fd);
  return 0;
}
