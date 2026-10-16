//#include "imageu/image_utils.hpp"
//#include "files/file_utils.hpp"
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

    char * mode = "ipc";

    char * host = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0) {
            if (i + 1 < argc) {
                mode = argv[i + 1];
            } else {
                fprintf(stderr, "err: required to set mode after -m flag (tcp | ipc)\n");
                return 1;
            }
        } 
        else {
            host = argv[i];
        }
    }

    if (host == NULL)
    {
        if (strcmp(mode, "ipc") == 0)
        {
            host = SOCK_PATH;
        }
        else if (strcmp(mode, "tcp") == 0)
        {
            host = IP_ADDR;
        }
        
    }
    
    
    

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    
    
    server::cserver srv;
    
    if (strcmp(mode, "ipc") == 0)
    {
        if (server::ipc::init(&srv, host))
        {
            std::cout << "init success" << std::endl;
            while (keep_running) {
                if (server::ipc::check_connection(&srv))
                {
                    std::cout << "==========" << std::endl;
                    if (!server::ipc::run(&srv, server::convert))
                    {
                        continue;
                    }
                    
                }

            }
        } 
    }
    else if (strcmp(mode, "tcp") == 0)
    {
        if (server::tcp::init(&srv, host))
        {
            std::cout << "init success" << std::endl;
            while (keep_running) {
                if (server::tcp::check_connection(&srv))
                {
                    std::cout << "==========" << std::endl;
                    if (!server::tcp::run(&srv, server::convert))
                    {
                        continue;
                    }
                    
                }
            }
        }
        
    }
    

       

    return 0;
}