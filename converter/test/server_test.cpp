#include "../server/converter_server.hpp"


volatile static std::sig_atomic_t keep_running = 1;

extern "C" void handle_signal(int signum){
    if (signum == SIGINT || signum == SIGTERM)
    {
        keep_running = 0;
    } 
}

// CURRENT STATE: PROTOTYPE DEVELOPING
// @TODO: change implementation for sending data(image info) to converter
namespace server {

    bool run(){
        struct sigaction sa;
        sa.sa_handler = handle_signal;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGINT, &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);

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

        std::cout << "Server listening: " << SOCK_PATH << "\n" << "Status: " << keep_running << "\n";
        
        struct sockaddr_un client_addr;
        char buffer[65];
        int length = sizeof(client_addr);
        int client_fd = accept(
                        server_fd,
                        (sockaddr *)&client_addr,
                        (socklen_t *)&length
                    );
            
        std::cout << "Client FD = " << client_fd << "\n";
            
        if (client_fd < 0 && errno != EINTR)
        {
            perror("accept");       
        }
        else {
            while (keep_running) {   
                memset(buffer, '\0', sizeof(buffer));
                int count = read(client_fd, buffer, sizeof(buffer));
                if (count > 0)
                {
                    std::cout << "Got: " << buffer << "\n with count: " << count << "\n";
                    write(client_fd, buffer, count);
                }            
            }
        }

        
        if (client_fd)
        {
            close(client_fd);
        }

        return result;
    }
    
}

int main(){

    if (!server::run())
    {
        std::cerr << "Server stopped\n";
    }
    
    return 0;
}