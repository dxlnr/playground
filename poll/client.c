#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main(void) {
  int sock;
  struct sockaddr_in addr;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) return 1;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(5555);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
  {
    close(sock);
    return 1;
  }
  const char *msg = "'Hello from client'";
  if (write(sock, msg, strlen(msg)) < 0)
  {
    close(sock);
    return 1;
  }
  printf("(Client) Message sent, closing\n");
  close(sock);
  return 0;
}
