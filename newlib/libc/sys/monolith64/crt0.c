#include <fcntl.h>
#include <stdint.h>

extern void exit(int);
extern int main(int, char **, char **);
void _start(void) {
    uintptr_t *sp;
//  asm volatile("pop %rbp");
    asm volatile("mov %%rsp, %0" : "=r"(sp));

    int argc = sp[2];
    char **argv = (char **)&sp[3];
    char **envp = argv + argc + 1;

    int rc = main(argc, argv, envp);
    exit(rc);
}