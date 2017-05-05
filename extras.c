#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ficl.h"

#ifdef WIN32
#include <io.h>
#define getcwd _getcwd
#include "dlfcn.h"
#else
#include <dlfcn.h>
#endif

#include "curterm.h"

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

    addPrimitive(dictionary, "(init-multi)", ficlPrimitiveInitMulti);
    addPrimitive(dictionary, "(process)", ficlPrimitiveProcess);
    addPrimitive(dictionary, "(run)",     ficlPrimitiveRun);
    addPrimitive(dictionary, "(stop)",    ficlPrimitiveStop);
    addPrimitive(dictionary, "(pause)",   ficlPrimitivePause);

    return;
}

