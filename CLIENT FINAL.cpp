#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
/*
 * This program creates a socket and initiates a connection with the socket
 * given in the command line.  One message is sent over the connection and
 * then the socket is closed, ending the connection. The form of the command
 * line is streamwrite hostname portnumber
 */

void* client2sock(void *sock)
{
   	char buf [1024];
  	int s = *((int *)sock);
   	int readcount;

	while (true)
	{
        bzero(buf, sizeof(buf));
        readcount = read(STDIN_FILENO, buf, 1024);
        write(s, buf , readcount);
    }
    pthread_exit(NULL);
}

void* sock2client(void *sock)
{
   	char buf [1024];
   	int readcount;
  	int s = *((int *)sock);
	while (true)
	{
        bzero(buf, sizeof(buf));
        readcount = read(s, buf, 1024);
        write(STDOUT_FILENO, buf , readcount);
        buf[readcount - 1] = '\0';
        if (buf == "Disconnected")
        {
            exit(0);
        }
    }
    pthread_exit(NULL);
}



int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	struct hostent *hp;
    pthread_t thread1, thread2;

	/* Create socket */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}
	/* Connect socket using name specified by command line. */
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);
	if (hp == 0)
	{
		fprintf(stderr, "%s: unknown host\n", argv[1]);

		exit(2);
	}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(atoi(argv[2]));

	if (connect(sock,(struct sockaddr *) &server,sizeof(server)) < 0)
	{
		perror("connecting stream socket");
		exit(1);
	}


//    char test [100];
//    int l = sprintf(test, "sock: %d\n", sock);
//    write(STDOUT_FILENO, test, l);
//    write(sock, "Client here", 11);

  	pthread_create(&thread1, NULL, &client2sock, (void*) &sock);
	pthread_create(&thread2, NULL, &sock2client, (void*) &sock);


	pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);


	close(sock);
}
