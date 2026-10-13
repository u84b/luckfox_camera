#include "converter_server.hpp"


namespace server {
    
    std::string current = "frame.raw";
    std::string converted = "frame.png";
    int width = 640;
    int height = 480;
    
    bool init(cserver * srv){
        bool result = false;

        unlink(SOCK_PATH);

        srv->server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        std::memset(&srv->addr, 0, sizeof(struct sockaddr_un));
        std::memset(&srv->cli_addr, 0, sizeof(struct sockaddr_un));
        std::memset(&srv->cpoll, 0, sizeof(struct pollfd));
        srv->addr.sun_family = AF_UNIX;
        std::strncpy(srv->addr.sun_path, SOCK_PATH, sizeof(srv->addr.sun_path) - 1);
        srv->client_fd = -1;
        srv->length = sizeof(srv->cli_addr);

        if (bind(srv->server_fd, (struct sockaddr*)&srv->addr, sizeof(srv->addr)) == -1)
        {
            std::cerr << "bind failed" << std::endl;
            close(srv->server_fd);
            return result;
        }

        std::cout << "bind successfull" << std::endl;

        if (listen(srv->server_fd, 2) < 0)
        {
            std::cerr << "Listen failed" << std::endl;
            close(srv->server_fd);
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

            srv->client_fd = accept(srv->server_fd, 
                (struct sockaddr*)&srv->cli_addr, (socklen_t*)&srv->length);

            if (srv->client_fd < 0 && errno != EINTR)
            {
                std::cerr << "client fd accept error" << std::endl;
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
    
    // if false ==> continue main while loop
    bool run(cserver * srv, const std::function<void(cserver *, int)>& func){
        bool result = false;


        if (srv->client_fd > 0)
        {
            int ready = poll(&srv->cpoll, 1, -1);

            if (ready < 0)
            {
                std::cerr << "socket fd" << std::endl;
                return result;
            }

            if (srv->cpoll.revents & POLLHUP)
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
                int count = read(srv->client_fd, srv->buffer, sizeof(srv->buffer));
                if (count > 0)
                {

                    func(srv, count);

                }
                if (write(srv->client_fd, srv->buffer, count) == -1)
                {
                    std::cerr << "Failed feedback for client" << std::endl;
                }
            }

        }
        
        result = true;
        return result;
    }

    void convert(cserver * srv, int count){
        std::cout << "Got: " << srv->buffer << "\n with count: " << count << std::endl;
  
        current.assign(srv->buffer, count);
        converted = get_filename_without_extension(current).append(".png");

        if (!nv12_to_png(server::current, server::converted, server::width, server::height))
        {
            std::cerr << "Conversion failed" << std::endl;
            return;
        }
        else {
            std::cout << "Done: " << server::converted << "\n";
        }
                    
        if (write(srv->client_fd, srv->buffer, count) == -1)
        {
            std::cerr << "Failed feedback for client" << std::endl;
        }
    }
    
}