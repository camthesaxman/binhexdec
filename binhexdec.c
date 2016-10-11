#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#define UNUSED_PARAMETER(x) (void)(x)
#define SIZE_BITS(type) (8 * sizeof(type))

#define CLIENT_WIDTH 600
#define CLIENT_HEIGHT 160
#define EDIT_WIDTH 500
#define EDIT_HEIGHT 20
#define BIN_Y 20
#define DEC_Y 70
#define HEX_Y 120
#define TEXT_BUFFER_LEN 1024

enum {
    ID_EDIT_BIN,
    ID_EDIT_DEC,
    ID_EDIT_HEX
};

static unsigned long int number = 0;
static TCHAR binEditBuffer[TEXT_BUFFER_LEN];
static TCHAR decEditBuffer[TEXT_BUFFER_LEN];
static TCHAR hexEditBuffer[TEXT_BUFFER_LEN];
static TCHAR newEditBuffer[TEXT_BUFFER_LEN];
static HFONT hDefGuiFont;
static HFONT hMonospaceFont;

static BOOL validate_binary(const TCHAR *text)
{
    for(const TCHAR *p = text; *p != '\0'; p++)
    {
        if(*p != '0' && *p != '1')
            return FALSE;
    }
    return TRUE;        
}

static BOOL validate_decimal(const TCHAR *text)
{
    for(const TCHAR *p = text; *p != '\0'; p++)
    {
        if(*p < '0' || *p > '9')
            return FALSE;
    }
    return TRUE;
}

static BOOL validate_hexadecimal(const TCHAR *text)
{
    for(const TCHAR *p = text; *p != '\0'; p++)
    {
        if(*p < '0' || toupper(*p) > 'F')
            return FALSE;
        if(*p > '9' && toupper(*p) < 'A')
            return FALSE;
    }
    return TRUE;
}

static unsigned long int bin_to_n(const TCHAR *text)
{
    unsigned long int n = 0;
    int shift = 0;
    
    for(const TCHAR *p = &text[_tcslen(text) - 1]; p >= text; p--)
    {
        if(*p == '1')
            n |= 1 << shift;
        shift++;
    }
    return n;
}

static unsigned long int dec_to_n(const TCHAR *text)
{
    unsigned long int n = 0;
    int mul = 1;
    
    for(const TCHAR *p = &text[_tcslen(text) - 1]; p >= text; p--)
    {
        int digit = *p - '0';
        
        n += digit * mul;
        mul *= 10;
    }
    return n;
}

static unsigned long int hex_to_n(const TCHAR *text)
{
    unsigned long int n = 0;
    int shift = 0;
    
    for(const TCHAR *p = &text[_tcslen(text) - 1]; p >= text; p--)
    {
        int digit = (isdigit(*p)) ? (*p - '0') : (0xA + toupper(*p) - 'A');
        
        n |= digit << shift;
        shift += 4;
    }
    return n;
}

static void reverse_string(TCHAR *string, int len)
{
    int last = len - 1; //index of last character
    
    for(int i = 0; i != len / 2; i++)
    {
        TCHAR temp = string[i];
        
        string[i] = string[last - i];
        string[last - i] = temp;
    }
}

static void n_to_bin(unsigned long int n, TCHAR *text)
{
    int i = 0;
    
    if(n == 0)
    {
        text[0] = '0';
        text[1] = '\0';
        return;
    }
    while(n != 0)
    {
        text[i] = (n & 1) ? '1' : '0';
        i++;
        n >>= 1;
    }
    text[i] = '\0';
    reverse_string(text, i);
}

static void n_to_dec(unsigned long int n, TCHAR *text)
{
    int i = 0;
    
    if(n == 0)
    {
        text[0] = '0';
        text[1] = '\0';
        return;
    }
    while(n != 0)
    {
        text[i] = (n % 10) + '0'; 
        i++;
        n /= 10;
    }
    text[i] = '\0';
    reverse_string(text, i);
}

static void n_to_hex(unsigned long int n, TCHAR *text)
{
    int i = 0;
    
    if(n == 0)
    {
        text[0] = '0';
        text[1] = '\0';
        return;
    }
    while(n != 0)
    {
        int digit = n & 0xF;
        
        if(digit < 10)
            text[i] = digit + '0';
        else
            text[i] = digit - 10 + 'A';
        i++;
        n >>= 4;
    }
    text[i] = '\0';
    reverse_string(text, i);
}
    
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND hBinEditCtl;
    static HWND hDecEditCtl;
    static HWND hHexEditCtl;
    static BOOL changedByUser;
    
    switch(msg)
    {
        case WM_PAINT:
        {
            HDC hDC;
            PAINTSTRUCT ps;
            
            hDC = BeginPaint(hWnd, &ps);
            SetBkMode(hDC, TRANSPARENT);
            SelectObject(hDC, hDefGuiFont);
            TextOut(hDC, 10, BIN_Y, TEXT("Binary:"), strlen("Binary:"));
            TextOut(hDC, 10, DEC_Y, TEXT("Decimal:"), strlen("Decimal:"));
            TextOut(hDC, 10, HEX_Y, TEXT("Hexadecimal:"), strlen("Hexadecimal:"));
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_COMMAND:
            if(HIWORD(wParam) == EN_UPDATE)
            {
                if(changedByUser == FALSE)  //SetWindowText causes another EN_UPDATE to be sent immediately.
                    break;                  //We want to only process EN_UPDATE when the user changes the text.
                switch(LOWORD(wParam))
                {
                    case ID_EDIT_BIN:
                        GetWindowText(hBinEditCtl, newEditBuffer, TEXT_BUFFER_LEN);
                        if(validate_binary(newEditBuffer))
                        {
                            _tcscpy(binEditBuffer, newEditBuffer);
                            number = bin_to_n(binEditBuffer);
                            changedByUser = FALSE;
                            n_to_dec(number, decEditBuffer);
                            SetWindowText(hDecEditCtl, decEditBuffer);
                            n_to_hex(number, hexEditBuffer);
                            SetWindowText(hHexEditCtl, hexEditBuffer);
                            changedByUser = TRUE;
                        }
                        else
                            SetWindowText(hBinEditCtl, binEditBuffer);
                        break;
                    case ID_EDIT_DEC:
                        GetWindowText(hDecEditCtl, newEditBuffer, TEXT_BUFFER_LEN);
                        if(validate_decimal(newEditBuffer))
                        {
                            _tcscpy(decEditBuffer, newEditBuffer);
                            number = dec_to_n(decEditBuffer);
                            changedByUser = FALSE;
                            n_to_bin(number, binEditBuffer);
                            SetWindowText(hBinEditCtl, binEditBuffer);
                            n_to_hex(number, hexEditBuffer);
                            SetWindowText(hHexEditCtl, hexEditBuffer);
                            changedByUser = TRUE;
                        }
                        else
                            SetWindowText(hDecEditCtl, decEditBuffer);
                        break;
                    case ID_EDIT_HEX:
                        GetWindowText(hHexEditCtl, newEditBuffer, TEXT_BUFFER_LEN);
                        if(validate_hexadecimal(newEditBuffer))
                        {
                            _tcscpy(hexEditBuffer, newEditBuffer);
                            number = hex_to_n(hexEditBuffer);
                            changedByUser = FALSE;
                            n_to_bin(number, binEditBuffer);
                            SetWindowText(hBinEditCtl, binEditBuffer);
                            n_to_dec(number, decEditBuffer);
                            SetWindowText(hDecEditCtl, decEditBuffer);
                            changedByUser = TRUE;
                        }
                        else
                            SetWindowText(hHexEditCtl, hexEditBuffer);
                        break;
                }
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_CREATE:
            changedByUser = FALSE;
            hBinEditCtl = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_CHILD | WS_VISIBLE | ES_CENTER,
              80, BIN_Y, EDIT_WIDTH, EDIT_HEIGHT, hWnd, (HMENU)ID_EDIT_BIN, NULL, NULL);
            hDecEditCtl = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_CHILD | WS_VISIBLE | ES_CENTER,
              80, DEC_Y, EDIT_WIDTH, EDIT_HEIGHT, hWnd, (HMENU)ID_EDIT_DEC, NULL, NULL);
            hHexEditCtl = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_CHILD | WS_VISIBLE | ES_CENTER,
              80, HEX_Y, EDIT_WIDTH, EDIT_HEIGHT, hWnd, (HMENU)ID_EDIT_HEX, NULL, NULL);
            SendMessage(hBinEditCtl, WM_SETFONT, (LPARAM)hMonospaceFont, FALSE);
            SendMessage(hDecEditCtl, WM_SETFONT, (LPARAM)hMonospaceFont, FALSE);
            SendMessage(hHexEditCtl, WM_SETFONT, (LPARAM)hMonospaceFont, FALSE);
            _tcscpy(binEditBuffer, TEXT("0"));
            _tcscpy(decEditBuffer, TEXT("0"));
            _tcscpy(hexEditBuffer, TEXT("0"));
            SetWindowText(hBinEditCtl, binEditBuffer);
            SetWindowText(hDecEditCtl, decEditBuffer);
            SetWindowText(hHexEditCtl, hexEditBuffer);
            changedByUser = TRUE;
            break;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UNUSED_PARAMETER(hPrevInstance);
    UNUSED_PARAMETER(lpCmdLine);
    LPTSTR wndClassName = TEXT("wc");
    WNDCLASS wc;
    RECT rect;
    MSG msg;
    
    InitCommonControls();
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = wndClassName;
    RegisterClass(&wc);
    hDefGuiFont = GetStockObject(DEFAULT_GUI_FONT);
    hMonospaceFont = GetStockObject(ANSI_FIXED_FONT);
    rect.left = 0;
    rect.top = 0;
    rect.right = CLIENT_WIDTH;
    rect.bottom = CLIENT_HEIGHT;
    AdjustWindowRect(&rect, WS_CAPTION, FALSE);
    HWND hWnd = CreateWindow(
      wndClassName, TEXT("Binary/Decimal/Hexadecimal Converter"), WS_CAPTION | WS_SYSMENU,
      CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
      NULL, NULL, hInstance, NULL
    );
    ShowWindow(hWnd, nCmdShow);
    while(GetMessage(&msg, hWnd, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}