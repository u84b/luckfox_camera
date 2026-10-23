#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define PORT 8080

// temporary
#define CONNECTIONS_COUNT 1 // in the future we may need multiple clients or connections

typedef struct {
    struct sockaddr_un addr[CONNECTIONS_COUNT];
    struct sockaddr_in net_addr[CONNECTIONS_COUNT];
    int client_fd[CONNECTIONS_COUNT];
    int server_fd[CONNECTIONS_COUNT];
    unsigned short int connected[CONNECTIONS_COUNT]; // for better padding of the structure
    unsigned short int net_connected[CONNECTIONS_COUNT];
} app_client;


// @TODO: a single interface for working with IPC and TCP/IP

int client_init(app_client * const client, char * sock_path, int index);
int client_config(app_client * const client, int index);
int client_check_connection(app_client * const client, int index);
int client_write(app_client * const client, char *bytes, int index);
int client_close(app_client * const client, int index);


int net_client_init(app_client * const client, char * hostname, int index);
int net_client_config(app_client * const client, int index);
int net_client_check_connection(app_client * const client, int index);
size_t sendn(int srv_fd, const void * bytes, size_t length);
int send_header(int srv_fd, const char * header_name);
int send_filesize(int srv_fd, const uint32_t digit);
int send_message(int srv_fd, char * bytes);
int recv_response(int srv_fd, void * buf);
size_t send_block(int srv_fd, FILE * fd, const uint32_t filesize, uint8_t * buffer);
int send_file(int srv_fd, FILE * fd, const char * header_name);
int net_client_write(app_client * const client, char * bytes, int index);
int net_client_close(app_client * const client, int index);