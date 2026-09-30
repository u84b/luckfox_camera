#include "../app/app.h"
#include <signal.h>
#include <sys/time.h>

#define BUTTON_PIN 54
#define DEBOUNCE_TIME 100

volatile static sig_atomic_t keep_running = 1;

void handle_termination(int signum){
    if (signum == SIGTERM || signum == SIGINT){ // it was obvious but I haven't changed it till now
        keep_running = 0;
    }
}

long long get_current_time_ms(){
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000LL);
}

int main(){  
    // button bt;
    long long last_trigger_time = 0;
    int result = -1;
    int gpfd = -1;
    //int poll_count = 0;
    
    
    // button_init(&bt);

    signal(SIGTERM, handle_termination);
    signal(SIGINT, handle_termination);


    if (gpio_configuration(BUTTON_PIN) < 0)
    {
        printf("GPIO configuration error\n");
        goto end;
    }

    int ret = gpio_monitor_pin_value(&gpfd, BUTTON_PIN, O_RDONLY);
    
    if (ret < 0)
    {
        printf("Failed monitoring GPIO: %m\n");
        goto end;
    }
    
    else if (ret == 0) {

        struct pollfd gpio_poll;
        memset(&gpio_poll, 0, sizeof(struct pollfd));
        gpio_poll.fd = gpfd;
        gpio_poll.events = POLLPRI;
        gpio_poll.revents = 0;


        while (keep_running)
        {
            int ready = poll(&gpio_poll, 1, 1000);
            
            if (ready < 0)
            {
                printf("An error occured: %m\n");
                goto end;
            }
            
            else if (gpio_poll.revents & POLLPRI)
            {

                int rd = gpio_read(&gpfd, 54);

                long long now = get_current_time_ms();

                if ((now - last_trigger_time) < DEBOUNCE_TIME)
                {
                    continue;
                }
                
                last_trigger_time = now;

                if (rd < 0) {
                    printf("Failed reading GPIO this time: %m\n");
                    continue;
                }
                else if (rd == 0) {
                    printf("GPIO low\n");   
                }
            }   
        }
    }
    result = 0;

end:
    gpio_unexport(54);
    gpio_close(&gpfd, 54);
    puts("ending");
    return result;
}