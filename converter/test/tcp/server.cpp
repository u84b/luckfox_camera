#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include "server.hpp"

#define PORT 8080

namespace server {

    std::string current = "frame.raw";
    std::string converted = "frame.png";
    int width = 640;
    int height = 480;

    bool init(cserver * srv){
        bool result = false;

        srv->server_fd = -1;
        srv->client_fd = -1;
        srv->length = sizeof(srv->cli_addr);

        std::memset(&srv->addr, 0, sizeof(srv->addr));
        std::memset(&srv->cli_addr, 0, sizeof(srv->cli_addr));
        std::memset(&srv->cpoll, 0, sizeof(srv->cpoll));

        srv->server_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (srv->server_fd == -1)
        {
            std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
            return result;
        }

        int opt = 1;

        // задаём флаги для сокета
        if (setsockopt(srv->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        {
            std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
            close(srv->server_fd);
            srv->server_fd = -1;
            return result;
        }

        srv->addr.sin_family = AF_INET;
        srv->addr.sin_port = htons(PORT);

        // преобразуем строковое представление IP адреса в понятный компьютеру вид
        if (inet_pton(AF_INET, IP_ADDR, &srv->addr.sin_addr) != 1)
        {
            std::cerr << "Invalid IP_ADDR: " << IP_ADDR << std::endl;
            close(srv->server_fd);
            srv->server_fd = -1;
            return result;
        }

        // связываем сокет с конкретным адресом
        if (bind(srv->server_fd, (struct sockaddr*)&srv->addr, sizeof(srv->addr)) == -1)
        {
            std::cerr << "bind failed: " << strerror(errno) << std::endl;
            close(srv->server_fd);
            srv->server_fd = -1;
            return result;
        }

        std::cout << "bind successful" << std::endl;
        
        // ожидаем входящие подключения
        
        if (listen(srv->server_fd, 2) == -1)
        {
            std::cerr << "listen failed: " << strerror(errno) << std::endl;
            close(srv->server_fd);
            srv->server_fd = -1;
            return result;
        }

        std::cout << "listened" << std::endl;

        result = true;
        return result;
    }

    bool check_connection(cserver * srv){
        bool result = false;

        if (srv->client_fd == -1)
        {
            std::cout << "Server fd: " << srv->server_fd
            << " ; Length: "
            << srv->length << " ; " << std::endl;

            srv->length = sizeof(srv->cli_addr);

            srv->client_fd = accept(srv->server_fd,
                (struct sockaddr*)&srv->cli_addr,
                (socklen_t*)&srv->length);

            if (srv->client_fd == -1)
            {
                if (errno != EINTR)
                {
                    std::cerr << "client fd accept error: " << strerror(errno) << std::endl;
                }

                return result;
            }

            std::cout << "Client FD = " << srv->client_fd << std::endl;

            srv->cpoll.fd = srv->client_fd;
            srv->cpoll.events = POLLIN;
            srv->cpoll.revents = 0;

            std::cout << "server connection checked" << std::endl;
        }
        else {

            result = true;

        }

        return result;
    }

    // return --> continue in main while loop
    bool run(cserver * srv, const std::function<void(cserver *, int)>& func){
        bool result = false;

        if (srv->client_fd > 0)
        {
            int ready = poll(&srv->cpoll, 1, -1);

            if (ready < 0)
            {
                if (errno == EINTR)
                {
                    return true;
                }

                std::cerr << "poll failed: " << strerror(errno) << std::endl;
                return result;
            }

            if (ready == 0)
            {
                return true;
            }

            if (srv->cpoll.revents & (POLLERR | POLLNVAL))
            {
                std::cerr << "Client socket error: " << srv->cpoll.revents << std::endl;
                close(srv->client_fd);
                srv->client_fd = -1;
                std::memset(&srv->cpoll, 0, sizeof(srv->cpoll));
                return result;
            }

            if (srv->cpoll.revents & (POLLHUP | POLLRDHUP))
            {
                std::cout << "Client FD = " << srv->client_fd << " disconnected" << std::endl;
                close(srv->client_fd);
                srv->client_fd = -1;
                std::memset(&srv->cpoll, 0, sizeof(srv->cpoll));
                return result;
            }

            if (srv->cpoll.revents & POLLIN)
            {
                std::cout << "revents = " << srv->cpoll.revents << std::endl;

                std::memset(srv->buffer, '\0', sizeof(srv->buffer));

                ssize_t count = read(srv->client_fd, srv->buffer, sizeof(srv->buffer));

                if (count == 0)
                {
                    std::cout << "Client FD = " << srv->client_fd << " disconnected" << std::endl;
                    close(srv->client_fd);
                    srv->client_fd = -1;
                    std::memset(&srv->cpoll, 0, sizeof(srv->cpoll));
                    return result;
                }

                if (count < 0)
                {
                    if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
                    {
                        std::cerr << "read failed: " << strerror(errno) << std::endl;
                        close(srv->client_fd);
                        srv->client_fd = -1;
                        std::memset(&srv->cpoll, 0, sizeof(srv->cpoll));
                    }

                    return result;
                }

                func(srv, static_cast<int>(count));

                ssize_t sent = 0;

                while (sent < count)
                {
                    ssize_t written = write(srv->client_fd, srv->buffer + sent, count - sent);

                    if (written < 0) // checking for errors during feedback 
                    {
                        if (errno == EINTR)
                        {
                            continue;
                        }

                        std::cerr << "Failed feedback for client: " << strerror(errno) << std::endl;
                        close(srv->client_fd);
                        srv->client_fd = -1;
                        std::memset(&srv->cpoll, 0, sizeof(srv->cpoll));
                        return result;
                    }

                    if (written == 0) // zero value
                    {
                        std::cerr << "Client feedback write returned zero" << std::endl;
                        close(srv->client_fd);
                        srv->client_fd = -1;
                        std::memset(&srv->cpoll, 0, sizeof(srv->cpoll));
                        return result;
                    }

                    sent += written;
                }
            }
            else if (srv->cpoll.revents != 0)
            {
                std::cerr << "Unexpected poll events: " << srv->cpoll.revents << std::endl;
                return result;
            }
        }

        result = true;
        return result;
    }

}


// I used it for tests

int main(void)
{
    int server_fd;
    int client_fd;

    struct sockaddr_in address;
    socklen_t address_len = sizeof(address);

    const char *message = "Good morning Vietnam";

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 1);

    printf("Waiting for connection on port %d...\n", PORT);

    client_fd = accept(
        server_fd,
        (struct sockaddr *)&address,
        &address_len
    );

    send(client_fd, 
        message, 
        strlen(message), 
        0);

    printf("Message sent\n");

    char buffer[128];

    recv(client_fd, 
        &buffer, 
        sizeof(buffer) - 1, 
        0);

    printf("Received: %s\n", buffer);

    close(client_fd);
    close(server_fd);

    return 0;
}