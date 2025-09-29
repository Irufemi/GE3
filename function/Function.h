#pragma once

/*プロジェクトを作ろう*/

#include <Windows.h>

/*DirectX12を初期化しよう*/

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>

/*CrashHandler*/

//Debug用のあれやこれやを使えるようにする
#include <DbgHelp.h>

#include <strsafe.h>

/*三角形を表示しよう*/

#include <dxcapi.h>
#include "../math/Vector4.h"

/*開発のUIを出そう*/

#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*テクスチャを貼ろう*/

#include "../externals/DirectXTex/DirectXTex.h"

/*テクスチャを正しく配置しよう*/

///事前準備

#include "../externals/DirectXTex/d3dx12.h"

#include <vector>

/*objファイルを読んでみよう*/

#include "../math/ModelData.h"
#include "../math/MaterialData.h"

#include <wrl.h>

#include "../math/ObjModel.h"


/*サウンド再生*/
#include "../math/SoundData.h"

/*ウィンドウを作ろう*/

///ウィンドウプロシージャ

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

/*ログを出そう*/

std::wstring ConvertString(const std::string& str);

std::string ConvertString(const std::wstring& str);

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {

    //時刻を取得して、時刻を名前に入れたファイルを作成。Dumpディレクトリ以下に出力
    SYSTEMTIME time;
    GetLocalTime(&time);
    wchar_t filePath[MAX_PATH] = { 0 };
    CreateDirectory(L"./Dumps", nullptr);
    StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
    HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
    //processId(このexeのId)とクラッシュ(例外)の発生したthreadIdを取得
    DWORD processId = GetCurrentProcessId();
    DWORD threadId = GetCurrentThreadId();
    //設定情報を入力
    MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
    minidumpInformation.ThreadId = threadId;
    minidumpInformation.ExceptionPointers = exception;
    minidumpInformation.ClientPointers = TRUE;
    //Dumpを出力。MiniDumpNormalは最低限の情報を出力するフラグ
    MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);
    //ほかに関連づけられているSEH例外ハンドラがあれば実行。通常はプロセスを終了する
    return EXCEPTION_EXECUTE_HANDLER;

}

/*三角形を表示しよう*/

IDxcBlob* CompileShader(
    //CompilerするShaderファイルへのパス
    const std::wstring& filePath,
    //Compilerに使用するProfile
    const wchar_t* profile,
    //初期化で生成したものを3つ
    const Microsoft::WRL::ComPtr<IDxcUtils>& dxcUtils,
    const Microsoft::WRL::ComPtr<IDxcCompiler3>& dxcCompiler,
    const Microsoft::WRL::ComPtr<IDxcIncludeHandler>& includeHandler,
    std::ostream& os
);

/*三角形の色を変えよう*/

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes);

/*開発用のUIを出そう*/

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(const Microsoft::WRL::ComPtr<ID3D12Device>& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

/*テクスチャを貼ろう*/

///Textureデータを読む

DirectX::ScratchImage LoadTexture(const std::string& flilePath);

///DirectX12のTextureResourceを作る

Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metadata);

//void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

/*テクスチャを正しく配置しよう*/

Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages, const Microsoft::WRL::ComPtr<ID3D12Device>& device, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);

/*前後関係を正しくしよう*/

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height);

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

/*objjファイルを読んでみよう*/

///　ModelData構造体と読み込み関数

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

ObjModel LoadObjFileM(const std::string& directoryPath, const std::string& filename);

// f行の頂点データを安全にパースする関数例
bool ParseObjFaceToken(const std::string& token, int& posIdx, int& uvIdx, int& normIdx);

/// MaterialData構造体と読み込み関数

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string filename);

/*サウンド再生*/

SoundData SoundLoadWave(const char* filename);

///音声データの解放

//音声データ解放
void SoundUnload(SoundData* soundData);

///サウンドの再生

//音声再生
void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);