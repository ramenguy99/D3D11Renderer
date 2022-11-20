#include "win32.h"

global_variable win32_state Win32;

internal void
Win32_FatalError(char* Message)
{
#ifdef BUILD_DEBUG
    __debugbreak();
#else
    MessageBox(0, Message, "Assertion failed!", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
    ExitProcess(1);
#endif
}

internal void*
Win32_ReadEntireFile(char* Path, u32* OutBytesRead)
{    
    void* Result = 0;
    HANDLE FileHandle;
    FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0,0);
    
    if(FileHandle == INVALID_HANDLE_VALUE) return 0;
    
    LARGE_INTEGER FileSize = {};
    b32 Success = GetFileSizeEx(FileHandle, &FileSize);
    if(!(Success && FileSize.QuadPart <= 0xFFFFFFFF)) return 0;
    uint32 FileSize32 = (uint32)FileSize.QuadPart;
    Result = VirtualAlloc(0, FileSize32, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if(!Result) return 0;
    
    DWORD BytesRead = 0;
    Success = ReadFile(FileHandle, Result, FileSize32, &BytesRead, 0);
    if(!(Success && FileSize32 == BytesRead))
    {
        VirtualFree(Result, 0, MEM_RELEASE);
        return 0;
    }
    
    if(OutBytesRead)
    {
        *OutBytesRead = BytesRead;
    }
    CloseHandle(FileHandle);
    
    return Result;
}

internal void
Win32_FreeFileMemory(void* Memory)
{
    VirtualFree(Memory, 0, MEM_RELEASE);
}

internal b32
Win32_ReadAtOffset(HANDLE File, void* Data, u64 Size, u64 Offset)
{
    Assert(Size <= UINT_MAX);
    
    OVERLAPPED Overlapped = {};
    Overlapped.Offset = (u32)((Offset >> 0) & 0xFFFFFFFF);
    Overlapped.OffsetHigh = (u32)((Offset >> 32) & 0xFFFFFFFF);
    
    DWORD BytesRead;
    b32 Success = ReadFile(File, Data, (u32)Size, &BytesRead, &Overlapped) &&
    (Size == BytesRead);
    return Success;
}

LRESULT CALLBACK
Win32_WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    if (ImGui_ImplWin32_WndProcHandler(Window, Message, WParam, LParam))
        return true;
    
    LRESULT Result = 0;
    
    switch(Message) {
        case WM_SIZE: {
            s32 Width = LOWORD(LParam);
            s32 Height = HIWORD(LParam);
            Win32.WindowSize = ivec2(Width, Height);
            Win32.WindowResized = true;
        } break;
        
        case WM_ACTIVATE: {
        } break;
        
        case WM_KEYDOWN: {
            uint32 VKCode = (uint32)WParam;
            bool32 WasDown = (LParam & (1 << 30)) != 0;
            bool32 IsDown = (LParam & (1 << 31)) == 0;
            bool32 AltIsDown = LParam & (1 << 29);
            
            if(VKCode == VK_ESCAPE) {
                PostQuitMessage(0);
                Win32.WindowQuit = true;
            }
            
            //Only one shot
            if(IsDown) {
                switch(VKCode) {
                    case VK_F2: InspectorData.ShowDemoWindow = !InspectorData.ShowDemoWindow; break;
                    case VK_F1: InspectorData.ShowEditor = !InspectorData.ShowEditor; break;
                }
            }
        } break;
        
        case WM_MOUSEMOVE: {
            
        } break;
        
        case WM_CLOSE: {
            Win32.WindowQuit = true;
        } break;
        
        default: {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        }break;
    }
    
    return Result;
}

internal HWND
Win32_CreateWindow(char* Name, s32 SizeX, s32 SizeY, bool Visible = true)
{
    local_persist char* WindowClassName;
    
    HWND Window = 0;
    s32 LastError;
    WNDCLASSA WindowClass = {0};
    HINSTANCE Instance = GetModuleHandle(0);
    
    if(!WindowClassName)
    {
        // NOTE: can specify CS_OWNDC to get a permanent DC
        WindowClass.style = CS_HREDRAW | CS_VREDRAW;
        WindowClass.lpfnWndProc = Win32_WindowProc;
        WindowClass.hInstance = Instance;
        //  WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
        //  WindowClass.hIcon;
        WindowClass.lpszClassName = "WindowClass";
        if(!RegisterClassA(&WindowClass))
        {
            Win32_FatalError("Failed to RegisterClass");
        }
        
        WindowClassName = (char*)WindowClass.lpszClassName;
    }
    
    //Make SizeX and SizeY be the size of the client area
    RECT WindowRect = {};
    WindowRect.right = SizeX;
    WindowRect.bottom = SizeY;
    AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, false);
    SizeX = WindowRect.right - WindowRect.left;
    SizeY = WindowRect.bottom - WindowRect.top;
    
    DWORD WindowStyle = WS_OVERLAPPEDWINDOW;// | WS_POPUP;
    WindowStyle |= Visible ? WS_VISIBLE : 0;
    Window = CreateWindowExA(0, WindowClassName, Name, WindowStyle, 
                             CW_USEDEFAULT,  CW_USEDEFAULT,  SizeX,  SizeY, 0, 0, Instance, 0);
    
    if(!Window)
    {
        // NOTE: Failed CreateWindowExA
        LastError = GetLastError();
        Win32_FatalError("Failed to CreateWindowEx");
    }
    
    return Window;
}

internal s64
Win32_GetCurrentCounter()
{
    LARGE_INTEGER CurrentCounter;
    QueryPerformanceCounter(&CurrentCounter);
    return CurrentCounter.QuadPart;
}

internal float32
Win32_GetSecondsElapsed(s64 CounterBegin, s64 CounterEnd)
{
    static s64 PerformanceFrequency;
    if(!PerformanceFrequency)
    {
        LARGE_INTEGER Frequency;
        QueryPerformanceFrequency(&Frequency);
        PerformanceFrequency = Frequency.QuadPart;
    }
    
    s64 CounterElapsed = CounterEnd - CounterBegin;
    f32 SecondsElapsed = (f32)CounterElapsed / PerformanceFrequency;
    
    return SecondsElapsed;
}

internal vec2 
Win32_GetMousePosition(HWND Window)
{
    POINT MousePosition = {};
    GetCursorPos(&MousePosition);
    ScreenToClient(Window, &MousePosition);
    
    vec2 Result = vec2((f32)MousePosition.x, (f32)MousePosition.y);
    
    return Result;
}