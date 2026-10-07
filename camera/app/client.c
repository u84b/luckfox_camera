#include "client.h"

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

        if (connect(client->client_fd[0],
             (struct sockaddr *)&client->addr[0],
              sizeof(client->addr)) < 0)
        {
            printf("Failed to connect: %m\n");
            close(client->client_fd[0]);
        } else client->connected[0] = 1;
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