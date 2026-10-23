#include "converter_server.hpp"

#define HEADER_SIZE 128
#define BLOCK_SIZE 4096

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

    int sendn(int srv_fd, const void * bytes, size_t length){
        puts("sendn called");
        int result = -1;
        size_t sent = 0;
        const uint8_t * buf = (uint8_t *) bytes;

        while (sent < length) {
            ssize_t n = send(srv_fd, buf + sent, length - sent, 0);
            if (n < 0) {
                if (errno == EINTR)
                    continue;

                perror("send");
                return -1;
            }
            if (n == 0) {
                fprintf(stderr, "Connection closed during send\n");
                return -1;
            }
            sent += (size_t)n;
        }

        result = 0;
    end:
        return result;
    }

    ssize_t recvn(int fd, void * buffer, size_t length) {
        puts("recvn called");
        size_t  received = 0;
        uint8_t *ptr     = (uint8_t *)buffer;
        while (received < length) {
            ssize_t r = recv(fd, ptr + received, length - received, 0);
            if (r < 0 && errno == EINTR) 
                continue;
            if (r == 0)  
                return 0;   /* peer closed */
            if (r < 0)   
                return -1;  /* error */
            received += (size_t)r;
        }
        return (ssize_t) received;
    }



ssize_t receive_header(int fd, uint8_t *buf)
{
    ssize_t received = recvn(fd, buf, HEADER_SIZE);

    if (received != HEADER_SIZE)
        return received;

    buf[HEADER_SIZE - 1] = '\0';

    printf("Received header: %s\n", (char *)buf);

    return received;
}

ssize_t receive_filesize(int fd, uint32_t *size)
{
    uint32_t net_len;

    ssize_t received = recvn(fd, &net_len, sizeof(net_len));

    if (received != sizeof(net_len))
        return received;

    *size = ntohl(net_len);

    printf("Received filesize: %u\n", *size);

    return received;
}

ssize_t receive_message(int fd, uint32_t size, char **message)
{
    *message = NULL;

    char *buffer = (char *) calloc((size_t)size + 1, sizeof(char));

    if (!buffer)
        return -1;

    if (size == 0) {
        *message = buffer;
        return 0;
    }

    ssize_t received = recvn(fd, buffer, size);

    if (received != (ssize_t)size) {
        free(buffer);
        return received;
    }

    buffer[size] = '\0';
    *message = buffer;

    return received;
}

ssize_t receive_block(int srv_fd, FILE *fd,
                      uint32_t filesize, uint8_t *buffer)
{
    if (filesize == 0)
        return 0;

    ssize_t received = recvn(srv_fd, buffer, filesize);

    if (received != (ssize_t)filesize)
        return received;

    size_t written = fwrite(buffer, 1, filesize, fd);

    if (written != filesize)
        return -1;

    return (ssize_t)written;
}

int receive_file(int srv_fd, const char *output_path)
{
    int result = -1;
    uint8_t header[HEADER_SIZE] = {0};
    uint32_t filesize = 0;
    uint32_t remaining = 0;
    FILE *file = NULL;
    uint8_t *buffer = NULL;

    if (receive_header(srv_fd, header) != HEADER_SIZE)
        goto end;

    if (receive_filesize(srv_fd, &filesize) != sizeof(uint32_t))
        goto end;

    file = fopen(output_path, "wb");

    if (!file) {
        perror("fopen");
        goto end;
    }

    buffer = static_cast<uint8_t *>(malloc(BLOCK_SIZE));

    if (!buffer) {
        perror("malloc");
        goto end;
    }

    remaining = filesize;

    while (remaining > 0) {
        uint32_t block_size =
            remaining > BLOCK_SIZE ? BLOCK_SIZE : remaining;

        ssize_t received =
            receive_block(srv_fd, file, block_size, buffer);

        if (received != (ssize_t)block_size) {
            fprintf(stderr, "Failed receiving file block\n");
            goto end;
        }

        remaining -= (uint32_t)received;
    }

    if (fflush(file) != 0) {
        perror("fflush");
        goto end;
    }

    result = 0;

end:
    free(buffer);

    if (file && fclose(file) != 0)
        result = -1;

    if (result == 0)
        printf("File received successfully: %s (%u bytes)\n",
               output_path, filesize);

    return result;
}

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
