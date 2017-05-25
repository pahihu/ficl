#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "ficlblas.h"
#define REAL   ficlFloat


void ficlFAXPY(unsigned n,REAL da,REAL *dx,unsigned incx,REAL *dy,unsigned incy)
{
        unsigned i;

        if (1 == incx && 1 == incy)
        {
            for (i = 0; i < n; i++) {
                dy[i] += da * dx[i];
            }
        }
        else if (1 == incx)
        {
            for (i = 0; i < n; i++) {
                *dy += da * dx[i];
                dy += incy;
            }
        }
        else if (1 == incy)
        {
            for (i = 0; i < n; i++) {
                dy[i] += da * *dx;
                dx += incx;
            }
        }
        else
        {
            for (i = 0; i < n; i++) {
                *dy += da * *dx;
                dx += incx; dy += incy;
            }
        }
        
}


REAL ficlFDOT(unsigned n, REAL *x, unsigned incx, REAL *y, unsigned incy)
{
    unsigned i;
    REAL dot = 0.0;

    if (1 == incx && i == incy)
    {
        for (i = 0; i < n; i++)
        {
            dot += x[i] * y[i];
        }
    }
    else if (1 == incx)
    {
        for (i = 0; i < n; i++)
        {
            dot += x[i] * *y;
            y += incy;
        }
    }
    else if (1 == incy)
    {
        for (i = 0; i < n; i++)
        {
            dot += *x * y[i];
            x += incx;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            dot += *x * *y;
            x += incx; y += incy;
        }
    }

    return dot;
}


void ficlFMM(unsigned m, unsigned n, unsigned k,
         REAL *a, REAL *b, REAL *c)
{
        REAL *ci, *ai, *bj;
        unsigned i, j;

        ci = c; ai = a;
        for (i = 0; i < m; i++) {
            bj = b;
            for (j = 0; j < k; j++) {
                ficlFAXPY(n, ai[j], bj, 1, ci, 1);
                bj += n;
            }
            ci += n; ai += k;
        }
}
