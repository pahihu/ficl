/* 
** mingw32.c
** Andras Pahi, pahia@t-online.hu
** based on win32.c by Larry Hastings, larry@hastings.org
**/

#include <conio.h>
#include <sys/stat.h>
#include "../ficl.h"
#include "gw.h"

long ficlGetGMTOffset(void)
{
  TIME_ZONE_INFORMATION tzi;

  GetTimeZoneInformation(&tzi);
  return tzi.Bias * 60;
}


char* ficlIntegerToString(char *buf, ficlInteger n)
{
  sprintf(buf, "%lld", n);
  return buf;
}

char* ficlUnsignedToString(char *buf, ficlUnsigned u)
{
  sprintf(buf, "%llu", u);
  return buf;
}

int ficlFileTruncate(ficlFile *ff, ficlOff_t size)
{
    HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(ff->f));
    LARGE_INTEGER liOffset;
    LARGE_INTEGER liNewOffset;

    liOffset.QuadPart = size;
    if (FALSE == SetFilePointerEx(hFile, liOffset, &liNewOffset, FILE_BEGIN))
        return 0;
    if (liOffset.QuadPart != liNewOffset.QuadPart)
        return 0;
    return !SetEndOfFile(hFile);
}

int ficlFileStatus(char *filename, int *status)
{
    /*
	** The Windows documentation for GetFileAttributes() says it returns
    ** INVALID_FILE_ATTRIBUTES on error.  There's no such #define.  The
    ** return value for error is -1, so we'll just use that.
	*/
  DWORD attributes = GetFileAttributes(filename);
	if (attributes == -1)
	{
		*status = GetLastError();
		return -1;
	}
    *status = attributes;
    return 0;
}


#ifdef __MINGW64__

ficlOff_t ficlFileSize(ficlFile *ff)
{
	if (ff == NULL)
		return -1ULL;

	return _filelengthi64(_fileno(ff->f));
}

#else

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

#endif


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
    if (message != NULL)
        fputs(message, stdout);
    else
        fflush(stdout);
    return;
}


/*
**
** === Console handling ===
**
*/

static HANDLE hConOut;

/* : AT-XY ( col row -- ) */
static void ficlPrimitiveAtXY(ficlVm *vm)
{
	COORD cp;
	int row, col;
	
	row = ficlStackPopInteger(vm->dataStack);
	col = ficlStackPopInteger(vm->dataStack);
	cp.X = col;
	cp.Y = row;
	SetConsoleCursorPosition(hConOut,cp);
}

/* : ?AT ( -- col row ) */
static void ficlPrimitiveQAt(ficlVm *vm)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(hConOut, &csbi);
	ficlStackPushInteger(vm->dataStack, csbi.dwCursorPosition.X);
	ficlStackPushInteger(vm->dataStack, csbi.dwCursorPosition.Y);
}

/* : SET-FG ( color -- ) */
static void ficlPrimitiveSetFG(ficlVm *vm)
{
	WORD wAttributes;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int fg, bg;

	GetConsoleScreenBufferInfo(hConOut, &csbi);
	// fg = csbi.wAttributes & 15;
	bg = (csbi.wAttributes >> 4) & 15;

	fg = ficlStackPopInteger(vm->dataStack);
	wAttributes = ((bg & 15) << 4) + (fg & 15);
	SetConsoleTextAttribute(hConOut, wAttributes);
}

/* : SET-BG ( color -- ) */
static void ficlPrimitiveSetBG(ficlVm *vm)
{
	WORD wAttributes;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int fg, bg;

	GetConsoleScreenBufferInfo(hConOut, &csbi);
	fg = csbi.wAttributes & 15;
	// bg = (csbi.wAttributes >> 4) & 15;

	bg = ficlStackPopInteger(vm->dataStack);
	wAttributes = ((bg & 15) << 4) + (fg & 15);
	SetConsoleTextAttribute(hConOut, wAttributes);
}

#if 0
/* : FORM ( -- h w ) */
static void ficlPrimitiveForm(ficlVm *vm)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int dx, dy;

	GetConsoleScreenBufferInfo(hConOut, &csbi);
	dx = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
	dy = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;
	ficlStackPushInteger(vm->dataStack, dy);
	ficlStackPushInteger(vm->dataStack, dx);
}

/* : COLORS! ( bg fg -- ) */
static void ficlPrimitiveColorsStore(ficlVm *vm)
{
	WORD wAttributes;
	int fg, bg;

	fg = ficlStackPopInteger(vm->dataStack);
	bg = ficlStackPopInteger(vm->dataStack);
	wAttributes = ((bg & 15) << 4) + (fg & 15);
	SetConsoleTextAttribute(hConOut, wAttributes);
}

/* : COLORS ( -- bg fg ) */
static void ficlPrimitiveColors(ficlVm *vm)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int fg, bg;

	GetConsoleScreenBufferInfo(hConOut, &csbi);
	fg = csbi.wAttributes & 15;
	bg = (csbi.wAttributes >> 4) & 15;
	ficlStackPushInteger(vm->dataStack, bg);
	ficlStackPushInteger(vm->dataStack, fg);
}
#endif

/* : PAGE ( -- ) */
static void ficlPrimitivePage(ficlVm *vm)
{
	COORD pos = {0, 0};
	DWORD n, size;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	
	GetConsoleScreenBufferInfo(hConOut, &csbi);
	size = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacter(hConOut, ' ', size, pos, &n);
	FillConsoleOutputAttribute(hConOut, csbi.wAttributes, size, pos, &n);
	SetConsoleCursorPosition(hConOut, pos);
}


/*
**
** === Time ===
**
*/

/* : MS ( msec -- ) */
static void ficlPrimitiveMS(ficlVm *vm)
{
	ficlUnsigned u = ficlStackPopUnsigned(vm->dataStack);
	
	Sleep(u);
}


/*
**
** === Simple FFI ===
**
*/

/* : (STD-CALL) ( argN ... arg1 n std-fn -- ret ) */
static void ficlPrimitiveStdCall(ficlVm *vm)
{
	int _stdcall (*fn)();
	int narg;
	int i, arg[8];
	int ret;

	fn = (int _stdcall (*)()) ficlStackPopPointer(vm->dataStack);
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

/*
**
** Graphics support
**
*/
static COLORREF fore;		/* BGR !!! */
static POINT xy;

/* : GW-OPEN ( x y -- ) */
static void ficlPrimitiveGwOpen(ficlVm *vm)
{
	int x, y;

	y = ficlStackPopInteger(vm->dataStack);
	x = ficlStackPopInteger(vm->dataStack);

	gwOpen("FiCL", x, y);
	fore = RGB(0,0,0);
	xy.x = 0; xy.y = 0;
	MoveToEx(GW, 0, 0, NULL);
}

/* : GW-DO-EVENTS ( -- ) */
static void ficlPrimitiveGwDoEvents(ficlVm *vm)
{
	gwDoEvents();
}

/* : GW-WAIT-KEY ( -- ) */
static void ficlPrimitiveGwWaitKey(ficlVm *vm)
{
	gwWaitForKey();
}

/* : GW-CLOSE ( -- ) */
static void ficlPrimitiveGwClose(ficlVm *vm)
{
	gwClose(GW);
}


/* : SET-PIXEL ( x y -- ) */
static void ficlPrimitiveSetPixel(ficlVm *vm)
{
	int x, y, rgb;

	y   = ficlStackPopInteger(vm->dataStack);
	x   = ficlStackPopInteger(vm->dataStack);
	SetPixelV(GW, x, y, fore);
}

/* : AT ( x y -- ) */
static void ficlPrimitiveAt(ficlVm *vm)
{
	int x, y;

	y = ficlStackPopInteger(vm->dataStack);
	x = ficlStackPopInteger(vm->dataStack);
	xy.x = x;
	xy.y = y;
	MoveToEx(GW, xy.x, xy.y, NULL);
}

/* : +AT ( dx dy -- ) */
static void ficlPrimitivePlusAt(ficlVm *vm)
{
	int dx, dy;

	dy = ficlStackPopInteger(vm->dataStack);
	dx = ficlStackPopInteger(vm->dataStack);
	xy.x += dx;
	xy.y += dy;
	MoveToEx(GW, xy.x, xy.y, NULL);
}

/* : LINE ( n1 n2 -- ) */
static void ficlPrimitiveLine(ficlVm *vm)
{
	int n1, n2;
	POINT pt;

	n2 = ficlStackPopInteger(vm->dataStack);
	n1 = ficlStackPopInteger(vm->dataStack);

	pt = xy;
	pt.x -= n1;
	MoveToEx(GW, pt.x, pt.y, NULL);
	pt.x += n2+1;
	LineTo(GW, pt.x, pt.y);
	xy.y++;
	MoveToEx(GW, xy.x, xy.y, NULL);
}

/* : RECT ( x y -- ) */
static void ficlPrimitiveRect(ficlVm *vm)
{
	int x, y;

	y = ficlStackPopInteger(vm->dataStack);
	x = ficlStackPopInteger(vm->dataStack);
	Rectangle(GW, xy.x, xy.y, x, y);
}

/* : BOX ( x y -- ) */
static void ficlPrimitiveBox(ficlVm *vm)
{
	HBRUSH old_hbrush, hbrush;

	hbrush = CreateSolidBrush(fore);
	old_hbrush = SelectObject(GW, hbrush);

	ficlPrimitiveRect(vm);

	SelectObject(GW, old_hbrush);
	DeleteObject(hbrush);
}

/* : COLOR ( rgb -- ) */
static void ficlPrimitiveColor(ficlVm *vm)
{
	int rgb, r, g, b;
	HPEN old_hpen, hpen;

	rgb = ficlStackPopInteger(vm->dataStack);
	/* r:4 g:5 b:4 */
	r = ((rgb >> 16) & 0xFF) >> 4;
	g = ((rgb >>  8) & 0xFF) >> 3;
	b = (rgb & 0xFF) >> 4;

	fore = RGB(r<<4,g<<3,b<<4);
	hpen = CreatePen(PS_SOLID, 1, fore);
	old_hpen = SelectObject(GW, hpen);
	DeleteObject(old_hpen);
}

static void ficlPrimitiveXY(ficlVm *vm)
{
	ficlUnsigned u;

	u = ((xy.x & 0xFFFF) << 16) | (xy.y & 0xFFFF);
	ficlStackPushUnsigned(vm->dataStack, u);
}

/**************************************************************************
                        f i c l C o m p i l e P l a t f o r m
** Build Mingw32 platform extensions into the system dictionary
**************************************************************************/

#define addPrimitive(d,nm,fn) \
   ficlDictionarySetPrimitive(d,nm,fn,FICL_WORD_DEFAULT)

void ficlSystemCompilePlatform(ficlSystem *system)
{
    HMODULE hModule;
    ficlDictionary *dictionary = system->dictionary;
    FICL_SYSTEM_ASSERT(system, dictionary);
    
	 hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
    addPrimitive(dictionary, "at-xy",  ficlPrimitiveAtXY);
    addPrimitive(dictionary, "?at",    ficlPrimitiveQAt);      /* iForth */
    addPrimitive(dictionary, "set-fg", ficlPrimitiveSetFG);
    addPrimitive(dictionary, "set-bg",	ficlPrimitiveSetBG);
    addPrimitive(dictionary, "page",   ficlPrimitivePage);
    addPrimitive(dictionary, "ms",     ficlPrimitiveMS);

    addPrimitive(dictionary, "(std-call)",  ficlPrimitiveStdCall);

    addPrimitive(dictionary, "gw-open", 	  ficlPrimitiveGwOpen);
    addPrimitive(dictionary, "gw-do-events",ficlPrimitiveGwDoEvents);
    addPrimitive(dictionary, "gw-wait-key", ficlPrimitiveGwWaitKey);
    addPrimitive(dictionary, "gw-close", 	  ficlPrimitiveGwClose);

    addPrimitive(dictionary, "color",       ficlPrimitiveColor);
    addPrimitive(dictionary, "set-pixel",   ficlPrimitiveSetPixel);
    addPrimitive(dictionary, "at",		ficlPrimitiveAt);
    addPrimitive(dictionary, "+at",		ficlPrimitiveAt);
    addPrimitive(dictionary, "line",  	ficlPrimitiveLine);
    addPrimitive(dictionary, "rect",	ficlPrimitiveRect);
    addPrimitive(dictionary, "box",		ficlPrimitiveBox);
    addPrimitive(dictionary, "xy",		ficlPrimitiveXY);

    /*
    ** Every other Win32-specific word is implemented in Ficl, with multicall or whatnot.
    ** (Give me a lever, and a place to stand, and I will move the Earth.)
    ** See softcore/win32.fr for details.  --lch
    */
    hModule = LoadLibrary("kernel32.dll");
    ficlDictionarySetConstantPointer(dictionary, "kernel32.dll", hModule);
    ficlDictionarySetConstantPointer(dictionary, "(get-proc-address)", GetProcAddress(hModule, "GetProcAddress"));
    FreeLibrary(hModule);

    return;
}
