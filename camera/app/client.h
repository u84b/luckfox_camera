#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

// temporary
#define CONNECTIONS_COUNT 1 // in the future we may need multiple clients or connections

typedef struct {
    struct sockaddr_un addr[CONNECTIONS_COUNT];
    int client_fd[CONNECTIONS_COUNT];
    unsigned short int connected[CONNECTIONS_COUNT]; // for better padding of the structure
} app_client;


// @TODO: a single interface for working with IPC and TCP/IP

int client_init(app_client * const client, char * sock_path, int index);
int client_config(app_client * const client, int index);
int client_check_connection(app_client * const client, int index);
int client_write(app_client * const client, char *bytes, int index);

int net_client_init(app_client * const client, char * hostname, int index);
int net_client_config(app_client * const client, int index);
int net_client_check_connection();