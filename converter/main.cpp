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

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    server::cserver srv;

    if (server::init(&srv))
    {
        std::cout << "init success" << std::endl;
        while (keep_running) {
            if (server::check_connection(&srv))
            {
                std::cout << "==========" << std::endl;
                if (!run(&srv, server::convert))
                {
                    continue;
                }
                
            }

        }
    }

    return 0;
}