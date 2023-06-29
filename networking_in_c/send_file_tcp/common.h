#include <sys/types.h>

ssize_t tcp_send(int sockfd, void *buf, ssize_t payload_len);
ssize_t tcp_recv(int sockfd, void *buf, ssize_t payload_len);