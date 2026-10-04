#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include <csignal>

#define SOCK_PATH "/tmp/local.sock"


// UNUSED IN main.cpp NOW
namespace server {

    // typedef struct {



    // } converter_server;

    bool start();
    bool run();
    bool prepare();

}