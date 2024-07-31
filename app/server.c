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
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define PORT 4221

const char OK_msg[] = "HTTP/1.1 200 OK\r\n\r\n";
const char NOTFOUND_msg[] = "HTTP/1.1 404 Not Found\r\n\r\n";
char echo_resp_template[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s";
char file_resp_template[] = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %ld\r\n\r\n";
char response[BUFFER_SIZE];
ssize_t server_fd, connected_fd, request_fd;
size_t OK_msg_length = sizeof(OK_msg) - 1;
size_t NOTFOUND_msg_length = sizeof(NOTFOUND_msg) - 1;

typedef struct {
    int connected_fd;
    char *directory_path;
} thread_data_t;

// Function to handle client requests
void *handle_client(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int connected_fd = data->connected_fd;
    char *directory_path = data->directory_path;
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
        targetTokenizer(recv_buf, connected_fd, directory_path);
    }
}

// Function to handle URL parsing and response
void targetTokenizer(char str[], int connected_fd, char *directory_path) {
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

    // Handle file requests
    if (strncmp(target, "/files/", 7) == 0) {
        char file_path[BUFFER_SIZE];
        snprintf(file_path, sizeof(file_path), "%s%s", directory_path, target + 7);  // skip "/files/"

        struct stat st;
        if (stat(file_path, &st) == -1) {
            // File does not exist
            send(connected_fd, NOTFOUND_msg, NOTFOUND_msg_length, MSG_CONFIRM);
        } else {
            // File exists, read the file and send the response
            FILE *file = fopen(file_path, "rb");
            if (!file) {
                send(connected_fd, NOTFOUND_msg, NOTFOUND_msg_length, MSG_CONFIRM);
                return;
            }

            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            char *file_content = malloc(file_size);
            fread(file_content, 1, file_size, file);
            fclose(file);

            snprintf(response, BUFFER_SIZE, file_resp_template, file_size);
            send(connected_fd, response, strlen(response), MSG_CONFIRM);
            send(connected_fd, file_content, file_size, MSG_CONFIRM);

            free(file_content);
        }
    } else if (target != NULL && strcmp(target, "/") == 0) {
        send(connected_fd, OK_msg, OK_msg_length, MSG_CONFIRM);
    } else if (strncmp(target, "/echo/", 6) == 0) {
        echo = target + 6;
        echo_len = strlen(echo);
        snprintf(response, BUFFER_SIZE, echo_resp_template, echo_len, echo);
        send(connected_fd, response, strlen(response), MSG_CONFIRM);
    } else if (strncmp(target, "/user-agent", 11) == 0) {
        for (cpy_pch = strtok(usragent_cpy, "\r\n"); cpy_pch; cpy_pch = strtok(NULL, "\r\n")) {
            if (strncmp(cpy_pch, "User-Agent: ", 12) == 0) {
                char *agent = cpy_pch + strlen("User-Agent: ");
                printf("The extracted user-agent is: %s", agent);
                snprintf(response, BUFFER_SIZE, echo_resp_template, strlen(agent), agent);
                printf("The response being sent is: %s", response);
                send(connected_fd, response, strlen(response), MSG_CONFIRM);
            }
        }
    } else {
        send(connected_fd, NOTFOUND_msg, NOTFOUND_msg_length, MSG_CONFIRM);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "--directory") != 0) {
        fprintf(stderr, "Usage: %s --directory <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *directory_path = argv[2];

    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    printf("Logs from your program will appear here!\n");

    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    client_addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("Socket creation failed: %s...\n", strerror(errno));
        return 1;
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        printf("SO_REUSEADDR failed: %s \n", strerror(errno));
        return 1;
    }

    struct sockaddr_in serv_addr = { .sin_family = AF_INET,
                                     .sin_port = htons(PORT),
                                     .sin_addr = { htonl(INADDR_ANY) },
                                   };

    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
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
    while (1) {
        connected_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (connected_fd == -1) {
            printf("Connection Failed: %s\n", strerror(errno));
            close(server_fd);
            return -1;
        }
        printf("Client Connected: %d\n", ++clients);

        // Allocate memory for thread data
        thread_data_t *data = malloc(sizeof(thread_data_t));
        data->connected_fd = connected_fd;
        data->directory_path = directory_path;

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
