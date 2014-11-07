/********************************************************************
   F A S T   F O U R I E R   T R A N S F O R M   P R O G R A M S

  by Wang Jian-Sheng 4 Nov 1998, added fft2D(), 11 Apr 2003 
---------------------------------------------------------------------

  Reference: "Computational Frameworks for the Fast Fourier 
              Transform", Charles Van Loan, SIAM, 1992.

  There are many FFT algorithms, the most important ones are
     COOLEY-TUKEY:  in place, bit reversal
     STOCKHAM AUTOSORT:  additional memory size of input data
     MIXED RADIX:  20% less operations comparing to Cooley-Tukey
     PRIME FACTOR: arbitrary length n

  We use a combination of the Stockham autosort algorithm 1.7.2, 
  page 57, and multirow Cooley-Tukey (3.1.7), page 124, of the 
  reference above.  

  The discrete Fourier transform is defined by
  y[k] = sum_(j=0,n-1) x[j] exp(-2 Pi sqrt[-1] j k/n), 
  k=0,1,...,n-1.  The factor (1/n) is not included.  
  If y[]<-x[]; fft(x,n,1); fft(x,n,-1); then y[]==x[]/n is true.
  Three dimensional transform is generalized straightforwardly.

   Interface and usage:
   1D Fourier transform 
   Use: fft(x, n, flag)
      x    : an array of structure type Complex;
      n    : the size of data, must be a power of 2;
      flag : 1 for forward transform, -1 for inverse transform.

   3D Fourier transform
   Use :  fft3D(x, n1, n2, n3, flag)
     x    : 1D array of type Complex representing 3D array; 
            mapping through C convention, i.e., 
            (i,j,k) -> k + n3*j + n2*n3*i;
     n1, n2, n3 : dimensions in three directions;
     flag : same as in 1D.

   2D FFT is similar but with n1 and n2 only.

**********************************************************************/

/* Data type and new names for flexibility:
  
    real:    Basic data type for floating point computations
             (typedef double  real;)
    Complex: Structure for Complex numbers, real and imaginary parts 
             are referred as c.Re, c.Im.
             (typedef struct { real Re; real Im; }  Complex;)
*/
/* Inclusion of standard C libraries */

#include "fft.h"
#include "complex.h"
#include "convention.h"

#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define REALSIZE 8 /* in units of byte */

/* Mathematical functions and constants */
#undef M_PI
#if (REALSIZE == 16)
#define sin sinl
#define cos cosl
#define fabs fabsl
#define M_PI 3.1415926535897932384626433832795L
#else
#define M_PI 3.1415926535897932385E0
#endif
using namespace fft;
/*----------------------------------------------------------------------*/
/* Truncated Stockham algorithm for multi-column vector,
       X(n1,n2) <- F_n1 X(n1,n2)
   x[] input data of size n, viewed as n1 = n/n2 by n2 dimensional 
   array, flag =+1 forward transform, -1 for backward transform, y[] is 
   working space, which must be n in size.  This function is supposed 
   to be internal (static), not used by application.  Note that the 
   terminology of column or row respect to algorithms in the Loan's 
   book is reversed, because we use row major convention of C.
*/
static void stockham(Complex x[], int n, int flag, int n2, Complex y[])
{
    Complex *y_orig, *tmp;
    int i, j, k, k2, Ls, r, jrs;
    int half, m, m2;
    real wr, wi, tr, ti;

    y_orig = y;
    r = half = n >> 1;
    Ls = 1; /* Ls=L/2 is the L star */

    while (r >= n2) { /* loops log2(n/n2) times */
        tmp = x;      /* swap pointers, y is always old */
        x = y;        /* x is always for new data */
        y = tmp;
        m = 0;     /* m runs over first half of the array */
        m2 = half; /* m2 for second half, n2=n/2 */
        for (j = 0; j < Ls; ++j) {
            wr = cos(M_PI * j / Ls);         /* real and imaginary part */
            wi = -flag * sin(M_PI * j / Ls); /* of the omega */
            jrs = j * (r + r);
            for (k = jrs; k < jrs + r; ++k) { /* "butterfly" operation */
                k2 = k + r;
                tr = wr * y[k2].Re - wi * y[k2].Im; /* Complex multiply, w*y */
                ti = wr * y[k2].Im + wi * y[k2].Re;
                x[m].Re = y[k].Re + tr;
                x[m].Im = y[k].Im + ti;
                x[m2].Re = y[k].Re - tr;
                x[m2].Im = y[k].Im - ti;
                ++m;
                ++m2;
            }
        }
        r >>= 1;
        Ls <<= 1;
    };

    if (y != y_orig) {            /* copy back to permanent memory */
        for (i = 0; i < n; ++i) { /* if it is not already there */
            y[i] = x[i];          /* performed only if log2(n/n2) is odd */
        }
    }

    assert(Ls == n / n2);      /* ensure n is a power of 2  */
    assert(1 == n || m2 == n); /* check array index within bound  */
}

/* The Cooley-Tukey multiple column algorithm, see page 124 of Loan.
   x[] is input data, overwritten by output, viewed as n/n2 by n2
   array. flag = 1 for forward and -1 for backward transform.
*/
void cooley_tukey(Complex x[], int n, int flag, int n2)
{
    Complex c;
    int i, j, k, m, p, n1;
    int Ls, ks, ms, jm, dk;
    real wr, wi, tr, ti;

    n1 = n / n2;               /* do bit reversal permutation */
    for (k = 0; k < n1; ++k) { /* This is algorithms 1.5.1 and 1.5.2. */
        j = 0;
        m = k;
        p = 1; /* p = 2^q,  q used in the book */
        while (p < n1) {
            j = 2 * j + (m & 1);
            m >>= 1;
            p <<= 1;
        }
        assert(p == n1); /* make sure n1 is a power of two */
        if (j > k) {
            for (i = 0; i < n2; ++i) { /* swap k <-> j row */
                c = x[k * n2 + i];     /* for all columns */
                x[k * n2 + i] = x[j * n2 + i];
                x[j * n2 + i] = c;
            }
        }
    }
    /* This is (3.1.7), page 124 */
    p = 1;
    while (p < n1) {
        Ls = p;
        p <<= 1;
        jm = 0; /* jm is j*n2 */
        dk = p * n2;
        for (j = 0; j < Ls; ++j) {
            wr = cos(M_PI * j / Ls);         /* real and imaginary part */
            wi = -flag * sin(M_PI * j / Ls); /* of the omega */
            for (k = jm; k < n; k += dk) {   /* "butterfly" */
                ks = k + Ls * n2;
                for (i = 0; i < n2; ++i) { /* for each row */
                    m = k + i;
                    ms = ks + i;
                    tr = wr * x[ms].Re - wi * x[ms].Im;
                    ti = wr * x[ms].Im + wi * x[ms].Re;
                    x[ms].Re = x[m].Re - tr;
                    x[ms].Im = x[m].Im - ti;
                    x[m].Re += tr;
                    x[m].Im += ti;
                }
            }
            jm += n2;
        }
    }
}

void NormalizeArray(Complex array[], int num, int flag)
{
    if (flag == 1)
        return;
    for (int i = 0; i < num; i++)
        *(array + i) /= real(num);
}

int GetFlag(Dir direction)
{
    return (direction == fft::FORTH) ? 1 : -1;
}

/* 1D Fourier transform: 
   Simply call stockham with proper arguments.  
   Allocated working space of size n dynamically.
*/
void fft::fft(Complex x[], int n, Dir direction)
{
    Complex *y;

    int flag = GetFlag(direction);
    y = (Complex *)malloc(n * sizeof(Complex));
    assert(NULL != y);
    stockham(x, n, flag, 1, y);
    NormalizeArray(x, n, flag);
    free(y);
}

/* 3D Fourier transform:  
   The index for x[m] is mapped to (i,j,k) by
   m = k + n3*j + n3*n2*i, i.e. the row major convention of C.
   All indices start from 0.
   This algorithm requires working space of n2*n3.
   Stockham is efficient, good stride feature, but takes extra
   memory same size as input data; Cooley-Tukey is in place,
   so we take a compromise of the two.
*/

void fft::fft2D(Complex x[], int n1, int n2, Dir direction)
{
    Complex *y;
    int i, n;

    int flag = GetFlag(direction);
    n = n1 * n2;
    y = (Complex *)malloc(n2 * sizeof(Complex));
    assert(NULL != y);

    for (i = 0; i < n; i += n2) { /* FFT in y */
        stockham(x + i, n2, flag, 1, y);
    }
    free(y);
    cooley_tukey(x, n, flag, n2); /* FFT in x */
    NormalizeArray(x, n, flag);
}

/**
*  careful, fft3D will use an array of n2*n3 size in heap as a cache
*/
void fft::fft3D(Complex x[], int n1, int n2, int n3, Dir direction)
{
    static Complex *y = NULL;
    static int cn23 = 1;
    int i, n, n23;

    int flag = GetFlag(direction);
    n23 = n2 * n3;
    n = n1 * n23;

    if (cn23 != n23) {
        if (y != NULL)
            free(y);
        y = (Complex *)malloc(n23 * sizeof(Complex));
        cn23 = n23;
    }
    assert(NULL != y);

    for (i = 0; i < n; i += n3) { /* FFT in z */
        stockham(x + i, n3, flag, 1, y);
    }
    for (i = 0; i < n; i += n23) { /* FFT in y */
        stockham(x + i, n23, flag, n3, y);
    }
    cooley_tukey(x, n, flag, n23); /* FFT in x */
    NormalizeArray(x, n, flag);
    cn23 = n23;
}
/**
*  careful, fft4D will use an array of n2*n3*n4 size in heap as a cache
*/
void fft::fft4D(Complex *x, int n1, int n2, int n3, int n4, Dir direction, bool *Mask)
{
    static Complex *y = NULL;
    static int cn234 = 1;
    int i, n, n34, n234;

    int flag = GetFlag(direction);
    n34 = n3 * n4;
    n234 = n2 * n34;
    n = n1 * n234;

    if (cn234 != n234) {
        if (y != NULL)
            free(y);
        y = (Complex *)malloc(n234 * sizeof(Complex));
    }
    assert(NULL != y);

    for (i = 0; i < n; i += n4) { /* FFT in t */
        stockham(x + i, n4, flag, 1, y);
    }
    for (i = 0; i < n; i += n34) { /* FFT in z */
        stockham(x + i, n34, flag, n4, y);
    }
    for (i = 0; i < n; i += n234) { /* FFT in y */
        stockham(x + i, n234, flag, n34, y);
    }
    cooley_tukey(x, n, flag, n234); /* FFT in x */
    NormalizeArray(x, n, flag);
    cn234 = n234;
}

void fft::fft(Complex *x, int *size, int dim, Dir direction, bool *Mask)
{
    static Complex *y = NULL;
    static size_t LastMemorySize = 1;
    int i, n, n34, n234;
    size_t CurrentMemorySize = 1;
    size_t ArraySize[dim - 1];

    int flag = GetFlag(direction);

    //    n4
    //    n34 = n3 * n4;
    //    n234 = n2 * n34;
    //    n1234 = n1 * n234;

    for (int i = dim-1; i >=0; i--)
    {
        CurrentMemorySize *= size[i];
        ArraySize[i]=CurrentMemorySize;
    }
    ArraySize[0] = CurrentMemorySize * size[0];

    if (CurrentMemorySize != LastMemorySize) {
        if (y != NULL)
            free(y);
        y = (Complex *)malloc(n234 * sizeof(Complex));
    }
    assert(NULL != y);

    for (i = 0; i < n; i += n4) { /* FFT in t */
        stockham(x + i, n4, flag, 1, y);
    }
    for (i = 0; i < n; i += n34) { /* FFT in z */
        stockham(x + i, n34, flag, n4, y);
    }
    for (i = 0; i < n; i += n234) { /* FFT in y */
        stockham(x + i, n234, flag, n34, y);
    }
    cooley_tukey(x, n, flag, n234); /* FFT in x */
    NormalizeArray(x, n, flag);
    cn234 = n234;
    //    if (dim == 1)
    //        fft(x, size[0], direction, Mask);
    //    else if (dim == 2)
    //        fft2D(x, size[0], size[1], direction, Mask);
    //    else if (dim == 3)
    //        fft3D(x, size[0], size[1], size[2], direction, Mask);
    //    else if (dim == 4)
    //        fft4D(x, size[0], size[1], size[2], size[3], direction, Mask);
    //    else
    //        assert(true);
}
