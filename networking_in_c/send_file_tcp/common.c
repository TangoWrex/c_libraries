#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

ssize_t tcp_send(int sockfd, void* buf, ssize_t payload_len)
{
    ssize_t bytes_sent;
    if ((bytes_sent = send(sockfd, buf, payload_len, 0)) < 0) {
        perror("send failed");
    }
    return bytes_sent;
}

ssize_t tcp_recv(int sockfd, void* buf, ssize_t payload_len)
{
    ssize_t total_bytes_recvd = 0;
    ssize_t curr_bytes_recvd = 0;
    while ((curr_bytes_recvd = recv(sockfd, &buf[total_bytes_recvd], sizeof buf - curr_bytes_recvd, 0)) > 0
        && curr_bytes_recvd < total_bytes_recvd) {

        total_bytes_recvd += curr_bytes_recvd;
    }
    if (curr_bytes_recvd < 0) {
        perror("recv");
    } else if (curr_bytes_recvd == 0) {
        // EOS on the socket: close it, exit the thread, etc
        return 0;
    }

    return total_bytes_recvd;
}
