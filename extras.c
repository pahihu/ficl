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
    ficlCountedString *counted = (ficlCountedString *)vm->pad;
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
    ficlCountedString *counted = (ficlCountedString *)vm->pad;

    ficlVmGetString(vm, counted, '\n');
    if (FICL_COUNTED_STRING_GET_LENGTH(*counted) > 0)
    {
        int returnValue = system(FICL_COUNTED_STRING_GET_POINTER(*counted));
        if (returnValue)
        {
            sprintf(vm->pad, "System call returned %d\n", returnValue);
            ficlVmTextOut(vm, vm->pad);
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
#define BUFFER_SIZE 256
static void ficlPrimitiveLoad(ficlVm *vm)
{
    char    buffer[BUFFER_SIZE];
    char    filename[BUFFER_SIZE];
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
    while (fgets(buffer, BUFFER_SIZE, f))
    {
        int length = strlen(buffer) - 1;

        line++;
        if (length <= 0)
            continue;

        if (buffer[length] == '\n')
            buffer[length--] = '\0';

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

    f = fopen(vm->pad, "w");
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
	
   do {
      ch = getkey();
   } while (ch > 255);
	
	ficlStackPushInteger(vm->dataStack, ch);
}

/* : KEY? ( -- flag ) */
static void ficlPrimitiveKeyQ(ficlVm *vm)
{
   int ret = has_key();
	ficlStackPushInteger(vm->dataStack, ret ? FICL_TRUE : FICL_FALSE);
}

/* : EKEY ( -- code ) */
static void ficlPrimitiveEkey(ficlVm *vm)
{
   int ch = getkey();
	ficlStackPushInteger(vm->dataStack, ch);
}

/* : UTIME ( -- sec ) */
static void ficlPrimitiveUTime(ficlVm *vm)
{
	time_t t = time(NULL);

	ficlStackPushInteger(vm->dataStack, (ficlInteger)t);
}

/* : (DLOPEN) ( ca u -- hnd ) */
static void ficlPrimitiveDlOpen(ficlVm *vm)
{
   void *ret;
   int  length = ficlStackPopInteger(vm->dataStack);
   void *addr  = (void *)ficlStackPopPointer(vm->dataStack);

   char *path = (char*)malloc(length + 1);
   memcpy(path, addr, length);
   path[length] = 0;

   ret = dlopen(path, RTLD_NOW);
   free(path);

   ficlStackPushPointer(vm->dataStack, ret);
}

/* : (DLSYM) ( ca u hnd -- addr ) */
static void ficlPrimitiveDlSym(ficlVm *vm)
{
   void *ret;
   void *hnd   = (void *)ficlStackPopPointer(vm->dataStack);
   int  length = ficlStackPopInteger(vm->dataStack);
   void *addr  = (void *)ficlStackPopPointer(vm->dataStack);

   char *symname = (char*)malloc(length + 1);
   memcpy(symname, addr, length);
   symname[length] = 0;

   ret = dlsym(hnd, symname);
   free(symname);

   ficlStackPushPointer(vm->dataStack, ret);
}

/* : (C-CALL) ( argN ... arg1 fn  -- ret ) */
static void ficlPrimitiveCCall(ficlVm *vm)
{
	int (*fn)();
	int narg;
	int i, arg[8];
	int ret;

	fn = (int (*)()) ficlStackPopPointer(vm->dataStack);
	narg = ficlStackPopInteger(vm->dataStack);
	for (i = 0; i < narg; i++) {
		arg[i] = ficlStackPopInteger(vm->dataStack);
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

	idx = ficlStackPopInteger(vm->dataStack);
	cb_tbl[idx].nargs = ficlStackPopInteger(vm->dataStack);
	cb_tbl[idx].xt    = ficlStackPopPointer(vm->dataStack);

	ficlStackPushPointer(vm->dataStack, cb_tbl[idx].fn);
}

#if 0
static ficlVm *gShadowVm  = NULL;
static ficlVm *gRunningVm = NULL;

static void ResetVmPointers(ficlVm *vm)
{
   vm->callback.vm     = vm;
   vm->dataStack->vm   = vm;
   vm->returnStack->vm = vm;
   vm->floatStack->vm  = vm;
}

static void ficlPrimitiveInitMulti(ficlVm *vm)
{
   ficlSystem *system = vm->callback.system;
   ficlVm     *pList, *pointedToVm;

   if (gShadowVm)
      return;

   gShadowVm = ficlSystemCreateVm(system);
   system->vmList = system->vmList->link;

   pList = system->vmList;
   pointedToVm = 0;
   for ( ; pList ; pList = pList->link) {
      if (vm == pList->link) {
         pointedToVm = pList;
         break;
      }
   }

   memcpy(gShadowVm, vm, sizeof(ficlVm));
   ResetVmPointers(gShadowVm);

   if (pointedToVm)
      pointedToVm->link = gShadowVm;

   if (vm == system->vmList)
      system->vmList = gShadowVm;

   gRunningVm = gShadowVm;
}

/* : (PROCESS) (  -- 'vm ) */
static void ficlPrimitiveProcess(ficlVm *vm)
{
   static int gProcessID = 1;
   ficlSystem *system = vm->callback.system;
   ficlVm     *newVm;

   newVm  = ficlSystemCreateVm(system);
   newVm->user[0].i = gProcessID++;
   if (32768 == gProcessID) gProcessID = 1;
   ficlStackPushPointer(vm->dataStack, newVm);
}

/* : (RUN) ( xt 'vm -- ) */
static void ficlPrimitiveRun(ficlVm *vm)
{
   ficlWord *word;
   ficlVm   *otherVm;

   otherVm = ficlStackPopPointer(vm->dataStack);
   word    = ficlStackPopPointer(vm->dataStack);
   otherVm->runningWord = word;
   otherVm->ip = (ficlIp)(word->param);
}

#define SWAP(T,a,b)  { T _tmp=a; a=b; b=_tmp; }

static void ficlPrimitivePS(ficlVm *vm)
{
   int i;
   ficlSystem *system = vm->callback.system;
   ficlVm     *pList  = system->vmList;

   fprintf(stderr,"\npList = ");
   for (i = 0; pList; i++) {
      fprintf(stderr,"%ld=%p ",pList->user[0].i,pList);
      pList = pList->link;
   }
}

/* : (PAUSE) ( -- ) */
static void ficlPrimitivePause(ficlVm *vm)
{
   ficlSystem *system = vm->callback.system;
   ficlVm     *nextVm, *oldLink;

   if (NULL == gRunningVm)
      ficlVmThrowError(vm, "Error: multi-tasking not initialized.");

   // ficlPrimitivePS(vm);

   /* select next VM: if end of list, select head */
   // fprintf(stderr,"\n-I-PAUSE:  pid is #%ld",vm->user[0].i);
   nextVm = gRunningVm->link;
   if (NULL == nextVm) {
      // fprintf(stderr,"\n-I-PAUSE: next is head");
      nextVm = system->vmList;
   }

   /* same as running VM, do nothing */
   if (nextVm == gRunningVm) {
      // fprintf(stderr,"\n-I-PAUSE: only one, skip");
      ficlStackPushInteger(vm->dataStack, FICL_FALSE);
      return;
   }

   // fprintf(stderr,"\n-I-PAUSE: next is #%ld",nextVm->user[0].i);
   ficlStackPushInteger(vm->dataStack, FICL_TRUE);

   /* save state */
   oldLink = gRunningVm->link;
   memcpy(gRunningVm, vm, sizeof(ficlVm));
   gRunningVm->link = oldLink;
   ResetVmPointers(gRunningVm);

   /* load state of next VM */
   memcpy(vm, nextVm, sizeof(ficlVm));
   ResetVmPointers(vm);

   gRunningVm = nextVm;
   // ficlStackPushInteger(vm->dataStack, FICL_TRUE);

   // ficlPrimitivePS(vm);
}

/* : (STOP) ( -- ) */
static void ficlPrimitiveStop(ficlVm *vm)
{
   ficlSystem *system = vm->callback.system;
   ficlVm     *pList;
   int         pid    = vm->user[0].i;

   /* schedule next VM, if it is the same, then do nothing */
   /* i.e. cannot stop root process */
   // fprintf(stderr,"\n-I-STOP: current is #%d (%p)",pid,vm);
   ficlPrimitivePause(vm);
   // fprintf(stderr,"\n-I-STOP:    next is #%ld (%p)",vm->user[0].i,vm);
   if (pid == vm->user[0].i) {
      ficlStackPopInteger(vm->dataStack);
      return;
   }

   /* search process by pid */
   for (pList = system->vmList; pList; pList = pList->link) {
      // fprintf(stderr,"\n-I-STOP: check #%ld (%p)",pList->user[0].i,pList);
      if (pid == pList->user[0].i)
         break;
   }

   if (pList) {
      // fprintf(stderr,"\n-I-STOP: destroy (%p)",pList);
      ficlSystemDestroyVm(pList);
   }
}
#endif

#if FICL_WANT_MULTITHREADED


static void* runThread(void *arg)
{
   ficlVm *vm = (ficlVm *) arg;

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
      checkReturnCode(0, "pthread_cancel", rc);

      // rc = pthread_join(vm->threadID, &ptr);
      // checkReturnCode(0, "pthread_join", rc);
   }

   pthread_cond_destroy(&vm->threadAwake);
   pthread_mutex_destroy(&vm->threadStopMutex);
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

   // memset(otherVm, 0, sizeof(ficlVm));
   otherVm = ficlVmCreate(otherVm, system->stackSize, system->stackSize);
   otherVm->static_alloc = FICL_TRUE;
   ficlSystemInitVm(system, otherVm);
}


/* : ACTIVATE ( xt addr -- ) */
static void ficlPrimitiveActivate(ficlVm *vm)
{
   ficlVm     *otherVm;
   ficlWord   *word;

   int        rc;
   const char *where;
   pthread_attr_t threadAttr;

   FICL_STACK_CHECK(vm->dataStack, 2, 0);
   otherVm = ficlStackPopPointer(vm->dataStack);
   word    = ficlStackPopPointer(vm->dataStack);

   otherVm->runningWord = word;
   // newVm->ip = (ficlIp)(word->param);

   where = "pthread_mutex_init";
   rc = pthread_mutex_init(&otherVm->threadStopMutex, NULL);
   if (rc)
      goto errout;

   where = "pthread_cond_init";
   rc = pthread_cond_init(&otherVm->threadAwake, NULL);
   if (rc) 
   {
      pthread_mutex_destroy(&otherVm->threadStopMutex);
      goto errout;
   }

   pthread_attr_init(&threadAttr);
   pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

   where = "pthread_create";
   ficlVmSetThreadActive(otherVm, FICL_TRUE);
   rc = pthread_create(&otherVm->threadID, &threadAttr, runThread, otherVm);
   pthread_attr_destroy(&threadAttr);
   if (rc)
   {
      pthread_mutex_destroy(&otherVm->threadStopMutex);
      pthread_cond_destroy(&otherVm->threadAwake);
      ficlVmSetThreadActive(otherVm, FICL_FALSE);
   }

errout:
   checkReturnCode(vm, where, rc);
}


/* : PAUSE ( -- ) */
static void ficlPrimitivePause(ficlVm *vm)
{
   if (FICL_TRUE == ficlVmIsThreadActive(vm))
   {
      pthread_testcancel();
   }
   sched_yield();
}


/* : STOP ( -- ) */
static void ficlPrimitiveStop(ficlVm *vm)
{
   int rc;

   if (FICL_TRUE == ficlVmIsThreadActive(vm))
   {
      rc = pthread_mutex_lock(&vm->threadStopMutex);
      checkReturnCode(vm, "pthread_mutex_lock", rc);

      rc = pthread_cond_wait(&vm->threadAwake, &vm->threadStopMutex);
      checkReturnCode(vm, "pthread_cond_wait", rc);

      rc = pthread_mutex_unlock(&vm->threadStopMutex);
      checkReturnCode(vm, "pthread_cond_wait", rc);
   }
}


/* : AWAKE ( addr -- ) */
static void ficlPrimitiveAwake(ficlVm *vm)
{
   ficlVm *otherVm;
   int    rc;

   FICL_STACK_CHECK(vm->dataStack, 1, 0);
   otherVm = ficlStackPopPointer(vm->dataStack);

   if (FICL_TRUE == ficlVmIsThreadActive(otherVm))
   {
      rc = pthread_cond_signal(&otherVm->threadAwake);
      checkReturnCode(vm, "pthread_cond_signal", rc);
   }
}


/* : HALT ( -- ) */
static void ficlPrimitiveHalt(ficlVm *vm)
{
   if (FICL_TRUE == ficlVmIsThreadActive(vm))
   {
      ficlVmSetThreadActive(vm, FICL_FALSE);
      ficlVmTerminateThread(vm, FICL_FALSE);
      pthread_exit(NULL);
   }
}


/* : TERMINATE ( addr -- ) */
static void ficlPrimitiveTerminate(ficlVm *vm)
{
   ficlVm *otherVm;
   int    rc;

   FICL_STACK_CHECK(vm->dataStack, 1, 0);
   otherVm = ficlStackPopPointer(vm->dataStack);

   if (FICL_TRUE == ficlVmIsThreadActive(otherVm))
   {
      ficlVmSetThreadActive(otherVm, FICL_FALSE);
      ficlVmTerminateThread(otherVm, FICL_TRUE);
   }
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


#endif /* FICL_WANT_MULTITHREADED */


/* : BSWAP16 ( uw1 -- uw2 ) */
static void ficlPrimitiveBSwap16(ficlVm *vm)
{
    ficlUnsigned16 u;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    u = ficlStackPopUnsigned(vm->dataStack);
    ficlStackPushUnsigned(vm->dataStack, bswap16(u));
}

/* : BSWAP32 ( u1 -- u2 ) */
static void ficlPrimitiveBSwap32(ficlVm *vm)
{
    ficlUnsigned32 u;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    u = ficlStackPopUnsigned(vm->dataStack);
    ficlStackPushUnsigned(vm->dataStack, bswap32(u));
}

#if defined(__LP64__) || defined(__MINGW64__)

/* : BSWAP64 ( ud1 -- ud2 ) */
static void ficlPrimitiveBSwap64(ficlVm *vm)
{
    ficlUnsigned u;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    u = ficlStackPopUnsigned(vm->dataStack);
    ficlStackPushUnsigned(vm->dataStack, bswap64(u));
}

#else

/* : BSWAP64 ( ud1 -- ud2 ) */
static void ficlPrimitiveBSwap64(ficlVm *vm)
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

#endif

#define addPrimitive(d,nm,fn) \
   ficlDictionarySetPrimitive(d,nm,fn,FICL_WORD_DEFAULT)

void ficlSystemCompileExtras(ficlSystem *system)
{
    ficlDictionary *dictionary = ficlSystemGetDictionary(system);

    addPrimitive(dictionary, "break",    ficlPrimitiveBreak);
    addPrimitive(dictionary, "load",     ficlPrimitiveLoad);
    addPrimitive(dictionary, "spewhash", ficlPrimitiveSpewHash);
    addPrimitive(dictionary, "system",   ficlPrimitiveSystem);

#if !defined(FICL_ANSI) || defined(__MINGW32__)
    addPrimitive(dictionary, "clock",    ficlPrimitiveClock);
    ficlDictionarySetConstant(dictionary,  "clocks/sec", CLOCKS_PER_SEC);
    addPrimitive(dictionary, "pwd",      ficlPrimitiveGetCwd);
    addPrimitive(dictionary, "cd",       ficlPrimitiveChDir);
	 addPrimitive(dictionary, "get-msecs",ficlPrimitiveGetMSecs);
#endif /* FICL_ANSI */

    ficlDictionarySetConstant(dictionary,  "ficl-os",  FICL_OS);

    addPrimitive(dictionary, "key",      ficlPrimitiveKey);
    addPrimitive(dictionary, "key?",     ficlPrimitiveKeyQ);
    addPrimitive(dictionary, "ekey",     ficlPrimitiveEkey);
    addPrimitive(dictionary, "utime",    ficlPrimitiveUTime);
    addPrimitive(dictionary, "prepterm", ficlPrimitivePrepterm);
    addPrimitive(dictionary, "(dlopen)", ficlPrimitiveDlOpen);
    addPrimitive(dictionary, "(dlsym)",  ficlPrimitiveDlSym);
    addPrimitive(dictionary, "(c-call)", 	 ficlPrimitiveCCall);
    addPrimitive(dictionary, "(callback)", ficlPrimitiveCallback);

#if 0
    addPrimitive(dictionary, "(init-multi)", ficlPrimitiveInitMulti);
    addPrimitive(dictionary, "(process)", ficlPrimitiveProcess);
    addPrimitive(dictionary, "(run)",     ficlPrimitiveRun);
    addPrimitive(dictionary, "(stop)",    ficlPrimitiveStop);
    addPrimitive(dictionary, "(pause)",   ficlPrimitivePause);
#endif

#if FICL_WANT_MULTITHREADED
    addPrimitive(dictionary, "/task",     ficlPrimitiveSlashTask);
    addPrimitive(dictionary, "construct", ficlPrimitiveConstruct);
    addPrimitive(dictionary, "activate",  ficlPrimitiveActivate);
    addPrimitive(dictionary, "awake",     ficlPrimitiveAwake);
    addPrimitive(dictionary, "terminate", ficlPrimitiveTerminate);
    addPrimitive(dictionary, "pause",     ficlPrimitivePause);
    addPrimitive(dictionary, "stop",      ficlPrimitiveStop);
    addPrimitive(dictionary, "halt", 	  ficlPrimitiveHalt);

    addPrimitive(dictionary, "his",       ficlPrimitiveHis);

    addPrimitive(dictionary, "atomic@",   ficlPrimitiveAtomicFetch);
    addPrimitive(dictionary, "atomic!",   ficlPrimitiveAtomicStore);
    addPrimitive(dictionary, "atomic-xchg", ficlPrimitiveAtomicXchg);
    addPrimitive(dictionary, "atomic-cas", ficlPrimitiveAtomicCas);

    addPrimitive(dictionary, "/mutex", 	  ficlPrimitiveSlashMutex);
    addPrimitive(dictionary, "mutex-init",ficlPrimitiveMutexInit);
    addPrimitive(dictionary, "get",	  ficlPrimitiveMutexGet);
    addPrimitive(dictionary, "release",	  ficlPrimitiveMutexRelease);
#endif

    addPrimitive(dictionary, "bswap16",   ficlPrimitiveBSwap16);
    addPrimitive(dictionary, "bswap32",   ficlPrimitiveBSwap32);
    addPrimitive(dictionary, "bswap64",   ficlPrimitiveBSwap64);
#if defined(__LP64__) || defined(__MINGW64__)
    addPrimitive(dictionary, "bswap",     ficlPrimitiveBSwap64);
#else
    addPrimitive(dictionary, "bswap",     ficlPrimitiveBSwap32);
#endif

    return;
}

