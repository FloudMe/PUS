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
	
    /* Deskryptory dla gniazda nasluchujacego i polaczonego: */
    int             listenfd, connfd;

    /* Wartosc zwracana przez funkcje. */
    int             retval; 
	
    /* Gniazdowe struktury adresowe (dla klienta i serwera): */
    struct          sockaddr_in client_addr, server_addr;

    /* Rozmiar struktur w bajtach: */
    socklen_t       client_addr_len, server_addr_len;

    /* Bufor wykorzystywany przez write() i read(): */
    char            buff[256];
	
	/* Bufor dla adresu IP klienta w postaci kropkowo-dziesietnej: */
    char            addr_buff[256];
	
	/* Lista deskryptorow , Pomocnicza lista deskryptorow*/
    fd_set main_fd, helper_fd;
	
	/* Maksymalny numer deskryptora pliku */
    int fd_max;
	
	if (argc != 2) {
        fprintf(stderr, "Invocation: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
	
	// Wyczyszczenie list
    FD_ZERO(&main_fd);
    FD_ZERO(&helper_fd);
	
	/* Utworzenie gniazda dla protokolu TCP: */
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
	
	/* Wyzerowanie struktury adresowej serwera: */
    memset(&server_addr, 0, sizeof(server_addr));

    /* Domena komunikacyjna (rodzina protokolow): */
    server_addr.sin_family          =       AF_INET;

    /* Adres nieokreslony (ang. wildcard address): */
    server_addr.sin_addr.s_addr     =       htonl(INADDR_ANY);

    /* Numer portu: */
    server_addr.sin_port            =       htons(atoi(argv[1]));

    /* Rozmiar struktury adresowej serwera w bajtach: */
    server_addr_len                 =       sizeof(server_addr);
   
	/* Powiazanie "nazwy" (adresu IP i numeru portu) z gniazdem: */
    if (bind(listenfd, (struct sockaddr*) &server_addr, server_addr_len) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }
   
    /* Przeksztalcenie gniazda w gniazdo nasluchujace: */
    if (listen(listenfd, 10) == -1) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }
   
    // FD_SET sluzy do dodania gniazda nasluchujacego
    FD_SET(listenfd, &main_fd);
   
    // Do zmiennej fd_max wpisujemy najwiekszy deskryptor
    fd_max = listenfd; 
	
	fprintf(stdout, "Server is listening for incoming connection...\n");
   
    while (1) {
		
		// Przypisanie do pomocniczej listy, liste glowna 
        helper_fd = main_fd;
		
        // Synchronizacja wielokrotnych operacji wejscia/wyjscia przy uzyciu selecta
        if (select(fd_max + 1, &helper_fd, NULL, NULL, NULL) == - 1) {
            perror("select()");
            exit(EXIT_FAILURE);
        }
       
        // Przechodzenie przez wysztkie obecne polaczenia, aby wyszukac dane do odzczytania
        for(int i = 0; i <= fd_max; i++) {

            // Sprawdzenie czy polaczenie jest na liscie
            if(FD_ISSET(i, &helper_fd)) {
                if(i == listenfd) {
					
                    /* Funkcja pobiera polaczenie z kolejki polaczen oczekujacych na zaakceptowanie
					* i zwraca deskryptor dla gniazda polaczonego: */
					client_addr_len = sizeof(client_addr);
 
                    if((connfd = accept(listenfd, (struct sockaddr*) &client_addr, &client_addr_len)) == -1) {
                        perror("accept()");
                    }
					else {
						/* Dodajemy do main listy oraz sprawdzenie czy 
                         * nowy deskryptor nie jest wiekszy od wczesniejszego
                         * najwiekszego deskryptora */
                        FD_SET(connfd, &main_fd);
                        if(connfd > fd_max) {
                            fd_max = connfd;
                        }
						fprintf(
								stdout, "TCP connection accepted from %s:%d on socket %d\n",
								inet_ntop(AF_INET, &client_addr.sin_addr, addr_buff, sizeof(addr_buff)),
								ntohs(client_addr.sin_port),
								connfd
							   );
                    }
                }
				else {
                    // Sprawdzanie czy klient sie rozlaczyl
                    if((retval = recv(i, buff, sizeof( buff ), 0 )) <= 0) {
                        if(retval == 0) {
                            printf("Client on socket %d disconnected\n", i);
                        }
						else {
                            perror("recv()");
                        }
                        close(i);
						
						// Usuwanie z listy polaczenia
                        FD_CLR(i, &main_fd);
                    }
					else {
                        // Petla do wysylania komunikatow oprocz wysylajacego
                        for(int j = 0; j <= fd_max; j++) {
                            if(FD_ISSET(j, &main_fd)) {
								
                                if(j != listenfd && j != i) {
                                    if(send(j, buff, retval, 0) == - 1) {
                                        perror("send()");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
   
    exit(EXIT_SUCCESS);
}