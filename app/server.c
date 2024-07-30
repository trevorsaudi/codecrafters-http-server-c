#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define PORT 4221

const char OK_msg[] = "HTTP/1.1 200 OK\r\n\r\n";
const char NOTFOUND_msg[] = "HTTP/1.1 404 Not Found\r\n\r\n" ;
int server_fd, connected_fd, request_fd;
size_t OK_msg_length = sizeof(OK_msg);

void targetTokenizer(char str[], int connected_fd){
    char *pch;
    char *target;
    pch = strtok(str, " ");
    target = pch = strtok(NULL, " ");
	if(target == "/"){
		send(connected_fd, OK_msg, OK_msg_length, MSG_CONFIRM);
	}else{
		send(connected_fd, NOTFOUND_msg, OK_msg_length, MSG_CONFIRM);
	}
}

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");
	
	
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;
	client_addr_len = sizeof(client_addr);
	
	char recv_buf[BUFFER_SIZE];
	size_t recv_buf_len;
	recv_buf_len = sizeof(recv_buf);

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
									 .sin_port = htons(PORT),
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


	if((connected_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len)) != -1){
		printf("Client connected\n");
		send(connected_fd, OK_msg, OK_msg_length, MSG_CONFIRM);
	} 
	
	while(1){
	memset(recv_buf, 0,BUFFER_SIZE);
	if (request_fd = recv(connected_fd, recv_buf, recv_buf_len , MSG_WAITALL) == -1){
		printf("Error encountered when receiving data: ", strerror(errno));
		return -1;
	}else if (request_fd == 0){
		printf("Client has closed the connection");
		break;
	}else{
	targetTokenizer(recv_buf, request_fd);
	
	}
	}
	
	
	close(server_fd);

	return 0;
}


// we will write a parser that checks the http request, breaks it when it finds a carriage return, then populates a struct with the details 
// we can then extract the URL from the request and parse the target


