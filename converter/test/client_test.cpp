#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <csignal>

#define SOCK_PATH "/tmp/local.sock"

volatile static sig_atomic_t keep_running = 1;

extern "C" void handle_signal(int signum){
    if (signum == SIGINT || signum == SIGTERM)
    {
        keep_running = 0;
    } 
}


int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // 1. Create the client socket
    std::string message;

    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // 2. Define the server address to connect to
    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    // 3. Connect to the server
    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "Connection failed. Is the server running?\n";
        close(client_fd);
        return 1;
    }

    while (keep_running) {
        std::cout << "Input message: ";
        std::getline(std::cin, message);
        std::cout << "Message: " << message << "\n";
        if (message.empty()) continue;

        std::cout << "Length: " << message.length();
        //const char* message = "/home/u84/works/luckfox_camera/converter/frame.png";
        if (write(client_fd, message.c_str(), message.length()) == -1) {
            std::cerr << "Write failed\n";
        } else {
            std::cout << "\nMessage sent successfully.\n";
        }
    }

    // 4. Send data to the server


    // 5. Cleanup
    close(client_fd);
    return 0;
}
