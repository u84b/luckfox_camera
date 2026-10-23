#include "server.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>

static volatile sig_atomic_t keep_running = 1;

extern "C" void handle_signal(int signum) {
    if (signum == SIGTERM || signum == SIGINT)
    {
        keep_running = 0;
    }
    
}

#define IP_ADDR "192.168.0.142"
#define PORT 8080



ssize_t recvn(int fd, void *buf, size_t n) {
    puts("recvn called");
    size_t  received = 0;
    uint8_t *ptr     = (uint8_t *)buf;
    while (received < n) {
        ssize_t r = recv(fd, ptr + received, n - received, 0);
        if (r < 0 && errno == EINTR) 
            continue;
        if (r == 0)  
            return 0;   /* peer closed */
        if (r < 0)   
            return -1;  /* error */
        received += (size_t)r;
    }
    return (ssize_t)received;
}


int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP_ADDR, &address.sin_addr) != 1)
    {
        close(server_fd);
        server_fd = -1;
        return -1;
    }

    if (bind(server_fd, (sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Waiting for connection on port %d...\n", PORT);

    sockaddr_in peer{};
    socklen_t peer_len = sizeof(peer);

    int client_fd = accept(
        server_fd, (sockaddr *)&peer, &peer_len);

    if (client_fd < 0) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    std::string pending;
    

    while (true) {
        uint32_t net_len;

        if (recvn(client_fd, &net_len, sizeof(net_len)) <= 0){
            puts("recvn length failed");
            return -1;
        }
            
        uint32_t payload_len = ntohl(net_len);
        printf("The number is: %" PRIu32 "\n", payload_len); 
        void *buffer = malloc(payload_len);
        if (!buffer)
        {
            puts("buffer is NULL");
            return -1;
        }

        if (recvn(client_fd, buffer, payload_len) <= 0)
        {
            free(buffer);
            puts("recvn message failed");
            return -1;
        }

        printf("Received: %s\n", (char*) buffer);

        free(buffer);
    }

    if (!pending.empty()) {
        printf("Received unterminated final message: %s\n",
               pending.c_str());
    }

    close(client_fd);
    close(server_fd);

    return 0;
}
