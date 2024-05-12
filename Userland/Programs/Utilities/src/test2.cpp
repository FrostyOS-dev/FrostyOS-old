#include <stdio.h>

int main() {
    for (int i = 10'000; i >= 0; i--)
        dbgprintf("%d\n", i);
    return 0;
}