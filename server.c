#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile int run = 1;
static struct sockaddr_in serv_addr, client_addr;

void sigINT_handler(int n);

int main(int argc, char* argv[])
{
	int serv_sock, s, n, pid, client_addr_len = sizeof(client_addr);

	/* Set up the address struct we will bind to */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	/* Set the port to bind to */
	serv_addr.sin_port = htons(6010);

	/* Create the socket */
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	/* Set socket options */
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)1, sizeof(int));

	/* Attempt to bind to all interfaces on provided port */
	if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("FATAL: Failed to bind");
		exit(1);
	}

	/* Attempt listen on the socket */
	if (listen(serv_sock, 10) < 0) {
		perror("FATAL: Failed to listen on socket");
		exit(1);
	}

	/* Attempt to cleanly close sockets on ctrl+c */
	struct sigaction a;
	a.sa_handler = sigINT_handler;
	a.sa_flags = 0;
	sigemptyset(&a.sa_mask);
	sigaction(SIGINT, &a, NULL);
	/* */

	char buff[1024];
	while(run) {
		/* Accept incoming connections */
		s = accept(serv_sock, (struct sockaddr*)&client_addr, &client_addr_len);
		/* fork and let child handle connection */
		if (run && (pid = fork()) == 0) {
			/* Child */
			printf("[+] Received new client connection from %s.\n", inet_ntoa(client_addr.sin_addr));
			while(1) {
				/* Do stuff here! */
				/* */
				n = read(s, buff, sizeof(buff));
				if (n <= 0) {
					printf("[-] Client connection closed.\n");
				}
			}
		}
		close(s);
	}
	close(serv_sock);
}

/* Handler for ctrl+c interrupt */
void sigINT_handler(int n) {
	run = 0;
}
