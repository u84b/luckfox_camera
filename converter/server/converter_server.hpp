#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <functional>
#include "../imageu/image_utils.hpp"
#include "../files/file_utils.hpp"

#define SOCK_PATH "/tmp/local.sock"


// UNUSED IN main.cpp NOW
namespace server {

    extern std::string current;
    extern std::string converted;
    extern int width;
    extern int height;

    typedef struct {
        struct sockaddr_un addr;
        struct sockaddr_un cli_addr;
        char buffer[64];
        struct pollfd cpoll;
        int server_fd;
        int client_fd;
        int length;

    } cserver;

    bool init(cserver * srv);
    bool check_connection(cserver * srv);
    bool config();
    bool run(cserver * srv, const std::function<void(cserver *, int)>& func);
    bool prepare();

    void convert(cserver * srv, int count);

}