/*
	TVTest プラグイン
*/


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <tchar.h>

#define TVTEST_PLUGIN_CLASS_IMPLEMENT
#include "TVTestPlugin.h"


// プラグインクラス
class CPlugin : public TVTest::CTVTestPlugin
{
private:
	static LRESULT CALLBACK EventCallback(UINT Event, LPARAM lParam1, LPARAM lParam2, void* pClientData);
	static void SetLockWindowSizeWndProc(HWND hwnd);
	static void RestoreOriginalWndProc(HWND hwndz);
	friend LRESULT CALLBACK LockWindowSizeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	bool GetPluginInfo(TVTest::PluginInfo* pInfo) override;
	bool Initialize() override;
	bool Finalize() override;
};


bool CPlugin::GetPluginInfo(TVTest::PluginInfo* pInfo)
{
	// プラグインの情報を返す
	pInfo->Type = TVTest::PLUGIN_TYPE_NORMAL;
	pInfo->Flags = TVTest::PLUGIN_FLAG_ENABLEDEFAULT;
	pInfo->pszPluginName = L"ウィンドウサイズ固定";
	pInfo->pszCopyright = L"Public Domain";
	return true;
}


bool CPlugin::Initialize()
{
	// 初期化処理

	// イベントコールバック関数を登録
	m_pApp->SetEventCallback(EventCallback, this);

	return true;
}


bool CPlugin::Finalize()
{
	// 終了処理
	
	return true;
}


// イベントコールバック関数
// 何かイベントが起きると呼ばれる
LRESULT CALLBACK CPlugin::EventCallback(
	UINT Event, LPARAM lParam1, LPARAM lParam2, void* pClientData)
{
	CPlugin* pThis = static_cast<CPlugin*>(pClientData);
	auto appWindow = pThis->m_pApp->GetAppWindow();

	switch (Event)
	{
	case TVTest::EVENT_STARTUPDONE:
		SetLockWindowSizeWndProc(appWindow);
		return TRUE;

	case TVTest::EVENT_PLUGINENABLE:
	{
		auto enabled = lParam1;

		if (enabled)
		{
			SetLockWindowSizeWndProc(appWindow);
		}
		else
		{
			RestoreOriginalWndProc(appWindow);
		}
		return TRUE;
	}
	case TVTest::EVENT_CLOSE:
		RestoreOriginalWndProc(appWindow);
		return TRUE;
	}

	return 0;
}


// グローバル変数
static WNDPROC oldWndProc = nullptr;

// カスタムウィンドウプロシージャ
LRESULT CALLBACK LockWindowSizeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_SYSCOMMAND:
		// サイズ変更コマンドをキャンセルするが、移動は許可する
		if ((wParam & 0xfff0) == SC_SIZE)
		{
			return 0; // サイズ変更をキャンセルする
		}
		return CallWindowProc(oldWndProc, hwnd, msg, wParam, lParam);

	// ViewとかTitleBarとかStatusBarに吸い取られてSetCursorが上書きされる
	//case WM_SETCURSOR:
		//{
		//	// サイズ変更領域でのカーソル変更を防ぐ
		//	LRESULT hitTest = LOWORD(lParam);

		//	if (hitTest == HTBOTTOMRIGHT || hitTest == HTBOTTOM || hitTest == HTRIGHT ||
		//		hitTest == HTTOP || hitTest == HTTOPLEFT || hitTest == HTTOPRIGHT ||
		//		hitTest == HTBOTTOMLEFT || hitTest == HTLEFT)
		//	{
		//		SetCursor(LoadCursor(nullptr, IDC_ARROW));
		//		return TRUE;
		//	}

		//	// 他の領域ではデフォルトの処理
		//	return CallWindowProc(oldWndProc, hwnd, msg, wParam, lParam);
		//}
	case WM_CLOSE:
		CPlugin::RestoreOriginalWndProc(hwnd);
		PostMessage(hwnd, WM_CLOSE, wParam, lParam);
		return 0;

	default:
		return CallWindowProc(oldWndProc, hwnd, msg, wParam, lParam);
	}
}

void CPlugin::SetLockWindowSizeWndProc(HWND hwnd)
{
	if (oldWndProc == nullptr) {
		// 元のウィンドウプロシージャを取得
		oldWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)LockWindowSizeWndProc);
	}
}

void CPlugin::RestoreOriginalWndProc(HWND hwnd)
{
	if (oldWndProc) {
		(WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oldWndProc);
		oldWndProc = nullptr;
	}
}



TVTest::CTVTestPlugin* CreatePluginClass()
{
	return new CPlugin;
}
