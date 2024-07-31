#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define PORT 4221

const char OK_msg[] = "HTTP/1.1 200 OK\r\n\r\n";
const char NOTFOUND_msg[] = "HTTP/1.1 404 Not Found\r\n\r\n" ;
char echo_resp_template [] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s";
char response [BUFFER_SIZE];
ssize_t server_fd, connected_fd, request_fd;
size_t OK_msg_length = sizeof(OK_msg);
size_t NOTFOUND_msg_length = sizeof(OK_msg);

typedef struct {
	int connected_fd;
}thread_data_t;

//create a thread function that handles client requests


void *handle_client(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int connected_fd = data->connected_fd;
    char recv_buf[BUFFER_SIZE];
    size_t recv_buf_len = sizeof(recv_buf);
    ssize_t request_fd;

    free(data);  // Free the allocated memory

    while (1) {
        memset(recv_buf, 0, BUFFER_SIZE);
        request_fd = recv(connected_fd, recv_buf, recv_buf_len, 0);
        if (request_fd == -1) {
            printf("Error encountered when receiving data: %s\n", strerror(errno));
            close(connected_fd);
            pthread_exit(NULL);
        } else if (request_fd == 0) {
            printf("Client has closed the connection\n");
            close(connected_fd);
            pthread_exit(NULL);
        }
        targetTokenizer(recv_buf, connected_fd);
    }
}
void targetTokenizer(char str[], int connected_fd){

	char usragent_cpy[BUFFER_SIZE];
	char *user_agent;
	char *cpy_pch;
    char *pch;
    char *target;
	char *echo;
	int echo_len;
	strcpy(usragent_cpy, str);
    pch = strtok(str, " ");
    target = pch = strtok(NULL, " ");
	// echo = strtok(target, "/");
    // echo = strtok(NULL, "");
	// echo_len = sizeof(echo);
	// char *checker = NULL;
    // checker = strstr(target, "/echo");
	// printf("The value of target and echo is: %s %s\n", target, echo);
	if (target != NULL && strcmp(target, "/") == 0) {
        send(connected_fd, OK_msg, sizeof(OK_msg) - 1, MSG_CONFIRM);
    }else if(strncmp(target, "/echo/", 6) == 0){
		echo = target + 6;
		echo_len = strlen(echo);
		snprintf(response, BUFFER_SIZE, echo_resp_template, echo_len, echo);
		//printf("The value of echo is: %s \n", response);
		send(connected_fd, response,strlen(response), MSG_CONFIRM);
	}else if(strncmp(target, "/user-agent", 11) == 0){

		for(cpy_pch = strtok(usragent_cpy, "\r\n"); cpy_pch; cpy_pch = strtok(NULL, "\r\n")){
			if(strncmp(cpy_pch, "User-Agent: ", 11) == 0){
			//printf("Found user agent: %s!", cpy_pch);

			char* agent = cpy_pch + strlen("User-Agent: ");
			printf("The extracted user-agent is: %s", agent);
			snprintf(response, BUFFER_SIZE, echo_resp_template, strlen(agent),agent);
			printf("The response being sent is: %s",response);
			send(connected_fd, response ,strlen(response), MSG_CONFIRM);
			}
		}


	} else {
        send(connected_fd, NOTFOUND_msg, sizeof(NOTFOUND_msg) - 1, MSG_CONFIRM);
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

	int clients = 0;
	int childpid;
	while(1){


	connected_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	if(connected_fd == -1){
		printf("Connection Failed: %s\n",strerror(errno));
		close(server_fd);
		return -1;
	}
	printf("Client Connected: %d\n",++clients);
	       // Allocate memory for thread data
        thread_data_t *data = malloc(sizeof(thread_data_t));
        data->connected_fd = connected_fd;
 		pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, data) != 0) {
            printf("Failed to create thread: %s\n", strerror(errno));
            close(connected_fd);
            free(data);  // Free the allocated memory if thread creation fails
        } else {
            pthread_detach(thread_id);  // Detach the thread to free resources when done
        }
	}



	close(server_fd);

	return 0;
}


// we will write a parser that checks the http request, breaks it when it finds a carriage return, then populates a struct with the details
// we can then extract the URL from the request and parse the target


