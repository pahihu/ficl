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
#include "ficl.h"

#if FICL_WANT_FLOAT

#include "ficlmath.h"

#ifdef FICL_USE_FDLIBM
#define fmin(x,y)	((x) < (y) ? (x) : (y))
#define fmax(x,y)	((x) > (y) ? (x) : (y))
#define lrint		rint
#define isinf(x)	(!finite(x))
#define log2(x)		(log(x)/rLog2)
#define exp2(x)		(pow(2.0,x))

#define FP_NAN		1
#define FP_INFINITE	2
#define FP_ZERO		3
#define FP_NORMAL	4

int fpclassify(double x)
{
    static double zero = 0.0;
    int ret = FP_NORMAL;

    if (x == zero)
        ret = FP_ZERO;
    else if (isnan(x))
        ret = FP_NAN;
    else if (!finite(x))
        ret = FP_INFINITE;
    return ret;
}

int signbit(double x)
{
    union {
        double d;
        unsigned long long ull;
    } u;
    u.d = x;
    return (u.ull & 0x8000000000000000ULL) ? 0 == 0 : 0;
}

#endif

#include "ficlblas.h"

#define FICL_FLOAT_ALIGNMENT    sizeof(ficlFloat)

/**************************************************************************
                        d f a l i g n P t r
** Aligns the given pointer to sizeof(double) address units.
** Returns the aligned pointer value.
**************************************************************************/
static void *ficlDFAlignPointer(void *ptr)
{
    ficlInteger p = (ficlInteger)ptr;
    if (p & (sizeof(double) - 1))
	ptr = (void *)((p & ~(sizeof(double) - 1)) + sizeof(double));
    return ptr;
}


/**************************************************************************
                        d i c t D F A l i g n
** Align the dictionary's free space pointer to float.
**************************************************************************/
static void ficlDictionaryDFAlign(ficlDictionary *dictionary)
{
    dictionary->here = ficlDFAlignPointer(dictionary->here);
}


/**************************************************************************
                        d f a l i g n
** dfalign ( -- )
** If the data-space pointer is not double aligned, reserve enough space to
** make it so.
**************************************************************************/
static void ficlPrimitiveDFAlign(ficlVm *vm)
{
    ficlDictionary *dictionary = ficlVmGetDictionary(vm);
    FICL_IGNORE(vm);
    ficlDictionaryDFAlign(dictionary);
}


/**************************************************************************
                        d f a l i g n e d
** dfaligned ( addr -- df-addr )
** f-addr is the first double-aligned address greater than or equal to addr.
**************************************************************************/
static void ficlPrimitiveDFAligned(ficlVm *vm)
{
    void *addr;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    addr = ficlStackPopPointer(vm->dataStack);
    ficlStackPushPointer(vm->dataStack, ficlDFAlignPointer(addr));
}


/*******************************************************************
** dfloat+ ( df-addr1 -- df-addr2 )
*******************************************************************/
static void ficlPrimitiveDFloatPlus(ficlVm *vm)
{
    double *fp;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    fp = ficlStackPopPointer(vm->dataStack);
    ficlStackPushPointer(vm->dataStack, fp + 1);
}


/*******************************************************************
** dfloats ( n1 -- n2 )
*******************************************************************/
static void ficlPrimitiveDFloats(ficlVm *vm)
{
    ficlInteger n;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    n = ficlStackPopInteger(vm->dataStack);
    ficlStackPushInteger(vm->dataStack, n * sizeof(double));
}


/*******************************************************************
** df! ( addr -- ) ( F: r -- )
*******************************************************************/
static void ficlPrimitiveDFStore(ficlVm *vm)
{
    ficlFloat r;
    double *ptr;

    FICL_STACK_CHECK(vm->floatStack, 1, 0);
    FICL_STACK_CHECK(vm->dataStack, 1, 0);

    r = ficlStackPopFloat(vm->floatStack);
    ptr = ficlStackPopPointer(vm->dataStack);

    ptr[0] = (double)r;
}


/*******************************************************************
** df@ ( addr -- ) ( F: -- r )
*******************************************************************/
static void ficlPrimitiveDFFetch(ficlVm *vm)
{
    ficlFloat r;
    double *ptr;

    FICL_STACK_CHECK(vm->floatStack, 0, 1);
    FICL_STACK_CHECK(vm->dataStack, 1, 0 );

    ptr = ficlStackPopPointer(vm->dataStack);
    r = (ficlFloat) ptr[0];

    ficlStackPushFloat(vm->floatStack, r);
}


/**************************************************************************
                        s f a l i g n P t r
** Aligns the given pointer to sizeof(float) address units.
** Returns the aligned pointer value.
**************************************************************************/
static void *ficlSFAlignPointer(void *ptr)
{
    ficlInteger p = (ficlInteger)ptr;
    if (p & (sizeof(float) - 1))
	ptr = (void *)((p & ~(sizeof(float) - 1)) + sizeof(float));
    return ptr;
}


/**************************************************************************
                        d i c t S F A l i g n
** Align the dictionary's free space pointer to float.
**************************************************************************/
static void ficlDictionarySFAlign(ficlDictionary *dictionary)
{
    dictionary->here = ficlSFAlignPointer(dictionary->here);
}


/**************************************************************************
                        s f a l i g n
** sfalign ( -- )
** If the data-space pointer is not float aligned, reserve enough space to
** make it so.
**************************************************************************/
static void ficlPrimitiveSFAlign(ficlVm *vm)
{
    ficlDictionary *dictionary = ficlVmGetDictionary(vm);
    FICL_IGNORE(vm);
    ficlDictionarySFAlign(dictionary);
}


/**************************************************************************
                        s f a l i g n e d
** sfaligned ( addr -- sf-addr )
** f-addr is the first float-aligned address greater than or equal to addr.
**************************************************************************/
static void ficlPrimitiveSFAligned(ficlVm *vm)
{
    void *addr;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    addr = ficlStackPopPointer(vm->dataStack);
    ficlStackPushPointer(vm->dataStack, ficlSFAlignPointer(addr));
}


/*******************************************************************
** sfloat+ ( f-addr1 -- f-addr2 )
*******************************************************************/
static void ficlPrimitiveSFloatPlus(ficlVm *vm)
{
    float *fp;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    fp = ficlStackPopPointer(vm->dataStack);
    ficlStackPushPointer(vm->dataStack, fp + 1);
}


/*******************************************************************
** sfloats ( n1 -- n2 )
*******************************************************************/
static void ficlPrimitiveSFloats(ficlVm *vm)
{
    ficlInteger n;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    n = ficlStackPopInteger(vm->dataStack);
    ficlStackPushInteger(vm->dataStack, n * sizeof(float));
}


/*******************************************************************
** sf! ( addr -- ) ( F: r -- )
*******************************************************************/
static void ficlPrimitiveSFStore(ficlVm *vm)
{
    ficlFloat r;
    float *ptr;

    FICL_STACK_CHECK(vm->floatStack, 1, 0);
    FICL_STACK_CHECK(vm->dataStack, 1, 0);

    r = ficlStackPopFloat(vm->floatStack);
    ptr = ficlStackPopPointer(vm->dataStack);

    ptr[0] = (float)r;
}


/*******************************************************************
** sf@ ( addr -- ) ( F: -- r )
*******************************************************************/
static void ficlPrimitiveSFFetch(ficlVm *vm)
{
    ficlFloat r;
    float *ptr;

    FICL_STACK_CHECK(vm->floatStack, 0, 1);
    FICL_STACK_CHECK(vm->dataStack, 1, 0 );

    ptr = ficlStackPopPointer(vm->dataStack);
    r = (ficlFloat) ptr[0];

    ficlStackPushFloat(vm->floatStack, r);
}


#if FICL_PLATFORM_ALIGNMENT - 4

#warning Using double-precision FP wrappers.

#define ficlFAlignPointer		ficlDFAlignPointer
#define ficlDictionaryFAlign		ficlDictionaryDFAlign
#define ficlPrimitiveFAlign		ficlPrimitiveDFAlign
#define ficlPrimitiveFAligned		ficlPrimitiveDFAligned
#define ficlPrimitiveFloatPlus		ficlPrimitiveDFloatPlus
#define ficlPrimitiveFloats		ficlPrimitiveDFloats
#define ficlPrimitiveFStore		ficlPrimitiveDFStore
#define ficlPrimitiveFFetch		ficlPrimitiveDFFetch

#else

#warning Using single-precision FP wrappers.

#define ficlFAlignPointer		ficlSFAlignPointer
#define ficlDictionaryFAlign		ficlDictionarySFAlign
#define ficlPrimitiveFAlign		ficlPrimitiveSFAlign
#define ficlPrimitiveFAligned		ficlPrimitiveSFAligned
#define ficlPrimitiveFloatPlus		ficlPrimitiveSFloatPlus
#define ficlPrimitiveFloats		ficlPrimitiveSFloats
#define ficlPrimitiveFStore		ficlPrimitiveSFStore
#define ficlPrimitiveFFetch		ficlPrimitiveSFFetch

#endif

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
	return ficlDictionaryAppendConstantInstruction(dictionary, s, ficlInstructionFConstantParen, *(ficlInteger*)(&value));
}


ficlWord   *ficlDictionarySetFConstant(ficlDictionary *dictionary, char *name, float value)
{
    ficlString s;

    FICL_STRING_SET_FROM_CSTRING(s, name);
    return ficlDictionarySetConstantInstruction(dictionary, s, ficlInstructionFConstantParen, *(ficlInteger*)(&value));
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
    int expof10, sign, scale;
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

    scale = precision;
    if (expof10 < 0)
        scale = scale - expof10 - 1;
    value   *= pow(10.0, scale);
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

    value = display * pow(10.0, -scale);

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
    else if (u > FICL_MAX_FLOAT_PRECISION)
        u = FICL_MAX_FLOAT_PRECISION;

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
            ficlUnsignedToString(tmp, u);
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
** fdot ( F: r -- )
*******************************************************************/
static void ficlPrivateFDot(ficlVm *vm)
{
    ficlFloat r;
    double rDisp;
    int i, j, prec, len, rSign, rExp;
    char *pHold;
    char tmp[32];

    FICL_STACK_CHECK(vm->floatStack, 1, 0);

    len  = 0;
    prec = vm->precision;
    r = ficlStackPopFloat(vm->floatStack);
    switch (fpclassify(r))
    {
        case FP_INFINITE:
            sprintf(vm->pob, r < 0 ? "-inf " : "inf ");
            break;
        case FP_NAN:
            sprintf(vm->pob, "nan ");
            break;
        case FP_ZERO:
            if (copysign(1.0, r) < 0)
              vm->pob[len++] = '-';
            sprintf(vm->pob + len, "0. ");
            break;
        default:
            r = ficlRepresentPriv(r, prec, &rSign, &rDisp, &rExp);
            pHold = vm->pob;
            if (rSign)
                *pHold++ = '-';
            ficlUnsignedToString(tmp, (ficlUnsigned) rDisp);
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
                    int cnt = prec;
                    *pHold++ = '0'; *pHold++ = '.';
                    for (i = rExp + 1; (cnt > 0) && (i < 0); cnt--, i++)
                        *pHold++ = '0';
                    for (i = 0; (cnt > 0) && (i < prec); cnt--, i++)
                        *pHold++ = tmp[i];
                }
                else
                {
                    for (i = 0; i < rExp + 1; i++)
                        *pHold++ = tmp[i];
                    *pHold++ = '.';
                    for (j = 0; j < prec; i++, j++)
                        *pHold++ = tmp[i];
                }
            }
            // zero supression
            len = pHold - vm->pob - 1;
            // while ('0' == vm->pob[len]) len--;
            vm->pob[++len] = ' ';
            vm->pob[++len] = '\0';
    }
}

/*******************************************************************
** Display a float in decimal format.
** (f.) ( F: r -- ) ( -- c-addr len )
*******************************************************************/
static void ficlPrimitiveParenFDot(ficlVm *vm)
{
    ficlInteger len;

    FICL_STACK_CHECK(vm->dataStack, 0, 2);

    ficlPrivateFDot(vm);
    len = strlen(vm->pob);
    if (len) len--;
    
    ficlStackPushPointer(vm->dataStack, vm->pob);
    ficlStackPushInteger(vm->dataStack, len);
}

/*******************************************************************
** Display a float in decimal format.
** f. ( r -- )
*******************************************************************/
static void ficlPrimitiveFDot(ficlVm *vm)
{
    ficlPrivateFDot(vm);
    ficlVmTextOut(vm, vm->pob);
}

/*******************************************************************
** Display a float in scientific format.
** fsdot ( F: r -- )
*******************************************************************/
static void ficlPrivateFSDot(ficlVm *vm)
{
    ficlFloat r;
    double rDisp;
    int prec, len, rSign, rExp, scale;

    FICL_STACK_CHECK(vm->floatStack, 1, 0);

    len  = 0;
    prec = vm->precision;
    r    = ficlStackPopFloat(vm->floatStack);
    switch (fpclassify(r))
    {
        case FP_INFINITE:
            sprintf(vm->pob, r < 0 ? "-inf " : "inf ");
            break;
        case FP_NAN:
            sprintf(vm->pob, "nan ");
            break;
        case FP_ZERO:
            if (copysign(1.0, r) < 0)
                vm->pob[len++] = '-';
            sprintf(vm->pob + len, "%.*fE0 ", prec - 1, 0.0);
            break;
        default:
            r = ficlRepresentPriv(r, prec, &rSign, &rDisp, &rExp);
            if (rSign)
                vm->pob[len++] = '-';
            if (rExp < 0)
                scale = 1 - prec;
            else
                scale = - (rExp + prec);
            rDisp *= pow(10.0, scale);
            sprintf(vm->pob + len, "%.*fE%d ", prec, rDisp, rExp);
    }
}

/*******************************************************************
** Display a float in scientific format.
** (fs.) ( F: r -- ) ( -- c-addr len )
*******************************************************************/
static void ficlPrimitiveParenFSDot(ficlVm *vm)
{
    ficlInteger len;

    FICL_STACK_CHECK(vm->dataStack, 0, 2);

    ficlPrivateFSDot(vm);
    len = strlen(vm->pob);
    if (len) len--;

    ficlStackPushPointer(vm->dataStack, vm->pob);
    ficlStackPushInteger(vm->dataStack, len);
}

/*******************************************************************
** Display a float in scientific format.
** fs. ( r -- )
*******************************************************************/
static void ficlPrimitiveFSDot(ficlVm *vm)
{
    ficlPrivateFSDot(vm);
    ficlVmTextOut(vm, vm->pob);
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
    char buffer[128];
    sprintf(buffer, "[%p %3d] %16f (0x%0*lx)\n",
            cell,
            context->count++,
            (double)(cell->f),
            FICL_INTEGER_HEX_DIGITS, cell->i);
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
** Floating square.
** x**2 ( r1 -- r2 )
*******************************************************************/
static void ficlPrimitiveXStarStarTwo(ficlVm *vm)
{
    ficlFloat r;

    FICL_STACK_CHECK(vm->floatStack, 1, 1);

    r = ficlStackPopFloat(vm->floatStack);
    ficlStackPushFloat(vm->floatStack, (ficlFloat) r*r);
}

/*******************************************************************
** Quadratic formula solver.
** fsolve2 ( c b a -- r1 r2 ff)
*******************************************************************/
static void ficlPrimitiveFSolveTwo(ficlVm *vm)
{
    ficlFloat a, b, c;
    ficlFloat disc, r1, r2;

    FICL_STACK_CHECK(vm->dataStack,  0, 1);
    FICL_STACK_CHECK(vm->floatStack, 3, 2);

    a = ficlStackPopFloat(vm->floatStack);
    b = ficlStackPopFloat(vm->floatStack);
    c = ficlStackPopFloat(vm->floatStack);

    disc = b*b - 4.0*a*c;
    if (disc >=0)
    {
        disc = sqrt(disc);
        a *= 2.0;
        r1 = (-b + disc) / a;
        r2 = (-b - disc) / a;
        ficlStackPushFloat(vm->floatStack, r1);
        ficlStackPushFloat(vm->floatStack, r2);
        ficlStackPushInteger(vm->dataStack, FICL_TRUE);
    }
    else
        ficlStackPushInteger(vm->dataStack, FICL_FALSE);
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
** FAXPY1
** faxpy1 ( f_x f_y u -- ) ( F: ra -- )
*******************************************************************/
static void ficlPrimitiveFaxpy1(ficlVm *vm)
{
    ficlFloat ra;
    ficlFloat *x, *y;
    ficlUnsigned n;

    FICL_STACK_CHECK(vm->floatStack, 1, 0);
    FICL_STACK_CHECK(vm->dataStack, 3, 0);

    ra = ficlStackPopFloat(vm->floatStack);

    n   = ficlStackPopUnsigned(vm->dataStack);
    y    = ficlStackPopPointer(vm->dataStack);
    x    = ficlStackPopPointer(vm->dataStack);

    ficlFAXPY(n, ra, x, 1, y, 1);
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
** FDOT with increment of 1.
** fdot1 ( f_x f_y u -- ) ( F: -- r )
*******************************************************************/
static void ficlPrimitiveFdot1(ficlVm *vm)
{
    ficlFloat r;
    ficlFloat *x, *y;
    ficlUnsigned n;

    FICL_STACK_CHECK(vm->dataStack, 3, 0);
    FICL_STACK_CHECK(vm->floatStack, 0, 1);

    n    = ficlStackPopUnsigned(vm->dataStack);
    y    = ficlStackPopPointer(vm->dataStack);
    x    = ficlStackPopPointer(vm->dataStack);

    r = ficlFDOT(n, x, 1, y, 1);
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

#define FBINARY(name,fop) \
static void ficlPrimitive##name(ficlVm *vm) 	\
{ 						\
    ficlFloat r1, r2;				\
						\
    FICL_STACK_CHECK(vm->floatStack, 2, 1);	\
						\
    r2 = ficlStackPopFloat(vm->floatStack);	\
    r1 = ficlStackPopFloat(vm->floatStack);	\
    ficlStackPushFloat(vm->floatStack, fop(r1,r2)); \
}

#define FUNARY(name,fop) \
static void ficlPrimitive##name(ficlVm *vm) 	\
{ 						\
    ficlFloat r1;				\
						\
    FICL_STACK_CHECK(vm->floatStack, 1, 1);	\
						\
    r1 = ficlStackPopFloat(vm->floatStack);	\
    ficlStackPushFloat(vm->floatStack, fop(r1)); \
}

/*******************************************************************
** fmin ( r1 r2 -- r3 )
*******************************************************************/
FBINARY(FMin,fmin)


/*******************************************************************
** fmax ( r1 r2 -- r3 )
*******************************************************************/
FBINARY(FMax,fmax)


/*******************************************************************
** floor ( r1 -- r2 )
*******************************************************************/
FUNARY(Floor,floor)


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
static double rLog2;

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
FUNARY(FRound,rint)


/*******************************************************************
** ftrunc ( r1 -- r2 )
*******************************************************************/
FUNARY(FTrunc,trunc)


/*******************************************************************
** f** ( r1 r2 -- r3 )
*******************************************************************/
FBINARY(FStarStar,pow)


/*******************************************************************
** fabs ( r1 -- r2 )
*******************************************************************/
FUNARY(FAbs,fabs)


/*******************************************************************
** Display a float in engineering format.
** fedot ( F: r -- )
*******************************************************************/
extern char* eng(double, int, int);

static void ficlPrivateFEDot(ficlVm *vm)
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
            sprintf(vm->pob, r < 0 ? "-inf " : "inf ");
            break;
        case FP_NAN:
            sprintf(vm->pob, "nan ");
            break;
        case FP_ZERO:
            if (copysign(1.0, r) < 0)
                vm->pob[len++] = '-';
            sprintf(vm->pob + len, "%.*fE0 ", prec - 1, 0.0);
            break;
        default:
            r = ficlRepresentPriv(r, prec, &rSign, &rDisp, &rExp);
            if (rExp < 0)
            {
                eng   = -((2 + abs(rExp)) / 3) * 3;
                scale = rExp + 1 - eng - prec;
            }
            else
            {
                eng   = (rExp / 3) * 3;
                scale = -(eng + prec);
            }
            rDisp *= pow(10.0, scale);
            if (rSign)
                vm->pob[len++] = '-';
            sprintf(vm->pob + len, "%.*fE%d", prec, rDisp, eng);
    }
}

/*******************************************************************
** Display a float in engineering format.
** (fe.) ( F: r -- ) ( -- c-addr n )
*******************************************************************/
static void ficlPrimitiveParenFEDot(ficlVm *vm)
{
    ficlInteger len;

    FICL_STACK_CHECK(vm->dataStack, 0, 2);

    ficlPrivateFEDot(vm);
    len = strlen(vm->pob);
    if (len) len--;

    ficlStackPushPointer(vm->dataStack, vm->pob);
    ficlStackPushInteger(vm->dataStack, len);
}


/*******************************************************************
** Display a float in engineering format.
** fe. ( F: r -- )
*******************************************************************/
static void ficlPrimitiveFEDot(ficlVm *vm)
{
    ficlPrivateFEDot(vm);
    ficlVmTextOut(vm, vm->pob);
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
    else if (prec > FICL_MAX_FLOAT_PRECISION)
        prec = FICL_MAX_FLOAT_PRECISION;
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
    if (r3 == 0.0)
    {
        d1.f = r1;
        d2.f = r2;
        if (d1.u == d2.u)
            flag = FICL_TRUE;
    }
    else if (signbit(r3))
    {
        if (fabs(r1 - r2) < fabs(r3) * (fabs(r1) + fabs(r2)))
            flag = FICL_TRUE;
    }
    else
    {
        if (fabs(r1 - r2) < r3)
            flag = FICL_TRUE;
    }
    ficlStackPushUnsigned(vm->dataStack, flag);
}

/*******************************************************************
** f<= ( F: r1 r2 -- ) ( -- flag )
*******************************************************************/
static void ficlPrimitiveFLessEqual(ficlVm *vm)
{
    ficlFloat r1, r2;

    FICL_STACK_CHECK(vm->floatStack, 2, 0);
    FICL_STACK_CHECK(vm->dataStack,  0, 1);

    r2 = ficlStackPopFloat(vm->floatStack);
    r1 = ficlStackPopFloat(vm->floatStack);
    ficlStackPushInteger(vm->dataStack, FICL_BOOL(r1 <= r2));
}

/*******************************************************************
** f>= ( F: r1 r2 -- ) ( -- flag )
*******************************************************************/
static void ficlPrimitiveFGreaterEqual(ficlVm *vm)
{
    ficlFloat r1, r2;

    FICL_STACK_CHECK(vm->floatStack, 2, 0);
    FICL_STACK_CHECK(vm->dataStack,  0, 1);

    r2 = ficlStackPopFloat(vm->floatStack);
    r1 = ficlStackPopFloat(vm->floatStack);
    ficlStackPushInteger(vm->dataStack, FICL_BOOL(r1 >= r2));
}

/*******************************************************************
** f<> ( F: r1 r2 -- ) ( -- flag )
*******************************************************************/
static void ficlPrimitiveFNotEqual(ficlVm *vm)
{
    ficlFloat r1, r2;

    FICL_STACK_CHECK(vm->floatStack, 2, 0);
    FICL_STACK_CHECK(vm->dataStack,  0, 1);

    r2 = ficlStackPopFloat(vm->floatStack);
    r1 = ficlStackPopFloat(vm->floatStack);
    ficlStackPushInteger(vm->dataStack, FICL_BOOL(r1 != r2));
}

/*******************************************************************
** fnan? ( r -- ) ( -- flag )
*******************************************************************/
static void ficlPrimitiveFNaNQ(ficlVm *vm)
{
    ficlFloat r;

    FICL_STACK_CHECK(vm->floatStack, 1, 0);
    FICL_STACK_CHECK(vm->dataStack,  0, 1);

    r = ficlStackPopFloat(vm->floatStack);
    ficlStackPushInteger(vm->dataStack, FICL_BOOL(isnan(r)));
}

/*******************************************************************
** finfinite? ( r -- ) ( -- flag )
*******************************************************************/
static void ficlPrimitiveFInfiniteQ(ficlVm *vm)
{
    ficlFloat r;

    FICL_STACK_CHECK(vm->floatStack, 1, 0);
    FICL_STACK_CHECK(vm->dataStack,  0, 1);

    r = ficlStackPopFloat(vm->floatStack);
    ficlStackPushInteger(vm->dataStack,
                            isnan(r) ? FICL_FALSE : FICL_BOOL(isinf(r)));
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
    double power;
    double accum = 0.0;
    double mant = 0.1;
    ficlInteger exponent = 0;
    char flag = 0;
    FloatParseState estate = FPS_START;
#ifdef FICL_USE_STRTOD
    char buff[128], *endptr, *ptr;
    int dynamic;
    extern double gay_strtod(const char *, char **);
#endif


    FICL_STACK_CHECK(vm->floatStack, 0, 1);

	
    /*
    ** floating point numbers only allowed in base 10 
    */
    if (vm->base != 10)
        return(0);

    trace = FICL_STRING_GET_POINTER(s);
    length = FICL_STRING_GET_LENGTH(s);

    /*
    ** strip leading-trailing space
    */
    while (length && ' ' == *trace)
    {
        trace++; length--;
    }

    while (length && ' ' == trace[length-1])
        length--;

    if (length)
    {

#ifdef FICL_USE_STRTOD

    dynamic = 0;
    if (0 == trace[length])
        ptr = trace;
    else if (length < 128)
    {
        strncpy(buff, trace, length);
        ptr = buff;
    }
    else
    {
        ptr = (char *)malloc(length * sizeof(char));
        if (NULL == ptr)
            return (0);
        strncpy(ptr, trace, length);
        dynamic = 1;
    }
    if (length && (ptr[length-1] == 'e' || ptr[length-1] == 'E'))
        length--;
    ptr[length] = '\0';

    accum = gay_strtod(ptr, &endptr);

    if (dynamic)
        free(ptr);

    /* strtod() succeeded, skip parsing */
    if (0 == *endptr)
        goto Lparsed;

    accum = 0.0;
#endif

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
                else if ((c == 'e') || (c == 'E') || (c == 'd') || (c == 'D'))
                {
                    estate = FPS_STARTEXP;
                }
                else if ((c == '+') || (c == '-'))
                {
                    goto LStartExp;
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
                if ((c == 'e') || (c == 'E') || (c == 'd') || (c == 'D'))
                {
                    estate = FPS_STARTEXP;
                }
                else if ((c == '+') || (c == '-'))
                {
                    goto LStartExp;
                }
                else
                {
                    digit = (unsigned char)(c - '0');
                    if (digit > 9)
                        return(0);

                    accum += digit * mant;
                    mant *= 0.1;
                }
                break;
            }
            /* Start processing the exponent part of number. */
            /* Look for sign. */
            case FPS_STARTEXP:
            {
LStartExp:
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
        power = pow(10.0, exponent);
        accum *= power;
    }

    } /* not all space? */
Lparsed:

    ficlStackPushFloat(vm->floatStack, (ficlFloat) accum);
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

/*******************************************************************
** facos ( r1 -- r2 )
*******************************************************************/
FUNARY(FACos,acos)


/*******************************************************************
** facosh ( r1 -- r2 )
*******************************************************************/
FUNARY(FACosH,acosh)


/*******************************************************************
** falog ( r1 -- r2 )
*******************************************************************/
static void ficlPrimitiveFALog(ficlVm *vm)
{
    ficlFloat r1;

    FICL_STACK_CHECK(vm->floatStack, 1, 1);

    r1 = ficlStackPopFloat(vm->floatStack);
    ficlStackPushFloat(vm->floatStack, pow(10.0,r1));
}



/*******************************************************************
** fasin ( r1 -- r2 )
*******************************************************************/
FUNARY(FASin,asin)


/*******************************************************************
** fasinh ( r1 -- r2 )
*******************************************************************/
FUNARY(FASinH,asinh)


/*******************************************************************
** fatan ( r1 -- r2 )
*******************************************************************/
FUNARY(FATan,atan)


/*******************************************************************
** fatan2 ( r1 r2 -- r2 )
*******************************************************************/
FBINARY(FATan2,atan2)


/*******************************************************************
** fatanh ( r1 -- r2 )
*******************************************************************/
FUNARY(FATanH,atanh)


/*******************************************************************
** fcos ( r1 -- r2 )
*******************************************************************/
FUNARY(FCos,cos)


/*******************************************************************
** fcosh ( r1 -- r2 )
*******************************************************************/
FUNARY(FCosH,cosh)


/*******************************************************************
** fexp ( r1 -- r2 )
*******************************************************************/
FUNARY(FExp,exp)


/*******************************************************************
** fexpm1 ( r1 -- r2 )
*******************************************************************/
FUNARY(FExpM1,expm1)


/*******************************************************************
** 2**x ( r1 -- r2 )
*******************************************************************/
FUNARY(TwoStarStarX,exp2)


/*******************************************************************
** fln ( r1 -- r2 )
*******************************************************************/
FUNARY(FLn,log)


/*******************************************************************
** flnp1 ( r1 -- r2 )
*******************************************************************/
FUNARY(FLnP1,log1p)


/*******************************************************************
** flog ( r1 -- r2 )
*******************************************************************/
FUNARY(FLog,log10)


/*******************************************************************
** log2 ( r1 -- r2 )
*******************************************************************/
FUNARY(Log2,log2)


/*******************************************************************
** fsin ( r1 -- r2 )
*******************************************************************/
FUNARY(FSin,sin)


/*******************************************************************
** fsincos ( r1 -- r2 r3 )
*******************************************************************/
static void ficlPrimitiveFSinCos(ficlVm *vm)
{
    ficlFloat r1;

    FICL_STACK_CHECK(vm->floatStack, 1, 2);

    r1 = ficlStackPopFloat(vm->floatStack);
    ficlStackPushFloat(vm->floatStack, sin(r1));
    ficlStackPushFloat(vm->floatStack, cos(r1));
}

/*******************************************************************
** fsinh ( r1 -- r2 )
*******************************************************************/
FUNARY(FSinH,sinh)


/*******************************************************************
** ftan ( r1 -- r2 )
*******************************************************************/
FUNARY(FTan,tan)


/*******************************************************************
** ftanh ( r1 -- r2 )
*******************************************************************/
FUNARY(FTanH,tanh)


/*******************************************************************
** poly ( F: x -- fx ) ( coeffs degree -- )
*******************************************************************/
static void ficlPrimitivePoly(ficlVm *vm)
{
    ficlUnsigned i, degree;
    ficlFloat *coeffs;
    ficlFloat fx, x;

    FICL_STACK_CHECK(vm->dataStack, 2, 0);
    FICL_STACK_CHECK(vm->floatStack, 1, 1);

    x = ficlStackPopFloat(vm->floatStack);
    degree = ficlStackPopUnsigned(vm->dataStack);
    coeffs = ficlStackPopPointer(vm->dataStack);

    i = 0; fx = coeffs[i++];
    for (; i <= degree; i++)
        fx = coeffs[i] + x * fx;

    ficlStackPushFloat(vm->floatStack, fx);
}


/*******************************************************************
** odd-poly ( F: x -- fx ) ( coeffs degree -- )
*******************************************************************/
static void ficlPrimitiveOddPoly(ficlVm *vm)
{
    ficlUnsigned i, j, degree;
    ficlFloat *coeffs;
    ficlFloat fx, x;

    FICL_STACK_CHECK(vm->dataStack, 2, 0);
    FICL_STACK_CHECK(vm->floatStack, 1, 1);

    x = ficlStackPopFloat(vm->floatStack);
    degree = ficlStackPopUnsigned(vm->dataStack);
    coeffs = ficlStackPopPointer(vm->dataStack);

    i = 0; fx = coeffs[i++]; j = i;
    for (; i <= degree; i++)
    {
        fx *= x;
        if (0 == (i & 1))
            fx += coeffs[j++];
    }

    ficlStackPushFloat(vm->floatStack, fx);
}


#endif  /* FICL_WANT_FLOAT */

/**************************************************************************
** Add float words to a system's dictionary.
** system -- Pointer to the Ficl sytem to add float words to.
**************************************************************************/
void ficlSystemCompileFloat(ficlSystem *system)
{
#if FICL_WANT_FLOAT

#define PRIMDEF(name,primsuff) \
    ficlDictionarySetPrimitive(dictionary, name, ficlPrimitive##primsuff, FICL_WORD_DEFAULT)

    r2IntegerMax = ldexp(1.0, 2 * 8 * sizeof(ficlUnsigned) - 1) - 1.0;
    rLog2 = log(2.0);

    ficlDictionary *dictionary = ficlSystemGetDictionary(system);
    ficlDictionary *environment = ficlSystemGetEnvironment(system);

    FICL_SYSTEM_ASSERT(system, dictionary);
    FICL_SYSTEM_ASSERT(system, environment);

    ficlDictionarySetPrimitive(dictionary, "fconstant", ficlPrimitiveFConstant,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fvalue", ficlPrimitiveFConstant,         FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f2constant", ficlPrimitiveF2Constant,    FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f2value",  ficlPrimitiveF2Constant,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fdepth",    ficlPrimitiveFDepth,         FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fliteral",  ficlPrimitiveFLiteralImmediate,     FICL_WORD_IMMEDIATE);
    ficlDictionarySetPrimitive(dictionary, "(f.)",      ficlPrimitiveParenFDot,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f.",        ficlPrimitiveFDot,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "(fs.)",     ficlPrimitiveParenFSDot,     FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fs.",       ficlPrimitiveFSDot,          FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f.s",       ficlVmDisplayFloatStack,     FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "fsqrt",     ficlPrimitiveFSqrt,          FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "float+",    ficlPrimitiveFloatPlus,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "floats",    ficlPrimitiveFloats,         FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fmin",      ficlPrimitiveFMin,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fmax",      ficlPrimitiveFMax,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "falign",    ficlPrimitiveFAlign,         FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "faligned",  ficlPrimitiveFAligned,       FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "floor",     ficlPrimitiveFloor,          FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "d>f",       ficlPrimitiveDToF,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f>d",       ficlPrimitiveFToD,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fround",    ficlPrimitiveFRound,         FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "ftrunc",    ficlPrimitiveFTrunc,         FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "ftruncate", ficlPrimitiveFTrunc,         FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f**",       ficlPrimitiveFStarStar,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fabs",      ficlPrimitiveFAbs,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "(fe.)",     ficlPrimitiveParenFEDot,     FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fe.",       ficlPrimitiveFEDot,          FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "precision", ficlPrimitivePrecision,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "set-precision", ficlPrimitiveSetPrecision,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f~",        ficlPrimitiveFProximate,     FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "represent", ficlPrimitiveRepresent,      FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "faxpy",     ficlPrimitiveFaxpy,          FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "faxpy1",    ficlPrimitiveFaxpy1,          FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fdot",      ficlPrimitiveFdot,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fdot1",     ficlPrimitiveFdot1,           FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fmm",       ficlPrimitiveFmm,            FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f<=",	    ficlPrimitiveFLessEqual,     FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f>=",	    ficlPrimitiveFGreaterEqual,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "f<>",	    ficlPrimitiveFNotEqual,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "fnan?",	    ficlPrimitiveFNaNQ,    	     FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "finfinite?", ficlPrimitiveFInfiniteQ,     FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "df@",	    ficlPrimitiveDFFetch,        FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "df!",	    ficlPrimitiveDFStore,        FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "dfalign",   ficlPrimitiveDFAlign,        FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "dfaligned", ficlPrimitiveDFAligned,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "dfloat+",   ficlPrimitiveDFloatPlus,     FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "dfloats",   ficlPrimitiveDFloats,        FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "sf@",	    ficlPrimitiveSFFetch,        FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "sf!",	    ficlPrimitiveSFStore,        FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "sfalign",   ficlPrimitiveSFAlign,        FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "sfaligned", ficlPrimitiveSFAligned,      FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "sfloat+",   ficlPrimitiveSFloatPlus,     FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "sfloats",   ficlPrimitiveSFloats,        FICL_WORD_DEFAULT);

    PRIMDEF( "facos",    FACos);
    PRIMDEF( "facosh",   FACosH);
    PRIMDEF( "falog",    FALog);
    PRIMDEF( "fasin",    FASin);
    PRIMDEF( "fasinh",   FASinH);
    PRIMDEF( "fatan",    FATan);
    PRIMDEF( "fatan2",   FATan2);
    PRIMDEF( "fatanh",   FATanH);
    PRIMDEF( "fcos",     FCos);
    PRIMDEF( "fcosh",    FCosH);
    PRIMDEF( "fexp",     FExp);
    PRIMDEF( "fexpm1",   FExpM1);
    PRIMDEF( "fln",      FLn);
    PRIMDEF( "flnp1",    FLnP1);
    PRIMDEF( "flog",     FLog);
    PRIMDEF( "fsin",     FSin);
    PRIMDEF( "fsincos",  FSinCos);
    PRIMDEF( "fsinh",    FSinH);
    PRIMDEF( "ftan",     FTan);
    PRIMDEF( "ftanh",    FTanH);
    PRIMDEF( "log2",     Log2);
    PRIMDEF( "2**x",     TwoStarStarX);

    PRIMDEF( "poly",     Poly);
    PRIMDEF( "odd-poly", OddPoly);
    PRIMDEF( "fsolve2",  FSolveTwo);
    PRIMDEF( "x**2",     XStarStarTwo);
    ficlDictionarySetConstant(environment, "max-precision", FICL_MAX_FLOAT_PRECISION);

#if FICL_WANT_LOCALS
    ficlDictionarySetPrimitive(dictionary, "(flocal)",   ficlPrimitiveFLocalParen,   FICL_WORD_COMPILE_ONLY);
    ficlDictionarySetPrimitive(dictionary, "(f2local)",  ficlPrimitiveF2LocalParen,  FICL_WORD_COMPILE_ONLY);
#endif /* FICL_WANT_LOCALS */

    ficlDictionarySetConstant(environment, "floating",       FICL_FALSE);  /* not all required words are present */
    ficlDictionarySetConstant(environment, "floating-ext",   FICL_FALSE);
    ficlDictionarySetConstant(environment, "floating-stack", system->stackSize);
#else /* FICL_WANT_FLOAT */
	/* get rid of unused parameter warning */
	system = NULL;
#endif
    return;
}

/* vim: set ts=4 sw=4 et: */
