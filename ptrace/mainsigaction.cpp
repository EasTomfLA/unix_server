#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <signal.h>
#include <setjmp.h>
static jmp_buf env;
void sigsegv_handler(int sig) {
    printf("Caught SIGSEGV signal!\n");
    // 跳过异常处理代码，返回到之前设置的jmp_buf位置
    longjmp(env, 1);
}

int main(int argc, const char* argv[]) {
    printf("pid:%d\r\n", getpid());

    // 注册SIGSEGV信号处理程序
    struct sigaction sa;
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
    sa.sa_handler = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, NULL);

    // int *p2 = NULL;
    // *p2 = 0;

      // 尝试读取不存在或不可读的地址
    int *p = NULL;  // 不存在的地址
    if (setjmp(env) == 0) {
        *p = 1;
    } else {
        printf("Back to normal flow.\n");
    }
    printf("恢复原始异常处理\r\n");
    sigaction(SIGSEGV, &sabak, NULL);
    int *p2 = NULL;
    *p2 = 0;

    printf("main exit\r\n");
    return 0;
}