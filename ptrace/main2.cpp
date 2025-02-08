#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    while (true) {
        printf("i am main2, pid:%d please trace me!\r\n", getpid());
        sleep(5);
    }

    printf("main2 exit\r\n");
    return 0;
}