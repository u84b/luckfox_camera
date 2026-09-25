#include <converter_server.hpp>


volatile static std::sig_atomic_t keep_running = 1;

extern "C" void handle_signal(int signum){
    keep_running = 0;
}

// CURRENT STATE: PROTOTYPE DEVELOPING
// @TODO: change implementation for sending data(image info) to converter
namespace server {

    bool run(){

        signal(SIGINT, handle_signal);
        signal(SIGTERM, handle_signal);

        bool result = false;

        // CREATING EXACTLY OUR SOCKET FOR CONVERTER SERVER
        int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

        if (server_fd < 0)
        {
            std::cerr << "Failed to create socket\n";
            return result;
        }
        
        unlink(SOCK_PATH);

        struct sockaddr_un addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);
        // NOW WE NEED TO CONNECT SERVER TO FILE
        if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        {
            std::cerr << "Bind failed\n";
            close(server_fd);
            return false;
        }
        // LISTEN FOR NEW CLIENTS :)
        if (listen(server_fd, 2) < -1)
        {
            std::cerr << "Listen failed\n";
            close(server_fd);
            return false;
        }

        std::cout << "Server listening: " << SOCK_PATH << "\n";

        while (keep_running) {
            struct sockaddr_un client_addr;
            int length = sizeof(client_addr);

            int client_fd = accept(
                server_fd,
                (sockaddr *)&client_addr,
                (socklen_t *)&length
            );

            char buffer[65];
            memset(buffer, '\0', sizeof(buffer));
            int count = read(client_fd, buffer, sizeof(buffer));
            if (count > 0)
            {
                std::cout << "Got: " << buffer << "\n";
                write(client_fd, buffer, sizeof(buffer));
            }
            close(client_fd);
        }
        

        return result;
    }
    
}