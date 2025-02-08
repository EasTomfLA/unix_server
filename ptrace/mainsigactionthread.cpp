#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <signal.h>
#include <setjmp.h>
static jmp_buf env;
void sigsegv_handler(int sig) {
    printf("Caught SIGSEGV signal!\n");
    // 跳过异常处理代码，返回到之前设置的jmp_buf位置
    longjmp(env, 1);
}

static jmp_buf myenv;
void mySigHandler(int sig) {
    printf("Caught signal %d\n", sig);
    // do something to handle the signal
    longjmp(myenv, 1);
}

void* threadFunc(void* arg) {
    printf("thread pid:%d\r\n", getpid());
    struct sigaction sabak;
    sigaction(SIGSEGV, NULL, &sabak);

    if (sabak.sa_handler == SIG_DFL) {
        // 之前使用了默认行为
        printf("之前使用的默认异常处理行为\r\n");
    } else if (sabak.sa_handler == SIG_IGN) {
        // 之前忽略了该信号
        printf("之前忽略该异常\r\n");
    } else {
        // 之前注册了信号处理程序
        printf("之前注册了该异常处理\r\n");
    }
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGSEGV);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    // register SIGSEGV signal handler
    struct sigaction sa;
    sa.sa_handler = mySigHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // do something that may cause segmentation fault
    int* p = NULL;
    if (setjmp(myenv) == 0) {
        *p = 1;
        int *p2 = NULL;
        *p2 = 1;
        int *p3 = NULL;
        *p3 = 1;
    } else {
        printf("thread Back to normal flow.\n");
    }

    printf("恢复原始异常处理\r\n");
    sigaction(SIGSEGV, &sabak, NULL);

    return NULL;
}

static jmp_buf myenv2;
void sigsegv_handler2(int sig) {
    printf("handle2 Caught SIGSEGV signal!\n");
    // 跳过异常处理代码，返回到之前设置的jmp_buf位置
    longjmp(myenv2, 1);
}
void* threadFunc2(void* arg) {
    printf("thread2 in pid:%d\r\n", getpid());
    sleep(2);
    printf("thread2 sleep out pid:%d\r\n", getpid());
    int* p = NULL;
    if (setjmp(myenv2) == 0) {
        *p = 1;
        int *p2 = NULL;
        *p2 = 1;
        int *p3 = NULL;
        *p3 = 1;
    } else {
        printf("thread2 Back to normal flow.\n");
    }
    printf("thread2 finish pid:%d\r\n", getpid());
    return NULL;
}

int main(int argc, const char* argv[]) {
    printf("pid:%d\r\n", getpid());

    struct sigaction sa;
    sa.sa_handler = sigsegv_handler2;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("main sigaction failed");
        exit(EXIT_FAILURE);
    }

    pthread_t tid;
    int ret = pthread_create(&tid, NULL, threadFunc, NULL);
    if (ret != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    
    pthread_t tid2;
    ret = pthread_create(&tid2, NULL, threadFunc2, NULL);
    if (ret != 0) {
        perror("pthread_create 2");
        exit(EXIT_FAILURE);
    }


    // int *p2 = NULL;
    // *p2 = 0;

    // wait for the thread to finish
    pthread_join(tid, NULL);
    pthread_join(tid2, NULL);
    printf("main exit\r\n");
    return 0;
}