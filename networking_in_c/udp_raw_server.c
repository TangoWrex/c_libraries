#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sysexits.h>
#include <unistd.h>

#define PCKT_LEN    8192
#define HEADER_SIZE (sizeof(struct iphdr) + sizeof(struct udphdr))

int main(int argc, char const * argv[])
{
    if (argc != 2)
    {
        printf("Error: Invalid parameters!\n");
        printf("Usage: %s  <source port> \n", argv[0]);
        return EX_USAGE;
    }

    struct addrinfo hints = { 0 };
    hints.ai_family       = PF_UNSPEC;
    hints.ai_socktype     = SOCK_DGRAM;
    hints.ai_flags        = AI_PASSIVE;
    struct addrinfo * results;

    int err = getaddrinfo(NULL, argv[1], &hints, &results);
    if (err != 0)
    {
        fprintf(stderr, "Cannot get address: %s\n", gai_strerror(err));
        return EX_NOHOST;
    }

    // create a raw socket with UDP protocol
    int raw_socket = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_socket < 0)
    {
        perror("Could not create socket");
        freeaddrinfo(results);
        return EX_OSERR;
    }
    printf("A raw socket was created.\n");

    if (bind(raw_socket, results->ai_addr, results->ai_addrlen) == -1)
    {
        perror("Could not bind socket");
        close(raw_socket);
        freeaddrinfo(results);
        return EX_OSERR;
    }
    printf("Bound to port %s\n", argv[1]);
    freeaddrinfo(results);

    char buffer[PCKT_LEN];
    memset(buffer, 0, PCKT_LEN);

    ssize_t                 received;
    struct sockaddr_storage client;
    socklen_t               client_sz = sizeof(client);
    char                    addr[INET6_ADDRSTRLEN];
    unsigned short          port = 0;
    for (;;)
    {
        if ((received = recvfrom(raw_socket,
                                 buffer,
                                 sizeof(buffer) - 1,
                                 0,
                                 (struct sockaddr *)&client,
                                 &client_sz)) < 0)
        {
            perror("Unable to receive");
            close(raw_socket);
            return EX_UNAVAILABLE;
        }

        buffer[received] = '\0';

        struct iphdr *  ip  = (struct iphdr *)buffer;
        struct udphdr * udp = (struct udphdr *)(buffer + sizeof(struct iphdr));

        // find the udp port
        if (udp->dest == htons(atoi(argv[1])))
        {
            if (ip->version == 6)
            {
                inet_ntop(AF_INET6, &ip->saddr, addr, sizeof(addr));
            }
            else
            {
                inet_ntop(AF_INET, &ip->saddr, addr, sizeof(addr));
            }

            port = ntohs(udp->source);

            printf("Received from %s:%hu\n\n", addr, port);
            break;
        }
    }

    close(raw_socket);
    return 0;
}