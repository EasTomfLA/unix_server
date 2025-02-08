#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int ptrace_with_pid(int pid);

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        printf("arguments len not good!\n\r");
        return 0;
    }
    printf("pid:%d\r\n", getpid());
    // printf("arg0:%s arg1:%s\r\n", argv[0], argv[1]);

    pid_t tPid = atol(argv[1]);
    printf("ptrace target pid is %d\r\n", tPid);

    ptrace_with_pid(tPid);
    printf("main exit\r\n");
    return 0;
}

#define NT_ARM_HW_WATCH 0x403
#define NT_ARM_HW_BREAK 0x402
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/uio.h>

int ptrace_with_pid(int pid) 
{
    long pRet = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    printf("attach pid:%d pRet:%lu\r\n", pid, pRet);
    if (pRet < 0) {
        printf("attach failed:%s\r\n", strerror(errno));
        return 0;
    }

    int status;
    printf("waitpid 1\r\n");
    waitpid(pid, &status, 0);
    printf("waitpid 2\r\n");

    printf("getregs 1\r\n");
    struct user_hwdebug_state dbg;
    struct iovec iov;
    struct user_hwdebug_state state;
    int data_size = sizeof(struct user_hwdebug_state);
    // 获取硬件监视器寄存器状态
    iov.iov_base = &dbg;
    iov.iov_len = data_size;
    pRet = ptrace(PTRACE_GETREGSET, pid, NT_ARM_HW_BREAK, &iov);
    if (pRet < 0) {
        printf("getregset failed %s\r\n", strerror(errno));
        return 1;
    }

    // 打印硬件监视器寄存器状态
    printf("Debug register state:\r\n");
    for (int i = 0; i < 16; i++) {
        printf("%d addr:%llx ctrl:%x pad:%x\r\n", i, dbg.dbg_regs[i].addr,
                dbg.dbg_regs[i].ctrl, dbg.dbg_regs[i].pad);
    }
    printf("getregs 2\r\n");

    printf("detach 1\r\n");
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    printf("detach 2\r\n");
    printf("end\r\n");
    return 0;

}