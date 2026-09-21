#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

#define SOCK_PATH "/tmp/local.sock"

namespace server {

    typedef struct {



    } converter_server;

    bool start();
    bool run();
    bool prepare();

}