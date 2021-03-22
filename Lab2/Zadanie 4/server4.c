#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_ntop() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <time.h>
#include <errno.h>


int main(int argc, char** argv) {

    /* Bufor wykorzystywany przez write() i read(): */
    char            buff[256];

    /* Bufor dla adresu IP klienta w postaci kropkowo-dziesietnej: */
    char            addr_buff[256];

    // Wiadomość do przesłania klientowi, który się podłączy:
    char *message = "Laboratorium PUS";

    // Gniazdo struktury adresowej
    struct sockaddr_in server_addr;

    // Rozmiar struktury w bajtach
    socklen_t server_addr_len;

    /* Deskryptory dla gniazda nasluchujacego i polaczonego: */
    int             listenfd, connfd;


    if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Utworzenie gniazda dla protokolu TCP: */
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej serwera: */
    memset(&server_addr, 0, sizeof(server_addr));

    /* Rozmiar struktury adresowej serwera w bajtach: */
    server_addr_len                 =       sizeof(server_addr);

    /* Domena komunikacyjna (rodzina protokolow): */
    server_addr.sin_family          =       AF_INET;

    /* Numer portu: */
    server_addr.sin_port            =       htons(atoi(argv[1]));

    // Odbieranie danych z dowolnego interfejsu sieciowego:
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Powiazanie "nazwy" (adresu IP i numeru portu) z gniazdem: */
    if (bind(listenfd, (struct sockaddr*) &server_addr, server_addr_len) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    /* Przeksztalcenie gniazda w gniazdo nasluchujace: */
    if (listen(listenfd, 2) == -1) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Server is listening for incoming connection...\n");

    while (1)
    {
        connfd = accept(listenfd, (struct sockaddr*)&server_addr, &server_addr_len);
        fprintf(stdout, "Connected. Address: %s:%d.\n",
                inet_ntop(AF_INET, &server_addr.sin_addr, addr_buff, sizeof(addr_buff)),
                ntohs(server_addr.sin_port));
        sleep(1);
        write(connfd, message, strlen(message));
        close(connfd);
    }
    close(listenfd);


    return 0;
}