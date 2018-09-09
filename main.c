/*
** stub main for testing Ficl
** $Id: //depot/gamejones/ficl/main.c#2 $
*/
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

#include <stdio.h>
#include <stdlib.h>

#include "ficl.h"

ficlVm *f_vm;
ficlSystem *f_system;

#include <signal.h>

int signals[] = {
#ifdef SIGQUIT
	SIGQUIT,
#endif
#ifdef SIGILL
	SIGILL,
#endif
#ifdef SIGABRT
	SIGABRT,
#endif
#ifdef SIGFPE
	SIGFPE,
#endif
#ifdef SIGSEGV
	SIGSEGV,
#endif
#ifdef SIGBUS
	SIGBUS,
#endif
#ifdef SIGSYS
	SIGSYS,
#endif
#ifdef SIGTRAP
	SIGTRAP,
#endif
#ifdef SIGXCPU
	SIGXCPU,
#endif
#ifdef SIGXFSZ
	SIGXFSZ
#endif
};

jmp_buf mainLoop;

static void sig_handler(int sig)
{
	if (SIG_ERR == signal (sig, sig_handler))
    {
		sprintf(f_vm->pad, "Error: signal reinstall failed\n");
		ficlVmErrorOut(f_vm, f_vm->pad);
	}
	longjmp (mainLoop, sig);
}

static void install_handlers(void)
{
	int i;

	for (i = 0; i < sizeof(signals) / sizeof(int); i++)
		signal (signals[i], sig_handler);
}

static void usage()
{
	fprintf(stderr,"usage: ficl [-sN] [-dN] [-eN] [file1 file2 ...]\n");
	exit(1);
}

#if FICL_WANT_MULTITHREADED

pthread_mutex_t dictionaryMutex;
pthread_mutex_t systemMutex;

extern int recursiveMutexInit(pthread_mutex_t *mutex);

int ficlDictionaryLock(ficlDictionary *dictionary, ficlUnsigned lockIncrement)
{
    int rc = 0;

    if (FICL_FALSE == lockIncrement)
        rc = pthread_mutex_unlock(&dictionaryMutex);
    else
        rc = pthread_mutex_lock(&dictionaryMutex);

    return rc;
}

int ficlSystemLock(ficlSystem *system, ficlUnsigned lockIncrement)
{
    int rc = 0;

    if (FICL_FALSE == lockIncrement)
        rc = pthread_mutex_unlock(&systemMutex);
    else
        rc = pthread_mutex_lock(&systemMutex);

    return rc;
}

#endif

int main(int argc, char **argv)
{
    int returnValue = 0;
    char buffer[256];
	int sig;
	int done;
	int narg;
	int prompt;
	ficlSystemInformation fsi;
    unsigned gotSignal;

	install_handlers();
	ficlSystemInformationInitialize(&fsi);

	narg = 1;
	while ((narg < argc) && ('-' == *argv[narg]))
    {
		switch (argv[narg][1]) {
		case 's': fsi.stackSize = atoi(argv[narg]+2); break;
		case 'e': fsi.environmentSize = atoi(argv[narg]+2); break;
		case 'd': fsi.dictionarySize = atoi(argv[narg]+2); break;
		default:	 usage();
		}
		narg++;
	}

#if FICL_WANT_MULTITHREADED
    recursiveMutexInit(&dictionaryMutex);
    recursiveMutexInit(&systemMutex);
#endif

    f_system = ficlSystemCreate(&fsi);
    f_vm = ficlSystemCreateVm(f_system);

    returnValue = ficlVmEvaluate(f_vm, ".ver .( " __DATE__ " ) cr quit");

    /*
    ** load files specified on command-line
    */
    while (narg < argc)
    {
        sprintf(buffer, ".( loading %s ) cr load %s\n cr", argv[narg], argv[narg]);
		returnValue = ficlVmEvaluate(f_vm, buffer);
		narg++;
	}

    gotSignal = 0;
	if ((sig = setjmp(mainLoop)))
    {
	    sprintf(f_vm->pad, "Error: got signal (%d)\n", sig);
		ficlVmErrorOut(f_vm, f_vm->pad);
        ficlVmDisplayReturnStack(f_vm);
        ficlVmAbort(f_vm);
	}

	done = 0;
	prompt = 1;
    /* while (returnValue != FICL_VM_STATUS_USER_EXIT) */
	while (!done)
	{
		if (prompt) {
            int i, depth = ficlStackDepth(f_vm->dataStack);
            if (FICL_VM_STATE_INTERPRET == f_vm->state)
            {
                for (i = 0; i < FICL_MIN(16,depth); i++)
                    fputs(".", stdout);
			    fputs("ok> ", stdout);
            }
            else
			    fputs("  ] ", stdout);
        }
		fgets(buffer, sizeof(buffer), stdin);
		prompt = 1;
		returnValue = ficlVmEvaluate(f_vm, buffer);
		switch (returnValue)
        {
		    case FICL_VM_STATUS_OUT_OF_TEXT:
                if (f_vm->restart)
                    prompt = 0;
                break;
		    case FICL_VM_STATUS_ERROR_EXIT:
                ficlVmAbort(f_vm);
			    break;
		    case FICL_VM_STATUS_USER_EXIT:
			    done = 1;
			    break;
		    default:
			    sprintf(f_vm->pad, "Error: got exception (%d)\n", returnValue);
			    ficlVmErrorOut(f_vm, f_vm->pad);
			    break;
		}
    }

#if FICL_WANT_MULTITHREADED
    pthread_mutex_destroy(&dictionaryMutex);
    pthread_mutex_destroy(&systemMutex);
#endif

    ficlSystemDestroy(f_system);
    return 0;
}

// vim:ts=4:sw=4:et
