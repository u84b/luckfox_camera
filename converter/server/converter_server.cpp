#include "converter_server.hpp"


//namespace server {

namespace server{

    std::string current = "frame.raw";
    std::string converted = "frame.png";
    int width = 640;
    int height = 480;

        void convert(cserver * srv, int count){
            std::cout << "Got: " << srv->buffer << "\n with count: " << count << std::endl;
    
            current.assign(srv->buffer, count);
            converted = get_filename_without_extension(current).append(".png");

            // if (!nv12_to_png(current, converted, width, height))
            // {
            //     std::cerr << "Conversion failed" << std::endl;
            //     return;
            // }
            // else {
            //     std::cout << "Done: " << converted << "\n";
            // }
                        
            if (write(srv->client_fd, srv->buffer, count) == -1)
            {
                std::cerr << "Failed feedback for client" << std::endl;
            }
        }

        void convert(cserver * srv){

        }
}

namespace server::ipc{

        
    bool init(cserver * srv, char * host){
        bool result = false;

        strncpy(srv->hostname, host, sizeof(host));

        unlink(srv->hostname);

        srv->server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        std::memset(&srv->addr, 0, sizeof(struct sockaddr_un));
        std::memset(&srv->cli_addr, 0, sizeof(struct sockaddr_un));
         std::memset(&srv->cpoll, 0, sizeof(struct pollfd));
        srv->addr.ipc.sun_family = AF_UNIX;
        std::strncpy(srv->addr.ipc.sun_path, srv->hostname, sizeof(srv->addr.ipc.sun_path) - 1);
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
    };

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

}
//}

namespace server::tcp{
    std::string current = "frame.raw";
    std::string converted = "frame.png";

    int width = 640;
    int height = 480;

    char *msg_buf = NULL;

    bool init(cserver * srv, char * host){

        bool result = false;

        //msg_buf = (char*) calloc(128, sizeof(char));

        strncpy(srv->hostname, host, sizeof(host));

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

        srv->addr.tcp.sin_family = AF_INET;
        srv->addr.tcp.sin_port = htons(PORT);

        // преобразуем строковое представление IP адреса в понятный компьютеру вид
        if (inet_pton(AF_INET, srv->hostname, &srv->addr.tcp.sin_addr) != 1)
        {
            std::cerr << "Invalid IP_ADDR: " << srv->hostname << std::endl;
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

                //std::memset(srv->buffer, '\0', sizeof(srv->buffer));
                msg_buf = (char*) calloc(128, sizeof(char));

                ssize_t count = read(srv->client_fd, msg_buf, strlen(msg_buf)+1);

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

                //func(srv, static_cast<int>(count));

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

                    if (written == 0)
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
