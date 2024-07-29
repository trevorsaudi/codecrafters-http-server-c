#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

struct ResponseBody{
char statusline[50];
char headers[500];
};



int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");
	
	int server_fd, client_addr_len, connected_fd, request_fd;
	struct sockaddr_in client_addr;
	\
	const char OK_msg[] = "HTTP/1.1 200 OK\r\n\r\n";
	const char NOTFOUND_msg[] = "HTTP/1.1 404 Not Found\r\n\r\n" ;
	char recv_buf[];
	size_t recv_buf_len;
	recv_buf_len = sizeof(recv_buf);

	size_t OK_msg_length = sizeof(OK_msg);
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);


	if((connected_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len)) != -1){
		printf("Client connected\n");
		send(connected_fd, OK_msg, OK_msg_length, MSG_CONFIRM);
	} 
	
	if (request_fd = recv(connected_fd, recv_buf, recv_buf_len , MSG_WAITALL) == -1){
		printf("Error encountered when receiving data: ", strerror(errno));
		return -1;
	}
	print(recv_buf);

	close(server_fd);

	return 0;
}


// we will write a parser that checks the http request, breaks it when it finds a carriage return, then populates a struct with the details 
// we can then extract the URL from the request and parse the target


GET /index.html HTTP/1.1\r\n
Host: localhost:4221\r\n
User-Agent: curl/7.64.1\r\n
Accept: */*\r\n\r\n
