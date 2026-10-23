#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <functional>
// #include "../imageu/image_utils.hpp"
#include "../files/file_utils.hpp"

#define SOCK_PATH "/tmp/local.sock"
#define IP_ADDR "192.168.0.142"
#define PORT 8080


namespace server {
    extern std::string current;
    extern std::string converted;
    extern int width;
    extern int height;
    
    typedef struct {
        union {
            struct sockaddr_un ipc;
            struct sockaddr_in tcp;
        } addr;

        union {
            struct sockaddr_un ipc;
            struct sockaddr_in tcp;
        }cli_addr;
        
        char buffer[64];
        char hostname[64];
        struct pollfd cpoll;
        int server_fd;
        int client_fd;
        int length;
    } cserver;
    
    void convert(cserver * srv, int count);
    
    namespace ipc{

        bool init(cserver * srv, char * host);
        bool check_connection(cserver * srv);
        bool config();
        bool run(cserver * srv, const std::function<void(cserver *, int)>& func);
        bool prepare();

        
    }

    

    namespace tcp{
        
        bool init(cserver * srv, char * host);
        bool check_connection(cserver * srv);
        bool run(cserver * srv, const std::function<void(cserver *, int)>& func);

        int sendn(int srv_fd, const void * bytes, size_t length);
        ssize_t recvn(int fd, void * buffer, size_t length);
        ssize_t recvn(int fd, void * buffer, size_t length);
        ssize_t receive_header(int fd, uint8_t * buf);
        ssize_t receive_filesize(int fd, uint32_t size);
        ssize_t receive_message(int fd, uint32_t size);
        ssize_t receive_block(int srv_fd, FILE *fd, uint32_t filesize, uint8_t *buffer);
        int receive_file(int srv_fd, FILE * fd, const char * header_name);

        

    } 

}