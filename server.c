#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <ifaddrs.h>

#define TRUE                                1
#define FALSE                               0
#define PORT                                0
#define MAX                                 30

void p_listening_on(int lport) {
    struct ifaddrs *ifap, *tptr;
    char host[NI_MAXHOST];
    int family, s;
    if (getifaddrs(&ifap) != 0) {
        perror("getting available interfaces failed");
        exit(EXIT_FAILURE);
    }
    tptr = ifap;
    for (tptr = ifap; tptr != NULL; tptr = tptr->ifa_next) {
        if (tptr->ifa_addr == NULL)
            continue;
        family = tptr->ifa_addr->sa_family;
        if (family == AF_INET) {
            if ((s = getnameinfo(
                    tptr->ifa_addr, sizeof(struct sockaddr_in),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST)) != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("listening on %s %d\n", host, lport);
        }
    }
    freeifaddrs(ifap);
}

int main(int argc , char *argv[]) {
    int opt = TRUE;
    int ss, addrlen, ns, cs[MAX], activity, i, val, sd, new_socket, j;
    int max_fds;
    struct sockaddr_in address;


    char buffer[MAX][1025];
    char client_id_buf[2055];
    fd_set readfds;
    char *message ="server: welcome clients.\n";
    memset(cs, 0, MAX*sizeof(cs[0]));
    if((ss = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if(setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(ss, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(ss, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    int tmp_len = sizeof(address);
    if (getsockname(ss, (struct sockaddr*)&address, &tmp_len) == -1) {
        perror("getsockname");
    }
    p_listening_on(ntohs(address.sin_port));


    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(TRUE) {
        FD_ZERO(&readfds);
        FD_SET(ss, &readfds);
        max_fds = ss;
        for (i = 0; i < MAX; i++)
        {
            sd = cs[i];
            if(sd > 0)
                FD_SET(sd, &readfds);
            if(sd > max_fds)
                max_fds = sd;
        }

        activity = select(max_fds + 1, &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        if (FD_ISSET(ss, &readfds)) {
            if ((ns = accept(ss, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));


            if(send(ns, message, strlen(message), 0) != strlen(message)) {
                perror("send");
            }

            puts("Welcome message sent successfully");

            for (i = 0;i < MAX; i++)
            {
                if(cs[i] == 0)
                {
                    cs[i] = ns;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        for (i =0; i < MAX; i++)
        {
            sd = cs[i];
            if (sd > 0) {
                if (FD_ISSET(sd, &readfds)) {
                    if ((val = recv(sd, buffer[i], 1024, 0)) == 0) {
                        getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                        printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                        close(sd);
                        cs[i] = 0;
                    }
                    else {
                        for (j = 0; j < MAX; ++j) {
                            if (cs[j] > 0 && j != i) {
                                sprintf(client_id_buf, "client %d: %s", cs[i], buffer[i]);
                                send(cs[j], client_id_buf, strlen(client_id_buf), 0);
                            }
                        }
                    }
                }
            }
            memset(buffer[i], 0, sizeof(buffer[i]));
        }
    }
    return 0;
}

