#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ficl.h"

#if FICL_WANT_FILE
/*
**
** fileaccess.c
**
** Implements all of the File Access word set that can be implemented in portable C.
**
*/

static void pushIor(ficlVm *vm, int success)
{
    int ior;
    if (success)
        ior = 0;
    else
        ior = errno;
    ficlStackPushInteger(vm->dataStack, ior);
}


static ficl2Unsigned ficlOffsTo2Unsigned(off_t offs)
{
    ficl2Unsigned ud;

#if defined(__LP64__) || defined(__MINGW64__)
    FICL_2UNSIGNED_SET((ficlUnsigned) 0, (ficlUnsigned) offs, ud);
#else
    {
        ficlUnsigned high, low;

        high = FICL_UNSIGNED_MAX & (offs >> (8 * sizeof(ficlUnsigned)));
        low  = FICL_UNSIGNED_MAX & offs;
        FICL_2UNSIGNED_SET(high, low, ud);
    }
#endif

    return ud;
}


static off_t ficl2UnsignedToOffs(ficl2Unsigned ud)
{
    off_t offs;

#if defined(__LP64__) || defined(__MINGW64__)
    offs = (off_t) FICL_2UNSIGNED_GET_LOW(ud);
#else
    offs   = FICL_2UNSIGNED_GET_HIGH(ud);
    offs <<= 8 * sizeof(ficlUnsigned);
    offs  += FICL_2UNSIGNED_GET_LOW(ud);
#endif

    return offs;
}


static void ficlFileOpen(ficlVm *vm, char *writeMode) /* ( c-addr u fam -- fileid ior ) */
{
    int fam;
    ficlUnsigned length;
    void *address;
    char mode[4];
    FILE *f;
    char *filename;

    FICL_STACK_CHECK(vm->dataStack, 3, 2);

    fam = ficlStackPopInteger(vm->dataStack);
    length = ficlStackPopUnsigned(vm->dataStack);
    address = (void *)ficlStackPopPointer(vm->dataStack);

    filename = (char *)malloc(length + 1);
    memcpy(filename, address, length);
    filename[length] = 0;

    *mode = 0;

    switch (FICL_FAM_OPEN_MODE(fam))
    {
        case 0:
            ficlStackPushPointer(vm->dataStack, NULL);
            ficlStackPushInteger(vm->dataStack, EINVAL);
            goto EXIT;
        case FICL_FAM_READ:
            strcat(mode, "r");
            break;
        case FICL_FAM_WRITE:
			// 150510AP
			// 			OPEN-FILE	CREATE-FILE
			// 	r/o		"r"			 -
			// 	w/o		 -			"w"
			// 	r/w		"r+"		"w+"
            strcat(mode, "w" /*writeMode*/);
            break;
        case FICL_FAM_READ | FICL_FAM_WRITE:
            strcat(mode, writeMode);
            strcat(mode, "+");
            break;
    }

    strcat(mode, (fam & FICL_FAM_BINARY) ? "b" : "t");

    f = fopen(filename, mode);
    if (f == NULL)
        ficlStackPushPointer(vm->dataStack, NULL);
    else
    {
        ficlFile *ff = (ficlFile *)malloc(sizeof(ficlFile));
        strcpy(ff->filename, filename);
        ff->f = f;
        ficlStackPushPointer(vm->dataStack, ff);

        fseek(f, 0, SEEK_SET);
    }
    pushIor(vm, f != NULL);
	
EXIT:
	free(filename);
}



static void ficlPrimitiveOpenFile(ficlVm *vm) /* ( c-addr u fam -- fileid ior ) */
{
    ficlFileOpen(vm, "r" /*"a"*/);
}


static void ficlPrimitiveCreateFile(ficlVm *vm) /* ( c-addr u fam -- fileid ior ) */
{
    ficlFileOpen(vm, "w");
}


static int ficlFileClose(ficlFile *ff) /* ( fileid -- ior ) */
{
    FILE *f = ff->f;
    free(ff);
    return !fclose(f);
}

static void ficlPrimitiveCloseFile(ficlVm *vm) /* ( fileid -- ior ) */
{
    ficlFile *ff;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    ff = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    pushIor(vm, ficlFileClose(ff));
}

static void ficlPrimitiveDeleteFile(ficlVm *vm) /* ( c-addr u -- ior ) */
{
    ficlUnsigned length;
    void *address;
    char *filename;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    length = ficlStackPopUnsigned(vm->dataStack);
    address = (void *)ficlStackPopPointer(vm->dataStack);

    filename = (char *)malloc(length + 1);

    memcpy(filename, address, length);
    filename[length] = 0;

    pushIor(vm, !unlink(filename));
	free(filename);
}

static void ficlPrimitiveRenameFile(ficlVm *vm) /* ( c-addr1 u1 c-addr2 u2 -- ior ) */
{
    ficlUnsigned length;
    void *address;
    char *from;
    char *to;

    FICL_STACK_CHECK(vm->dataStack, 4, 1);

    length = ficlStackPopUnsigned(vm->dataStack);
    address = (void *)ficlStackPopPointer(vm->dataStack);
    to = (char *)malloc(length + 1);
    memcpy(to, address, length);
    to[length] = 0;

    length = ficlStackPopUnsigned(vm->dataStack);
    address = (void *)ficlStackPopPointer(vm->dataStack);

    from = (char *)malloc(length + 1);
    memcpy(from, address, length);
    from[length] = 0;

    pushIor(vm, !rename(from, to));

	free(from);
	free(to);
}

static void ficlPrimitiveFileStatus(ficlVm *vm) /* ( c-addr u -- x ior ) */
{
	int status;
	int ior;
    ficlUnsigned length;
    void *address;
    char *filename;

    FICL_STACK_CHECK(vm->dataStack, 2, 2);

    length = ficlStackPopUnsigned(vm->dataStack);
    address = (void *)ficlStackPopPointer(vm->dataStack);

    filename = (char *)malloc(length + 1);

    memcpy(filename, address, length);
    filename[length] = 0;

	ior = ficlFileStatus(filename, &status);
	free(filename);

    ficlStackPushInteger(vm->dataStack, status);
    ficlStackPushInteger(vm->dataStack, ior);
}


static void ficlPrimitiveFilePosition(ficlVm *vm) /* ( fileid -- ud ior ) */
{
    ficlFile *ff;
    off_t offs;
    ficl2Unsigned ud;

    FICL_STACK_CHECK(vm->dataStack, 1, 2);

    ff   = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    offs = ftello(ff->f);

    ud = ficlOffsTo2Unsigned(offs);

    ficlStackPush2Unsigned(vm->dataStack, ud);
    pushIor(vm, offs != (off_t) -1);
}


static void ficlPrimitiveFileSize(ficlVm *vm) /* ( fileid -- ud ior ) */
{
    ficlFile *ff;
    off_t size;
    ficl2Unsigned ud;

    FICL_STACK_CHECK(vm->dataStack, 1, 2);

    ff = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    size = ficlFileSize(ff);

    ud = ficlOffsTo2Unsigned(size);

    ficlStackPush2Unsigned(vm->dataStack, ud);
    pushIor(vm, size != (off_t) -1);
}


#define nLINEBUF 256
static void ficlPrimitiveIncludeFile(ficlVm *vm) /* ( i*x fileid -- j*x ) */
{
    ficlFile *ff;
    ficlCell id = vm->sourceId;
    int  except = FICL_VM_STATUS_OUT_OF_TEXT;
    off_t currentPosition, totalSize;
    off_t size;
	ficlString s;

    FICL_STACK_CHECK(vm->dataStack, 1, 0);

    ff = (ficlFile *)ficlStackPopPointer(vm->dataStack);

    vm->sourceId.p = (void *)ff;

    currentPosition = ftello(ff->f);
    totalSize = ficlFileSize(ff);
    size = totalSize - currentPosition;

    if ((totalSize != -1) && (currentPosition != -1) && (size > 0))
    {
        char *buffer = (char *)malloc(size);
        size_t got = fread(buffer, 1, size, ff->f);
        if (got == size)
		{
			FICL_STRING_SET_POINTER(s, buffer);
			FICL_STRING_SET_LENGTH(s, size);
            except = ficlVmExecuteString(vm, s);
		}
    }

    if ((except < 0) && (except != FICL_VM_STATUS_OUT_OF_TEXT))
        ficlVmThrow(vm, except);
	
    /*
    ** Pass an empty line with SOURCE-ID == -1 to flush
    ** any pending REFILLs (as required by FILE wordset)
    */
    vm->sourceId.i = -1;
	FICL_STRING_SET_FROM_CSTRING(s, "");
    ficlVmExecuteString(vm, s);

    vm->sourceId = id;
    ficlFileClose(ff);
}



static void ficlPrimitiveReadFile(ficlVm *vm) /* ( c-addr u1 fileid -- u2 ior ) */
{
    ficlFile *ff;
    ficlUnsigned length;
    void *address;
    ficlUnsigned result;

    FICL_STACK_CHECK(vm->dataStack, 3, 2);

    ff = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    length = ficlStackPopUnsigned(vm->dataStack);
    address = (void *)ficlStackPopPointer(vm->dataStack);

    clearerr(ff->f);
    result = fread(address, 1, length, ff->f);

    ficlStackPushUnsigned(vm->dataStack, result);
    pushIor(vm, ferror(ff->f) == 0);
}



static void ficlPrimitiveReadLine(ficlVm *vm) /* ( c-addr u1 fileid -- u2 flag ior ) */
{
    ficlFile *ff;
    ficlUnsigned length;
    char *address;
    int error;
    int flag;

    FICL_STACK_CHECK(vm->dataStack, 3, 3);

    ff = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    length = ficlStackPopUnsigned(vm->dataStack);
    address = (char *)ficlStackPopPointer(vm->dataStack);

    if (feof(ff->f))
    {
        ficlStackPushUnsigned(vm->dataStack, (ficlUnsigned) -1);
        ficlStackPushInteger(vm->dataStack, 0);
        ficlStackPushInteger(vm->dataStack, 0);
        return;
    }

    clearerr(ff->f);
    *address = 0;
    fgets(address, length, ff->f);

    error = ferror(ff->f);
    if (error != 0)
    {
        ficlStackPushUnsigned(vm->dataStack, (ficlUnsigned) -1);
        ficlStackPushInteger(vm->dataStack, 0);
        ficlStackPushInteger(vm->dataStack, error);
        return;
    }

    length = strlen(address);
    flag = (length > 0);
    if (length && ((address[length - 1] == '\r') || (address[length - 1] == '\n')))
        length--;
    
    ficlStackPushUnsigned(vm->dataStack, length);
    ficlStackPushInteger(vm->dataStack, flag);
    ficlStackPushInteger(vm->dataStack, 0); /* ior */
}



static void ficlPrimitiveWriteFile(ficlVm *vm) /* ( c-addr u1 fileid -- ior ) */
{
    ficlFile *ff;
    ficlUnsigned written, length;
    void *address;

    FICL_STACK_CHECK(vm->dataStack, 3, 1);

    ff = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    length = ficlStackPopUnsigned(vm->dataStack);
    address = (void *)ficlStackPopPointer(vm->dataStack);

    clearerr(ff->f);
#ifdef DEBUG
	 fprintf(stderr,"ficlPrimitiveWriteFile: length   = %ld\n", (long) length);
	 fprintf(stderr,"ficlPrimitiveWriteFile: position = %ld\n", (long) ftell(ff->f));
#endif
    written = fwrite(address, 1, length, ff->f);
#ifdef DEBUG
	 fprintf(stderr,"ficlPrimitiveWriteFile: written  = %ld\n", (long) written);
	 fprintf(stderr,"ficlPrimitiveWriteFile: position = %ld\n", (long) ftell(ff->f));
#endif
    pushIor(vm, ferror(ff->f) == 0);
}



static void ficlPrimitiveWriteLine(ficlVm *vm) /* ( c-addr u1 fileid -- ior ) */
{
    ficlFile *ff;
    ficlUnsigned length;
    void *address;

    FICL_STACK_CHECK(vm->dataStack, 3, 1);

    ff = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    length = ficlStackPopUnsigned(vm->dataStack);
    address = (void *)ficlStackPopPointer(vm->dataStack);

    clearerr(ff->f);
    if (fwrite(address, 1, length, ff->f) == length)
        fwrite("\n", 1, 1, ff->f);
    pushIor(vm, ferror(ff->f) == 0);
}



static void ficlPrimitiveRepositionFile(ficlVm *vm) /* ( ud fileid -- ior ) */
{
    ficlFile *ff;
    ficl2Unsigned ud;
    off_t offs;

    FICL_STACK_CHECK(vm->dataStack, 3, 1);

    ff = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    ud = ficlStackPop2Unsigned(vm->dataStack);

    offs = ficl2UnsignedToOffs(ud);

    pushIor(vm, fseeko(ff->f, offs, SEEK_SET) == 0);
}


static void ficlPrimitiveFlushFile(ficlVm *vm) /* ( fileid -- ior ) */
{
    ficlFile *ff;

    FICL_STACK_CHECK(vm->dataStack, 1, 1);

    ff = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    pushIor(vm, fflush(ff->f) == 0);
}


static void ficlPrimitiveUse(ficlVm *vm) /* ( fileid -- ) */
{
    ficlFile *ff;

    FICL_STACK_CHECK(vm->dataStack, 1, 0);

    ff = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    ficlVmSetOutFile(vm, ff);
}


ficlFile *ficlStdIn, *ficlStdOut, *ficlStdErr;


#if FICL_PLATFORM_HAS_FTRUNCATE

static void ficlPrimitiveResizeFile(ficlVm *vm) /* ( ud fileid -- ior ) */
{
    ficlFile *ff;
    ficl2Unsigned ud;
    off_t offs;

    FICL_STACK_CHECK(vm->dataStack, 3, 1);

    ff = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    ud = ficlStackPop2Unsigned(vm->dataStack);

    offs = ficl2UnsignedToOffs(ud);

    pushIor(vm, ficlFileTruncate(ff, offs) == 0);
}

#endif /* FICL_PLATFORM_HAS_FTRUNCATE */

#endif /* FICL_WANT_FILE */



void ficlSystemCompileFile(ficlSystem *system)
{
#if !FICL_WANT_FILE
    FICL_IGNORE(system);
#else
    ficlDictionary *dictionary = ficlSystemGetDictionary(system);
    ficlDictionary *environment = ficlSystemGetEnvironment(system);

    FICL_SYSTEM_ASSERT(system, dictionary);
    FICL_SYSTEM_ASSERT(system, environment);

    ficlStdIn = (ficlFile*) malloc(sizeof(ficlFile));
    ficlStdIn->f = stdin;
    strcpy(&ficlStdIn->filename[0], "<stdin>");

    ficlStdOut = (ficlFile*) malloc(sizeof(ficlFile));
    ficlStdOut->f = stdout;
    strcpy(&ficlStdOut->filename[0], "<stdout>");

    ficlStdErr = (ficlFile*) malloc(sizeof(ficlFile));
    ficlStdErr->f = stderr;
    strcpy(&ficlStdErr->filename[0], "<stderr>");

    ficlDictionarySetPrimitive(dictionary, "create-file", ficlPrimitiveCreateFile,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "open-file", ficlPrimitiveOpenFile,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "close-file", ficlPrimitiveCloseFile,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "include-file", ficlPrimitiveIncludeFile,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "read-file", ficlPrimitiveReadFile,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "read-line", ficlPrimitiveReadLine,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "write-file", ficlPrimitiveWriteFile,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "write-line", ficlPrimitiveWriteLine,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "file-position", ficlPrimitiveFilePosition,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "file-size", ficlPrimitiveFileSize,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "reposition-file", ficlPrimitiveRepositionFile,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "file-status", ficlPrimitiveFileStatus,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "flush-file", ficlPrimitiveFlushFile,  FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "delete-file", ficlPrimitiveDeleteFile,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "rename-file", ficlPrimitiveRenameFile,  FICL_WORD_DEFAULT);

    ficlDictionarySetConstant(dictionary,  "stdin",  (ficlUnsigned) ficlStdIn);
    ficlDictionarySetConstant(dictionary,  "stdout", (ficlUnsigned) ficlStdOut);
    ficlDictionarySetConstant(dictionary,  "stderr", (ficlUnsigned) ficlStdErr);
    ficlDictionarySetPrimitive(dictionary, "use",    ficlPrimitiveUse,  FICL_WORD_DEFAULT);

#if FICL_PLATFORM_HAS_FTRUNCATE
    ficlDictionarySetPrimitive(dictionary, "resize-file", ficlPrimitiveResizeFile,  FICL_WORD_DEFAULT);

    ficlDictionarySetConstant(environment, "file", FICL_TRUE);
    ficlDictionarySetConstant(environment, "file-ext", FICL_TRUE);
#else /*  FICL_PLATFORM_HAS_FTRUNCATE */
    ficlDictionarySetConstant(environment, "file", FICL_FALSE);
    ficlDictionarySetConstant(environment, "file-ext", FICL_FALSE);
#endif /* FICL_PLATFORM_HAS_FTRUNCATE */

#endif /* !FICL_WANT_FILE */
}
