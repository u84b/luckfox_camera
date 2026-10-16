#include "server.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

static volatile sig_atomic_t keep_running = 1;

extern "C" void handle_signal(int signum) {
    if (signum == SIGTERM || signum == SIGINT)
    {
        keep_running = 0;
    }
    
}



// I used it for tests




#define PORT 8080

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
    char buffer[128];

    while (true) {
        ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);

        if (n < 0) {
            if (errno == EINTR)
                continue;

            perror("recv");
            break;
        }

        if (n == 0) {
            printf("Client disconnected\n");
            break;
        }

        // Process every byte received. A read may contain
        // partial messages or multiple complete messages.
        for (ssize_t i = 0; i < n; ++i) {
            if (buffer[i] == '\n') {
                if (!pending.empty() &&
                    pending.back() == '\r') {
                    pending.pop_back();
                }

                printf("Received: %s\n", pending.c_str());
                pending.clear();
            } else {
                pending += buffer[i];
            }
        }
    }

    if (!pending.empty()) {
        printf("Received unterminated final message: %s\n",
               pending.c_str());
    }

    close(client_fd);
    close(server_fd);

    return 0;
}
