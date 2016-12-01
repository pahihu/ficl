#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../ficl.h"

#ifdef darwin
#include <unistd.h>
#endif


int ficlFileTruncate(ficlFile *ff, ficlUnsigned size)
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
    FICL_IGNORE(callback);
    if (message != NULL) {
        fputs(message, stdout);
        fflush(stdout);
    }
    else
        fflush(stdout);
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


long ficlFileSize(ficlFile *ff)
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

#define addPrimitive(d,nm,fn) \
   ficlDictionarySetPrimitive(d,nm,fn,FICL_WORD_DEFAULT)

void ficlSystemCompilePlatform(ficlSystem *system)
{
    ficlDictionary *dictionary = ficlSystemGetDictionary(system);

    addPrimitive(dictionary, "ms",    ficlPrimitiveMS);

    return;
}


