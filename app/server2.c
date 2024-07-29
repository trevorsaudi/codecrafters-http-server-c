#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(){
    
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    int server_fd, client_addr_len;
    struct sockaddr_in client_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1){
        printf("The socket creation failed: %s....\n", strerror(errno));
        return -1;
    }


    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        
    }



    return 0;
}