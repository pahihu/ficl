// File gw.h - M. Covington 1996
// A graphics window for console applications

#ifndef _INC_WINDOWS
#include <windows.h>        // for Windows API
#endif

#ifndef _INC_CONIO
#include <conio.h>          // for kbhit() only
#endif


// Globals

int     gwWidth,gwHeight;   // Size of drawing area
HWND    gwHwnd  = NULL;     // Handle to window
HBITMAP gwBitmap;           // Bitmap that holds its contents
HDC     GW      = NULL;     // Device context on which we draw


// gwWndProc(...)
//
// Window procedure for the child window.
// Called by Windows, not by user's program.

LRESULT CALLBACK gwWndProc(HWND hWnd, UINT message,
                       WPARAM wParam, LPARAM lParam)
{
   HDC hdc;
   PAINTSTRUCT ps;

   if (message == WM_DESTROY)
   {
     // Before destroying the window,
     // delete resources that belong to it.
     SelectObject(GW,gwBitmap);
     DeleteDC(GW);
     DeleteObject(gwBitmap);
     // GW = gwHwnd = NULL;   // update globals
	 GW = NULL;
	 gwHwnd = NULL;
     PostQuitMessage(0);
     return 0;
   }
   else if (message == WM_PAINT)
   {
     // To paint the window, blit the bitmap into it
     hdc = BeginPaint(gwHwnd,&ps);
     BitBlt(hdc,0,0,gwWidth,gwHeight,GW,0,0,SRCCOPY);
     EndPaint (gwHwnd,&ps);
     return 0;
   }
   else
     return (DefWindowProc(hWnd,message,wParam,lParam));
}


// gwDoEvents()
//
// Procedure to handle events so the graphics window doesn't
// "freeze."  Main program must call this REPEATEDLY when not
// doing anything else.

void gwDoEvents()
{
   // Get the console output up to date
   #if defined(_INC_OSTREAM)
   cout.flush();
   #endif
   fflush(stdout);
   fflush(stderr);

   // If graphics window is not open, exit harmlessly
   if (!gwHwnd) return;

   // Get the contents of the window up to date
   InvalidateRect(gwHwnd,NULL,FALSE);
   UpdateWindow(gwHwnd);

   // If the window has messages pending,
   // pass them along to be processed
   MSG msg;
   if (PeekMessage(&msg,gwHwnd,0,0,PM_REMOVE))
   {
     TranslateMessage(&msg);
     DispatchMessage(&msg);
   }
}


// gwWaitForKey()
//
// Calls gwDoEvents() repeatedly until a key is pressed.

void gwWaitForKey()
{
   while (!kbhit()) { gwDoEvents(); Sleep(100); }
}


// gwClose()
//
// Removes the graphics window.  Optional at end of program.

void gwClose()
{
    DestroyWindow(gwHwnd);  // sends WM_DESTROY message
}


// gwOpen(title,width,height)
//
// Creates and open the graphics window.
// Initializes globals gwHeight, gwWidth, gwHwnd, gwBitmap, GW.

void gwOpen(LPSTR title, int width, int height)
{
   WNDCLASS  wndclass;     // Window class
   HINSTANCE hinstance;    // Application instance
   RECT      wrect, crect; // Window size data
   HDC       hdc;          // Window device context

   if(gwHwnd) gwClose();  // close the existing graphics window

   // Initialize globals
   gwWidth = width;
   gwHeight = height;

   // Windows 95/NT trick to get application instance
   hinstance = GetModuleHandle(NULL);

   // Register the window class
   wndclass.style          = CS_HREDRAW | CS_VREDRAW;
   wndclass.lpfnWndProc    = gwWndProc;
   wndclass.cbClsExtra     = 0;
   wndclass.cbWndExtra     = 0;
   wndclass.hInstance      = hinstance;
   wndclass.hIcon          = LoadIcon(NULL,IDI_APPLICATION);
   wndclass.hCursor        = LoadCursor(NULL,IDC_ARROW);
   wndclass.hbrBackground  = (HBRUSH) GetStockObject(WHITE_BRUSH);
   wndclass.lpszMenuName   = NULL;
   wndclass.lpszClassName  = "gwWindow";
   RegisterClass(&wndclass);

   // Create the window
   gwHwnd = CreateWindow(
     "gwWindow",         // class name
     title,              // title
     WS_OVERLAPPED,      // style
     CW_USEDEFAULT,      // x position
     CW_USEDEFAULT,      // y position
     width,              // x size
     height,             // y size
     GetActiveWindow(),  // parent
     NULL,               // menu ptr
     hinstance,          // app instance
     NULL);              // init params

   // Adjust the window size.  User gave the size of the
   // client area; we need to know the size of the whole thing.
   // Unlike AdjustWindowRect, this algorithm takes the border
   // and title bar into account.
   GetClientRect(gwHwnd,&crect);
   GetWindowRect(gwHwnd,&wrect);
   SetWindowPos(gwHwnd,HWND_TOP,wrect.left,wrect.top,
     wrect.right - wrect.left + width - crect.right,
     wrect.bottom - wrect.top + height - crect.bottom,
     0);

   // Create a bitmap in memory to hold the graphics
   hdc = GetDC(gwHwnd);            // dc of window
   GW = CreateCompatibleDC(hdc);   // dc of memory bitmap
   gwBitmap = CreateCompatibleBitmap(hdc,width,height);
   ReleaseDC(gwHwnd,hdc);
   SelectObject(GW,gwBitmap);

   // Fill the bitmap with white
   BitBlt(GW,0,0,gwWidth,gwHeight,NULL,0,0,WHITENESS);

   // Put the window on the screen
   InvalidateRect(gwHwnd,NULL,FALSE);
   ShowWindow(gwHwnd,SW_SHOWNORMAL);
}

// End of gw.h
