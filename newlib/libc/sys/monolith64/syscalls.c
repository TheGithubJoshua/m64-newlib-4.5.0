/* note these headers are all provided by newlib - you don't need to provide them */
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#undef errno
extern int errno;

void _exit() {
	register uint64_t rax asm("rax") = 9;
	asm volatile(
	    "int $0x69"
	    :
	    : "r"(rax)
	    : "memory"
	);
}

int close(int file) {
	long ret;
	asm volatile(
	    "int $0x69"
	    : "=a"(ret)
	    : "a"(3), "D"(file)
	    : "memory"
	);
	if (ret < 0) { errno = -ret; return -1; }
	return ret;
}

//char **environ; /* pointer to array of char * strings that define the current environment variables */
int execve(char *name, char **argv, char **env) {
	long ret;

	asm volatile (
	    "mov $20, %%rax\n\t"   // syscall number: execve
	    "mov %1, %%rdi\n\t"    // path
	    "mov %2, %%rsi\n\t"    // argv
	    "mov %3, %%rdx\n\t"    // envp
	    "int $0x69\n\t"
	    "mov %%rax, %0\n\t"
	    : "=r"(ret)
	    : "r"(name), "r"(argv), "r"(env)
	    : "rax", "rdi", "rsi", "rdx", "memory"
	);

	return (int)ret;
};

int fork() {
	long ret;
	asm volatile(
	    "int $0x69"
	    : "=a"(ret)
	    : "a"(25)
	    : "memory"
	);
	if (ret < 0) { errno = -ret; return -1; }
	return ret;

}

int fstat(int fd, struct stat *st) {
    long ret = 19; // syscall number for fstat in rax

    asm volatile(
        "int $0x69"
        : "+a"(ret)       // rax: input = syscall number, output = return value
        : "D"(st),        // rdi = pointer to struct stat
          "S"(fd)         // rsi = fd (adjust if your kernel expects fd in rdi)
        : "memory"
    );

    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    return 0;
}

int getpid() {
	long ret;
	asm volatile(
	    "int $0x69"
	    : "=a"(ret)
	    : "a"(24)
	    : "memory"
	);
	if (ret < 0) { errno = -ret; return -1; }
	return ret;
}
int isatty(int file) { errno = ENOSYS; return -1; }
int kill(int pid, int sig) {
	long ret = 23; // syscall number

	asm volatile(
	    "int $0x69"
	    : "+a"(ret)       // rax
	    : "D"(pid),       // rdi
	      "S"(sig)        // rsi
	    : "memory"
	);

	if (ret < 0) {
	    errno = -ret;
	    return -1;
	}

	return 0;
}

void (*signal(int sig, void(*func)(int))) (int) {
	long ret = 22; // syscall number

	asm volatile(
	    "int $0x69"
	    : "+a"(ret)       // rax
	    : "D"(sig),       // rdi
	      "S"(func)         // rsi
	    : "memory"
	);

	if (ret < 0) {
	    errno = -ret;
	    return (void*)-1;
	}

	return (void*)ret;
}
int link(char *old, char *new) { errno = ENOSYS; return -1; }
int lseek(int file, int ptr, int dir) { errno = ENOSYS; return -1; }
int open(const char *name, int flags, ...) {
	long ret;
	asm volatile(
	    "int $0x69"
	    : "=a"(ret)                     // output: rax → ret
	    : "a"((unsigned long)2),        // input: syscall number in rax
	      "D"(name),                     // rdi
	      "S"(flags)                     // rsi
	    : "rdx", "rcx", "r11", "memory" // clobbered registers
	);

	if (ret < 0) {
	    errno = -ret;
	    return -1;
	}
	return ret;
}

int read(int file, char *ptr, int len) {
	long ret;
	uint64_t file_ptr = (uint64_t)file;
	uint64_t buf_ptr  = (uint64_t)ptr;
	uint64_t size     = (uint64_t)len;

	asm volatile(
	    "mov %[num], %%rax\n\t"
	    "mov %[p],   %%rdi\n\t"
	    "mov %[s],   %%rsi\n\t"
	    "mov %[d],   %%rdx\n\t"
	    "int $0x69\n\t"
	    "mov %%rax, %[ret]"
	    : [ret] "=r"(ret)
	    : [num] "r"((unsigned long)0),
	      [p]   "r"(file_ptr),
	      [s]   "r"(buf_ptr),
	      [d]   "r"(size)
	    : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
	);

	if (ret < 0) {
	    errno = -ret;
	    return -1;
	}
	return ret;
}
int stat(const char *file, struct stat *st) { errno = ENOSYS; return -1; }
clock_t times(struct tms *buf) { errno = ENOSYS; return -1; }
int unlink(char *name) { errno = ENOSYS; return -1; }
int wait(int *status) { errno = ENOSYS; return -1;}
int write(int file, char *ptr, int len) {
	long ret;
	uint64_t buf_ptr = (uint64_t)ptr;

	asm volatile (
	    "mov %[num], %%rax\n\t"
	    "mov %[a1],  %%rdi\n\t"
	    "mov %[a2],  %%rsi\n\t"
	    "mov %[a3],  %%rdx\n\t"
	    "int $0x69\n\t"
	    "mov %%rax, %[ret]\n\t"
	    : [ret] "=r" (ret)
	    : [num] "r" ((unsigned long)1),          /* syscall number */
	      [a1]  "r" ((unsigned long)file),
	      [a2]  "r" (buf_ptr),
	      [a3]  "r" ((unsigned long)len)
	    : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
	);

	if (ret < 0) {
	    errno = -ret;
	    return -1;
	}
	return ret;
}

caddr_t sbrk(intptr_t incr) {
    long ret;

    asm volatile (
        "mov %[num], %%rax\n\t"  /* syscall number */
        "mov %[a1], %%rdi\n\t"   /* incr */
        "int $0x69\n\t"          
        "mov %%rax, %[ret]\n\t" 
        : [ret] "=r"(ret)
        : [num] "r"((unsigned long)21),
          [a1]  "r"((unsigned long)incr)
        : "rax", "rdi", "rcx", "r11", "memory"
    );

    // Handle error
    if (ret < 0) {
        errno = -ret;
        return (caddr_t)-1;
    }

    return (caddr_t)ret;
}
int gettimeofday(struct timeval *p, void *z) {
	long ret = 26; // syscall number for fstat in rax

	asm volatile(
	    "int $0x69"
	    : "+a"(ret)       // syscall no
	    : "D"(p),        // p
	      "S"(z)         // z
	    : "memory"
	);

	return 0;
}
