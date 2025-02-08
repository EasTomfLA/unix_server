#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>


#include <pthread.h>
#include <errno.h>
#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

static void *
thread_start(void *arg)
{
    int n = 0;
    while (true) {
        printf("i am new thread, n=%d\n\r", n++);
        sleep(1);
    }
    return nullptr;
}

int main(int argc, const char* argv[]) {
    printf("pid:%d\r\n", getpid());

    int s;
    pthread_t tid;
    s = pthread_create(&tid, nullptr,
                                  &thread_start, nullptr);
    if (s != 0)
        handle_error_en(s, "pthread_create");

    while(true) {
        printf("main create thread suss, do nothing\r\n");
        sleep(5);
    }

    printf("main exit\r\n");
    return 0;
}