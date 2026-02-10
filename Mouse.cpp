//--------------------------------------------------------------------------------------
// File: mouse.cpp
//
// 便利なマウスモジュール
//
//--------------------------------------------------------------------------------------
// 2020/02/11
//     DirectXTKより、なんちゃってC言語用にシェイプアップ改変
//
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------
#include "mouse.h"

#include <windowsx.h>
#include <assert.h>


#define SAFE_CLOSEHANDLE(h) if(h){CloseHandle(h); h = NULL;}


static Mouse_State        g_State = {};
static Mouse_State        g_StateOld = {};
static HWND               gWindow = NULL;
static Mouse_PositionMode gMode = MOUSE_POSITION_MODE_ABSOLUTE;
static HANDLE             gScrollWheelValue = NULL;
static HANDLE             gRelativeRead = NULL;
static HANDLE             gAbsoluteMode = NULL;
static HANDLE             gRelativeMode = NULL;
static int                gLastX = 0;
static int                gLastY = 0;
static int                gRelativeX = INT32_MAX;
static int                gRelativeY = INT32_MAX;
static bool               gInFocus = true;


static void clipToWindow(void);


void Mouse_Initialize(HWND window)
{
    RtlZeroMemory(&g_State, sizeof(g_State));

    assert(window != NULL);

    RAWINPUTDEVICE Rid;
    Rid.usUsagePage = 0x01 /* HID_USAGE_PAGE_GENERIC */;
    Rid.usUsage = 0x02     /* HID_USAGE_GENERIC_MOUSE */;
    Rid.dwFlags = RIDEV_INPUTSINK;
    Rid.hwndTarget = window;
    RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE));

    gWindow = window;
    gMode = MOUSE_POSITION_MODE_ABSOLUTE;

    if (!gScrollWheelValue) { gScrollWheelValue = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE); }
    if (!gRelativeRead) { gRelativeRead = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE); }
    if (!gAbsoluteMode) { gAbsoluteMode = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE); }
    if (!gRelativeMode) { gRelativeMode = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE); }

    gLastX = 0;
    gLastY = 0;
    gRelativeX = INT32_MAX;
    gRelativeY = INT32_MAX;

    gInFocus = true;
}

void Mouse_Finalize(void)
{
    SAFE_CLOSEHANDLE(gScrollWheelValue);
    SAFE_CLOSEHANDLE(gRelativeRead);
    SAFE_CLOSEHANDLE(gAbsoluteMode);
    SAFE_CLOSEHANDLE(gRelativeMode);
}

void Mouse_GetState(Mouse_State* pState)
{
    memcpy(pState, &g_State, sizeof(g_State));
    pState->positionMode = gMode;

    DWORD Result = WaitForSingleObjectEx(gScrollWheelValue, 0, FALSE);
    if (Result == WAIT_FAILED) { return; }

    if (Result == WAIT_OBJECT_0) {

        pState->scrollWheelValue = 0;
    }

    if (pState->positionMode == MOUSE_POSITION_MODE_RELATIVE) {

        Result = WaitForSingleObjectEx(gRelativeRead, 0, FALSE);
        if (Result == WAIT_FAILED) { return; }

        if (Result == WAIT_OBJECT_0) {
            pState->x = 0;
            pState->y = 0;
        }
        else {
            SetEvent(gRelativeRead);
        }
    }
}

void Mouse_ResetScrollWheelValue(void)
{
    SetEvent(gScrollWheelValue);
}

void Mouse_SetMode(Mouse_PositionMode mode)
{
    if (gMode == mode)
        return;

    SetEvent((mode == MOUSE_POSITION_MODE_ABSOLUTE) ? gAbsoluteMode : gRelativeMode);

    assert(gWindow != NULL);

    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.dwFlags = TME_HOVER;
    tme.hwndTrack = gWindow;
    tme.dwHoverTime = 1;
    TrackMouseEvent(&tme);
}

bool Mouse_IsConnected(void)
{
    return GetSystemMetrics(SM_MOUSEPRESENT) != 0;
}

bool Mouse_IsVisible(void)
{
    if (gMode == MOUSE_POSITION_MODE_RELATIVE) {
        return false;
    }

    CURSORINFO info = { sizeof(CURSORINFO), 0, nullptr, {} };
    GetCursorInfo(&info);

    return (info.flags & CURSOR_SHOWING) != 0;
}

void Mouse_SetVisible(bool visible)
{
    if (gMode == MOUSE_POSITION_MODE_RELATIVE) {
        return;
    }

    CURSORINFO info = { sizeof(CURSORINFO), 0, nullptr, {} };
    GetCursorInfo(&info);

    bool isVisible = (info.flags & CURSOR_SHOWING) != 0;

    if (isVisible != visible) {
        ShowCursor(visible);
    }
}

void Mouse_ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    HANDLE evts[3] = {
        gScrollWheelValue,
        gAbsoluteMode,
        gRelativeMode
    };

    switch (WaitForMultipleObjectsEx(_countof(evts), evts, FALSE, 0, FALSE))
    {
    case WAIT_OBJECT_0:
        g_State.scrollWheelValue = 0;
        ResetEvent(evts[0]);
        break;

    case (WAIT_OBJECT_0 + 1):
    {
        gMode = MOUSE_POSITION_MODE_ABSOLUTE;
        ClipCursor(nullptr);

        POINT point;
        point.x = gLastX;
        point.y = gLastY;

        // リモートディスクトップに対応するために移動前にカーソルを表示する
        ShowCursor(TRUE);

        if (MapWindowPoints(gWindow, nullptr, &point, 1)) {
            SetCursorPos(point.x, point.y);
        }

        g_State.x = gLastX;
        g_State.y = gLastY;
    }
    break;

    case (WAIT_OBJECT_0 + 2):
    {
        ResetEvent(gRelativeRead);

        gMode = MOUSE_POSITION_MODE_RELATIVE;
        g_State.x = g_State.y = 0;
        gRelativeX = INT32_MAX;
        gRelativeY = INT32_MAX;

        ShowCursor(FALSE);

        clipToWindow();
    }
    break;

    case WAIT_FAILED:
        return;
    }

    switch (message)
    {
    case WM_ACTIVATEAPP:
        if (wParam) {

            gInFocus = true;

            if (gMode == MOUSE_POSITION_MODE_RELATIVE) {

                g_State.x = g_State.y = 0;
                ShowCursor(FALSE);
                clipToWindow();
            }
        }
        else {
            int scrollWheel = g_State.scrollWheelValue;
            memset(&g_State, 0, sizeof(g_State));
            g_State.scrollWheelValue = scrollWheel;
            gInFocus = false;
        }
        return;

    case WM_INPUT:
        if (gInFocus && gMode == MOUSE_POSITION_MODE_RELATIVE) {

            RAWINPUT raw;
            UINT rawSize = sizeof(raw);

            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &rawSize, sizeof(RAWINPUTHEADER));

            if (raw.header.dwType == RIM_TYPEMOUSE) {

                if (!(raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) {

                    g_State.x = raw.data.mouse.lLastX;
                    g_State.y = raw.data.mouse.lLastY;

                    ResetEvent(gRelativeRead);
                }
                else if (raw.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP) {

                    // リモートディスクトップなどに対応
                    const int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                    const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

                    int x = (int)((raw.data.mouse.lLastX / 65535.0f) * width);
                    int y = (int)((raw.data.mouse.lLastY / 65535.0f) * height);

                    if (gRelativeX == INT32_MAX) {
                        g_State.x = g_State.y = 0;
                    }
                    else {
                        g_State.x = x - gRelativeX;
                        g_State.y = y - gRelativeY;
                    }

                    gRelativeX = x;
                    gRelativeY = y;

                    ResetEvent(gRelativeRead);
                }
            }
        }
        return;


    case WM_MOUSEMOVE:
        break;

    case WM_LBUTTONDOWN:
        g_State.leftButton = true;
        break;

    case WM_LBUTTONUP:
        g_State.leftButton = false;
        break;

    case WM_RBUTTONDOWN:
        g_State.rightButton = true;
        break;

    case WM_RBUTTONUP:
        g_State.rightButton = false;
        break;

    case WM_MBUTTONDOWN:
        g_State.middleButton = true;
        break;

    case WM_MBUTTONUP:
        g_State.middleButton = false;
        break;

    case WM_MOUSEWHEEL:
        g_State.scrollWheelValue += GET_WHEEL_DELTA_WPARAM(wParam);
        return;

    case WM_XBUTTONDOWN:
        switch (GET_XBUTTON_WPARAM(wParam))
        {
        case XBUTTON1:
            g_State.xButton1 = true;
            break;

        case XBUTTON2:
            g_State.xButton2 = true;
            break;
        }
        break;

    case WM_XBUTTONUP:
        switch (GET_XBUTTON_WPARAM(wParam))
        {
        case XBUTTON1:
            g_State.xButton1 = false;
            break;

        case XBUTTON2:
            g_State.xButton2 = false;
            break;
        }
        break;

    case WM_MOUSEHOVER:
        break;

    default:
        // マウスに対するメッセージは無かった…
        return;
    }

    if (gMode == MOUSE_POSITION_MODE_ABSOLUTE) {

        // すべてのマウスメッセージに対して新しい座標を取得する
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        g_State.x = gLastX = xPos;
        g_State.y = gLastY = yPos;
    }
}


// -------------------------------------------------- 追加分
// なぜかこいつは機能しない？
void MouseUpdate()
{
    if (Mouse_IsConnected()) // セーフ
    {
        Mouse_KeyCopy();
    }
}

// --------------------------------------------------
// クリックをしている間
// --------------------------------------------------
bool Mouse_IsClick(MOUSE_KEY key)
{
    return Mouse_IsClick(key, &g_State);
}

bool Mouse_IsClick(MOUSE_KEY key, const Mouse_State* pState)
{
    if (key <= 0x0004)
    {
        unsigned int* p = (unsigned int*)pState;
        return (p[0] & key) != 0; // 現在の state とキーのコードを AND演算する
    }
    return false;
}

// --------------------------------------------------
// クリックをやめたとき
// --------------------------------------------------
bool Mouse_IsClickUp(MOUSE_KEY key)
{
    return Mouse_IsClickUp(key, &g_State);
}

bool Mouse_IsClickUp(MOUSE_KEY key, const Mouse_State* pState)
{
    if (key <= 0x0004)
    {
        unsigned int* p = (unsigned int*)pState;
        unsigned int* pOld = (unsigned int*)&g_StateOld;

        return ((p[0] & key) == 0) && ((pOld[0] & key) != 0); // 現在離されていて、１フレーム前は押されていた
    }
    return false;
}

// --------------------------------------------------
// クリックをしたときだけ
// --------------------------------------------------
bool Mouse_IsClickTrigger(MOUSE_KEY key)
{
    return Mouse_IsClickTrigger(key, &g_State);
}

bool Mouse_IsClickTrigger(MOUSE_KEY key, const Mouse_State* pState)
{
    if (key <= 0x0004)
    {
        unsigned int* p = (unsigned int*)pState;
        unsigned int* pOld = (unsigned int*)&g_StateOld;

        return ((p[0] & key) != 0 && (pOld[0] & key) == 0);
    }
    return false;
}

bool Mouse_IsWheelScroll()
{
    return g_State.scrollWheelValue != 0;
}

XMFLOAT2 Mouse_GetPosition()
{
    return { (float)g_State.x, (float)g_State.y };
}

XMFLOAT2 Mouse_GetPositionOld()
{
    return { (float)g_StateOld.x, (float)g_StateOld.y };
}

XMFLOAT2 Mouse_GetPositionDiff()
{
    return { (float)g_State.x - (float)g_StateOld.x, (float)g_State.y - (float)g_StateOld.y };
}

// --------------------------------------------------
// マウスのX座標を返す
// --------------------------------------------------
int Mouse_GetPositionX()
{
    return g_State.x;
}
// --------------------------------------------------
// マスウのY座標を返す
// --------------------------------------------------
int Mouse_GetPositionY()
{
    return g_State.y;
}

// --------------------------------------------------
// マウスホイールの回転を返す
// そのフレームのマウスホイールの値の正負を返す。
// 毎フレーム値を０にする                            ----------（不具合が起きたらその時に対応）
// 
// 返り値：
// 　上：＋１
// 　下：－１
// 　回してない：０
// --------------------------------------------------
int Mouse_GetWheelScroll()
{
    int value = g_State.scrollWheelValue;
    //Mouse_ResetScrollWheelValue();
    g_State.scrollWheelValue = 0;

    if (value > 0)
        return 1;
    else if (value < 0)
        return -1;
    else
        return 0;
}

// --------------------------------------------------
// マウスホイールの値を返す
// --------------------------------------------------
int Mouse_GetWheelValue()
{
    return g_State.scrollWheelValue;
}

// --------------------------------------------------
// マウスの State の記録
// --------------------------------------------------
void Mouse_KeyCopy()
{
    g_StateOld = g_State;
}

void clipToWindow(void)
{
    assert(gWindow != NULL);

    RECT rect;
    GetClientRect(gWindow, &rect);

    POINT ul;
    ul.x = rect.left;
    ul.y = rect.top;

    POINT lr;
    lr.x = rect.right;
    lr.y = rect.bottom;

    MapWindowPoints(gWindow, NULL, &ul, 1);
    MapWindowPoints(gWindow, NULL, &lr, 1);

    rect.left = ul.x;
    rect.top = ul.y;

    rect.right = lr.x;
    rect.bottom = lr.y;

    ClipCursor(&rect);
}

