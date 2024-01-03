#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

// https://www.jianshu.com/p/c30860804713
int serv_listen(const char *name);

int main() {
    printf("pid:%d\r\n", getpid());
    std::cout << "Hello, World!" << std::endl;
    // flame:/proc/net # netstat -apn | grep 9842
    // unix  2      [ ACC ]     STREAM     LISTENING       125390 9842/unix_server   /data/local/tmp/xxtt
    // unix  2      [ ACC ]     STREAM     LISTENING       125389 9842/unix_server   @00003
    printf("serv_listen:%d\r\n", serv_listen("\0wahaha"));

    printf("serv_listen:%d\r\n", serv_listen("/data/local/tmp/xxtt"));

    sleep(10);
    return 0;
}


#include <sys/un.h>
int serv_listen(const char *name) {
    int fd, len, err, rval = -1;
    struct sockaddr_un un;
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        goto errout;
    }

    unlink(name);
    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, name);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(name);

    if(bind(fd, (struct sockaddr *)&un, len) < 0) {
        rval = -2;
        goto errout;
    }
    if(listen(fd, 10) < 0) {
        rval = -3;
        goto errout;
    }
    printf("serv listen ret\r\n");
    return(fd);
errout:
    printf("ser listen errout:%s\r\n", strerror(errno));
    err = errno;
    close(fd);
    errno = err;
    return(rval);
}