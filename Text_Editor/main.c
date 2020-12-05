#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <stdlib.h>
#include <assert.h>

#include "Model.h"
#include "Viewer.h"
#include "Menu.h"

#include "WindowData.h"


/**
 * @brief Loads the file and fills windowDataPtr's fields.
 * @param[in] hwnd Descriptor of current window.
 * @param[in] windowDataPtr Struct which fields should be changed according to opened file.
 */
static void Open_File(HWND hwnd, WindowData_t* windowDataPtr){
    OPENFILENAME ofn;
    char file_name[MAX_PATH] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile  = file_name;
    ofn.lpstrFilter = "Text Files(*.txt)\0*.txt\0\0";
    ofn.nFilterIndex = 1;
    ofn.nMaxFile = sizeof(file_name);
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if(GetOpenFileName(&ofn)){
        HDC hdc = GetDC(hwnd);

        DeleteModel(windowDataPtr->modelPtr);
        DeleteViewer(windowDataPtr->viewerPtr);

        windowDataPtr->modelPtr = calloc(1, sizeof(Model));
        windowDataPtr->viewerPtr = calloc(1, sizeof(Viewer));

        InitViewer(hdc, windowDataPtr->viewerPtr);
        InitModel(file_name, windowDataPtr->modelPtr);
    }
}



LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

TCHAR szClassName[ ] = _T("TextReaderWindow");

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;
    MSG messages;
    WNDCLASSEX wincl;

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_OWNDC | CS_DBLCLKS;
    wincl.cbSize = sizeof (WNDCLASSEX);

    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = sizeof(WindowData_t*);
    wincl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

    if (!RegisterClassEx (&wincl))
        return 0;

    hwnd = CreateWindowEx (
           0,
           szClassName,
           _T("Text Viewer"),
           WS_OVERLAPPEDWINDOW|WS_VSCROLL|WS_HSCROLL,
           CW_USEDEFAULT,
           CW_USEDEFAULT,
           544,
           375,
           HWND_DESKTOP,
           NULL,
           hThisInstance,
           lpszArgument
           );

    ShowWindow (hwnd, nCmdShow);

    while (GetMessage (&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return messages.wParam;
}


LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;

    WindowData_t* windowDataPtr = (WindowData_t*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    Viewer* viewerPtr = windowDataPtr == NULL ? NULL : windowDataPtr->viewerPtr;
    ViewerData* viewerDataPtr = viewerPtr == NULL ? NULL : viewerPtr->viewerData;
    Model* modelPtr = windowDataPtr == NULL ? NULL : windowDataPtr->modelPtr;

    switch (message)
    {
        case WM_DESTROY: {
            DeleteModel(modelPtr);
            DeleteViewer(viewerPtr);
            PostQuitMessage(0);
            break;
        }
        case WM_CREATE:{
            WindowData_t*  winDataPtr = calloc(1, sizeof(WindowData_t));
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG) winDataPtr);

            hdc = GetDC(hwnd);
            HFONT hFont = CreateFont(20, 10, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, "Consolas");
            SelectObject(hdc, hFont);
            ReleaseDC(hwnd, hdc);
            HMENU hmenu = CreateMenu();
            HMENU hSubFileMenu = CreatePopupMenu();
            HMENU hSubViewMenu = CreatePopupMenu();

            AppendMenu(hSubFileMenu, MF_STRING, IDM_OPEN,"Open");
            AppendMenu(hSubFileMenu, MF_STRING, IDM_CLOSE,"Close");
            AppendMenu(hSubFileMenu, MF_STRING, IDM_EXIT,"Exit");

            AppendMenu(hSubViewMenu, MF_STRING | MF_UNCHECKED, IDM_CHANGE_MODE, "Layout");

            AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR) hSubFileMenu, "File");
            AppendMenu(hmenu, MF_STRING | MF_POPUP, (UINT_PTR) hSubViewMenu, "View");

            SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
            SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);

            SetMenu(hwnd, hmenu);
            break;
        }
        case WM_PAINT:{
            if(viewerPtr != NULL && modelPtr != NULL) {
                PAINTSTRUCT ps;
                hdc = BeginPaint(hwnd, &ps);
                viewerPtr->print(modelPtr, hdc, viewerDataPtr, ps.rcPaint);
                EndPaint(hwnd, &ps);
                ReleaseDC(hwnd, hdc);
            }
            break;
        }
        case WM_KEYDOWN:{
            if(viewerPtr == NULL || modelPtr == NULL){
                break;
            }
            int shift = 1;
            switch (wParam) {
                case VK_UP:
                    shift = -1;
                case VK_DOWN:
                    if(viewerPtr->changeLine(viewerDataPtr, shift)){
                        assert(viewerDataPtr->curPos / viewerDataPtr->linesPerScroll < MAX_SCROLL);
                        ScrollWindow(hwnd, 0,-viewerDataPtr->cyChar * shift * viewerDataPtr->linesPerScroll,
                                     NULL, NULL);
                        SetScrollPos(hwnd, SB_VERT, viewerDataPtr->curPos / viewerDataPtr->linesPerScroll, TRUE);
                    }
                    break;
                case VK_LEFT:
                    shift = -1;
                case VK_RIGHT:
                    HorizontalShift(viewerDataPtr, shift);
                    SetScrollPos(hwnd, SB_HORZ, viewerDataPtr->curHorizontalShift, TRUE);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
            }
            UpdateWindow(hwnd);
            break;
        }
        case WM_VSCROLL: {
            if(viewerPtr == NULL || modelPtr == NULL){
                break;
            }
            int deltaLine = 0;
            switch (LOWORD(wParam)) {
                case SB_LINEDOWN:
                    deltaLine = 1;
                    break;
                case SB_LINEUP:
                    deltaLine = -1;
                    break;
                case SB_PAGEUP:
                    deltaLine = -1;
                case SB_PAGEDOWN: {
                    deltaLine = deltaLine == 0 ? 1 : -1;
                    RECT rect;
                    if(GetClientRect(hwnd, &rect)) {
                        int height = rect.bottom - rect.top;
                        deltaLine *=  height / viewerDataPtr->cyChar / viewerDataPtr->linesPerScroll;
                    }
                    break;
                }
                case SB_THUMBTRACK:
                    deltaLine = HIWORD(wParam) - GetScrollPos(hwnd, SB_VERT);
                    break;
            }

            if(viewerPtr->changeLine(viewerDataPtr, deltaLine)){
                ScrollWindow(hwnd, 0,-viewerDataPtr->cyChar * deltaLine * viewerDataPtr->linesPerScroll,
                             NULL, NULL);
                SetScrollPos(hwnd, SB_VERT,  viewerDataPtr->curPos / viewerDataPtr->linesPerScroll, TRUE);
                UpdateWindow(hwnd);
            }
            break;
        }
        case WM_HSCROLL:{
            if(viewerPtr == NULL || modelPtr == NULL){
                break;
            }
            int shift = 0;
            switch (LOWORD(wParam)) {
                case SB_LINERIGHT:
                    shift = 1;
                    break;
                case SB_LINELEFT:
                    shift = -1;
                    break;
                case SB_THUMBTRACK:
                    shift = HIWORD(wParam) - viewerDataPtr->curHorizontalShift;
                    break;
            }
            HorizontalShift(viewerDataPtr, shift);
            SetScrollPos(hwnd, SB_HORZ, viewerDataPtr->curHorizontalShift, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_SIZE:{
            if(viewerPtr != NULL && modelPtr != NULL) {
                viewerPtr->resizeWindow(modelPtr, viewerDataPtr, HIWORD(lParam), LOWORD(lParam), hwnd);

            }
            break;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDM_EXIT:
                    DestroyWindow(hwnd);
                    break;
                case IDM_CLOSE:
                    if(viewerPtr == NULL || modelPtr == NULL){
                        break;
                    }
                    DeleteModel(modelPtr);
                    DeleteViewer(viewerPtr);
                    windowDataPtr->modelPtr = NULL;
                    windowDataPtr->viewerPtr = NULL;
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case IDM_CHANGE_MODE: {
                    HMENU hmenu = GetMenu(hwnd);
                    HMENU hViewMenu = GetSubMenu(hmenu, 1);
                    if (GetMenuState(hViewMenu, IDM_CHANGE_MODE, MF_BYCOMMAND) == MF_CHECKED) {
                        CheckMenuItem(hViewMenu, IDM_CHANGE_MODE, MF_BYCOMMAND | MF_UNCHECKED);
                    } else {
                        CheckMenuItem(hViewMenu, IDM_CHANGE_MODE, MF_BYCOMMAND | MF_CHECKED);
                    }
                    if (viewerPtr == NULL || modelPtr == NULL) {
                        break;
                    }
                    ChangeMode(viewerPtr, modelPtr, hwnd);
                    break;
                }
                case IDM_OPEN: {
                    Open_File(hwnd, windowDataPtr);
                    RECT rect;
                    if(GetClientRect(hwnd, &rect))
                    {
                        int width = rect.right - rect.left;
                        int height = rect.bottom - rect.top;
                        windowDataPtr->viewerPtr->resizeWindow(windowDataPtr->modelPtr, windowDataPtr->viewerPtr->viewerData,
                                                               height, width, hwnd);
                    }
                    HMENU hmenu = GetMenu(hwnd);
                    HMENU hViewMenu = GetSubMenu(hmenu, 1);
                    CheckMenuItem(hViewMenu, IDM_CHANGE_MODE, MF_BYCOMMAND | MF_UNCHECKED);

                    InvalidateRect(hwnd, NULL, TRUE);
                    UpdateWindow(hwnd);
                    break;
                }
            }
            break;
        }
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
