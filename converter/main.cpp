#include "imageu/image_utils.hpp"
#include "files/file_utils.hpp"
#include "server/converter_server.hpp"

static volatile std::sig_atomic_t keep_running = 1;

extern "C" void signal_handler(int signum){
    if (signum == SIGTERM || signum == SIGINT)
    {
        keep_running = 0;
    }
    
}

// I already checked it directly on LuckFox single-board. It works correctly.

// @TODO: Implementing converter-server

int main(int argc, char* argv[])
{
    bool result = false;

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    std::string current = "frame.raw";
    std::string converted = "frame.png";
    const int width  = 640;
    const int height = 480;

    unlink(SOCK_PATH);

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct pollfd clipoll;
    memset(&clipoll, 0, sizeof(struct pollfd));

    if (server_fd < 0)
    {
        std::cerr << "Failed to create socket\n";
        return result;
    }

    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        std::cerr << "Bind failed\n";
        close(server_fd);
        return result;
    }
    
    if (listen(server_fd, 1) < -1)
    {
        std::cerr << "Listen failed\n";
        close(server_fd);
        return result;
    }    


    struct sockaddr_un client_addr;
    char buffer[65];
    int length = sizeof(client_addr);
    int client_fd = -1;

    while (keep_running) {

        if (client_fd > 0)
        {
            int ready = poll(&clipoll, 1, -1);

            if (ready < 0)
            {
                std::cerr << "socket fd\n";
                continue;
            }

            if (clipoll.revents & POLLHUP)
            {
                std::cout << "Client FD = " << client_fd << " disconnected\n";
                close(client_fd);
                client_fd = -1;
                std::memset(&clipoll, 0, sizeof(clipoll));
                continue;
            }
            
            if (clipoll.revents & POLLIN)
            {
                std::cout << "revents = " << clipoll.revents << "\n";
                std::memset(buffer, '\0', sizeof(buffer));
                int count = read(client_fd, buffer, sizeof(buffer));
                if (count > 0)
                {
                    std::cout << "Got: " << buffer << "\n with count: " << count << "\n";
                    
                    current.assign(buffer, count);
                    converted = get_filename_without_extension(current).append(".png");

                    if (!nv12_to_png(current, converted, width, height))
                    {
                        std::cerr << "Conversion failed\n";
                        return 1;
                    }
                    
                    if (write(client_fd, buffer, count) == -1)
                    {
                        std::cerr << "Failed feedback for client\n";
                    }
                }   
            }
        } else if (client_fd == -1){
            client_fd = accept(server_fd, (sockaddr *)&client_addr, (socklen_t *)&length);
            if (client_fd < 0 && errno != EINTR)
            {
                std::cerr << "client fd accept error\n";
            }

            std::cout << "Client FD = " << client_fd << "\n";

            clipoll.fd = client_fd;
            clipoll.events = POLLIN;
            clipoll.revents = 0;
            
        }
    }



    std::cout << "Done\n";
    return 0;
}