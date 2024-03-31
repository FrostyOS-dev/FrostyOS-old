#include <stdio.h>

int main() {
    for (int i = 0; i < 10'000; i++)
        dbgprintf("%d\n", i);
    return 0;
}