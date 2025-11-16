#include "desertbus.h"

void signal_handler(int sig, siginfo_t* info, void* ucontext) {
    ucontext = ucontext;
    switch (sig) {
    case SIGSEGV:
        crash("SIGSEGV at addr 0x%lx\n", (long)info->si_addr);

    case SIGINT:
        printf("Debug interupt caught");
        return;

    case SIGFPE:
        printf("Fatal arithmetic error! (its probably a division by zero)");
        crash("");

    default:
        timer_tick();
        return;
    }
}

void init_signal_handler() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = signal_handler;
    sigfillset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        printf("sigaction to catch SIGSEGV failed.");
        exit(-1);
    }
    if (sigaction(SIGFPE, &sa, NULL) == -1) {
        printf("sigaction to catch SIGFPE failed.");
        exit(-1);
    }
    sigaction(SIGRTMIN, &sa, NULL);
    sigaction(SIGWINCH, &sa, NULL);
}