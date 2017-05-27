#include <unistd.h>

#define FICL_WANT_PLATFORM (1)

#define FICL_PLATFORM_BASIC_TYPES     (1)
#define FICL_PLATFORM_INLINE          inline

#define FICL_PLATFORM_HAS_FTRUNCATE   (1)

#ifdef __LP64__

typedef char ficlInteger8;
typedef unsigned char ficlUnsigned8;
typedef short ficlInteger16;
typedef unsigned short ficlUnsigned16;
typedef int ficlInteger32;
typedef unsigned int ficlUnsigned32;
typedef long ficlInteger64;
typedef unsigned long ficlUnsigned64;

typedef ficlInteger64 ficlInteger;
typedef ficlUnsigned64 ficlUnsigned;
typedef double ficlFloat;

#define FICL_PLATFORM_HAS_2INTEGER    (0)
#define FICL_PLATFORM_ALIGNMENT       (8)

#else

typedef char ficlInteger8;
typedef unsigned char ficlUnsigned8;
typedef short ficlInteger16;
typedef unsigned short ficlUnsigned16;
typedef long ficlInteger32;
typedef unsigned long ficlUnsigned32;
typedef long long ficlInteger64;
typedef unsigned long long ficlUnsigned64;

typedef ficlInteger32 ficlInteger;
typedef ficlUnsigned32 ficlUnsigned;
typedef float ficlFloat;

#define FICL_PLATFORM_HAS_2INTEGER    (1)

#if FICL_PLATFORM_HAS_2INTEGER
typedef ficlInteger64 ficl2Integer;
typedef ficlUnsigned64 ficl2Unsigned;
#endif

#define FICL_PLATFORM_ALIGNMENT       (4)


#endif
