#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../ficl.h"

long ficlGetGMTOffset(void)
{
  struct tm *tim;
  time_t t;

  t = time(NULL);
  tim = localtime(&t);

  return tim->tm_gmtoff;
}

char* ficlIntegerToString(char *buf, ficlInteger n)
{
  sprintf(buf, "%ld", n);
  return buf;
}

char* ficlUnsignedToString(char *buf, ficlUnsigned u)
{
  sprintf(buf, "%lu", u);
  return buf;
}

int ficlFileTruncate(ficlFile *ff, ficlOff_t size)
{
	return ftruncate(fileno(ff->f), size);
}

void *ficlMalloc(size_t size)
{
    return malloc(size);
}

void *ficlRealloc(void *p, size_t size)
{
    return realloc(p, size);
}

void ficlFree(void *p)
{
    free(p);
}

void  ficlCallbackDefaultTextOut(ficlCallback *callback, char *message)
{
    ficlVm *vm;
    ficlFile *outFile;
    FILE *out;

    vm = (ficlVm*) callback->vm;
    outFile = vm ? vm->outFile : 0;
    out = outFile ? outFile->f : stdout;
    if (message != NULL) {
        if (0 == callback->quiet)
            fputs(message, out);
    }
    fflush(out);
    return;
}

int ficlFileStatus(char *filename, int *status)
{
    struct stat statbuf;
    if (stat(filename, &statbuf) == 0)
    {
        *status = statbuf.st_mode;
        return 0;
    }
    *status = ENOENT;
    return -1;
}


ficlOff_t ficlFileSize(ficlFile *ff)
{
    struct stat statbuf;
    if (ff == NULL)
        return -1;
	
    statbuf.st_size = -1;
    if (fstat(fileno(ff->f), &statbuf) != 0)
        return -1;
	
    return statbuf.st_size;
}

/* : MS ( u -- ) */
static void ficlPrimitiveMS(ficlVm *vm)
{
   useconds_t useconds;

   useconds = 1000 * ficlStackPopUnsigned(vm->dataStack);
   usleep(useconds);
}

/* : US ( u -- ) */
static void ficlPrimitiveUS(ficlVm *vm)
{
   useconds_t useconds;

   useconds = ficlStackPopUnsigned(vm->dataStack);
   usleep(useconds);
}


#define addPrimitive(d,nm,fn) \
   ficlDictionarySetPrimitive(d,nm,fn,FICL_WORD_DEFAULT)

void ficlSystemCompilePlatform(ficlSystem *system)
{
    ficlDictionary *dictionary = ficlSystemGetDictionary(system);

    addPrimitive(dictionary, "ms",    ficlPrimitiveMS);
    addPrimitive(dictionary, "us",    ficlPrimitiveUS);

    return;
}


