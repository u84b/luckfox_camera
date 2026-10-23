#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../app/app.h"

#define PORT 8080
#define ADDR "192.168.0.142"
#define LOCAL_ADDR "127.0.0.1"

// TCP/IP client test

int net_client_init(app_client * client, char * hostname, int index){
    int result = -1;

    puts("net_client_init");

    memset(&client->net_addr[index], 0, sizeof(client->net_addr[index]));

    puts("memset");
    
    client->server_fd[index] = socket(AF_INET, SOCK_STREAM, 0);
    
    client->net_connected[index] = 0;
    //client->net_client_fd[index] = -1;

    client->net_addr[index].sin_family = AF_INET;
    client->net_addr[index].sin_port = htons(PORT);

    puts("Ok and");

    inet_pton(
        AF_INET,
        hostname,
        &client->net_addr[index].sin_addr
    );



    result = 0;
    return result;
}

int net_client_check_connection(app_client * const client, int index){
    int result = -1;

    puts("net_client_check_connection");


    if (connect(
     client->server_fd[index], 
     (struct sockaddr*)&client->net_addr[index],
     sizeof(client->net_addr[index])) < 0)
    {
        printf("Failed to connect: %m\n");
        close(client->server_fd[index]);
    } else client->connected[index] = 1;
    
    result = 0;
    return result;
}

static int send_digit(int fd, const uint32_t * data, size_t length)
{
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = send(fd, data + sent, length - sent, 0);

        if (n < 0) {
            if (errno == EINTR)
                continue;

            perror("send");
            return -1;
        }

        if (n == 0) {
            fprintf(stderr, "Connection closed during send\n");
            return -1;
        }

        sent += (size_t)n;
    }

    return 0;
}

static int send_all(int fd, const char *data, size_t length)
{
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = send(fd, data + sent, length - sent, 0);

        if (n < 0) {
            if (errno == EINTR)
                continue;

            perror("send");
            return -1;
        }

        if (n == 0) {
            fprintf(stderr, "Connection closed during send\n");
            return -1;
        }

        sent += (size_t)n;
    }

    return 0;
}



int net_client_write_(app_client * const client, char * bytes, int index) {
    int result = -1;

    //char * buf = (char *) bytes;

    size_t length = strlen(bytes);


    if (send_all(client->server_fd[index], bytes, length) < 0)
    {
        printf("failed sending bytes: %m\n");
        return -1;
    }
    else {
        printf("message sent successfully\n");
    }
    
    if (send_all(client->server_fd[index], "\n", 1) < 0)
    {
        printf("failed sending delimiter: %m\n");
        return -1;
    }

    printf("%s\n", bytes);
    
    result = 0;
    return result;
}

uint32_t get_file_size(FILE* opened){
    fseek(opened, 0L, SEEK_END);
    uint32_t file_size = (uint32_t) ftell(opened);
    fseek(opened, 0L, SEEK_SET);
    return file_size;
}

int net_client_send_length(app_client * const client, uint32_t length, int index){
    const uint32_t net_len = htonl(length);

    if (send_digit(client->server_fd[index], &net_len, sizeof(net_len)) < 0)
    {
        printf("failed sending filesize\n");
        return -1;
    }
    return 0;
}

// prefixed length
int net_client_send_file(app_client * const client, char *filename, int index) {
    int result = -1;
    
    FILE* opened = fopen(filename, "rb");
    
    if (opened == NULL)
    {
        printf("file not found: %m\n");
        goto end;
    }

    uint32_t file_size = get_file_size(opened);

    net_client_send_length(client, file_size, 0);

    printf("File size: %lu\n", file_size);

    char *buffer = (char*) calloc(file_size, sizeof(char));

    if (buffer == NULL)
    {
        printf("failed memory allocation\n");
        goto end;
    }
    

    int r = fread(buffer, sizeof(char), file_size - 1, opened);

    int written = net_client_write_(client, buffer, 0);
    
    if (written < 0)
    {
        printf("failed writing to server\n");
    }
    
    free(buffer);
    fclose(opened);
    result = 0;
end:
    return result;
    
};

int net_client_close(app_client * const client, int index) {

    if (client != NULL)
    {
        if (client->server_fd[index] != -1)
        {
            close(client->server_fd[index]);
            client->server_fd[index] = -1;
        }
        
    }

    return 0;
}

int is_valid_ipv4(const char * hostname){
    struct in_addr dest;
    return inet_pton(AF_INET, hostname, &dest);
}

int main(int argc, char *argv[]){
    int result = -1;
    app_client client;
    char * hostname = ADDR;

    puts("nice");

    if (argc > 1)
    {
        printf("argc checked");

        if (argv[1] != NULL)
        {
            if (is_valid_ipv4(hostname))
            {
                strncpy(hostname, argv[1], sizeof(argv[1]));
            }
            else {
                strncpy(hostname, LOCAL_ADDR, sizeof(LOCAL_ADDR));
            }
        }
    }
    // else {
    //     strncpy(hostname, LOCAL_ADDR, sizeof(LOCAL_ADDR));
    // }



    

    char request[4][64] = {"message for you my dear server",
                            "another request to my dear server",
                            "request to my dear server 3: ressurection",
                            "slutshole lane: asscock redemption"};

    if (net_client_init(&client, hostname, 0) < 0)
    {
        printf("net client init failed: %m\n");
        goto end;   
    }

    

    if (net_client_check_connection(&client, 0) < 0)
    {
        printf("net client connection failed: %m\n");
        goto end;
    }
    
    

    // int requests_count = sizeof(request) / sizeof(request[0]);

    // for (size_t i = 0; i < requests_count; i++)
    // {
    //     if (net_client_write_(&client, request[i], 0) < 0)
    //     {
    //         printf("failed sending message #%d: %m\n", i);
    //     }
    //     printf("%s\n", request[i]);
    // }
    
    
    int sent = net_client_send_file(&client, "text.txt", 0);

    if (sent < 0)
    {
        printf("problem occured during sending file: %m\n");
    }
    

    printf("net_client_send_file success\n");
    
    result = 0;
end:
    net_client_close(&client, 0);
    return 0;
}

// int main(void)
// {
//     int sock_fd;

//     struct sockaddr_in server_address;

//     char buffer[128];

//     sock_fd = socket(AF_INET, SOCK_STREAM, 0);

//     server_address.sin_family = AF_INET;
//     server_address.sin_port = htons(PORT);

//     inet_pton(
//         AF_INET,
//         "192.168.0.142", // wlan0 ipv4
//         &server_address.sin_addr
//     );

//     connect(
//         sock_fd,
//         (struct sockaddr *)&server_address,
//         sizeof(server_address)
//     );

//     ssize_t received = recv(
//         sock_fd,
//         buffer,
//         sizeof(buffer) - 1,
//         0
//     );
//     buffer[received] = '\0';

//     printf("Received: %s\n", buffer);

//     memset(buffer, '\0', sizeof(buffer));
//     //std::fill(std::begin(buffer), std::end(buffer), '\0');

    

//     snprintf(buffer, sizeof(buffer) - 1, "Hello from Ho Chi Minh");
    
//     printf("What = %s\n", buffer);
    
//     ssize_t sent = send(
//         sock_fd, 
//         buffer, 
//         sizeof(buffer) - 1, 
//         0);
    
    
    

//     close(sock_fd);

//     return 0;
// }