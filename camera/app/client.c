#include "client.h"

uint32_t get_file_size(FILE* opened){
    fseek(opened, 0L, SEEK_END);
    uint32_t file_size = (uint32_t) ftell(opened);
    fseek(opened, 0L, SEEK_SET);
    return file_size;
};

int client_init(app_client * client, char * sock_path, int index){
    int result = -1;

    memset(&client->addr[index], 0, sizeof(client));
    client->connected[index] = 0;
    client->client_fd[index] = -1;

    client->addr[index].sun_family = AF_UNIX;
    strncpy(client->addr[index].sun_path, sock_path, sizeof(client->addr[index].sun_path) - 1);


    result = 0;
    return result;
}

int client_check_connection(app_client * const client, int index){
    int result = -1;

    if (client->client_fd[index] < 0)
    {
        client->client_fd[index] = socket(AF_UNIX, SOCK_STREAM, 0);
        if (client->client_fd[index] < 0)
        {
            printf("Failed to create socket: %m\n");
        }

        if (connect(client->client_fd[index],
             (struct sockaddr *)&client->addr[index],
              sizeof(client->addr)) < 0)
        {
            printf("Failed to connect: %m\n");
            close(client->client_fd[index]);
        } else client->connected[index] = 1;
    }
    
    result = 0;
    return result;
}

int client_write(app_client * client, char * bytes, int index){
    int result = -1;

    if (client->client_fd[index] < 0)
    {
        printf("wrong client fd: %m\n");
        goto end;
    }
    

    if (client->connected[index])
    {
        if (write(client->client_fd[index], bytes, strlen(bytes)) == -1)
        {
            printf("Write failed:%m\n");
        }
        else {
            printf("Message sent successfully\n");
            result = 0;
        }
        
    }
    
end:
    return result;
}

int client_close(app_client * const client, int index) {

    if (client != NULL)
    {
        if (client->client_fd[index] != -1)
        {
            close(client->client_fd[index]);
            client->client_fd[index] = -1;
        }

        if (client->server_fd[index] != -1)
        {
            close(client->server_fd[index]);
            client->server_fd[index] = -1;
        }
        
    }

    

    return 0;
}

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

size_t sendn(int srv_fd, const void * bytes, size_t length){
    size_t sent = 0;
    const uint8_t * buf = (uint8_t *) bytes;

    while (sent < length) {
        ssize_t n = send(srv_fd, buf + sent, length - sent, 0);

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
end:
    return sent;
}

ssize_t recvn(int fd, void * buffer, size_t length) {
    puts("recvn called");
    size_t  received = 0;
    uint8_t *ptr     = (uint8_t *)buffer;
    while (received < length) {
        ssize_t r = recv(fd, ptr + received, length - received, 0);
        if (r < 0 && errno == EINTR) 
            continue;
        if (r == 0)  
            return 0;   /* peer closed */
        if (r < 0)   
            return -1;  /* error */
        received += (size_t) r;
    }
    return (ssize_t) received;
}


int send_header(int srv_fd, const char * header_name){
    int result = -1;

    size_t length = strlen(header_name);

    size_t written = sendn(srv_fd, header_name, length);
    if (written < 0)
    {
        return -1;
    }
    else if (written < length)
    {
        printf("sending %lu bytes instead of %lu\n", written, length);
    }
    else 
    {
        printf("data sent successfully: %s\n", header_name);
    }

    result = 0;
end:
    return result;
}


int send_filesize(int srv_fd, const uint32_t digit){
    int result = -1;
    uint32_t net_len = htonl(digit);

    size_t written = sendn(srv_fd, &net_len, sizeof(net_len));

    if (written < 0){
        perror("send_filesize");
        goto end;
    }
    else{
        printf("Successfully sent filesize: %lu\n", digit);
    }

    result = 0;
end:
    return result;
}


int send_message(int srv_fd, char * bytes){
    int result = -1;

    if (sendn(srv_fd, bytes, strlen(bytes)) < 0){
    }else{
        printf("Successfully sent message: %s\n", bytes);
    }

    result = 0;
end:
    return result;
}



size_t send_block(int srv_fd, FILE * fd, const uint32_t filesize, uint8_t * buffer){
            //int result = -1;
            int sent = -1;
            buffer = (uint8_t*) calloc(filesize, sizeof(uint8_t));
            if (!buffer)
            {
                printf("Failed allocating memory: %m\n");
                return -1;
            }

            int r = fread(buffer, sizeof(char), filesize-1, fd);

            if (r < 0)
            {
                printf("failed reading file: %m\n");
                goto end;
            }
            else if (r < 4096){
                printf("sending %lu bytes instead of %lu\n", r, 4096);
                goto end;
            }
            size_t length = strlen(buffer);

            sent = sendn(srv_fd, buffer, length);
            if (sent < 0){
                printf("Failed sending block: %m\n");
                goto end;
            }
            else if (sent < length){
                printf("sending %lu instead of %lu\n", sent, length);
                goto end;
            }
            //result = 0;
end:
            free(buffer);
            return sent;
}

int send_file(int srv_fd, FILE * fd, const char * header_name){
    int result = -1;
    const uint32_t filesize = get_file_size(fd);

    if (send_header(srv_fd, header_name) < 0){
        printf("failed sending header\n");
        goto end;
    }
    
    if (send_filesize(srv_fd, filesize) < 0){
        printf("failed sending filesize");
        goto end;
    }

    if (filesize > 0)
    {
        
        if (filesize <= 4096)
        {
            uint8_t * buffer = NULL;
            size_t sent = send_block(srv_fd, fd, filesize, buffer);   
                if (sent < 0 || sent < 4096)
                {
                    printf("sent %lu bytes instead of %lu\n", sent, 4096);
                    goto end;
                }
        }
        else
        {
            size_t block_count = (size_t) filesize / 4096;
            size_t extra_package_size = (size_t) filesize % 4096;

            uint8_t * buffer = NULL;//(uint8_t*) calloc(block_count);

            for (size_t i = 0; i < block_count; i++)
            {
                size_t sent = send_block(srv_fd, fd, 4096, buffer);
                if (sent < 0 || sent < 4096)
                {
                    printf("sent %lu bytes instead of %lu\n", sent, 4096);
                    goto end;
                }
                
            }

            size_t sent = send_block(srv_fd, fd, extra_package_size, buffer);
            if (sent < 0 || sent < 4096)
            {
                printf("sent %lu bytes instead of %lu\n", sent, 4096);
                goto end;
            }
            
            
        }
    }
    

    result = 0;
end:
    return result;
}

// @TODO: creating a reliable feedback system
int recv_response(int srv_fd, void * buf){
    int result = -1;
    
    recvn(srv_fd, buf, sizeof(buf));

    result = 0;
end:
    return result;
}