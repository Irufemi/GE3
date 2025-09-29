#include "Function.h"

/*ウィンドウを作ろう*/

///ウィンドウプロシージャ

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

    /*開発UIを出そう*/

    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
        return true;
    }

    /*ウィンドウを作ろう*/

    //メッセージに応じてゲーム固有の処理を行う
    switch (msg) {
        //ウィンドウが破壊された
    case WM_DESTROY:
        //OSに対して、アプリの終了を伝える
        PostQuitMessage(0);
        return 0;
    }

    //標準のメッセージ処理を行う
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

