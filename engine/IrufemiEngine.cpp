#include "IrufemiEngine.h"
#include "../function/Function.h"

#include <cassert>
#include <DbgHelp.h>
#include <cstdint>
#include <format>

#include "../math/VertexData.h"
#include "../source/D3D12ResourceUtil.h"

#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxcompiler.lib")

//デストラクタ
IrufemiEngine::~IrufemiEngine() { Finalize(); }

// 初期化
void IrufemiEngine::Initialize(const std::wstring& title, const int32_t& clientWidth, const int32_t& clientHeight) {
    
    clientWidth_ = clientWidth;
    clientHeight_ = clientHeight;

    /*テクスチャを貼ろう*/

    ///COMの初期化

    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    assert(SUCCEEDED(hr));

    // AudioManagerの生成・Media Foundationの初期化
    audioManager_ = std::make_unique<AudioManager>();
    audioManager_->StartUp();

    /*CrashHandler*/
    SetUnhandledExceptionFilter(ExportDump);

    // ログを出せるようにする
    log_ = std::make_unique<Log>();
    log_->Initialize();


    /*ウィンドウを作ろう*/

    ///ウィンドウクラスを登録する

    WNDCLASS wc{};
    //ウィンドウプロシージャ
    wc.lpfnWndProc = WindowProc;
    //ウィンドウクラス名(なんでもいい)
    wc.lpszClassName = L"CG3WindowClass";
    //インスタンスハンドル
    wc.hInstance = GetModuleHandle(nullptr);
    //カーソル
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    //ウィンドウクラスを登録する
    RegisterClass(&wc);

    ///ウィンドウサイズを決める


    //ウィンドウサイズを表す構造体にクライアント領域を入れる
    RECT wrc = { 0,0,clientWidth_ ,clientHeight_ };

    //クライアント領域をもとに実際のサイズにwrcを変更してもらう
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    ///ウィンドウを生成して表示

    //ウィンドウの生成
    hwnd_ = CreateWindow(
        wc.lpszClassName,		//利用するクラス名
        title.c_str(),			        //タイトルバーの文字(何でも良い)
        WS_OVERLAPPEDWINDOW,	//よく見るウィンドウスタイル
        CW_USEDEFAULT,			//表示X座標(windowsに任せる)
        CW_USEDEFAULT,			//表示Y座標(windowsに任せる)
        wrc.right - wrc.left,	//ウィンドウ横幅
        wrc.bottom - wrc.top,	//ウィンドウ縦幅
        nullptr,				//親ウィンドウハンドル
        nullptr,				//メニューハンドル
        wc.hInstance,			//インスタンスハンドル
        nullptr					//オプション
    );

    /*エラー放置ダメ、絶対*/

    ///DebugLayer(デバッグレイヤー)

#ifdef _DEBUG
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController_.GetAddressOf())))) {
        //デバッグレイヤーを有効化する
        debugController_->EnableDebugLayer();
        //さらにGPU側でもチェックを行うようにする
        debugController_->SetEnableGPUBasedValidation(TRUE);
    }
#endif

    /*ウィンドウを作ろう*/

    ///ウィンドウを生成して表示

    //ウィンドウを表示する
    ShowWindow(hwnd_, SW_SHOW);

    /*DirectX12を初期化しよう*/

    ///DXGIFactoryの生成

    //DXGIファクトリーの生成
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
    //HRESULTはWndows系のエラーコードであり、
    //関数が成功したかどうかをSUCCEEDEDマクロで判定できる
    hr = CreateDXGIFactory(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
    //初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
    assert(SUCCEEDED(hr));

    ///使用するアダプタ(GPU)を決定する

    //使用するアダプタ用の変数。最初にnullptrを入れておく
    Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
    //良い順にアダプタを頼む
    for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(useAdapter.GetAddressOf())) != DXGI_ERROR_NOT_FOUND; i++) {
        //アダプター情報を取得する
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr = useAdapter->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr)); //取得できないのは一大事
        //ソフトウェアアダプタでなければ採用!
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            //採用したアダプタの情報をログに出力。wstringの方なので注意
            OutPutLog(log_->GetLogStream(), ConvertString(std::format(L"use Adapter:{}\n", adapterDesc.Description)));
            break;
        }
        useAdapter = nullptr; //ソフトウェアアダプタの場合は見なかったことにする
    }
    //適切なアダプタが見つからなかったので起動できない
    assert(useAdapter != nullptr);

    ///D3D12Deviceの生成

    //機能レベルとログ出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
    };
    const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
    //高い順に生成できるか試してく
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        //採用したアダプターでデバイスを生成
        hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(device_.GetAddressOf()));
        
        //指定した機能レベルでデバイスが生成できたかを確認
        if (SUCCEEDED(hr)) {
            //生成できたのでログ出力を行ってループを抜ける
            OutPutLog(log_->GetLogStream(), std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
            auto msg = std::format("Resource[{}] created at {} in {}:{}\n", "ID3D12Device", static_cast<const void*>(device_.Get()), __FILE__, __LINE__);
            OutputDebugStringA(msg.c_str());
            break;
        }
    }
    //デバイス生成がうまくいかなかったので起動できない
    assert(device_ != nullptr);
    OutPutLog(log_->GetLogStream(), "Complete create D3D12Device!!!\n"); //初期化完了のログを出す


    inputManager_ = std::make_unique<InputManager>();
    inputManager_->Initialize();

    // 生成が完了したのでuseAdapterを解放
    if (useAdapter) { useAdapter.Reset(); }

    // AudioManagerの初期化
    audioManager_->Initialize();

    // "resources"フォルダから音声ファイルをすべてロード
    audioManager_->LoadAllSoundsFromFolder("resources/");

    /*エラー放置ダメ、絶対*/

    ///エラー・警告、即ち停止

#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
    if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        //ヤバイエラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        //エラー時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        //警告時に止まる
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        ///エラーと警告の抑制(Windowsの不具合によるエラー表記などは解消不能。その場合に停止させないよう設定を行う)

        //抑制するメッセージのID
        D3D12_MESSAGE_ID denyIds[] = {
            //Windows11でのDXGIデバッグプレイヤーとDX12デバッグプレイヤーの相互作用バグによるエラーメッセージ
            //https://stackoverflow.com/questions/69805245/directXx-12-application-is-crashing-in-windows-11
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };
        //抑制するレベル
        D3D12_MESSAGE_SEVERITY severties[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severties);
        filter.DenyList.pSeverityList = severties;
        //指定したメッセージの表示を抑制する
        infoQueue->PushStorageFilter(&filter);

        ///エラー・警告、即ち停止

        //解放
        infoQueue.Reset();
    }
#endif

    /*画面の色を変えよう*/

    ///CommandQueueを生成する

    //コマンドキューを生成する
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(commandQueue_.GetAddressOf()));
    //コマンドキューの生成がうまくいかないので起動できない
    assert(SUCCEEDED(hr));

    ///CommandListを生成する

    //コマンドアロケータを生成する
    hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator_.GetAddressOf()));
    //コマンドアロケータの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    //コマンドリストを生成する
    hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(commandList_.GetAddressOf()));
    //コマンドリストの生成がうまくいかなかったので起動できない
    assert(SUCCEEDED(hr));

    ///SwapChainを生成する

    //スワップチェーンを生成する
    swapChainDesc_.Width = clientWidth_; //画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc_.Height = clientHeight_; //画面の高さ。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //色の形式
    swapChainDesc_.SampleDesc.Count = 1; //マルチサンプルしない
    swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //描画のターゲットとして利用する
    swapChainDesc_.BufferCount = 2; //ダブルバッファ
    swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //モニタにうつしたら、中身を破棄
    //コマンドキュー、ウィンドウハンドル、設定を渡して生成する
    hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue_.Get(), hwnd_, &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
    assert(SUCCEEDED(hr));

    // 作成したのでdxgiFactoryを解放
    if (dxgiFactory) { dxgiFactory.Reset(); }


    /*テクスチャを切り替えよう*/

    //DescriptorSize

    const uint32_t descriptorSizeSRV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    const uint32_t descriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    const uint32_t descriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    /*開発用のUIを出そう*/

    //作成関数を使う

    //RTV用のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShaderVisibleはfalse
    rtvDescriptorHeap = CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

    //SRV用のヒープでディスクリプタの数は128。SSRVはShader内で触るものなので、ShaderVisibleはtrue
    srvDescriptorHeap_ = CreateDescriptorHeap(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

    /*画面の色を変えよう*/

    ///SwapChainからResourceを引っ張ってくる

    //SwapChainからResourceを引っ張ってくる
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(swapChainResources[0].GetAddressOf()));
    //うまく取得できなければ起動できない
    assert(SUCCEEDED(hr));
    hr = swapChain->GetBuffer(1, IID_PPV_ARGS(swapChainResources[1].GetAddressOf()));
    assert(SUCCEEDED(hr));

    ///RTVを作る

    //RTVの設定
    rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //出力結果をSRGBに変換して書き込む
    rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; //2Dテクスチャとして書き込む
    //ディスクリプタの先頭を取得する
    D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    //RTVを2つ作るのでディスクリプタを2つ用意
    //まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
    rtvHandles[0] = rtvStartHandle;
    device_->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc_, rtvHandles[0]);
    //2つ目のディスクリプタハンドルを得る(自力で)
    rtvHandles[1].ptr = rtvHandles[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    //2つ目を作る
    device_->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc_, rtvHandles[1]);

    /*前後関係を正しくしよう*/

    //DepthtencilTexturreをウィンドウのサイズで作成
    depthStencilResource = CreateDepthStencilTextureResource(device_.Get(), clientWidth_, clientHeight_);

    ///DepthStencilView(DSV)

    //DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
    dsvDescriptorHeap = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

    //DSVの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //Format。基本的にはResourceに合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; //2dTexture
    //DSVHeapの先頭にDSVをつける
    device_->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    /*完璧な画面クリアを目指して*/

    ///FenceとEventを生成する

    //初期値0でFenceを作る
    hr = device_->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
    assert(SUCCEEDED(hr));

    //FenceのSignalを待つためのイベントを作成する
    fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent != nullptr);

    /*三角形を表示しよう*/

    ///DXCの初期化

    //dxcCompilerを初期化
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
    Microsoft::WRL::ComPtr <IDxcCompiler3> dxcCompiler = nullptr;
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxcUtils.GetAddressOf()));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(dxcCompiler.GetAddressOf()));
    assert(SUCCEEDED(hr));

    //現時点でincludeはしないが、includeに対応するための設定を行っておく
    Microsoft::WRL::ComPtr <IDxcIncludeHandler> includeHandler = nullptr;
    hr = dxcUtils->CreateDefaultIncludeHandler(includeHandler.GetAddressOf());
    assert(SUCCEEDED(hr));

    ///RootSignatureを生成する

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    /*テクスチャを貼ろう*/

    ///RootSignatureを書き換える

    ///DescriptorRange

    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0; //0からは始まる
    descriptorRange[0].NumDescriptors = 1; //数は1つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; //SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //offsetを自動計算

    /*たくさんの板ポリを出そう*/

    /// RootSignatureの変更

    D3D12_DESCRIPTOR_RANGE descriptorRangeForInstacing[1] = {};
    descriptorRangeForInstacing[0].BaseShaderRegister = 0; //0からは始まる
    descriptorRangeForInstacing[0].NumDescriptors = 1; //数は1つ
    descriptorRangeForInstacing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; //SRVを使う
    descriptorRangeForInstacing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //offsetを自動計算

    ///DescriptorTable

    D3D12_ROOT_PARAMETER rootParametersForInstacing[3] = {};
    rootParametersForInstacing[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
    rootParametersForInstacing[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
    rootParametersForInstacing[0].Descriptor.ShaderRegister = 0; //レジスタ番号0を使う
    rootParametersForInstacing[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //CBVを使う
    rootParametersForInstacing[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; //VertexShaderで使う
    rootParametersForInstacing[1].Descriptor.ShaderRegister = 0; //レジスタ番号0を使う
    rootParametersForInstacing[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstacing; //Tableの中身の配列を指定
    rootParametersForInstacing[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstacing); //Tableで利用する数
    rootParametersForInstacing[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //descriptorTableを使う
    rootParametersForInstacing[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
    rootParametersForInstacing[2].DescriptorTable.pDescriptorRanges = descriptorRange; //Tableの中身の配列を指定
    rootParametersForInstacing[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); //Tableで利用する数

    /*テクスチャを貼ろう*/

    ///DescriptorTable

    D3D12_ROOT_PARAMETER rootParameters[4] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
    rootParameters[0].Descriptor.ShaderRegister = 0; //レジスタ番号0を使う
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; //VertexShaderで使う
    rootParameters[1].Descriptor.ShaderRegister = 0; //レジスタ番号0を使う

    /*テクスチャを貼ろう*/

    ///DescriptorTable

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //descriptorTableを使う
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; //Tableの中身の配列を指定
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); //Tableで利用する数

    /*LambertianReflectance*/

    ///平行光源をShaderで使う

    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
    rootParameters[3].Descriptor.ShaderRegister = 1; //レジスタ番号1を使う


    /*テクスチャを貼ろう*/

    ///Samplerの設定

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; //バイリニアフィルタ
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //0~1の範囲外をリピート
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; //比較しない
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; //ありったけのMIpmapを使う
    staticSamplers[0].ShaderRegister = 0; //レジスタ番号0を使う
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    /*三角形の色を変えよう*/

    ///RootParameter

    descriptionRootSignature.pParameters = rootParametersForInstacing; //ルートパラメータ配列へのポインタ
    descriptionRootSignature.NumParameters = _countof(rootParametersForInstacing); //配列の長さ

    ///register(解説)

    //rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //b0のbと一致する
    //rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    //rootParameters[0].Descriptor.ShaderRegister = 0; //b0のbと一致する。もしb11と紐づけたいなら11となる

    /*三角形を表示しよう*/

    ///RootSignatureを生成する

    //シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob>  errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, signatureBlob.GetAddressOf(), errorBlob.GetAddressOf());
    if (FAILED(hr)) {
        OutPutLog(log_->GetLogStream(), reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    //バイナリを元に生成
    hr = device_->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(rootSignature_.GetAddressOf()));
    assert(SUCCEEDED(hr));

    // 生成が完了したのでsignatureBlob、errorBlobを解放
    if (signatureBlob) { signatureBlob.Reset(); }
    if (errorBlob) { errorBlob.Reset(); }

    /*テクスチャを貼ろう*/

    ///InputLayoutの拡張

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    /*LambertianReflectance*/

    ///法線の定義を追加する

    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    /*三角形を表示しよう*/

    ///InputLayoutの設定を行う

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    ///BlendStateの設定を行う

    //BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    //すべての色要素を書き込む
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    /*BlendMode*/

    /// BlendModeの設定(NormalBlend)

    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

    /*いろいろなBlend*/

    ///加算合成(AddBlend)

    /*blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;*/

    /// 減算合成(逆減算合成)(SubtractBlend)

    /*blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;*/

    /// 乗算合成(MultiplyBlend)

    /*blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;*/

    /// スクリーン合成(ScreenBlend)

    /*blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;*/

    /*BlendMode*/

    /// BlendModeの設定(NormalBlend)

    // ここからの下3行はα値のブレンド設定で基本的に使わない。書いてある通りにしておけばいい。
    // 参考にするのであれば下記のリンクを参照
    // https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/ns-d3d12-d3d12_render_target_blend_desc
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;


    ///RasterizerStateの設定を行う

    //RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    //裏面(時計回り)を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    //三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    ///ShaderをCompileする

    //Shaderをコンパイルする
    Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob = CompileShader(L"Particle.VS.hlsl", L"vs_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get(), log_->GetLogStream());
    assert(vertexShaderBlob != nullptr);

    Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob = CompileShader(L"Particle.PS.hlsl", L"ps_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get(), log_->GetLogStream());
    assert(pixelShaderBlob != nullptr);

    // コンパイルが完了したのでdxcUtils、dxcCompiler、includeHandlerを解放
    if (dxcUtils) { dxcUtils.Reset(); }
    if (dxcCompiler) { dxcCompiler.Reset(); }
    if (includeHandler) { includeHandler.Reset(); }

    /*前後関係を正しくしよう*/

    ///DepthStencilStateの設定を行う

    //DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    //Depthの機能を有効化する
    depthStencilDesc.DepthEnable = true;
    //書き込みします
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    //比較関数はLessEqual。つまり、近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    /*三角形を表示しよう*/

    ///PSOを生成する

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get(); // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() }; // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() }; // PixelShader

    // 生成が完了したのでvertexShaderBlobを解放
    if (vertexShaderBlob) { vertexShaderBlob.Reset(); }
    if (pixelShaderBlob) { pixelShaderBlob.Reset(); }

    /*前後関係を正しくしよう*/

    ///DepthStencilStateの設定を行う

    //DepthStencilの設定
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    /*三角形を表示しよう*/

    ///PSOを生成する

    graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
    //書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    //利用するトロポジ(形状)のタイプ。三角形
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    //どのように画面に色を打ち込むかの設定(気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    //実際に生成
    hr = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(graphicsPipelineState.GetAddressOf()));
    assert(SUCCEEDED(hr));

    //頂点リソース用のヒープを生成
    D3D12_HEAP_PROPERTIES uploadHeapProperties{};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; //UploadHeapを使う
    //頂点リソースの設定
    D3D12_RESOURCE_DESC vertexResourceDesc{};
    //バッファリソース、テクスチャの場合はまた別の設定をする
    vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

    /*テクスチャを貼ろう*/

    ///頂点データの拡張
    vertexResourceDesc.Width = sizeof(VertexData) * 3; //リソースのサイズ。今回はVertexDataを3頂点分

    /*三角形を表示しよう*/

    ///PSOを生成する

    //バッファの場合はこれらは1にする決まり
    vertexResourceDesc.Height = 1;
    vertexResourceDesc.DepthOrArraySize = 1;
    vertexResourceDesc.MipLevels = 1;
    vertexResourceDesc.SampleDesc.Count = 1;
    //バッファの場合はこれにする決まり
    vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    //実際に頂点リソースを作る
    Microsoft::WRL::ComPtr<ID3D12Resource> dummyVertexResource = nullptr;
    hr = device_->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(dummyVertexResource.GetAddressOf()));
    assert(SUCCEEDED(hr));

    // 頂点リソースを作ったのでdummyVertexResourceを解放
    if (dummyVertexResource) { dummyVertexResource.Reset(); }

    /*三角形を表示しよう*/

    ///ViewportとScissor(シザー)

    //クライアント領域のサイズと一緒にして画面全体に表示
    viewport.Width = static_cast<FLOAT>(clientWidth_);
    viewport.Height = static_cast<FLOAT>(clientHeight_);
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    //基本的にビューポートと同じ矩形が構成されるようにする
    scissorRect.left = 0;
    scissorRect.right = clientWidth_;
    scissorRect.top = 0;
    scissorRect.bottom = clientHeight;

    ui = std::make_unique <DebugUI>();
    ui->Initialize(commandList_.Get(), device_.Get(), hwnd_, swapChainDesc_, rtvDesc_, srvDescriptorHeap_.Get());


    drawManager = std::make_unique< DrawManager>();
    drawManager->Initialize(
        commandList_.Get(),
        commandQueue_.Get(),
        swapChain.Get(),
        fence.Get(),
        fenceEvent,
        commandAllocator_.Get(),
        srvDescriptorHeap_.Get(),
        rootSignature_.Get(),
        graphicsPipelineState.Get()
    );

    textureManager = std::make_unique <TextureManager>();
    textureManager->Initialize(device_.Get(), srvDescriptorHeap_.Get(), commandList_.Get(),commandQueue_.Get());
    textureManager->LoadAllFromFolder("resources/");

    ui->SetTextureManager(textureManager.get());
}

void IrufemiEngine::Finalize() {

    // 入力系の解放
    if (inputManager_) {
        inputManager_.reset();
    }
    // サウンド
    if (audioManager_) {
        audioManager_->Finalize();
        audioManager_.reset();
    }
    // 描画
    if (drawManager) {
        drawManager->Finalize();
        drawManager.reset();
    }
    // UI
    if (ui) {
        ui->Shutdown();
        ui.reset();
    }

    // GPU同期
    if (commandQueue_ && fence) {
        commandQueue_->Signal(fence.Get(), ++fenceValue);
        if (fence->GetCompletedValue() < fenceValue) {
            fence->SetEventOnCompletion(fenceValue, fenceEvent);
            WaitForSingleObject(fenceEvent, INFINITE);
        }
    }

    // フェンスイベント
    if (fenceEvent) {
        CloseHandle(fenceEvent);
        fenceEvent = nullptr;
    }

    // D3D12解放順: PSO/RootSig→DSV/RTV/SRV→バッファ→コマンド系→フェンス→SwapChain→Device
    graphicsPipelineState.Reset();
    rootSignature_.Reset();
    depthStencilResource.Reset();
    rtvDescriptorHeap.Reset();
    srvDescriptorHeap_.Reset();
    dsvDescriptorHeap.Reset();
    swapChainResources[0].Reset();
    swapChainResources[1].Reset();
    commandList_.Reset();
    commandAllocator_.Reset();
    commandQueue_.Reset();
    fence.Reset();
    swapChain.Reset();
    device_.Reset();

#ifdef _DEBUG
    debugController_.Reset();
#endif

    if (hwnd_) {
        CloseWindow(hwnd_);
        hwnd_ = nullptr;
    }

    // COM解放
    CoUninitialize();
}