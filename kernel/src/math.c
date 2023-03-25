#include "math.h"

int max(int a, int b) {
    return a > b ? a : b;
}

unsigned umax(unsigned a, unsigned b) {
    return a > b ? a : b;
}

long lmax(long a, long b) {
    return a > b ? a : b;
}

unsigned long ulmax(unsigned long a, unsigned long b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

unsigned umin(unsigned a, unsigned b) {
    return a < b ? a : b;
}

long lmin(long a, long b) {
    return a < b ? a : b;
}

unsigned long ulmin(unsigned long a, unsigned long b) {
    return a < b ? a : b;
}

int abs(int n) {
    return n < 0 ? (n * -1) : n;
}

unsigned uabs(unsigned n) {
    return n;
}

long labs(long n) {
    return n < 0 ? (n * -1) : n;
}

unsigned long ulabs(unsigned long n) {
    return n;
}

div_t div(int numer, int denom) {
    div_t ret;
    ret.rem = numer % denom;
    ret.quot = (numer - ret.rem) / denom;
    return ret;
}

udiv_t udiv(unsigned numer, unsigned denom) {
    udiv_t ret;
    ret.rem = numer % denom;
    ret.quot = (numer - ret.rem) / denom;
    return ret;
}

ldiv_t ldiv(long numer, long denom) {
    ldiv_t ret;
    ret.rem = numer % denom;
    ret.quot = (numer - ret.rem) / denom;
    return ret;
}

uldiv_t uldiv(unsigned long numer, unsigned long denom) {
    uldiv_t ret;
    ret.rem = numer % denom;
    ret.quot = (numer - ret.rem) / denom;
    return ret;
}
