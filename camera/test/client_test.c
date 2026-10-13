#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../app/app.h"

#define PORT 8080

// TCP/IP client test

int net_client_init(app_client * const client, char * hostname, int index){
    int result = -1;


    memset(&client->net_addr[index], 0, sizeof(client));
    
    client->server_fd[index] = socket(AF_INET, SOCK_STREAM, 0);
    
    client->net_connected[index] = 0;
    //client->net_client_fd[index] = -1;

    client->net_addr[index].sin_family = AF_INET;
    client->net_addr[index].sin_port = htons(PORT);

    inet_pton(
        AF_INET,
        "192.168.0.142",
        &client->net_addr[index].sin_addr);
        
    return result;
}

int net_client_check_connection(app_client * const client, int index){
    int result = -1;

    if (connect(
     client->server_fd[index], 
     (struct sockaddr*)&client->net_addr[0],
     sizeof(client->net_addr)) < 0)
    {
        printf("Failed to connect: %m\n");
        close(client->server_fd[index]);
    } else client->connected[index] = 1;
    
    result = 0;
    return result;
}

int net_client_write(app_client * const client, char * bytes, int index) {
    int result = -1;


    if (send(client->server_fd[index], bytes, strlen(bytes), 0) < 0)
    {
        printf("problem occured during sending bytes: %m\n");
    }
    else {
        printf("message sent successfully\n");
        result = 0;
    }
    

    
    return result;
}

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
        "192.168.0.142", // wlan0 ipv4
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

    memset(buffer, '\0', sizeof(buffer));
    //std::fill(std::begin(buffer), std::end(buffer), '\0');

    

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