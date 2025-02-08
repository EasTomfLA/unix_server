#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int main_fork();
int ptraceTest() ;
int ptraceChild() ;

int main(int argc, const char* argv[]) {
    // main_fork();
    // ptraceTest();
    ptraceChild();
    printf("main exit\r\n");
    return 0;
}

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/uio.h>
#include <errno.h>
#define log_dbg printf
#define NT_ARM_HW_WATCH 0x403
#define NT_ARM_HW_BREAK 0x402


int ptraceChild() 
{
    pid_t  child_pid = fork();
    if (child_pid == 0) {
        log_dbg("ptraceChild 子进程进入\r\n");
        // ptrace(PTRACE_TRACEME, 0, NULL, NULL);
       sleep(12);

        log_dbg("ptraceChild 子进程退出\r\n");
        // 子进程退出
        exit(0);
    } else if (child_pid > 0) {

       if (ptrace(PTRACE_ATTACH, child_pid, NULL, NULL) < 0) {
           log_dbg("ptrace child process failed\r\n");
           return 0;
       }
        // 父进程中等待子进程完成
        log_dbg("ptraceChild 父进程等待子进程停止 1\r\n");
        waitpid(child_pid, NULL, 0);
        log_dbg("ptraceChild 父进程等待子进程停止 2\r\n");

        // 使用ptrace读取硬件断点寄存器的值
        struct user_hwdebug_state dbg;
        struct iovec iov;
        int data_size = sizeof(struct user_hwdebug_state);
        // 获取硬件监视器寄存器状态
        iov.iov_base = &dbg;
        iov.iov_len = data_size;
        if (ptrace(PTRACE_GETREGSET, child_pid, NT_ARM_HW_BREAK, &iov) < 0) {
            log_dbg("ptraceChild getregset failed:%s\r\n", strerror(errno));
            return 0;
        }

       if (ptrace(PTRACE_DETACH, child_pid, NULL, NULL) < 0) {
           log_dbg("ptrace detect failed:%s\r\n", strerror(errno));
           return 0;
       }

        // 打印硬件监视器寄存器状态
        log_dbg("ptraceChild Debug register state:\n\r\n");
        for (int i = 0; i < 16; i++) {
            log_dbg("ptraceChild %d addr:%llx ctrl:%x pad:%x\r\n", i, dbg.dbg_regs[i].addr,
                    dbg.dbg_regs[i].ctrl, dbg.dbg_regs[i].pad);
        }
        log_dbg("pad:%x dbg_info:%x\r\n", dbg.pad, dbg.dbg_info);

        log_dbg("ptraceChild 父进程退出\r\n");
        // 父进程退出
        return 0;
    } else {
        perror("ptraceChild fork\r\n");
        return 0;
    }
}

int main_fork() {

    pid_t child_pid = fork();
    if (child_pid == 0)
    {
        sleep(10);
        // 使用ptrace跟踪父进程
        if (ptrace(PTRACE_ATTACH, getppid(), NULL, NULL) < 0)
        {
            log_dbg("ptrace parent process failed");
            return 0;
        }

        // 等待父进程暂停
        waitpid(getppid(), NULL, WSTOPPED);

        // 使用ptrace读取硬件断点寄存器的值
        struct user_hwdebug_state dbg;
        struct iovec iov;
        struct user_hwdebug_state state;
        int data_size = sizeof(struct user_hwdebug_state);
        // 获取硬件监视器寄存器状态
        iov.iov_base = &dbg;
        iov.iov_len = data_size;
        if (ptrace(PTRACE_GETREGSET, getppid(), NT_ARM_HW_BREAK, &iov) < 0)
        {
            log_dbg("ptrace getregset failed:%s", strerror(errno));
            return 0;
        }

        // 打印硬件监视器寄存器状态
        log_dbg("Debug register state:\n");
        for (int i = 0; i < 16; i++)
        {
            log_dbg("%d addr:%llx ctrl:%x pad:%x", i, dbg.dbg_regs[i].addr,
                    dbg.dbg_regs[i].ctrl, dbg.dbg_regs[i].pad);
        }

        // 恢复父进程执行
        if (ptrace(PTRACE_DETACH, getppid(), NULL, NULL) < 0)
        {
            log_dbg("ptrace detect failed:%s", strerror(errno));
            return 0;
        }

        log_dbg("子进程退出");
        // 子进程退出
        return 0;
    }
    else if (child_pid > 0)
    {
        // 父进程中等待子进程完成

        log_dbg("父进程等待子进程完成");
        //        ptrace(PT_TRACE_ME, 0, NULL, NULL);
        // 等待子进程退出
        waitpid(child_pid, NULL, 0);

        log_dbg("父进程退出");
        // 父进程退出
        return 0;
    }
    else
    {
        perror("fork");
        return 0;
    }
}


int ptraceSelf2() {
    log_dbg("ptraceSelf start");
    long pid = getpid();
    log_dbg("ptraceSelf trace me 1 pid:%d", pid);
    long pRet;

    log_dbg("ptraceSelf getregs 1");
    struct user_hwdebug_state dbg;
    struct iovec iov;
    struct user_hwdebug_state state;
    int data_size = sizeof(struct user_hwdebug_state);
    // 获取硬件监视器寄存器状态
    iov.iov_base = &dbg;
    iov.iov_len = data_size;
    pRet = ptrace(PTRACE_GETREGSET, pid, NT_ARM_HW_WATCH, &iov);
    if (pRet < 0) {
        log_dbg("ptrace getregset failed:%lx %s", pRet, strerror(errno));
        return 1;
    }

// 打印硬件监视器寄存器状态
    log_dbg("Debug register state:\n");
    for (int i = 0; i < 16; i++) {
        log_dbg("%d addr:%llx ctrl:%x pad:%x", i, dbg.dbg_regs[i].addr,
                dbg.dbg_regs[i].ctrl, dbg.dbg_regs[i].pad);
    }
    log_dbg("ptraceSelf getregs 2");

    log_dbg("ptraceSelf detach 1");
    log_dbg("ptraceSelf detach 2 pRet:%lu", pRet);
    log_dbg("ptraceSelf end");
    return 0;
}

int ptraceTest() 
{
    pid_t  child_pid = fork();
    if (child_pid == 0) {
        sleep(10);
        // 使用ptrace跟踪父进程
        if (ptrace(PTRACE_ATTACH, getppid(), NULL, NULL) < 0) {
            log_dbg("ptrace parent process failed");
            return 0;
        }

        // 等待父进程暂停
        waitpid(getppid(), NULL, WSTOPPED);

        // 使用ptrace读取硬件断点寄存器的值
        struct user_hwdebug_state dbg;
        struct iovec iov;
        struct user_hwdebug_state state;
        int data_size = sizeof(struct user_hwdebug_state);
        // 获取硬件监视器寄存器状态
        iov.iov_base = &dbg;
        iov.iov_len = data_size;
        if (ptrace(PTRACE_GETREGSET, getppid(), NT_ARM_HW_WATCH, &iov) < 0) {
            log_dbg("ptrace getregset failed:%s", strerror(errno));
            return 0;
        }

// 打印硬件监视器寄存器状态
        log_dbg("Debug register state:\n");
        for (int i = 0; i < 16; i++) {
            log_dbg("%d addr:%llx ctrl:%x pad:%x", i, dbg.dbg_regs[i].addr,
                    dbg.dbg_regs[i].ctrl, dbg.dbg_regs[i].pad);
        }

        // 恢复父进程执行
        if (ptrace(PTRACE_DETACH, getppid(), NULL, NULL) < 0) {
            log_dbg("ptrace detect failed:%s", strerror(errno));
            return 0;
        }

        log_dbg("子进程退出");
        // 子进程退出
        return 0;
    } else if (child_pid > 0) {
        // 父进程中等待子进程完成

        log_dbg("父进程等待子进程完成");
//        ptrace(PT_TRACE_ME, 0, NULL, NULL);
        // 等待子进程退出
        waitpid(child_pid, NULL, 0);

        log_dbg("父进程退出");
        // 父进程退出
        return 0;
    } else {
        perror("fork");
        return 0;
    }
}
