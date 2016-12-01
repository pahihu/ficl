/* 
** mingw32.c
** Andras Pahi, pahia@t-online.hu
** based on win32.c by Larry Hastings, larry@hastings.org
**/

#include <conio.h>
#include <sys/stat.h>
#include "../ficl.h"
#include "gw.h"

int ficlFileTruncate(ficlFile *ff, ficlUnsigned size)
{
    HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(ff->f));
    if (SetFilePointer(hFile, size, NULL, FILE_BEGIN) != size)
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

off64_t ficlFileSize64(ficlFile *ff)
{
	if (ff == NULL)
		return -1ULL;

	return _filelengthi64(_fileno(ff->f));
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

/* : KEY ( -- c ) */
static void ficlPrimitiveKey(ficlVm *vm)
{
	int ch;
	
	do {
		ch = _getch();
		if (0xE0 == ch)
			ch = 0;
	} while (0 == ch);
	
	ficlStackPushInteger(vm->dataStack, ch);
}

/* : KEY? ( -- flag ) */
static void ficlPrimitiveKeyQ(ficlVm *vm)
{
	int ret = _kbhit();
	ficlStackPushInteger(vm->dataStack, ret ? FICL_TRUE : FICL_FALSE);
}

/* : EKEY ( -- code ) */
static void ficlPrimitiveEkey(ficlVm *vm)
{
	int ch;
	
	ch = _getch();
	if (0 == ch) ch = 0xF0;
	if (0xE0 == ch || 0xF0 == ch) {
		ch <<= 8; ch += _getch();
	}
		
	ficlStackPushInteger(vm->dataStack, ch);
}

/* : GOTO-XY ( y x -- ) */
static void ficlPrimitiveGotoXY(ficlVm *vm)
{
	COORD cp;
	int x, y;
	
	x = ficlStackPopInteger(vm->dataStack);
	y = ficlStackPopInteger(vm->dataStack);
	cp.X = x;
	cp.Y = y;
	SetConsoleCursorPosition(hConOut,cp);
}

/* : XY? ( -- y x ) */
static void ficlPrimitiveXYQ(ficlVm *vm)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(hConOut, &csbi);
	ficlStackPushInteger(vm->dataStack, csbi.dwCursorPosition.Y);
	ficlStackPushInteger(vm->dataStack, csbi.dwCursorPosition.X);
}

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

/* : UTIME ( -- sec ) */
static void ficlPrimitiveUTime(ficlVm *vm)
{
	time_t t = time(NULL);

	ficlStackPushInteger(vm->dataStack, (ficlInteger)t);
}

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

/* : (C-CALL) ( argN ... arg1 n c-fn -- ret ) */
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
L7:	ficlStackPushInteger(vm->dataStack, a7);
L6:	ficlStackPushInteger(vm->dataStack, a6);
L5:	ficlStackPushInteger(vm->dataStack, a5);
L4:	ficlStackPushInteger(vm->dataStack, a4);
L3:	ficlStackPushInteger(vm->dataStack, a3);
L2:	ficlStackPushInteger(vm->dataStack, a2);
L1:	ficlStackPushInteger(vm->dataStack, a1);
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
void ficlSystemCompilePlatform(ficlSystem *system)
{
    HMODULE hModule;
    ficlDictionary *dictionary = system->dictionary;
    FICL_SYSTEM_ASSERT(system, dictionary);
    
	hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
    ficlDictionarySetPrimitive(dictionary, "key",    	 ficlPrimitiveKey,    	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "key?",     	 ficlPrimitiveKeyQ,    	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "ekey",     	 ficlPrimitiveEkey,    	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "goto-xy", 	 ficlPrimitiveGotoXY,   	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "xy?",  	 	 ficlPrimitiveXYQ,   	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "form",  	 ficlPrimitiveForm,   	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "colors!",	 ficlPrimitiveColorsStore, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "colors",	 ficlPrimitiveColors,	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "page",  	 ficlPrimitivePage,   	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "ms",  	 	 ficlPrimitiveMS,   	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "(c-call)", 	 ficlPrimitiveCCall,   	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "(std-call)", ficlPrimitiveStdCall,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "(callback)", ficlPrimitiveCallback,	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "gw-open", 	 ficlPrimitiveGwOpen,	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "gw-do-events",ficlPrimitiveGwDoEvents,	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "gw-wait-key",ficlPrimitiveGwWaitKey,FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "gw-close", 	 ficlPrimitiveGwClose,	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "color",		 ficlPrimitiveColor,	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "set-pixel",  ficlPrimitiveSetPixel,	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "at",		 ficlPrimitiveAt,		FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "+at",		 ficlPrimitiveAt,		FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "line",  	 ficlPrimitiveLine,		FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "rect",		 ficlPrimitiveRect,		FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "box",		 ficlPrimitiveBox,		FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "utime",		 ficlPrimitiveUTime,	FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "xy",		 ficlPrimitiveXY,		FICL_WORD_DEFAULT);

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
