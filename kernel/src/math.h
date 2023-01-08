#ifndef _KERNEL_MATH_H
#define _KERNEL_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int quot;
  int rem;
} div_t;

typedef struct {
    unsigned quot;
    unsigned rem;
} udiv_t;

typedef struct {
  long quot;
  long rem;
} ldiv_t;

typedef struct {
  unsigned long quot;
  unsigned long rem;
} uldiv_t;

int max(int a, int b);
unsigned umax(unsigned a, unsigned b);
long lmax(long a, long b);
unsigned long ulmax(unsigned long a, unsigned long b);

int min(int a, int b);
unsigned umin(unsigned a, unsigned b);
long lmin(long a, long b);
unsigned long ulmin(unsigned long a, unsigned long b);

int abs(int n);
unsigned uabs(unsigned n);
long labs(long n);
unsigned long ulabs(unsigned long n);

div_t div(int numer, int denom);
udiv_t udiv(unsigned numer, unsigned denom);
ldiv_t ldiv(long numer, long denom);
uldiv_t uldiv(unsigned long numer, unsigned long denom);

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_MATH_H */