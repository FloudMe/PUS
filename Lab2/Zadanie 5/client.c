#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>

int main(int argc, char** argv) {

    int sockfd;
    int retval;
    char buff[256];

    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *rp;
    struct sockaddr_storage addr_storage;

    socklen_t addr_storage_len=0;

    char hostbuff[10];
    char serverbuff[10];

    if (argc != 3) {
        fprintf(stderr, "[CLIENT] Invocation: %s <IPv4/IPv6 ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if ((retval = getaddrinfo(argv[1], argv[2], &hints, &result)) != 0){
        perror("[CLIENT] Translation error");
        exit(EXIT_FAILURE);
    }

    for(rp=result; rp!=NULL; rp=rp->ai_next){
        /* Utworzenie gniazda dla protokolu TCP: */
        sockfd = socket(rp->ai_family, rp->ai_socktype, 0);

        addr_storage.ss_family=rp->ai_family;

        if(sockfd == -1)
            continue;

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1){
            fprintf(stdout, "[CLIENT] New client connected.\n");
            break;
        }
    }
    if(sockfd==-1){
        fprintf(stdout, "[CLIENT] socket()");
        exit(EXIT_FAILURE);
    }

    addr_storage_len = sizeof(addr_storage);

    getsockname(sockfd, (struct sockaddr*)&addr_storage, &addr_storage_len);
    fprintf(stdout, "[CLIENT] ss_family: %d\n", addr_storage.ss_family);

    getnameinfo((struct sockaddr*)&addr_storage, addr_storage_len, hostbuff,
                sizeof(hostbuff), serverbuff, sizeof(serverbuff), 10);


    fprintf(stdout, "[CLIENT] Host=%s, Server=%s\n", hostbuff, serverbuff);

    memset(buff, 0, 256);
    retval = read(sockfd, buff, sizeof(buff));

    sleep(1);

    fprintf(stdout, "[CLIENT] Received server response: %s\n", buff);
    fprintf(stdout, "[CLIENT] Closing socket (sending FIN to server)...\n");

    close(sockfd);

    fprintf(stdout, "[CLIENT] Terminating application. After receiving FIN from server, "
                    "TCP connection will go into TIME_WAIT state.\n");

    exit(EXIT_SUCCESS);
}