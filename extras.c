#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "ficl.h"

#ifdef WIN32
#include <io.h>
#define getcwd _getcwd
#include "dlfcn.h"
#else
#include <dlfcn.h>
#endif

#include "curterm.h"
#include "bswap.h"

#if !defined(FICL_ANSI) || defined(__MINGW32__)

/*
** Ficl interface to _getcwd (Win32)
** Prints the current working directory using the VM's 
** textOut method...
*/
static void ficlPrimitiveGetCwd(ficlVm *vm)
{
    char *directory;

    directory = getcwd(NULL, 80);
    ficlVmTextOut(vm, directory);
    ficlVmTextOut(vm, "\n");
    free(directory);
    return;
}



/*
** Ficl interface to _chdir (Win32)
** Gets a newline (or NULL) delimited string from the input
** and feeds it to the Win32 chdir function...
** Example:
**    cd c:\tmp
*/
static void ficlPrimitiveChDir(ficlVm *vm)
{
    ficlCountedString *counted = (ficlCountedString *)vm->Pad;
    ficlVmGetString(vm, counted, '\n');
    if (counted->length > 0)
    {
       int err = chdir(counted->text);
       if (err)
        {
            ficlVmTextOut(vm, "Error: path not found\n");
            ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
        }
    }
    else
    {
        ficlVmTextOut(vm, "Warning (chdir): nothing happened\n");
    }
    return;
}

/* : CLOCK ( -- u ) */
static void ficlPrimitiveClock(ficlVm *vm)
{
    clock_t now = clock();
    ficlStackPushUnsigned(vm->dataStack, (ficlUnsigned)now);
    return;
}

/* : GET-MSECS ( -- u ) */
static void ficlPrimitiveGetMSecs(ficlVm *vm)
{
    clock_t now = clock();
    ficlStackPushUnsigned(
		vm->dataStack,
		(ficlUnsigned)(1000 * ((double)now / CLOCKS_PER_SEC))
	);
    return;
}

#endif /* FICL_ANSI */

/*
** Ficl interface to system (ANSI)
** Gets a newline (or NULL) delimited string from the input
** and feeds it to the ANSI system function...
** Example:
**    system del *.*
**    \ ouch!
*/
static void ficlPrimitiveSystem(ficlVm *vm)
{
    ficlCountedString *counted = (ficlCountedString *)vm->Pad;

    ficlVmGetString(vm, counted, '\n');
    if (FICL_COUNTED_STRING_GET_LENGTH(*counted) > 0)
    {
        int returnValue = system(FICL_COUNTED_STRING_GET_POINTER(*counted));
        if (returnValue)
        {
            sprintf(vm->Pad, "System call returned %d\n", returnValue);
            ficlVmTextOut(vm, vm->Pad);
            ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
        }
    }
    else
    {
        ficlVmTextOut(vm, "Warning (system): nothing happened\n");
    }
    return;
}



/*
** Ficl add-in to load a text file and execute it...
** Cheesy, but illustrative.
** Line oriented... filename is newline (or NULL) delimited.
** Example:
**    load test.f
*/
static void ficlPrimitiveLoad(ficlVm *vm)
{
    char    buffer[LINE_BUFFER_SIZE];
    char    filename[LINE_BUFFER_SIZE];
    ficlCountedString *counted = (ficlCountedString *)filename;
    int     line = 0;
    FILE   *f;
    int     result = 0;
    ficlCell    oldSourceId;
	ficlString s;

    ficlVmGetString(vm, counted, '\n');

    if (FICL_COUNTED_STRING_GET_LENGTH(*counted) <= 0)
    {
        ficlVmTextOut(vm, "Warning (load): nothing happened\n");
        return;
    }

    /*
    ** get the file's size and make sure it exists 
    */

    f = fopen(FICL_COUNTED_STRING_GET_POINTER(*counted), "r");
    if (!f)
    {
        ficlVmTextOut(vm, "Unable to open file ");
        ficlVmTextOut(vm, FICL_COUNTED_STRING_GET_POINTER(*counted));
        ficlVmTextOut(vm, "\n");
        ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
    }

    oldSourceId = vm->sourceId;
    vm->sourceId.p = (void *)f;

    /* feed each line to ficlExec */
    while (fgets(buffer, sizeof(buffer), f))
    {
        int length = strlen(buffer) - 1;

        line++;
        if (length <= 0)
            continue;

        if (buffer[length] == '\n')
            buffer[length--] = '\0';
        else
            ficlVmThrowError(vm, "Error: file <%s> line %d is too long", FICL_COUNTED_STRING_GET_POINTER(*counted), line);

        FICL_STRING_SET_POINTER(s, buffer);
        FICL_STRING_SET_LENGTH(s, length + 1);
        result = ficlVmExecuteString(vm, s);
        /* handle "bye" in loaded files. --lch */
        switch (result)
        {
            case FICL_VM_STATUS_OUT_OF_TEXT:
            case FICL_VM_STATUS_USER_EXIT:
                break;

            default:
                vm->sourceId = oldSourceId;
                fclose(f);
                ficlVmThrowError(vm, "Error loading file <%s> line %d (%d)", FICL_COUNTED_STRING_GET_POINTER(*counted), line, result);
                break; 
        }
    }
    /*
    ** Pass an empty line with SOURCE-ID == -1 to flush
    ** any pending REFILLs (as required by FILE wordset)
    */
    vm->sourceId.i = -1;
	FICL_STRING_SET_FROM_CSTRING(s, "");
    ficlVmExecuteString(vm, s);

    vm->sourceId = oldSourceId;
    fclose(f);

    /* handle "bye" in loaded files. --lch */
    if (result == FICL_VM_STATUS_USER_EXIT)
        ficlVmThrow(vm, FICL_VM_STATUS_USER_EXIT);
    return;
}



/*
** Dump a tab delimited file that summarizes the contents of the
** dictionary hash table by hashcode...
*/
static void ficlPrimitiveSpewHash(ficlVm *vm)
{
    ficlHash *hash = ficlVmGetDictionary(vm)->forthWordlist;
    ficlWord *word;
    FILE *f;
    unsigned i;
    unsigned hashSize = hash->size;

    if (!ficlVmGetWordToPad(vm))
        ficlVmThrow(vm, FICL_VM_STATUS_OUT_OF_TEXT);

    f = fopen(vm->Pad, "w");
    if (!f)
    {
        ficlVmTextOut(vm, "unable to open file\n");
        return;
    }

    for (i = 0; i < hashSize; i++)
    {
        int n = 0;

        word = hash->table[i];
        while (word)
        {
            n++;
            word = word->link;
        }

        fprintf(f, "%d\t%d", i, n);

        word = hash->table[i];
        while (word)
        {
            fprintf(f, "\t%s", word->name);
            word = word->link;
        }

        fprintf(f, "\n");
    }

    fclose(f);
    return;
}

/* : BREAK ( -- ) */
static void ficlPrimitiveBreak(ficlVm *vm)
{
    vm->state = vm->state;
    return;
}

/* : PREPTERM ( 0|1 -- ) */
static void ficlPrimitivePrepterm(ficlVm *vm)
{
   int dir = ficlStackPopInteger(vm->dataStack);

   prepterm(dir);
   cbreak(!dir);
}

/* : KEY ( -- c ) */
static void ficlPrimitiveKey(ficlVm *vm)
{
	int ch;
	
    FICL_STACK_CHECK(vm->dataStack, 0, 1);

    do {
        ch = getkey();
    } while (ch > 255);
	
	ficlStackPushInteger(vm->dataStack, ch);
}

/* : KEY? ( -- flag ) */
static void ficlPrimitiveKeyQ(ficlVm *vm)
{
    int ret = has_key();

    FICL_STACK_CHECK(vm->dataStack, 0, 1);

    ficlStackPushInteger(vm->dataStack, FICL_BOOL(ret));
}

/* : EKEY ( -- code ) */
static void ficlPrimitiveEkey(ficlVm *vm)
{
    int ch = getkey();

    FICL_STACK_CHECK(vm->dataStack, 0, 1);

	ficlStackPushInteger(vm->dataStack, ch);
}

/* : UTIME ( -- sec ) */
static void ficlPrimitiveUTime(ficlVm *vm)
{
	time_t t = time(NULL);

    FICL_STACK_CHECK(vm->dataStack, 0, 1);

	ficlStackPushInteger(vm->dataStack, (ficlInteger)t);
}

/* : NOW ( -- sec min hour ) */
static void ficlPrimitiveNow(ficlVm *vm)
{
    struct tm *tim;
    time_t t;

    FICL_STACK_CHECK(vm->dataStack, 0, 3);

    t = time(NULL);
    tim = localtime(&t);
    ficlStackPushInteger(vm->dataStack, tim->tm_sec);
    ficlStackPushInteger(vm->dataStack, tim->tm_min);
    ficlStackPushInteger(vm->dataStack, tim->tm_hour);
}

/* : TODAY ( -- day mon year ) */
static void ficlPrimitiveToday(ficlVm *vm)
{
    struct tm *tim;
    time_t t;

    FICL_STACK_CHECK(vm->dataStack, 0, 3);

    t = time(NULL);
    tim = localtime(&t);
    ficlStackPushInteger(vm->dataStack, tim->tm_mday);
    ficlStackPushInteger(vm->dataStack, tim->tm_mon  + 1);
    ficlStackPushInteger(vm->dataStack, tim->tm_year + 1900);
}

/* : TIME-ZONE ( -- minutes ) */
static void ficlPrimitiveTimeZone(ficlVm *vm)
{
    long gmtoff;

    FICL_STACK_CHECK(vm->dataStack, 0, 1);

    gmtoff = ficlGetGMTOffset();
    ficlStackPushInteger(vm->dataStack, gmtoff / 60);
}

/* : DST? ( -- flag ) */
static void ficlPrimitiveDstQ(ficlVm *vm)
{
    struct tm *tim;
    time_t t;

    FICL_STACK_CHECK(vm->dataStack, 0, 1);

    t = time(NULL);
    tim = localtime(&t);
    ficlStackPushInteger(vm->dataStack, FICL_BOOL(tim->tm_isdst));
}

/* : (DLOPEN) ( ca u -- hnd ) */
static void ficlPrimitiveDlOpen(ficlVm *vm)
{
   void *ret, *addr;
   int length;
   char *path;

   FICL_STACK_CHECK(vm->dataStack, 2, 1);

   length = ficlStackPopInteger(vm->dataStack);
   addr  = (void *)ficlStackPopPointer(vm->dataStack);

   if (length) {
      path = (char*)malloc(length + 1);
      memcpy(path, addr, length);
      path[length] = 0;
   }
   else
      path = NULL;

   ret = dlopen(path, RTLD_NOW);
   if (length)
      free(path);

   ficlStackPushPointer(vm->dataStack, ret);
}

/* : (DLSYM) ( ca u hnd -- addr ) */
static void ficlPrimitiveDlSym(ficlVm *vm)
{
   void *ret, *hnd, *addr;
   int length;
   char *symname;

   FICL_STACK_CHECK(vm->dataStack, 3, 1);

   hnd   = (void *)ficlStackPopPointer(vm->dataStack);
   length = ficlStackPopInteger(vm->dataStack);
   addr  = (void *)ficlStackPopPointer(vm->dataStack);

   symname = (char*)malloc(length + 1);
   memcpy(symname, addr, length);
   symname[length] = 0;

   ret = dlsym(hnd, symname);
   free(symname);

   ficlStackPushPointer(vm->dataStack, ret);
}

/* : (C-CALL) ( argN ... arg1 N fn  -- ret ) */
static void ficlPrimitiveCCall(ficlVm *vm)
{
    long (*fn)();
    long ret, arg[8];
    int i, narg;

    FICL_STACK_CHECK(vm->dataStack, 2, 0);

	fn = (long (*)()) ficlStackPopPointer(vm->dataStack);
	narg = ficlStackPopInteger(vm->dataStack);
    /* fprintf(stderr,"fn = %p narg = %d\n", fn, narg); */

    FICL_STACK_CHECK(vm->dataStack, narg, 1);

	for (i = 0; i < narg; i++) {
		arg[i] = ficlStackPopInteger(vm->dataStack);
        /* fprintf(stderr,"arg[%d] = 0x%lx\n",i,arg[i]); */
	}

	switch (narg) {
	case 0: ret = (*fn)(); break;
	case 1: ret = (*fn)(arg[0]); break;
	case 2: ret = (*fn)(arg[0], arg[1]); break;
	case 3: ret = (*fn)(arg[0], arg[1], arg[2]); break;
	case 4: ret = (*fn)(arg[0], arg[1], arg[2], arg[3]); break;
	case 5: ret = (*fn)(arg[0], arg[1], arg[2], arg[3], arg[4]); break;
	case 6: ret = (*fn)(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]); break;
	case 7: ret = (*fn)(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6]); break;
	default:
		break;
	}

	ficlStackPushInteger(vm->dataStack, ret);
}

/* callbacks */
static int do_cb(int n, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8);

static int cb0(int n, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) { return do_cb(0, a1, a2, a3, a4, a5, a6, a7, a8); }
static int cb1(int n, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) { return do_cb(1, a1, a2, a3, a4, a5, a6, a7, a8); }
static int cb2(int n, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) { return do_cb(2, a1, a2, a3, a4, a5, a6, a7, a8); }
static int cb3(int n, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) { return do_cb(3, a1, a2, a3, a4, a5, a6, a7, a8); }
static int cb4(int n, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) { return do_cb(4, a1, a2, a3, a4, a5, a6, a7, a8); }
static int cb5(int n, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) { return do_cb(5, a1, a2, a3, a4, a5, a6, a7, a8); }
static int cb6(int n, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) { return do_cb(6, a1, a2, a3, a4, a5, a6, a7, a8); }
static int cb7(int n, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) { return do_cb(7, a1, a2, a3, a4, a5, a6, a7, a8); }

typedef struct {
	void *xt;		/* ficlWord */
	int  nargs;
	void *fn;
} C_CALLBACK;

static C_CALLBACK cb_tbl[] = {
	{0, -1, cb0},
	{0, -1, cb1},
	{0, -1, cb2},
	{0, -1, cb3},
	{0, -1, cb4},
	{0, -1, cb5},
	{0, -1, cb6},
	{0, -1, cb7}
};

static int do_cb(int n, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
	ficlVm *vm;
	ficlWord *xt;
	int ret;

	if (-1 == cb_tbl[n].nargs)
		return 0;

	vm = ficlVmCreate(NULL, 64, 64);

	switch (cb_tbl[n].nargs) {
	case 0: goto L0;
	case 1: goto L1;
	case 2: goto L2;
	case 3: goto L3;
	case 4: goto L4;
	case 5: goto L5;
	case 6: goto L6;
	case 7: goto L7;
	case 8: goto L8;
	}

L8: ficlStackPushInteger(vm->dataStack, a8);
L7: ficlStackPushInteger(vm->dataStack, a7);
L6: ficlStackPushInteger(vm->dataStack, a6);
L5: ficlStackPushInteger(vm->dataStack, a5);
L4: ficlStackPushInteger(vm->dataStack, a4);
L3: ficlStackPushInteger(vm->dataStack, a3);
L2: ficlStackPushInteger(vm->dataStack, a2);
L1: ficlStackPushInteger(vm->dataStack, a1);
L0:

	xt = (ficlWord*) cb_tbl[n].xt;
	ficlVmExecuteWord(vm, xt);
	ret = ficlStackPopInteger(vm->dataStack);

	ficlVmDestroy(vm);

	return ret;
}

/* : (CALLBACK) ( xt nargs cbidx -- ptr ) */
static void ficlPrimitiveCallback(ficlVm *vm)
{
	int idx;

    FICL_STACK_CHECK(vm->dataStack, 3, 1);

	idx = ficlStackPopInteger(vm->dataStack);
	cb_tbl[idx].nargs = ficlStackPopInteger(vm->dataStack);
	cb_tbl[idx].xt    = ficlStackPopPointer(vm->dataStack);

	ficlStackPushPointer(vm->dataStack, cb_tbl[idx].fn);
}


#if FICL_WANT_MULTITHREADED


static void* runThread(void *arg)
{
   ficlVm *vm = (ficlVm *) arg;

   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   ficlVmExecuteXT(vm, vm->runningWord); 
   return NULL;
}


static void checkReturnCode(ficlVm *vm, const char* where, int rc)
{
   char msg[128];

   if (rc)
   {
      sprintf(msg, "%s(): %s", where, strerror(rc));
      if (vm)
          ficlVmThrowError(vm, msg);
      fprintf(stderr, "%s\n", msg);
   }
}


void ficlVmSetThreadActive(ficlVm *vm, ficlUnsigned flag)
{
   vm->threadActive = flag;
   __sync_synchronize();
}


ficlUnsigned ficlVmIsThreadActive(ficlVm *vm)
{
   __sync_synchronize();
   return vm->threadActive;
}


void ficlVmTerminateThread(ficlVm *vm, ficlUnsigned doCancel)
{
   int  rc;
   void *ptr;

   if (FICL_TRUE == doCancel)
   {
      rc = pthread_cancel(vm->threadID);
      /* NB. may return 'no such process' */
      checkReturnCode(0, "ficlVmTerminateThread - pthread_cancel", rc);

      rc = pthread_join(vm->threadID, &ptr);
      checkReturnCode(0, "ficlVmTerminateThread - pthread_join", rc);
   }

   pthread_cond_destroy(&vm->threadSignal);
   pthread_mutex_destroy(&vm->threadStopMutex);
   vm->threadWake = FICL_FALSE;
   __sync_synchronize();
}


int recursiveMutexInit(pthread_mutex_t *mutex)
{
   pthread_mutexattr_t mutexAttr;
   int                 rc;

   pthread_mutexattr_init(&mutexAttr);
   pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE);
   rc = pthread_mutex_init(mutex, &mutexAttr);
   pthread_mutexattr_destroy(&mutexAttr);

   return rc;
}


/* : /TASK ( -- n ) */
static void ficlPrimitiveSlashTask(ficlVm *vm)
{
   FICL_STACK_CHECK(vm->dataStack, 0, 1);

   ficlStackPushInteger(vm->dataStack, sizeof(ficlVm));
}


/* : CONSTRUCT ( addr -- ) */
static void ficlPrimitiveConstruct(ficlVm *vm)
{
   ficlSystem *system = vm->callback.system;
   ficlVm     *otherVm;

   FICL_STACK_CHECK(vm->dataStack, 1, 0);

   otherVm = ficlStackPopPointer(vm->dataStack);
   if (ficlDictionaryIncludes(system->dictionary, otherVm))
      ficlVmThrowError(vm, "Error: task space is not ALLOCATEd.");

   memset(otherVm, 0, sizeof(ficlVm));
   otherVm = ficlVmCreate(otherVm, system->stackSize, system->stackSize);
   ficlSystemInitVm(system, otherVm);
}


/* : ACTIVATE ( xt addr -- ) */
static void ficlPrimitiveActivate(ficlVm *vm)
{
   ficlVm     *otherVm;
   ficlWord   *word;

   int        rc;
   const char *where;

   FICL_STACK_CHECK(vm->dataStack, 2, 0);

   otherVm = ficlStackPopPointer(vm->dataStack);
   word    = ficlStackPopPointer(vm->dataStack);

   ficlVmReset(otherVm);
   otherVm->runningWord = word;
   // newVm->ip = (ficlIp)(word->param);

   where = "ACTIVATE - pthread_mutex_init";
   rc = pthread_mutex_init(&otherVm->threadStopMutex, NULL);
   if (rc)
      goto errout;

   where = "ACTIVATE - pthread_cond_init";
   rc = pthread_cond_init(&otherVm->threadSignal, NULL);
   if (rc) 
   {
      pthread_mutex_destroy(&otherVm->threadStopMutex);
      goto errout;
   }
   otherVm->threadWake = FICL_FALSE;
   __sync_synchronize();

   where = "ACTIVATE - pthread_create";
   ficlVmSetThreadActive(otherVm, FICL_TRUE);
   rc = pthread_create(&otherVm->threadID, NULL, runThread, otherVm);
   if (rc)
   {
      pthread_mutex_destroy(&otherVm->threadStopMutex);
      pthread_cond_destroy(&otherVm->threadSignal);
      ficlVmSetThreadActive(otherVm, FICL_FALSE);
   }

errout:
   checkReturnCode(vm, where, rc);
}


/* : PAUSE ( -- ) */
static void ficlPrimitivePause(ficlVm *vm)
{
   if (FICL_TRUE == ficlVmIsThreadActive(vm))
      pthread_testcancel();

   sched_yield();
}

#define FICL_CAS(x,y,z)	__sync_bool_compare_and_swap(x,y,z)


/* : STOP ( -- ) */
static void ficlPrimitiveStop(ficlVm *vm)
{
   int rc;
   int i;

   if (FICL_FALSE == ficlVmIsThreadActive(vm))
      return;

   if (FICL_CAS(&vm->threadWake, FICL_TRUE, FICL_FALSE))
      return;

   rc = pthread_mutex_lock(&vm->threadStopMutex);
   checkReturnCode(0, "STOP - pthread_mutex_lock", rc);

   while (!FICL_CAS(&vm->threadWake, FICL_TRUE, FICL_FALSE))
   {
      rc = pthread_cond_wait(&vm->threadSignal, &vm->threadStopMutex);
      checkReturnCode(0, "STOP - pthread_cond_wait", rc);
   }

   rc = pthread_mutex_unlock(&vm->threadStopMutex);
   checkReturnCode(0, "STOP - pthread_mutex_unlock", rc);
}


/* : AWAKEN ( addr -- ) */
static void ficlPrimitiveAwaken(ficlVm *vm)
{
   ficlVm *otherVm;
   int    rc;

   FICL_STACK_CHECK(vm->dataStack, 1, 0);

   otherVm = ficlStackPopPointer(vm->dataStack);

   if (FICL_FALSE == ficlVmIsThreadActive(otherVm))
      return;

   rc = pthread_mutex_lock(&otherVm->threadStopMutex);
   checkReturnCode(0, "AWAKEN - pthread_mutex_lock", rc);

   otherVm->threadWake = FICL_TRUE;
   __sync_synchronize();

   rc = pthread_cond_signal(&otherVm->threadSignal);
   checkReturnCode(0, "AWAKEN - pthread_cond_signal", rc);

   rc = pthread_mutex_unlock(&otherVm->threadStopMutex);
   checkReturnCode(0, "AWAKEN - pthread_mutex_unlock", rc);
}


/* : HALT ( -- ) */
static void ficlPrimitiveHalt(ficlVm *vm)
{
   if (FICL_FALSE == ficlVmIsThreadActive(vm))
      return;

   ficlVmSetThreadActive(vm, FICL_FALSE);
   ficlVmTerminateThread(vm, FICL_FALSE);
   pthread_exit(NULL);
}


/* : TERMINATE ( addr -- ) */
static void ficlPrimitiveTerminate(ficlVm *vm)
{
   ficlVm *otherVm;
   int    rc;

   FICL_STACK_CHECK(vm->dataStack, 1, 0);

   otherVm = ficlStackPopPointer(vm->dataStack);

   if (FICL_FALSE == ficlVmIsThreadActive(otherVm))
      return;

   ficlVmSetThreadActive(otherVm, FICL_FALSE);
   ficlVmTerminateThread(otherVm, FICL_TRUE);
}


/* : HIS ( task addr1 -- addr2 ) */
static void ficlPrimitiveHis(ficlVm *vm)
{
   void   *addr1, *addr2;
   ficlVm *otherVm;

   FICL_STACK_CHECK(vm->dataStack, 2, 1);

   addr1    = ficlStackPopPointer(vm->dataStack);
   otherVm  = ficlStackPopPointer(vm->dataStack);

   addr2    = (void *)&otherVm->user[0] + (addr1 - (void *)&vm->user[0]);

   ficlStackPushPointer(vm->dataStack, addr2);
}


/* : /MUTEX ( -- n ) */
static void ficlPrimitiveSlashMutex(ficlVm *vm)
{
   FICL_STACK_CHECK(vm->dataStack, 0, 1);

   ficlStackPushInteger(vm->dataStack, sizeof(pthread_mutex_t));
}


/* : MUTEX-INIT ( addr -- ) */
static void ficlPrimitiveMutexInit(ficlVm *vm)
{
   pthread_mutex_t     *mutex;
   pthread_mutexattr_t mutexAttr;

   FICL_STACK_CHECK(vm->dataStack, 1, 0);

   mutex = ficlStackPopPointer(vm->dataStack);

   recursiveMutexInit(mutex);
}


/* : GET ( addr -- ) */
static void ficlPrimitiveMutexGet(ficlVm *vm)
{
   pthread_mutex_t *mutex;
   int             rc;

   FICL_STACK_CHECK(vm->dataStack, 1, 0);

   mutex = ficlStackPopPointer(vm->dataStack);

   rc = pthread_mutex_lock(mutex);
   checkReturnCode(vm, "pthread_mutex_lock", rc);
}


/* : RELEASE ( addr -- ) */
static void ficlPrimitiveMutexRelease(ficlVm *vm)
{
   pthread_mutex_t *mutex;
   int             rc;

   FICL_STACK_CHECK(vm->dataStack, 1, 0);

   mutex = ficlStackPopPointer(vm->dataStack);

   rc = pthread_mutex_unlock(mutex);
   checkReturnCode(vm, "pthread_mutex_unlock", rc);
}


/* : ATOMIC@ ( addr -- x ) */
static void ficlPrimitiveAtomicFetch(ficlVm *vm)
{
   ficlCell *cell;

   FICL_STACK_CHECK(vm->dataStack, 1, 1);

   cell = (ficlCell *) ficlStackPopPointer(vm->dataStack);

   __sync_synchronize();
   ficlStackPush(vm->dataStack, cell[0]);
}


/* : ATOMIC! ( x addr -- ) */
static void ficlPrimitiveAtomicStore(ficlVm *vm)
{
   ficlCell *cell, x;

   FICL_STACK_CHECK(vm->dataStack, 2, 0);

   cell = (ficlCell *) ficlStackPopPointer(vm->dataStack);
   x    = ficlStackPop(vm->dataStack);

   cell[0] = x;
   __sync_synchronize();
}


/* : ATOMIC-XCHG ( x1 a-addr -- x2 ) */
static void ficlPrimitiveAtomicXchg(ficlVm *vm)
{
   ficlUnsigned *addr, x1, x2;

   FICL_STACK_CHECK(vm->dataStack, 2, 1);

   addr = (ficlUnsigned *) ficlStackPopPointer(vm->dataStack);
   x1   = ficlStackPopUnsigned(vm->dataStack);

   x2   = __sync_lock_test_and_set(addr, x1);

   ficlStackPushUnsigned(vm->dataStack, x2);
}


/* : ATOMIC-CAS ( expected desired a-addr -- prev ) */
static void ficlPrimitiveAtomicCas(ficlVm *vm)
{
   ficlUnsigned *addr, desired, expected, prev;

   FICL_STACK_CHECK(vm->dataStack, 3, 1);

   addr     = (ficlUnsigned *) ficlStackPopPointer(vm->dataStack);
   desired  = ficlStackPopUnsigned(vm->dataStack);
   expected = ficlStackPopUnsigned(vm->dataStack);

   prev = __sync_val_compare_and_swap(addr, expected, desired);

   ficlStackPushUnsigned(vm->dataStack, prev);
}


/* : ATOMIC-OP ( xt a-addr -- ) */
static void ficlPrimitiveAtomicOp(ficlVm *vm)
{
   ficlCell *addr, old_value, new_value;
   ficlWord *xt;

   FICL_STACK_CHECK(vm->dataStack, 2, 0);

   addr = ficlStackPopPointer(vm->dataStack);
   xt   = ficlStackPopPointer(vm->dataStack);
   
   do
   {
      __sync_synchronize();
      old_value = addr[0];
      ficlStackPush(vm->dataStack, old_value);
      ficlVmExecuteWord(vm, xt);
      new_value = ficlStackPop(vm->dataStack);
   } while(!FICL_CAS((ficlUnsigned*)addr, old_value.u, new_value.u));
}


#endif /* FICL_WANT_MULTITHREADED */


/* : WFLIP ( uw1 -- uw2 ) */
static void ficlPrimitiveWFlip(ficlVm *vm)
{
    ficlUnsigned16 u;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    u = ficlStackPopUnsigned(vm->dataStack);
    ficlStackPushUnsigned(vm->dataStack, bswap16(u));
}

/* : QFLIP ( uq1 -- uq2 ) */
static void ficlPrimitiveQFlip(ficlVm *vm)
{
    ficlUnsigned32 u;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    u = ficlStackPopUnsigned(vm->dataStack);
    ficlStackPushUnsigned(vm->dataStack, bswap32(u));
}

#if defined(__LP64__) || defined(__MINGW64__)

/* : XFLIP ( ux1 -- ux2 ) */
static void ficlPrimitiveXFlip(ficlVm *vm)
{
    ficlUnsigned u;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    u = ficlStackPopUnsigned(vm->dataStack);
    ficlStackPushUnsigned(vm->dataStack, bswap64(u));
}

#define ficlPrimitiveFlip	ficlPrimitiveXFlip

#else

/* : XFLIP ( ud1 -- ud2 ) */
static void ficlPrimitiveXFlip(ficlVm *vm)
{
    ficl2Unsigned ud;
#if FICL_PLATFORM_HAS_2INTEGER
#else
    ficlUnsigned high, low, tmp;
#endif

    FICL_STACK_CHECK(vm->dataStack, 2, 2);

    ud = ficlStackPop2Unsigned(vm->dataStack);
#if FICL_PLATFORM_HAS_2INTEGER
    ud = bswap64(ud);
#else
    low  = FICL_2UNSIGNED_GET_LOW(ud);
    high = FICL_2UNSIGNED_GET_HIGH(ud);
    tmp  = bswap32(low);
    low  = bswap32(high);
    high = tmp;
    FICL_2UNSIGNED_SET(high, low, ud);
#endif
    ficlStackPush2Unsigned(vm->dataStack, ud);
}

#define ficlPrimitiveFlip	ficlPrimitiveQFlip

#endif

/* : STICK ( xu ... x0 x u -- x xu-1 ... xu ) */
static void ficlPrimitiveStick(ficlVm *vm)
{
    ficlCell x;
    ficlInteger n;

    FICL_STACK_CHECK(vm->dataStack, 2, 0);

    n = ficlStackPopInteger(vm->dataStack);
    x = ficlStackPop(vm->dataStack);

    if (n < 0)
        return;

    FICL_STACK_CHECK(vm->dataStack, n + 1, n + 1);

    ficlStackStore(vm->dataStack, n, x);
}

/* : WORDKIND? ( xt -- kind ) */
static void ficlPrimitiveWordKindQ(ficlVm *vm)
{
    ficlWordKind kind;
    ficlWord *word;
    ficlInteger ret = FICL_FALSE;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    word = (ficlWord *)ficlStackPopPointer(vm->dataStack);
    kind = ficlWordClassify(word);

    ficlStackPushInteger(vm->dataStack, kind);
}

/* : CHARS+ ( addr1 u -- addr2 ) */
static void ficlPrimitiveCharsPlus(ficlVm *vm)
{
    ficlInteger n;
    void *addr;

    FICL_STACK_CHECK(vm->dataStack, 2, 1);

    n    = ficlStackPopInteger(vm->dataStack);
    addr = ficlStackPopPointer(vm->dataStack);
    ficlStackPushPointer(vm->dataStack, addr + sizeof(char) * n);
}

/* : CELLS+ ( addr1 u -- addr2 ) */
static void ficlPrimitiveCellsPlus(ficlVm *vm)
{
    ficlInteger n;
    void *addr;

    FICL_STACK_CHECK(vm->dataStack, 2, 1);

    n    = ficlStackPopInteger(vm->dataStack);
    addr = ficlStackPopPointer(vm->dataStack);
    ficlStackPushPointer(vm->dataStack, addr + sizeof(ficlCell) * n);
}

/* : ERRNO ( -- #error ) */
static void ficlPrimitiveErrno(ficlVm *vm)
{
    FICL_STACK_CHECK(vm->dataStack, 0, 1);

    ficlStackPushInteger(vm->dataStack, (ficlInteger) errno);
}

/* : WHY ( #error -- caddr u ) */
static void ficlPrimitiveWhy(ficlVm *vm)
{
    char *ptr;
    int  n;

    FICL_STACK_CHECK(vm->dataStack, 1, 2);

    n = (int) ficlStackPopInteger(vm->dataStack);
    ptr = strerror(n);
    ficlStackPushPointer(vm->dataStack, ptr);
    ficlStackPushInteger(vm->dataStack, strlen(ptr));
}

/* : @@ ( addr -- n ) */
static void ficlPrimitiveFetchFetch(ficlVm *vm)
{
    ficlCell cell, *ptr;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    ptr  = ficlStackPopPointer(vm->dataStack);
    cell = ptr[0];
    ptr  = cell.p;
    ficlStackPush(vm->dataStack, ptr[0]);
}

/* : @! ( n addr -- ) */
static void ficlPrimitiveFetchStore(ficlVm *vm)
{
    ficlCell n, cell, *addr;

    FICL_STACK_CHECK(vm->dataStack, 2, 0);

    addr = ficlStackPopPointer(vm->dataStack);
    n    = ficlStackPop(vm->dataStack);
    cell = addr[0];
    addr = cell.p;
    addr[0] = n;
}

#define P1  0
#define P2  1
#define S1  2
#define S2  3
/* : fuzzify ( memfns fuzzyins len in -- ) */
static void ficlPrimitiveFuzzify(ficlVm *vm)
{
    unsigned char input;
    unsigned char i, len;
    unsigned char *fuzzyIns, *memFns;
    int delta1, delta2, d12n, grade1, grade2;
    unsigned char grade;

    FICL_STACK_CHECK(vm->dataStack, 4, 0);

    input= ficlStackPopUnsigned(vm->dataStack);
    len  = ficlStackPopUnsigned(vm->dataStack);
    fuzzyIns = ficlStackPopPointer(vm->dataStack);
    memFns   = ficlStackPopPointer(vm->dataStack);

    for (i = 0; i < len; i++)
    {
        delta1 = input - memFns[P1];
        delta2 = memFns[P2] - input;
        d12n = (delta1 < 0) || (delta2 < 0);
        if (d12n) grade1 = 0; else grade1 = memFns[S1] * delta1;
        if (d12n) grade2 = 0; else grade2 = memFns[S2] * delta2;
        if (((memFns[S2] == 0) || (grade2 > 0xFF)) && !d12n) grade = 0xFF;
        else grade = grade2;
        if (((memFns[S1] == 0) || (grade1 > 0xFF)) && !d12n) grade = grade;
        else grade = grade1;
        fuzzyIns[i] = grade;
        memFns += 4;
    }
}
#undef P1
#undef P2
#undef S1
#undef S2

/* : defuzzify ( fuzzyouts singletons len -- out ) */
static void ficlPrimitiveDefuzzify(ficlVm *vm)
{
    unsigned char *fuzzyOuts, *singletons;
    unsigned char i, len;
    unsigned SiFi, Fi, out; 

    FICL_STACK_CHECK(vm->dataStack, 3, 1);

    len = ficlStackPopUnsigned(vm->dataStack);
    singletons = ficlStackPopPointer(vm->dataStack);
    fuzzyOuts  = ficlStackPopPointer(vm->dataStack);

    SiFi = 0; Fi = 0;
    for (i = 0; i < len; i++)
    {
        SiFi += singletons[i] * fuzzyOuts[i];
          Fi += fuzzyOuts[i];
    }

    out = SiFi / (ficlFloat) Fi;
    ficlStackPushUnsigned(vm->dataStack, out);
}

/* : rulez ( rules fuzzyvars -- ) */
static void ficlPrimitiveRulez(ficlVm *vm)
{
    unsigned char *rules, *fuzzyVars;
    unsigned char i, c, len, grade;
    int consequent;

    FICL_STACK_CHECK(vm->dataStack, 3, 0);

    fuzzyVars = ficlStackPopPointer(vm->dataStack);
    rules     = ficlStackPopPointer(vm->dataStack);

    c = *rules++; grade = 0xFF; consequent = 0;
    while (0xFF != c)
    {
        if (0xFE == c) {
            consequent = 1 - consequent;
            if (!consequent) grade = 0xFF;
        } else {
            if (!consequent)
                grade = FICL_MIN(grade, fuzzyVars[c]);
            else
                fuzzyVars[c] = FICL_MAX(grade, fuzzyVars[c]);
        }
        c = *rules++;
    }
}

/* : wrulez ( rules weights fuzzyvars -- ) */
static void ficlPrimitiveWRulez(ficlVm *vm)
{
    unsigned char *rules, *weights, *fuzzyVars;
    unsigned char c, grade;
    int consequent;

    FICL_STACK_CHECK(vm->dataStack, 3, 0);

    fuzzyVars = ficlStackPopPointer(vm->dataStack);
    weights   = ficlStackPopPointer(vm->dataStack);
    rules     = ficlStackPopPointer(vm->dataStack);

    c = *rules++; grade = 0xFF; consequent = 0;
    while (0xFF != c)
    {
        if (0xFE == c)
        {
            consequent = 1 - consequent;
            if (!consequent) 
                grade = 0xFF;
            else
                grade = (grade * *weights++) >> 8;
        } else
        {
            if (!consequent)
                grade = FICL_MIN(grade, fuzzyVars[c]);
            else
                fuzzyVars[c] = FICL_MAX(grade, fuzzyVars[c]);
        }
        c = *rules++;
    }
}

/* : bit? ( ix c-addr -- ff ) */
static void ficlPrimitiveBitQ(ficlVm *vm)
{
    unsigned char *addr, ret;
    int ix;

    FICL_STACK_CHECK(vm->dataStack, 2, 1);

    addr = ficlStackPopPointer(vm->dataStack);
    ix   = ficlStackPopInteger(vm->dataStack);

    ret = addr[ix >> 3] & (1U << (ix & 7));

    ficlStackPushInteger(vm->dataStack, FICL_BOOL(ret));
}

/* : bit-set ( ix c-addr -- ) */
static void ficlPrimitiveBitSet(ficlVm *vm)
{
    unsigned char *addr;
    int ix;

    FICL_STACK_CHECK(vm->dataStack, 2, 0);

    addr = ficlStackPopPointer(vm->dataStack);
    ix   = ficlStackPopInteger(vm->dataStack);

    addr[ix >> 3] |= (1U << (ix & 7));
}

/* : bit-reset ( ix c-addr -- ) */
static void ficlPrimitiveBitReset(ficlVm *vm)
{
    unsigned char *addr;
    int ix;

    FICL_STACK_CHECK(vm->dataStack, 2, 0);

    addr = ficlStackPopPointer(vm->dataStack);
    ix   = ficlStackPopInteger(vm->dataStack);

    addr[ix >> 3] &= ~(1U << (ix & 7));
}

/* : bit-flip ( ix c-addr -- ) */
static void ficlPrimitiveBitFlip(ficlVm *vm)
{
    unsigned char *addr;
    int ix;

    FICL_STACK_CHECK(vm->dataStack, 2, 1);

    addr = ficlStackPopPointer(vm->dataStack);
    ix   = ficlStackPopInteger(vm->dataStack);

    addr[ix >> 3] ^= (1U << (ix & 7));
}

/* : isqrt ( u -- u^0.5 ) */
static void ficlPrimitiveISqrt(ficlVm *vm)
{
    ficlUnsigned s, x0, x1;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);
    s = ficlStackPopUnsigned(vm->dataStack);
    if (s > 1)
    {
        x0 = s >> 1;
        x1 = (x0 + s / x0) >> 1;
        while (x1 < x0)
        {
            x0 = x1;
            x1 = (x0 + s / x0) >> 1;
        }
        s = x0;
    }
    ficlStackPushUnsigned(vm->dataStack, s);
}


#define addPrimitive(d,nm,fn) \
   ficlDictionarySetPrimitive(d,nm,fn,FICL_WORD_DEFAULT)

void ficlSystemCompileExtras(ficlSystem *system)
{
    ficlDictionary *dictionary = ficlSystemGetDictionary(system);
    ficlDictionary *environment = ficlSystemGetEnvironment(system);

    FICL_SYSTEM_ASSERT(system, dictionary);
    FICL_SYSTEM_ASSERT(system, environment);

    addPrimitive(dictionary, "break",    ficlPrimitiveBreak);
    addPrimitive(dictionary, "load",     ficlPrimitiveLoad);
    addPrimitive(dictionary, "spewhash", ficlPrimitiveSpewHash);
    addPrimitive(dictionary, "system",   ficlPrimitiveSystem);

#if !defined(FICL_ANSI) || defined(__MINGW32__)
    addPrimitive(dictionary, "clock",    ficlPrimitiveClock);
    ficlDictionarySetConstant(dictionary,  "clocks/sec", CLOCKS_PER_SEC);
    addPrimitive(dictionary, "pwd",      ficlPrimitiveGetCwd);
    addPrimitive(dictionary, "cd",       ficlPrimitiveChDir);
	addPrimitive(dictionary, "get-msecs", ficlPrimitiveGetMSecs);
#endif /* FICL_ANSI */

    ficlDictionarySetConstant(environment,  "ficl-os",  FICL_OS);

    addPrimitive(dictionary, "key",      ficlPrimitiveKey);
    addPrimitive(dictionary, "key?",     ficlPrimitiveKeyQ);
    addPrimitive(dictionary, "ekey",     ficlPrimitiveEkey);
    addPrimitive(dictionary, "utime",    ficlPrimitiveUTime);
    addPrimitive(dictionary, "prepterm", ficlPrimitivePrepterm);
    addPrimitive(dictionary, "(dlopen)", ficlPrimitiveDlOpen);
    addPrimitive(dictionary, "(dlsym)",  ficlPrimitiveDlSym);
    addPrimitive(dictionary, "(c-call)", 	 ficlPrimitiveCCall);
    addPrimitive(dictionary, "(callback)", ficlPrimitiveCallback);
    addPrimitive(dictionary, "now", 	 ficlPrimitiveNow);
    addPrimitive(dictionary, "today", 	 ficlPrimitiveToday);
    addPrimitive(dictionary, "time-zone", ficlPrimitiveTimeZone);
    addPrimitive(dictionary, "dst?", 	 ficlPrimitiveDstQ);

#if FICL_WANT_MULTITHREADED
    addPrimitive(dictionary, "/task",     ficlPrimitiveSlashTask);
    addPrimitive(dictionary, "construct", ficlPrimitiveConstruct);
    addPrimitive(dictionary, "activate",  ficlPrimitiveActivate);
    addPrimitive(dictionary, "awaken",    ficlPrimitiveAwaken);
    addPrimitive(dictionary, "terminate", ficlPrimitiveTerminate);
    addPrimitive(dictionary, "pause",     ficlPrimitivePause);
    addPrimitive(dictionary, "stop",      ficlPrimitiveStop);
    addPrimitive(dictionary, "halt", 	  ficlPrimitiveHalt);

    addPrimitive(dictionary, "his",       ficlPrimitiveHis);

    addPrimitive(dictionary, "atomic@",   ficlPrimitiveAtomicFetch);
    addPrimitive(dictionary, "atomic!",   ficlPrimitiveAtomicStore);
    addPrimitive(dictionary, "atomic-xchg", ficlPrimitiveAtomicXchg);
    addPrimitive(dictionary, "atomic-cas", ficlPrimitiveAtomicCas);
    addPrimitive(dictionary, "atomic-op", ficlPrimitiveAtomicOp);

    addPrimitive(dictionary, "/mutex", 	  ficlPrimitiveSlashMutex);
    addPrimitive(dictionary, "mutex-init", ficlPrimitiveMutexInit);
    addPrimitive(dictionary, "get",	      ficlPrimitiveMutexGet);
    addPrimitive(dictionary, "release",	  ficlPrimitiveMutexRelease);
#endif

    addPrimitive(dictionary, "wflip",   ficlPrimitiveWFlip);
    addPrimitive(dictionary, "qflip",   ficlPrimitiveQFlip);
    addPrimitive(dictionary, "xflip",   ficlPrimitiveXFlip);
    addPrimitive(dictionary, "flip",    ficlPrimitiveFlip);

    addPrimitive(dictionary, "chars+",  ficlPrimitiveCharsPlus);
    addPrimitive(dictionary, "cells+",  ficlPrimitiveCellsPlus);
    addPrimitive(dictionary, "errno",   ficlPrimitiveErrno);
    addPrimitive(dictionary, "why",     ficlPrimitiveWhy);

    addPrimitive(dictionary, "stick",     ficlPrimitiveStick);
    addPrimitive(dictionary, "wordkind?", ficlPrimitiveWordKindQ);

    addPrimitive(dictionary, "@@", ficlPrimitiveFetchFetch);
    addPrimitive(dictionary, "@!", ficlPrimitiveFetchStore);

    addPrimitive(dictionary, "fuzzify", ficlPrimitiveFuzzify);
    addPrimitive(dictionary, "rulez",   ficlPrimitiveRulez);
    addPrimitive(dictionary, "wrulez",  ficlPrimitiveWRulez);
    addPrimitive(dictionary, "defuzzify", ficlPrimitiveDefuzzify);

    addPrimitive(dictionary, "bit?", ficlPrimitiveBitQ);
    addPrimitive(dictionary, "bit-set", ficlPrimitiveBitSet);
    addPrimitive(dictionary, "bit-reset", ficlPrimitiveBitReset);
    addPrimitive(dictionary, "bit-flip", ficlPrimitiveBitFlip);

    addPrimitive(dictionary, "isqrt", ficlPrimitiveISqrt);

    return;
}

/* vim: set ts=4 sw=4 et: */
