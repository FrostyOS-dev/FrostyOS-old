#include <stdio.h>

#include <kernel/process.h>

int main(int, char**) {
    printf("My PID is %lu\n", getpid());
    char* argv[] = {"echo", "Hello, World!", nullptr};
    char* envv[] = {nullptr};
    printf("Child PID is %lu\n", exec("/data/bin/echo", argv, envv));
    return 0;
}