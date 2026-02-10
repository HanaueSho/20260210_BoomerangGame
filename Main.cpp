/*
	main.cpp
	2025  hanaue sho
	WinMain のエントリポイント
*/
#include "Main.h"
#include "DxDevice.h"
#include "SwapChainManager.h"
#include "Renderer.h"

#include "AudioSystem.h"
#include "Manager.h"
#include "Mouse.h"
#include "Keyboard.h"
#include <thread>
#include <chrono>

#include "DebugCount.h"

const char* CLASS_NAME = "AppClass";
const char* WINDOW_NAME = "DX11ゲーム";

// ==================================================
// ----- グローバル変数 -----
// ==================================================
static HWND			 g_Window;
static DxDevice		 g_Device;
static SwapChainManager g_Swap;

// ==================================================
// ----- プロトタイプ宣言 -----
// ==================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND GetWindow()
{
	return g_Window;
}


// ==================================================
// ----- Main -----
// ==================================================
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// ==================================================
	// ----- Windowの生成 -----
	// ==================================================
	WNDCLASSEX wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = 0;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = nullptr;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = CLASS_NAME;
		wcex.hIconSm = nullptr;

		RegisterClassEx(&wcex);


		RECT rc = { 0, 0, (LONG)SCREEN_WIDTH, (LONG)SCREEN_HEIGHT };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		g_Window = CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
	}

	// ==================================================
	// ----- DirectX 初期化 -----
	// ==================================================
	g_Device.Create(); // Device, DeviceContext 作成
	g_Swap.Create(g_Window, SCREEN_WIDTH, SCREEN_HEIGHT, g_Device.Device(), g_Device.DeviceContext()); // RTV, DSV 作成
	Renderer::Init(g_Device.Device(), g_Device.DeviceContext());

	// ==================================================
	// ----- アプリ初期化 -----
	// ==================================================
	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	AudioSystem::Init();
	Mouse_Initialize(GetWindow());
	Keyboard_Initialize();
	Manager::Init();

	// デバッグウィンドウ生成
	AllocConsole(); // コンソールを作成
	FILE* a = freopen("CONOUT$", "w", stdout); // stdoutをコンソールにリダイレクト
	printf("Hello, World!\n"); // コンソールに出力


	ShowWindow(g_Window, nCmdShow);
	UpdateWindow(g_Window);

	// ==================================================
	// ----- ループ処理 ----- （アキュムレータ方式）
	// ==================================================
	using clock = std::chrono::steady_clock;
	auto prev = clock::now();

	double accumulator = 0.0f;
	const double fixedDt = 1.0 / 60.0; // 物理レート統一

	// FPS用の積算
	double fpsElapsed = 0.0;
	int fpsFrames = 0;
	int lastPhysSteps = 0; // 直近フレームで何回 FixedStep したか

	bool running = true;
	MSG msg;
	while(running)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) // メッセージの処理
		{
			if (msg.message == WM_QUIT)
			{
				running = false; // ループ終了
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (!running) break;
		

		// ----- 時間計測 -----
		auto now = clock::now(); // 現在のフレームの時間
		double dt = std::chrono::duration<double>(now - prev).count(); // 前のフレームの時間と今のフレームの時間の差分
		prev = now; // 現在のフレームの時間の保存
		//dt = std::min(dt, 0.05); // フレーム最大 50ms

		// ----- 物理 -----
		//accumulator = std::min(accumulator + dt, 0.25); // 累積最大 250 ms
		accumulator += dt; 
		int steps = 0; 
		const int maxSteps = 6; // スパイラル対策
		while (accumulator >= fixedDt && steps < maxSteps)
		{
			// FixedUpdate （この中で PhysicsSytem が呼ばれているよ）
			Manager::FixedUpdate(fixedDt);
			
			// カウント
			accumulator -= fixedDt;
			steps++;
		}
		lastPhysSteps = steps;

		// ==================================================
		// ----- ゲーム更新、描画 -----
		// ==================================================
		g_Swap.BindAsRenderTarget();
		const float clearColor[4] = { 0.1f, 0.3f, 0.5f, 1.0f };
		g_Swap.Clear(clearColor);

		Manager::Update(dt);
		Manager::Draw();

		g_Swap.Present(1);

		MouseUpdate();
		keycopy();


		// ----- FPS計測とタイトル更新（１秒ごと） -----
		fpsElapsed += dt; // 毎ループ時間カウント
		fpsFrames += 1; // 毎ループフレームカウント
		if (fpsElapsed >= 1.0f) 
		{
			double fps = fpsFrames / fpsElapsed; // １秒平均FPS
			double ms = 1000.0 / (fps > 0 ? fps : 1); // 目安の１フレ所要時間
		#ifdef UNICODE
			wchar_t title[128];
			swprintf_s(title, L"%hs | FPS: %.1f (%.2f ms) | PhysSteps: %d", WINDOW_NAME, fps, ms, lastPhysSteps);
			SetWindowTextW(g_Window, title); // ウィンドウのバーに表示
		#else
			char title[128];
			snprintf(title, sizeof(title), "%s | FPS: %.1f (%.2f ms) | PhysSteps: %d", WINDOW_NAME, fps, ms, lastPhysSteps);
			SetWindowTextA(g_Window, title); // ウィンドウのバーに表示
		#endif		
			// 積算をリセット
			fpsElapsed -= 1.0f;
			fpsFrames	= 0;
		}
	} // while 終了

	timeEndPeriod(1);

	UnregisterClass(CLASS_NAME, wcex.hInstance);

	// ==================================================
	// ----- 終了処理 -----
	// ==================================================
	Renderer::Uninit();
	g_Swap.Destroy();
	Manager::Uninit();
	AudioSystem::Shutdown();

	CoUninitialize();

	return (int)msg.wParam;
}

// --------------------------------------------------
// WndProc
// Windowsのメッセージ配送先
// ウィンドウに発生する全イベント（キー、マウス、リサイズ、破棄など）がここに届く
// 自分で処理したメッセージ：return 0;
// 自分で処理しないメッセージ：DefWindowProcへ渡す（既定処理）
// --------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED && g_Swap.SwapChain())
		{
			UINT width  = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			g_Swap.Resize(width, height);
		}
		return 0;

	// キーボード（通常キー）-----
	case WM_KEYDOWN: // 物理キーが押された（オートリピートあり）
	case WM_KEYUP:	 // 物理キーが離された
		Keyboard_ProcessMessage(uMsg, wParam, lParam);
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		}
		return 0;

	// システムキー（ALT／F10押下絡み）-----
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		Keyboard_ProcessMessage(uMsg, wParam, lParam);
		break;
	
	// アプリのアクティブ／非アクティブの切り替え -----
	case WM_ACTIVATEAPP:
		Keyboard_ProcessMessage(uMsg, wParam, lParam); // ここで入力状態のクリアなどを行うと安全
		return 0;
		
	// RAW INPUT （生の入力：登録していれば飛んでくれる）
	case WM_INPUT:
		Mouse_ProcessMessage(uMsg, wParam, lParam);  // RAW マウスを使うならこちら
		return 0;

	// マウス（非RAWの通常メッセージ）-----
	case WM_MOUSEMOVE:	 // マウス移動
	case WM_LBUTTONDOWN: // 左クリック押下
	case WM_LBUTTONUP:	 // 左クリック離上
	case WM_RBUTTONDOWN: // 右クリック押下
	case WM_RBUTTONUP:	 // 右クリック離上
	case WM_MBUTTONDOWN: // 中央ボタン押下
	case WM_MBUTTONUP:	 // 中央ボタン離上
	case WM_MOUSEWHEEL:	 // マウスホイール
	case WM_XBUTTONDOWN: // サイドボタン押下
	case WM_XBUTTONUP:	 // サイドボタン離上
	case WM_MOUSEHOVER:
		Mouse_ProcessMessage(uMsg, wParam, lParam);
		return 0;

	// ウィンドウの破棄 -----
	case WM_DESTROY:
		PostQuitMessage(0); // メインループに WM_QUIT が入る
		return 0;
	default:
		break;
	}

	// ここに来たものは既定処理（サイズ変更、非クライアント領域、最小化等）
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

