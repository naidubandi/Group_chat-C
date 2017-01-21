#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>

#define TRUE                1
#define FALSE               0

int main(int argc , char *argv[])
{
    int sock, max_fds, activity;
    struct sockaddr_in server;
    char msg[1000], sreply[2000];
    fd_set readfds, writefds;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock==-1) {
        printf("error: Could not create socket");
    }
    puts("info: socket created");

    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&server, sizeof(server))<0) {
        perror("error: connect failed. Error");
        return 1;
    }
    puts("info: Connected\n");

    //keep communicating with server
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(0, &readfds);

        max_fds=sock;
        printf("\rme: ");
        fflush(stdout);

        activity = select(max_fds+1, &readfds , NULL , NULL , NULL);

        if ((activity<0) && (errno!=EINTR)) {
            printf("error: select error");
        }
        if (FD_ISSET(sock, &readfds)) {
            if(recv(sock, sreply, sizeof(sreply),0)>0) {
                printf("\r%s",sreply);
                fflush(stdout);
                memset(sreply,0,sizeof(sreply));
            }
        }
        if (FD_ISSET(0, &readfds)) {
            memset(msg,0,sizeof(msg));
            fgets(msg, sizeof(msg) - 1, stdin);
            fflush(stdout);
        }
        if(send(sock,msg,strlen(msg),0) < 0) {
            puts("error: Send failed");
            return 1;
        }
        memset(msg,0,sizeof(msg));
    }
    close(sock);
    return 0;
}
