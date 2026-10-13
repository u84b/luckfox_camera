#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#define PORT 8080

int main(void)
{
    int sock_fd;

    struct sockaddr_in server_address;

    char buffer[128];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    inet_pton(
        AF_INET,
        "xxx.xxx.xxx.xxx", // wlan0 ipv4
        &server_address.sin_addr
    );

    connect(
        sock_fd,
        (struct sockaddr *)&server_address,
        sizeof(server_address)
    );

    ssize_t received = recv(
        sock_fd,
        buffer,
        sizeof(buffer) - 1,
        0
    );
    buffer[received] = '\0';

    printf("Received: %s\n", buffer);

    std::fill(std::begin(buffer), std::end(buffer), '\0');

    

    snprintf(buffer, sizeof(buffer) - 1, "Hello from Ho Chi Minh");
    
    printf("What = %s\n", buffer);
    
    ssize_t sent = send(
        sock_fd, 
        buffer, 
        sizeof(buffer) - 1, 
        0);
    
    
    

    close(sock_fd);

    return 0;
}