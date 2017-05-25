#ifndef _FICLBLAS_H
#define _FICLBLAS_H

#include "ficl.h"


void ficlFAXPY(unsigned n, ficlFloat da, ficlFloat *dx, unsigned incx, ficlFloat *dy, unsigned incy);
ficlFloat ficlFDOT(unsigned n, ficlFloat *x, unsigned incx, ficlFloat *y, unsigned incy);
void ficlFMM(unsigned m, unsigned n, unsigned k, ficlFloat *a, ficlFloat *b, ficlFloat *c);


#endif
