/*******************************************************************
** f l o a t . c
** Forth Inspired Command Language
** ANS Forth FLOAT word-set written in C
** Author: Guy Carver & John Sadler (john_sadler@alum.mit.edu)
** Created: Apr 2001
** $Id: //depot/gamejones/ficl/float.c#15 $
*******************************************************************/
/*
** Copyright (c) 1997-2001 John Sadler (john_sadler@alum.mit.edu)
** All rights reserved.
**
** Get the latest Ficl release at http://ficl.sourceforge.net
**
** I am interested in hearing from anyone who uses Ficl. If you have
** a problem, a success story, a defect, an enhancement request, or
** if you would like to contribute to the Ficl release, please
** contact me by email at the address above.
**
** L I C E N S E  and  D I S C L A I M E R
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

#define _ISOC99_SOURCE  1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "ficl.h"

#if FICL_WANT_FLOAT

#include "ficlblas.h"

#define FICL_FLOAT_ALIGNMENT    sizeof(ficlFloat)

/**************************************************************************
                        f a l i g n P t r
** Aligns the given pointer to sizeof(ficlFloat) address units.
** Returns the aligned pointer value.
**************************************************************************/
void *ficlFAlignPointer(void *ptr)
{
	ficlInteger p = (ficlInteger)ptr;
	if (p & (FICL_FLOAT_ALIGNMENT - 1))
		ptr = (void *)((p & ~(FICL_FLOAT_ALIGNMENT - 1)) + FICL_FLOAT_ALIGNMENT);
    return ptr;
}


/**************************************************************************
                        d i c t F A l i g n
** Align the dictionary's free space pointer to float.
**************************************************************************/
static void ficlDictionaryFAlign(ficlDictionary *dictionary)
{
    dictionary->here = ficlFAlignPointer(dictionary->here);
}


/**************************************************************************
                        f a l i g n
** falign ( -- )
** If the data-space pointer is not float aligned, reserve enough space to
** make it so.
**************************************************************************/
static void ficlPrimitiveFAlign(ficlVm *vm)
{
    ficlDictionary *dictionary = ficlVmGetDictionary(vm);
    FICL_IGNORE(vm);
    ficlDictionaryFAlign(dictionary);
}


/**************************************************************************
                        f a l i g n e d
** faligned ( addr -- f-addr )
** f-addr is the first float-aligned address greater than or equal to addr.
**************************************************************************/
static void ficlPrimitiveFAligned(ficlVm *vm)
{
    void *addr;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    addr = ficlStackPopPointer(vm->dataStack);
    ficlStackPushPointer(vm->dataStack, ficlFAlignPointer(addr));
}

/*******************************************************************
** Create a floating point constant.
** fconstant ( r -"name"- )
*******************************************************************/
static void ficlPrimitiveFConstant(ficlVm *vm)
{
    ficlDictionary *dictionary = ficlVmGetDictionary(vm);
    ficlString name = ficlVmGetWord(vm);

    FICL_STACK_CHECK(vm->floatStack, 1, 0);

    ficlDictionaryAppendWord(dictionary, name, (ficlPrimitive)ficlInstructionFConstantParen, FICL_WORD_DEFAULT);
    ficlDictionaryAppendCell(dictionary, ficlStackPop(vm->floatStack));
}


ficlWord   *ficlDictionaryAppendFConstant(ficlDictionary *dictionary, char *name, float value)
{
	ficlString s;
	FICL_STRING_SET_FROM_CSTRING(s, name);
	return ficlDictionaryAppendConstantInstruction(dictionary, s, ficlInstructionFConstantParen, *(ficlInteger *)(&value));
}


ficlWord   *ficlDictionarySetFConstant(ficlDictionary *dictionary, char *name, float value)
{
    ficlString s;
    FICL_STRING_SET_FROM_CSTRING(s, name);
    return ficlDictionarySetConstantInstruction(dictionary, s, ficlInstructionFConstantParen, *(ficlInteger *)(&value));
}




static void ficlPrimitiveF2Constant(ficlVm *vm)
{
    ficlDictionary *dictionary = ficlVmGetDictionary(vm);
    ficlString name = ficlVmGetWord(vm);

    FICL_STACK_CHECK(vm->floatStack, 2, 0);

    ficlDictionaryAppendWord(dictionary, name, (ficlPrimitive)ficlInstructionF2ConstantParen, FICL_WORD_DEFAULT);
    ficlDictionaryAppendCell(dictionary, ficlStackPop(vm->floatStack));
    ficlDictionaryAppendCell(dictionary, ficlStackPop(vm->floatStack));
}


ficlWord   *ficlDictionaryAppendF2Constant(ficlDictionary *dictionary, char *name, double value)
{
    ficlString s;
    FICL_STRING_SET_FROM_CSTRING(s, name);
    return ficlDictionaryAppend2ConstantInstruction(dictionary, s, ficlInstructionF2ConstantParen, *(ficl2Integer *)(&value));
}


ficlWord   *ficlDictionarySetF2Constant(ficlDictionary *dictionary, char *name, double value)
{
    ficlString s;
    FICL_STRING_SET_FROM_CSTRING(s, name);
    return ficlDictionarySet2ConstantInstruction(dictionary, s, ficlInstructionF2ConstantParen, *(ficl2Integer *)(&value));
}

static ficlFloat ficlRepresentPriv(ficlFloat r, int precision, int *out_sign, double *out_mant, int *out_exp)
{
    int expof10, sign;
    double value, fract, display;

    sign = 0;

    value = (double) r;
    if (value < 0.0)
    {
        sign = 1;
        value = -value;
    }
    if (out_sign)
        *out_sign = sign;

    expof10 = lrint(floor(log10(value)));
    if (out_exp)
        *out_exp = expof10;

    value   *= pow(10.0, precision - 1 - expof10);
    fract    = modf(value, &display);
    if (fract >= 0.5)
    {
        int ndigits = lrint(floor(log10(display)));
        display += 1.0;
        if (ndigits != lrint(floor(log10(display))))
            *out_exp += 1;
    }
    if (out_mant)
        *out_mant = display;

    value = display * pow(10.0, expof10 - precision + 1.0);

    return (ficlFloat) value;
}

/*******************************************************************
** Place at c-addr the extern representation of the u most
** most significant digits of r.
** represent ( c-addr u -- n flag1 flag2 ) ( F: r -- )
*******************************************************************/
static void ficlPrimitiveRepresent(ficlVm *vm)
{
    ficlFloat r;
    double rDisp;
    int rSign, rExp, bufLen;
    char tmp[32];
    char *cAddr, *buffer;
    ficlUnsigned u, valid;

    FICL_STACK_CHECK(vm->floatStack, 1, 0);
    FICL_STACK_CHECK(vm->dataStack, 2, 3);


    r = ficlStackPopFloat(vm->floatStack);
    u = ficlStackPopUnsigned(vm->dataStack);
    cAddr = ficlStackPopPointer(vm->dataStack);

    valid = FICL_TRUE;
    rSign = 0;
    buffer = &tmp[0];
    // limit range of significant digits
    if (u < 1)
        u = 1;
    else if (u > FICL_FLOAT_PRECISION)
        u = FICL_FLOAT_PRECISION;

    switch (fpclassify(r))
    {
        case FP_INFINITE:
            buffer = r < 0 ? "-inf" : "inf";
            rSign  = r < 0 ? 1 : 0; 
            valid  = FICL_FALSE; 
            break;
        case FP_NAN:
            buffer = "nan";
            rSign  = copysign(1.0, r) < 0 ? 1 : 0;
            valid  = FICL_FALSE;
            break;
        case FP_ZERO:
            sprintf(tmp, "%0*u", (int) u, 0);
            rSign = copysign(1.0, r) < 0 ? 1 : 0;
            rExp  = 1;
            break;
        default:
            ficlRepresentPriv(r, u, &rSign, &rDisp, &rExp);
            u = (ficlUnsigned) rDisp;
            sprintf(tmp, "%lu", u);
            rExp += 1;
            break;
    }
    // copy buffer constents to output
    bufLen = strlen(buffer);
    strncpy(cAddr, buffer, bufLen);
    // implementation defined: if not a valid FP number, 
    // the returned exponent will be the length of the stored
    // string
    if (!valid)
        rExp = bufLen;
    ficlStackPushInteger(vm->dataStack, rExp);
    ficlStackPushInteger(vm->dataStack, rSign ? FICL_TRUE : FICL_FALSE);
    ficlStackPushUnsigned(vm->dataStack, valid);
}

/*******************************************************************
** Display a float in decimal format.
** f. ( r -- )
*******************************************************************/
static void ficlPrimitiveFDot(ficlVm *vm)
{
    ficlFloat r;
    double rDisp;
    int i, prec, len, rSign, rExp;
    char *pHold;
    char tmp[32];

    FICL_STACK_CHECK(vm->floatStack, 1, 0);

    len  = 0;
    prec = vm->precision;
    r = ficlStackPopFloat(vm->floatStack);
    switch (fpclassify(r))
    {
        case FP_INFINITE:
            sprintf(vm->pad, r < 0 ? "-inf " : "inf ");
            break;
        case FP_NAN:
            sprintf(vm->pad, "nan ");
            break;
        case FP_ZERO:
            if (copysign(1.0, r) < 0)
              vm->pad[len++] = '-';
            sprintf(vm->pad + len, "0. ");
            break;
        default:
            r = ficlRepresentPriv(r, prec, &rSign, &rDisp, &rExp);
            pHold = vm->pad;
            if (rSign)
                *pHold++ = '-';
            sprintf(tmp, "%ld", (ficlUnsigned) rDisp);
            if (rExp + 1 > prec)
            {
                for (i = 0; i < prec; i++)
                    *pHold++ = tmp[i];
                for (i = prec; i < rExp + 1; i++)
                    *pHold++ = '0';
                *pHold++ = '.';
            }
            else
            {
                if (rExp < 0)
                {
                    *pHold++ = '0'; *pHold++ = '.';
                    for (i = rExp + 1; i < 0; i++)
                        *pHold++ = '0';
                    for (i = 0; i < prec; i++)
                        *pHold++ = tmp[i];
                }
                else
                {
                    for (i = 0; i < rExp + 1; i++)
                        *pHold++ = tmp[i];
                    *pHold++ = '.';
                    for (; i < prec; i++)
                        *pHold++ = tmp[i];
                }
            }
            // zero supression
            len = pHold - vm->pad - 1;
            while ('0' == vm->pad[len]) len--;
            vm->pad[++len] = ' ';
            vm->pad[++len] = '\0';
    }
    ficlVmTextOut(vm, vm->pad);
}

/*******************************************************************
** Display a float in scientific format.
** fs. ( r -- )
*******************************************************************/
static void ficlPrimitiveSDot(ficlVm *vm)
{
    ficlFloat r;
    double rDisp;
    int prec, len, rSign, rExp;

    FICL_STACK_CHECK(vm->floatStack, 1, 0);

    len  = 0;
    prec = vm->precision;
    r    = ficlStackPopFloat(vm->floatStack);
    switch (fpclassify(r))
    {
        case FP_INFINITE:
            sprintf(vm->pad, r < 0 ? "-inf " : "inf ");
            break;
        case FP_NAN:
            sprintf(vm->pad, "nan ");
            break;
        case FP_ZERO:
            if (copysign(1.0, r) < 0)
                vm->pad[len++] = '-';
            sprintf(vm->pad + len, "%.*fE0 ", prec - 1, 0.0);
            break;
        default:
            r = ficlRepresentPriv(r, prec, &rSign, &rDisp, &rExp);
            if (rSign)
                vm->pad[len++] = '-';
            // 1 '.' prec - 1
            rDisp *= pow(10.0, 1 - prec);
            sprintf(vm->pad + len, "%.*fE%d ", prec - 1, rDisp, rExp);
    }
    ficlVmTextOut(vm, vm->pad);
}

/**************************************************************************
                        d i s p l a y FS t a c k
** Display the parameter stack (code for "f.s")
** f.s ( -- )
**************************************************************************/
struct stackContext
{
    ficlVm *vm;
    int count;
};

static ficlInteger ficlFloatStackDisplayCallback(void *c, ficlCell *cell)
{
    struct stackContext *context = (struct stackContext *)c;
    char buffer[64];
    sprintf(buffer, "[0x%p %3d] %16f (0x%08lx)\n", cell, context->count++, (double)(cell->f), cell->i);
    ficlVmTextOut(context->vm, buffer);
	return FICL_TRUE;
}



void ficlVmDisplayFloatStack(ficlVm *vm)
{
    struct stackContext context;
	context.vm = vm;
	context.count = 0;
    ficlStackDisplay(vm->floatStack, ficlFloatStackDisplayCallback, &context);
    return;
}



/*******************************************************************
** Do float stack depth.
** fdepth ( -- n )
*******************************************************************/
static void ficlPrimitiveFDepth(ficlVm *vm)
{
    int i;

    FICL_STACK_CHECK(vm->dataStack, 0, 1);

    i = ficlStackDepth(vm->floatStack);
    ficlStackPushInteger(vm->dataStack, i);
}

/*******************************************************************
** Compile a floating point literal.
*******************************************************************/
static void ficlPrimitiveFLiteralImmediate(ficlVm *vm)
{
    ficlDictionary *dictionary = ficlVmGetDictionary(vm);
	ficlCell cell;


    FICL_STACK_CHECK(vm->floatStack, 1, 0);


	cell = ficlStackPop(vm->floatStack);
	if (cell.f == 1.0f)
	{
		ficlDictionaryAppendUnsigned(dictionary, ficlInstructionF1);
	}
	else if (cell.f == 0.0f)
	{
		ficlDictionaryAppendUnsigned(dictionary, ficlInstructionF0);
	}
	else if (cell.f == -1.0f)
	{
		ficlDictionaryAppendUnsigned(dictionary, ficlInstructionFNeg1);
	}
	else
	{
		ficlDictionaryAppendUnsigned(dictionary, ficlInstructionFLiteralParen);
		ficlDictionaryAppendCell(dictionary, cell);
	}
}

/*******************************************************************
** Floating square root.
** fsqrt ( r1 -- r2 )
*******************************************************************/
static void ficlPrimitiveFSqrt(ficlVm *vm)
{
    ficlFloat r;

    FICL_STACK_CHECK(vm->floatStack, 1, 1);

    r = ficlStackPopFloat(vm->floatStack);
    ficlStackPushFloat(vm->floatStack, (ficlFloat) sqrt(r));
}

/*******************************************************************
** FAXPY
** faxpy ( f_x incx f_y incy u -- ) ( F: ra -- )
*******************************************************************/
static void ficlPrimitiveFaxpy(ficlVm *vm)
{
    ficlFloat ra;
    ficlFloat *x, *y;
    ficlUnsigned incx, incy;
    ficlUnsigned n;

    FICL_STACK_CHECK(vm->floatStack, 1, 0);
    FICL_STACK_CHECK(vm->dataStack, 5, 0);

    ra = ficlStackPopFloat(vm->floatStack);

    n   = ficlStackPopUnsigned(vm->dataStack);
    incy = ficlStackPopUnsigned(vm->dataStack);
    y    = ficlStackPopPointer(vm->dataStack);
    incx = ficlStackPopUnsigned(vm->dataStack);
    x    = ficlStackPopPointer(vm->dataStack);

    ficlFAXPY(n, ra, x, incx, y, incy);
}

/*******************************************************************
** FDOT
** fdot ( f_x incx f_y incy u -- ) ( F: -- r )
*******************************************************************/
static void ficlPrimitiveFdot(ficlVm *vm)
{
    ficlFloat r;
    ficlFloat *x, *y;
    ficlUnsigned incx, incy;
    ficlUnsigned n;

    FICL_STACK_CHECK(vm->dataStack, 5, 0);
    FICL_STACK_CHECK(vm->floatStack, 0, 1);

    n    = ficlStackPopUnsigned(vm->dataStack);
    incy = ficlStackPopUnsigned(vm->dataStack);
    y    = ficlStackPopPointer(vm->dataStack);
    incx = ficlStackPopUnsigned(vm->dataStack);
    x    = ficlStackPopPointer(vm->dataStack);

    r = ficlFDOT(n, x, incx, y, incy);
    ficlStackPushFloat(vm->floatStack, r);
}

/*******************************************************************
** FMM
** fmm ( f_a f_b f_c m k n -- )
** C = A x B, C is (M x N), A is (M x K), B is (K x N) matrices.
*******************************************************************/
static void ficlPrimitiveFmm(ficlVm *vm)
{
    ficlFloat *a, *b, *c;
    ficlUnsigned m, k, n;

    FICL_STACK_CHECK(vm->dataStack, 6, 0);

    n  = ficlStackPopUnsigned(vm->dataStack);
    k  = ficlStackPopUnsigned(vm->dataStack);
    m  = ficlStackPopUnsigned(vm->dataStack);
    c  = ficlStackPopPointer(vm->dataStack);
    b  = ficlStackPopPointer(vm->dataStack);
    a  = ficlStackPopPointer(vm->dataStack);

    ficlFMM(m, n, k, a, b, c);
}

/*******************************************************************
** float+ ( f-addr1 -- f-addr2 )
*******************************************************************/
static void ficlPrimitiveFloatPlus(ficlVm *vm)
{
    ficlFloat *fp;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    fp = ficlStackPopPointer(vm->dataStack);
    ficlStackPushPointer(vm->dataStack, fp + 1);
}

/*******************************************************************
** floats ( n1 -- n2 )
*******************************************************************/
static void ficlPrimitiveFloats(ficlVm *vm)
{
    ficlInteger n;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    n = ficlStackPopInteger(vm->dataStack);
    ficlStackPushInteger(vm->dataStack, n * sizeof(ficlFloat));
}

/*******************************************************************
** fmin ( r1 r2 -- r3 )
*******************************************************************/
static void ficlPrimitiveFMin(ficlVm *vm)
{
    ficlFloat r1, r2;

    FICL_STACK_CHECK(vm->floatStack, 2, 1);

    r2 = ficlStackPopFloat(vm->floatStack);
    r1 = ficlStackPopFloat(vm->floatStack);
    ficlStackPushFloat(vm->floatStack, r1 < r2 ? r1 : r2);
}

/*******************************************************************
** fmax ( r1 r2 -- r3 )
*******************************************************************/
static void ficlPrimitiveFMax(ficlVm *vm)
{
    ficlFloat r1, r2;

    FICL_STACK_CHECK(vm->floatStack, 2, 1);

    r2 = ficlStackPopFloat(vm->floatStack);
    r1 = ficlStackPopFloat(vm->floatStack);
    ficlStackPushFloat(vm->floatStack, r1 < r2 ? r2 : r1);
}

/*******************************************************************
** floor ( r1 -- r2 )
*******************************************************************/
static void ficlPrimitiveFloor(ficlVm *vm)
{
    ficlFloat r1;

    FICL_STACK_CHECK(vm->floatStack, 1, 1);

    r1 = ficlStackPopFloat(vm->floatStack);
    ficlStackPushFloat(vm->floatStack, floor(r1));
}

/*******************************************************************
** d>f ( d -- ) ( F: -- r )
*******************************************************************/
static void ficlPrimitiveDToF(ficlVm *vm)
{
    ficl2Integer d;
    ficl2Unsigned ud;
    ficlFloat r;
    int i, sign;

    FICL_STACK_CHECK(vm->dataStack, 2, 0);
    FICL_STACK_CHECK(vm->floatStack, 0, 1);

    d = ficlStackPop2Integer(vm->dataStack);

#if FICL_PLATFORM_HAS_2INTEGER
    r = (ficlFloat) d;
#else
    sign = 0;
    if (ficl2IntegerIsNegative(d))
    {
        sign = 1;
        d = ficl2IntegerNegate(d);
    }
    ud = FICL_2INTEGER_TO_2UNSIGNED(d);

    r = (ficlFloat) FICL_2UNSIGNED_GET_HIGH(ud);
    r = ldexp(r, 8 * sizeof(ficlUnsigned));
    r += (ficlFloat) FICL_2UNSIGNED_GET_LOW(ud);

    if (sign)
        r = -r;
#endif
    ficlStackPushFloat(vm->floatStack, r);
}

#define FICL_UNSIGNED_MSB   ((ficlUnsigned)1 << (8*sizeof(ficlUnsigned) - 1))
#define FICL_MAX_INTEGER    (FICL_UNSIGNED_MSB - 1)

static double r2IntegerMax;;

/*******************************************************************
** f>d ( -- d ) ( F: r -- )
*******************************************************************/
static void ficlPrimitiveFToD(ficlVm *vm)
{
    ficlFloat r;
    double frac, ipart;
    ficl2Integer d;
#if FICL_PLATFORM_HAS_2INTEGER
#else
    int sign;
    ficlUnsigned high, low;
    ficl2Unsigned ud;
    double rUnsignedMax;
#endif

    FICL_STACK_CHECK(vm->floatStack, 1, 0);
    FICL_STACK_CHECK(vm->dataStack, 0, 2);

    r = ficlStackPopFloat(vm->floatStack);
    frac = modf(r, &ipart);
#if FICL_PLATFORM_HAS_2INTEGER
    d = (ficl2Integer) ipart;
#else
    sign = 0;
    if (ipart < 0.0)
    {
        sign = 1;
        ipart = -ipart;
    }
    if (ipart > r2IntegerMax)
    {
        low  = 0;
        high = FICL_UNSIGNED_MSB;
        sign = 0;
    }
    else
    {
        rUnsignedMax = (double) (~ (ficlUnsigned) 0);
        low  = remainder(ipart, rUnsignedMax);
        high = ipart / rUnsignedMax;
        if (high > (ficlUnsigned) FICL_MAX_INTEGER)
        {
            low  = 0;
            high = FICL_UNSIGNED_MSB;
            sign = 0;
        }
    }

    FICL_2UNSIGNED_SET(high, low, ud);
    d = FICL_2UNSIGNED_TO_2INTEGER(ud);
    if (sign)
        d = ficl2IntegerNegate(d);

#endif
    ficlStackPush2Integer(vm->dataStack, d);
}

/*******************************************************************
** fround ( r1 -- r2 )
*******************************************************************/
static void ficlPrimitiveFRound(ficlVm *vm)
{
    ficlFloat r1;

    FICL_STACK_CHECK(vm->floatStack, 1, 1);

    r1 = ficlStackPopFloat(vm->floatStack);
    ficlStackPushFloat(vm->floatStack, rint(r1));
}

/*******************************************************************
** f** ( r1 r2 -- r3 )
*******************************************************************/
static void ficlPrimitiveFStarStar(ficlVm *vm)
{
    ficlFloat r1, r2;

    FICL_STACK_CHECK(vm->floatStack, 2, 1);

    r2 = ficlStackPopFloat(vm->floatStack);
    r1 = ficlStackPopFloat(vm->floatStack);
    ficlStackPushFloat(vm->floatStack, pow(r1, r2));
}

/*******************************************************************
** fabs ( r1 -- r2 )
*******************************************************************/
static void ficlPrimitiveFAbs(ficlVm *vm)
{
    ficlFloat r1;

    FICL_STACK_CHECK(vm->floatStack, 1, 1);

    r1 = ficlStackPopFloat(vm->floatStack);
    ficlStackPushFloat(vm->floatStack, fabs(r1));
}

/*******************************************************************
** Display a float in engineering format.
** fe. ( r -- )
*******************************************************************/
extern char* eng(double, int, int);
static void ficlPrimitiveEDot(ficlVm *vm)
{
    ficlFloat r;
    double rDisp;
    int len, prec, rSign, rExp, eng, scale;
        
    FICL_STACK_CHECK(vm->floatStack, 1, 0);

    len  = 0;
    prec = vm->precision;
    r    = ficlStackPopFloat(vm->floatStack);
    switch (fpclassify(r))
    {
        case FP_INFINITE:
            sprintf(vm->pad, r < 0 ? "-inf " : "inf ");
            break;
        case FP_NAN:
            sprintf(vm->pad, "nan ");
            break;
        case FP_ZERO:
            if (copysign(1.0, r) < 0)
                vm->pad[len++] = '-';
            sprintf(vm->pad + len, "%.*fE0 ", prec - 1, 0.0);
            break;
        default:
            r = ficlRepresentPriv(r, prec, &rSign, &rDisp, &rExp);
            if (rExp < 0)
            {
                eng = -((3 - rExp) / 3) * 3;
                scale = rExp - eng;
            }
            else
            {
                eng = (rExp / 3) * 3;
                scale = rExp - eng;
            }
            // 1 + scale '.' prec - 1 - scale
            rDisp *= pow(10.0, 1 + scale - prec);
            if (rSign)
                vm->pad[len++] = '-';
            sprintf(vm->pad + len, "%.*fE%d", prec - 1 - scale, rDisp, eng);
    }
    ficlVmTextOut(vm, vm->pad);
}

/*******************************************************************
** Return number of significant digits used by F. FE. FS.
** precision ( -- u )
*******************************************************************/
static void ficlPrimitivePrecision(ficlVm *vm)
{
    FICL_STACK_CHECK(vm->dataStack, 0, 1);

    ficlStackPushUnsigned(vm->dataStack, vm->precision);
}

/*******************************************************************
** Set the number of significant digits used by F. FE. FS.
** set-precision ( u -- )
*******************************************************************/
static void ficlPrimitiveSetPrecision(ficlVm *vm)
{
    ficlUnsigned prec;
    FICL_STACK_CHECK(vm->dataStack, 1, 0);

    prec = ficlStackPopUnsigned(vm->dataStack);
    if (prec < 1)
        prec = 1;
    else if (prec > FICL_FLOAT_PRECISION)
        prec = FICL_FLOAT_PRECISION;
    vm->precision = prec;
}

/*******************************************************************
** f~ ( F: r1 r2 r3 -- r3 ) ( -- flag )
*******************************************************************/
static void ficlPrimitiveFProximate(ficlVm *vm)
{
    union {
        ficlFloat f;
        ficlUnsigned u;
    } d1, d2;
    ficlUnsigned flag = FICL_FALSE;
    ficlFloat r1, r2, r3;

    FICL_STACK_CHECK(vm->floatStack, 3, 0);
    FICL_STACK_CHECK(vm->dataStack, 0, 1);

    r3 = ficlStackPopFloat(vm->floatStack);
    r2 = ficlStackPopFloat(vm->floatStack);
    r1 = ficlStackPopFloat(vm->floatStack);
    if (r3 > 0.0)
    {
        if (fabs(r1 - r2) < r3)
            flag = FICL_TRUE;
    }
    else if (r3 < 0.0)
    {
        if (fabs(r1 - r2) < fabs(r3 * (fabs(r1) + fabs(r2))))
            flag = FICL_TRUE;
    }
    else
    {
        d1.f = r1;
        d2.f = r2;
        if (d1.u == d2.u)
            flag = FICL_TRUE;
    }
    ficlStackPushUnsigned(vm->dataStack, flag);
}

/**************************************************************************
                     F l o a t P a r s e S t a t e
** Enum to determine the current segement of a floating point number
** being parsed.
**************************************************************************/
#define NUMISNEG 1
#define EXPISNEG 2

typedef enum _floatParseState
{
    FPS_START,
    FPS_ININT,
    FPS_INMANT,
    FPS_STARTEXP,
    FPS_INEXP
} FloatParseState;

/**************************************************************************
                     f i c l P a r s e F l o a t N u m b e r
** vm -- Virtual Machine pointer.
** s -- String to parse.
** Returns 1 if successful, 0 if not.
**************************************************************************/
int ficlVmParseFloatNumber( ficlVm *vm, ficlString s)
{
    unsigned char c;
	unsigned char digit;
    char *trace;
    ficlUnsigned length;
    float power;
    float accum = 0.0f;
    float mant = 0.1f;
    ficlInteger exponent = 0;
    char flag = 0;
    FloatParseState estate = FPS_START;


    FICL_STACK_CHECK(vm->floatStack, 0, 1);

	
    /*
    ** floating point numbers only allowed in base 10 
    */
    if (vm->base != 10)
        return(0);


    trace = FICL_STRING_GET_POINTER(s);
    length = FICL_STRING_GET_LENGTH(s);

    /* Loop through the string's characters. */
    while ((length--) && ((c = *trace++) != 0))
    {
        switch (estate)
        {
            /* At start of the number so look for a sign. */
            case FPS_START:
            {
                estate = FPS_ININT;
                if (c == '-')
                {
                    flag |= NUMISNEG;
                    break;
                }
                if (c == '+')
                {
                    break;
                }
            } /* Note!  Drop through to FPS_ININT */
            /*
            **Converting integer part of number.
            ** Only allow digits, decimal and 'E'. 
            */
            case FPS_ININT:
            {
                if (c == '.')
                {
                    estate = FPS_INMANT;
                }
                else if ((c == 'e') || (c == 'E'))
                {
                    estate = FPS_STARTEXP;
                }
                else
                {
                    digit = (unsigned char)(c - '0');
                    if (digit > 9)
                        return(0);

                    accum = accum * 10 + digit;

                }
                break;
            }
            /*
            ** Processing the fraction part of number.
            ** Only allow digits and 'E' 
            */
            case FPS_INMANT:
            {
                if ((c == 'e') || (c == 'E'))
                {
                    estate = FPS_STARTEXP;
                }
                else
                {
                    digit = (unsigned char)(c - '0');
                    if (digit > 9)
                        return(0);

                    accum += digit * mant;
                    mant *= 0.1f;
                }
                break;
            }
            /* Start processing the exponent part of number. */
            /* Look for sign. */
            case FPS_STARTEXP:
            {
                estate = FPS_INEXP;

                if (c == '-')
                {
                    flag |= EXPISNEG;
                    break;
                }
                else if (c == '+')
                {
                    break;
                }
            }       /* Note!  Drop through to FPS_INEXP */
            /*
            ** Processing the exponent part of number.
            ** Only allow digits. 
            */
            case FPS_INEXP:
            {
                digit = (unsigned char)(c - '0');
                if (digit > 9)
                    return(0);

                exponent = exponent * 10 + digit;

                break;
            }
        }
    }

    /* If parser never made it to the exponent this is not a float. */
    if (estate < FPS_STARTEXP)
        return(0);

    /* Set the sign of the number. */
    if (flag & NUMISNEG)
        accum = -accum;

    /* If exponent is not 0 then adjust number by it. */
    if (exponent != 0)
    {
        /* Determine if exponent is negative. */
        if (flag & EXPISNEG)
        {
            exponent = -exponent;
        }
        /* power = 10^x */
        power = (float)pow(10.0, exponent);
        accum *= power;
    }

    ficlStackPushFloat(vm->floatStack, accum);
    if (vm->state == FICL_VM_STATE_COMPILE)
        ficlPrimitiveFLiteralImmediate(vm);

    return(1);
}

#if FICL_WANT_LOCALS

static void ficlPrimitiveFLocalParen(ficlVm *vm)
{
   ficlLocalParen(vm, (int) FICL_FALSE, (int) FICL_TRUE);
}

static void ficlPrimitiveF2LocalParen(ficlVm *vm)
{
   ficlLocalParen(vm, (int) FICL_TRUE, (int) FICL_TRUE);
}

#endif /* FICL_WANT_LOCALS */

#endif  /* FICL_WANT_FLOAT */

/**************************************************************************
** Add float words to a system's dictionary.
** system -- Pointer to the Ficl sytem to add float words to.
**************************************************************************/
void ficlSystemCompileFloat(ficlSystem *system)
{
#if FICL_WANT_FLOAT

    r2IntegerMax = ldexp(1.0, 2 * 8 * sizeof(ficlUnsigned) - 1) - 1.0;

    ficlDictionary *dictionary = ficlSystemGetDictionary(system);
    ficlDictionary *environment = ficlSystemGetEnvironment(system);

    FICL_SYSTEM_ASSERT(system, dictionary);
    FICL_SYSTEM_ASSERT(system, environment);

    ficlDictionarySetPrimitive(dictionary, "fconstant", ficlPrimitiveFConstant,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fvalue", ficlPrimitiveFConstant,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f2constant", ficlPrimitiveF2Constant,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f2value",  ficlPrimitiveF2Constant,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fdepth",    ficlPrimitiveFDepth,         FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fliteral",  ficlPrimitiveFLiteralImmediate,     FICL_WORD_IMMEDIATE);
    ficlDictionarySetPrimitive(dictionary, "f.",        ficlPrimitiveFDot,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f.s",       ficlVmDisplayFloatStack,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fs.",       ficlPrimitiveSDot,           FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "fsqrt",     ficlPrimitiveFSqrt,          FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "float+",    ficlPrimitiveFloatPlus,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "floats",    ficlPrimitiveFloats,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fmin",      ficlPrimitiveFMin,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fmax",      ficlPrimitiveFMax,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "falign",    ficlPrimitiveFAlign,         FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "faligned",  ficlPrimitiveFAligned,       FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "floor",     ficlPrimitiveFloor,          FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "d>f",       ficlPrimitiveDToF,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f>d",       ficlPrimitiveFToD,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fround",    ficlPrimitiveFRound,         FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f**",       ficlPrimitiveFStarStar,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fabs",      ficlPrimitiveFAbs,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fe.",       ficlPrimitiveEDot,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "precision", ficlPrimitivePrecision,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "set-precision", ficlPrimitiveSetPrecision,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f~",        ficlPrimitiveFProximate,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "represent", ficlPrimitiveRepresent,      FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "faxpy",     ficlPrimitiveFaxpy,          FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fdot",      ficlPrimitiveFdot,          FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fmm",       ficlPrimitiveFmm,          FICL_WORD_DEFAULT);

#if FICL_WANT_LOCALS
    ficlDictionarySetPrimitive(dictionary, "(flocal)",   ficlPrimitiveFLocalParen,     FICL_WORD_COMPILE_ONLY);
    ficlDictionarySetPrimitive(dictionary, "(f2local)",  ficlPrimitiveF2LocalParen,  FICL_WORD_COMPILE_ONLY);
#endif /* FICL_WANT_LOCALS */

 /* 
    Missing words:

    fvariable
*/

    ficlDictionarySetConstant(environment, "floating",       FICL_FALSE);  /* not all required words are present */
    ficlDictionarySetConstant(environment, "floating-ext",   FICL_FALSE);
    ficlDictionarySetConstant(environment, "floating-stack", system->stackSize);
#else /* FICL_WANT_FLOAT */
	/* get rid of unused parameter warning */
	system = NULL;
#endif
    return;
}
