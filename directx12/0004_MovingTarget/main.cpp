#include <windows.h>
#include <wrl.h>

// DirectX12のコードセット
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "d3dx12.h"

// ライブラリ
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define WINDOW_CLASS	L"ターゲットの移動"
#define WINDOW_TITLE	WINDOW_CLASS
#define WINDOW_WIDTH	750
#define WINDOW_HEIGHT	500

// プロトタイプ宣言
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // ウィンドウプロシージャ
HRESULT OnInit(HWND hWnd);      // 初期化
HRESULT InitDevice(HWND hWnd);  // デバイス関連初期化
HRESULT InitView();             // ビュー関連初期化
HRESULT InitTraffic();          // トラフィック関連初期化
HRESULT InitShader();           // シェーダ関連初期化
HRESULT InitBuffer();           // バッファ関連初期化
HRESULT InitFence();            // フェンス関連初期化
VOID OnUpdate();				// 更新
VOID OnRender();                // 描画
VOID WaitForPreviousFrame();    // フレーム後処理
VOID OnDestroy();				// 解放

// usingディレクティブ
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// フレームカウントは最低2から(フロントバッファ・バックバッファ)
static const UINT	g_frameCount = 2;

// 頂点構造体
struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

// 定数構造体
struct ConstantBuffer
{
	XMFLOAT4 offset;
};

// パイプラインオブジェクト
ComPtr<ID3D12Device>				g_device;                           // デバイスインターフェイス
ComPtr<IDXGISwapChain3>				g_swapChain;                        // スワップチェインインターフェイス
ComPtr<ID3D12Resource>				g_renderTargets[g_frameCount];      // レンダーターゲットリソース
ComPtr<ID3D12CommandAllocator>		g_commandAllocator;                 // コマンドアロケータ(コマンドリストのメモリ確保を管理)
ComPtr<ID3D12CommandQueue>			g_commandQueue;                     // コマンドキュー(GPUに対してコマンドバッファの実行依頼を行う)
ComPtr<ID3D12RootSignature>			g_rootSignature;                    // ルートシグネチャ(描画リソースの対応付け)
ComPtr<ID3D12DescriptorHeap>		g_rtvHeap;                          // ディスクリプタヒープ(レンダーターゲットビュー)
ComPtr<ID3D12DescriptorHeap>		g_cbvHeap;							// ディスクリプタヒープ(定数バッファビュー)
ComPtr<ID3D12PipelineState>			g_pipelineState;                    // パイプラインステート
ComPtr<ID3D12GraphicsCommandList>	g_commandList;                      // コマンドリストインターフェイス(コマンドリストの生成と管理)
static UINT							g_rtvDescriptorSize = 0;            // ディスクリプタサイズ
// パイプラインステートとは、描画パイプラインの流れを指定する機能。
// ・描画に使用する各種シェーダコードの設定
// ・ラスタライズの設定
// ・ブレンド方法の設定
// ・頂点レイアウトの設定
// ・使用するRootSignatureの設定
// ・深度ステンシルの設定
// このような描画の一連の流れを指定する。

// シェーダ表示領域(ビューポート)を把握する上で必要なウィンドウ状態を格納
static CD3DX12_VIEWPORT	g_viewport(0.0f, 0.0f, static_cast<FLOAT>(WINDOW_WIDTH), static_cast<FLOAT>(WINDOW_HEIGHT));
static CD3DX12_RECT		g_scissorRect(0, 0, static_cast<LONG>(WINDOW_WIDTH), static_cast<LONG>(WINDOW_HEIGHT));

// GPU同期オブジェクト
static UINT			g_frameIndex = 0;       // フレームインデックス
static HANDLE		g_fenceEvent;           // フェンスハンドル
ComPtr<ID3D12Fence>	g_fence;                // フェンス(GPUと同期して実行完了待ちを行う)
static UINT64		g_fenceValue;           // フェンス値

// 頂点シェーダリソース
ComPtr<ID3D12Resource>		        g_vertexBuffer;					// 頂点バッファ
static D3D12_VERTEX_BUFFER_VIEW	    g_vertexBufferView;				// 頂点バッファビュー
ComPtr<ID3D12Resource>				g_constantBuffer;				// 定数バッファ
ConstantBuffer						g_constantBufferData = {};		// 定数バッファデータ
UINT8*								g_pCbvDataBegin = nullptr;		// 定数バッファデータ最初の値

// シェーダ表示領域の寸法(ビューポートのアスペクト比)
static const FLOAT g_aspectRatio = static_cast<FLOAT>(WINDOW_WIDTH) / static_cast<FLOAT>(WINDOW_HEIGHT);

// メイン関数
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, INT nCmdShow)
{
	// ウィンドウクラスを初期化
	WNDCLASSEX	windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = WINDOW_CLASS;
	RegisterClassEx(&windowClass);

	// ウィンドウのサイズを求める
	RECT windowRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// ウィンドウハンドルを作成
	HWND hWnd = CreateWindow(
		WINDOW_CLASS,
		WINDOW_TITLE,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	MSG	msg = {};
	if (SUCCEEDED(OnInit(hWnd)))   // DirectXの初期化
	{
		// ウィンドウの表示
		ShowWindow(hWnd, SW_SHOW);

		// メインループ
		while (msg.message != WM_QUIT)
		{
			// キュー内のメッセージを処理
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				OnUpdate();		// 更新
				OnRender();		// 描画
			}
		}
	}

	// WM_QUITメッセージの部分をWindowsに返す
	return static_cast<INT8>(msg.wParam);
}

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg) {
	case WM_DESTROY:    // 終了時
		OnDestroy();    // 解放
		PostQuitMessage(0);
		return 0;
	}

	// switch文が処理しなかったメッセージを処理
	return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

// 初期化
HRESULT OnInit(HWND hWnd)
{
	// デバイス関連初期化
	if (FAILED(InitDevice(hWnd))) return E_FAIL;

	// ビュー関連初期化
	if (FAILED(InitView())) return E_FAIL;

	// トラフィック関連初期化
	if (FAILED(InitTraffic())) return E_FAIL;

	// シェーダ関連初期化
	if (FAILED(InitShader())) return E_FAIL;

	// バッファ関連初期化
	if (FAILED(InitBuffer())) return E_FAIL;

	// フェンス関連初期化
	if (FAILED(InitFence())) return E_FAIL;

	return S_OK;
}

// デバイス関連初期化
HRESULT InitDevice(HWND hWnd)
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// DirectX12のデバッグレイヤーを有効
	{
		ComPtr<ID3D12Debug>	debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()))))
		{
			debugController->EnableDebugLayer();

			// 追加のデバッグレイヤーを有効
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	// ファクトリを作成
	ComPtr<IDXGIFactory4> factory;
	if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(factory.GetAddressOf()))))
	{
		MessageBox(NULL, L"ファクトリを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// DirectX12がサポートする利用可能なハードウェアアダプタを検索し取得
	HRESULT hr;
	ComPtr<IDXGIAdapter1> hardwareAdapter = nullptr;
	ComPtr<IDXGIAdapter1> adapter;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, adapter.GetAddressOf()); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 adapterDesc;
		adapter->GetDesc1(&adapterDesc);

		// どちらかがFALSEならFALSEでスルー
		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

		// アダプタがDirectX12に対応しているか確認
		hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			// デバイスを作成
			if (FAILED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(g_device.GetAddressOf()))))
			{
				MessageBox(NULL, L"選択したハードウェアデバイスがDirectX12に対応していません。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
				return E_FAIL;
			}
			break;
		}
	}

	// 関連付け解除
	hardwareAdapter = adapter.Detach();

	// ハードウェアで対応できない場合はWARPデバイス(ソフトウェア)の方を用いる
	if (FAILED(hr))
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		factory->EnumWarpAdapter(IID_PPV_ARGS(warpAdapter.GetAddressOf()));
		if (FAILED(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(g_device.GetAddressOf()))))
		{
			MessageBox(NULL, L"選択したWARPデバイスまでもがDirectX12に対応していません。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
	}

	// コマンドキューの設定
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// コマンドキューを作成
	if (FAILED(g_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(g_commandQueue.GetAddressOf()))))
	{
		MessageBox(NULL, L"コマンドキューを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// スワップチェインの設定
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = g_frameCount;                       // バックバッファの数
	swapChainDesc.Width = WINDOW_WIDTH;
	swapChainDesc.Height = WINDOW_HEIGHT;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;       // フロントバッファとバックバッファの入れ替え方法
	swapChainDesc.SampleDesc.Count = 1;
	// スワップチェインとは、レンダリング結果を出力するためのオブジェクト。
	// 紐づいたビデオアダプタやウィンドウに対してレンダリング結果を出力する。

	// スワップチェインを作成
	ComPtr<IDXGISwapChain1> swapChain;
	if (FAILED(factory->CreateSwapChainForHwnd(g_commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf())))
	{
		MessageBox(NULL, L"スワップチェインを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// Alt+Enterによる全画面遷移をできないようにする
	if (FAILED(factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER)))
	{
		MessageBox(NULL, L"画面の設定ができません。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// スワップチェインをキャスト
	swapChain.As(&g_swapChain);

	// バックバッファのインデックスを格納
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();

	return S_OK;
}

// ビュー関連初期化
HRESULT InitView()
{
	{
		// レンダーターゲットビュー用のディスクリプタヒープの設定
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = g_frameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		// ディスクリプタとは、GPUとリソースの橋渡しを行う役割のもの。
		// DX11までは隠蔽されていた。
		// ディスクリプタには3つの概念が存在する。
		// Descriptor：テクスチャなどのリソースをGPUと紐づける。
		// DescriptorHeap：DescriptorHeapからDescriptorを作成する。管理できるDescriptorの種類や数は事前に指定。
		// DescriptorTable：GPU上で使用するDescriptorの数や配置を制御する。DescriptorTableはRootSignatureで設定する。

		// RTVディスクリプタヒープを作成
		if (FAILED(g_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(g_rtvHeap.GetAddressOf()))))
		{
			MessageBox(NULL, L"RTVディスクリプタヒープを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}

		// ディスクリプタのサイズを取得
		g_rtvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// 定数バッファービュー用のディスクリプタヒープの設定
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		// CBVディスクリプタヒープを作成
		if (FAILED(g_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(g_cbvHeap.GetAddressOf()))))
		{
			MessageBox(NULL, L"CBVディスクリプタヒープを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
	}
	// ディスクリプタヒープというのは、GPU上に作られるデスクリプタを保存するための配列。
	// GPUメモリ上に存在する、様々なデータやバッファの種類や位置、大きさを示す構造体のようなもの。
	// 何らかのデータ配列として表されているということになる。
	// このように明示的に区切ることによって、その中の構造体のような配列からデータを参照しやすくしている。

	{
		// フレームリソースのハンドルを取得
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// フレームバッファとバックバッファのレンダーターゲットビューを作成
		for (UINT i = 0; i < g_frameCount; i++)
		{
			// RTVバッファを取得
			if (FAILED(g_swapChain->GetBuffer(i, IID_PPV_ARGS(g_renderTargets[i].GetAddressOf()))))
			{
				MessageBox(NULL, L"RTVバッファを取得できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
				return E_FAIL;
			}

			// レンダーターゲットビューを作成
			g_device->CreateRenderTargetView(g_renderTargets[i].Get(), nullptr, rtvHandle);

			// ハンドルのオフセット
			rtvHandle.Offset(1, g_rtvDescriptorSize);
		}
	}

	return S_OK;
}

// トラフィック関連初期化
HRESULT InitTraffic()
{
	// コマンドアロケーターを作成
	if (FAILED(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(g_commandAllocator.GetAddressOf()))))
	{
		MessageBox(NULL, L"コマンドアロケータを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// 単一のCBVを持つDescriptorTableで構成されるルートシグネチャを作成
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		// 現段階でサポートする最も高いバージョン。
		// CheckFeatureSupportが成功した場合、返されるHighestVersionはこれより大きくなることはない。

		// サポートバージョンチェック
		if (FAILED(g_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

		// ディスクリプタ区間の設定
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		
		// ルートパラメータの設定
		CD3DX12_ROOT_PARAMETER1 rootParam[1];
		rootParam[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

		// ルート署名フラグの設定
		D3D12_ROOT_SIGNATURE_FLAGS rsFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		// 入力レイアウトを許可し、特定のパイプラインステージへの不要なアクセスを拒否する。

		// ルート署名の設定
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC vrsDesc;
		vrsDesc.Init_1_1(_countof(rootParam), rootParam, 0, nullptr, rsFlags);

		// ルートシグネチャを作成
		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3DX12SerializeVersionedRootSignature(&vrsDesc, featureData.HighestVersion, signature.GetAddressOf(), error.GetAddressOf());
		if (FAILED(g_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&g_rootSignature))))
		{
			MessageBox(NULL, L"ルートシグネチャを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		// ルートシグネチャとは、ルート署名ということで描画リソースの対応付けを行っている。
		// 描画リソースとは定数バッファ、テクスチャ、サンプラとかのこと。
		// これらをシェーダに渡している。
		// 高速化のためにはプログラム側でメモリレイアウトを最適化させる必要があるらしい。
	}

	return S_OK;
}

// シェーダ関連初期化
HRESULT InitShader()
{
	// シェーダのコンパイルとロードを含むパイプラインステートを作成
	{
		ComPtr<ID3DBlob> vertexShader, pixelShader;

#if defined(_DEBUG)
		// グラフィックデバッグツールによるシェーダーのデバッグを有効
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		// 両方のシェーダをロードしコンパイル
		if (FAILED(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, vertexShader.GetAddressOf(), nullptr)))
		{
			MessageBox(NULL, L"頂点シェーダを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		if (FAILED(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, pixelShader.GetAddressOf(), nullptr)))
		{
			MessageBox(NULL, L"ピクセルシェーダを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}

		// 頂点インプットレイアウトを定義
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};

		// グラフィックパイプラインステートの設定
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
		gpsDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		gpsDesc.pRootSignature = g_rootSignature.Get();
		gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		gpsDesc.DepthStencilState.DepthEnable = FALSE;
		gpsDesc.DepthStencilState.StencilEnable = FALSE;
		gpsDesc.SampleMask = UINT_MAX;
		gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		gpsDesc.NumRenderTargets = 1;
		gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		gpsDesc.SampleDesc.Count = 1;

		// グラフィックパイプラインステートを作成
		if (FAILED(g_device->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(g_pipelineState.GetAddressOf()))))
		{
			MessageBox(NULL, L"グラフィックパイプラインステートを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
	}

	return S_OK;
}

// バッファ・コマンドリスト関連初期化
HRESULT InitBuffer()
{
	// コマンドリストを作成
	if (FAILED(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator.Get(), nullptr, IID_PPV_ARGS(g_commandList.GetAddressOf()))))
	{
		MessageBox(NULL, L"コマンドリストを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// コマンドリストをクローズ
	g_commandList->Close();
	// コマンドリストは記録状態で作成されるが、今回は初期化内でそこに何も入れないのですぐに閉じる。

	// 頂点バッファを作成
	{
		// 三角形のジオメトリを定義
		Vertex vertices[] =
		{
			{{ 0.0f,   0.25f * g_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f}},   // 上-赤
			{{ 0.25f, -0.25f * g_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f}},   // 右-緑
			{{-0.25f, -0.25f * g_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f}},   // 左-青
		};

		// 頂点バッファサイズ
		const UINT vertexBufferSize = sizeof(vertices);

		// 頂点バッファコミットリソースを作成
		if (FAILED(g_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(g_vertexBuffer.GetAddressOf()))))
		{
			MessageBox(NULL, L"頂点バッファコミットリソースを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		// コミットリソースというヒープを利用してシェーダにアップロードする。

		// 頂点バッファに頂点データをコピー
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// CPU上のこのリソースからバッファを読み込まない設定
		g_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		memcpy(pVertexDataBegin, vertices, sizeof(vertices));
		g_vertexBuffer->Unmap(0, nullptr);

		// 頂点バッファビューを初期化
		g_vertexBufferView.BufferLocation = g_vertexBuffer->GetGPUVirtualAddress();
		g_vertexBufferView.StrideInBytes = sizeof(Vertex);
		g_vertexBufferView.SizeInBytes = vertexBufferSize;
	}

	// 定数バッファを作成
	{
		// 定数バッファコミットリソースを作成
		if (FAILED(g_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(g_constantBuffer.GetAddressOf()))))
		{
			MessageBox(NULL, L"定数バッファコミットリソースを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}

		// 定数バッファビューの設定
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = g_constantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;    // CBサイズは256バイトに整列させる必要がある。

		// 定数バッファビューを作成
		g_device->CreateConstantBufferView(&cbvDesc, g_cbvHeap->GetCPUDescriptorHandleForHeapStart());

		// 定数バッファをマッピングして初期化
		CD3DX12_RANGE readRange(0, 0);        // CPU上のこのリソースから読み取るつもりはなし。
		g_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&g_pCbvDataBegin));
		memcpy(g_pCbvDataBegin, &g_constantBufferData, sizeof(g_constantBufferData));
		// アプリを閉じるまでマップ解除はしない。
		// リソースの存続中、マッピングされた状態を維持しても問題ない。
	}

	return S_OK;
}

// フェンス関連初期化
HRESULT InitFence()
{
	{
		// フェンスを作成
		if (FAILED(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(g_fence.GetAddressOf()))))
		{
			MessageBox(NULL, L"フェンスを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		// フェンスとは、同期オブジェクトとしてリソースがGPUにアップロードされるまで待機するもの。

		// フェンス値を1に設定
		g_fenceValue = 1;

		// フレーム同期に使用するイベントハンドラを作成
		g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (g_fenceEvent == nullptr)
		{
			MessageBox(NULL, L"フェンスイベントハンドラを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		// 待つ際にWindowsのEventを利用

		// フレーム後処理
		WaitForPreviousFrame();
		// コマンドリストが実行されるのを待つ。
		// メインループで同じコマンドリストを再利用しているが、
		// 今のところは、セットアップが完了するのを待ってから続行する。
	}

	return S_OK;
}

// 更新
VOID OnUpdate()
{
	const float translationSpeed = 0.005f;		// スピード
	const float offsetBounds = 1.25f;			// オフセット域

	// X軸右方向に移動
	g_constantBufferData.offset.x += translationSpeed;
	
	// 右側のオフセット域を出ると左側からやり直し
	if (g_constantBufferData.offset.x > offsetBounds)
		g_constantBufferData.offset.x = -offsetBounds;

	// 更新反映
	memcpy(g_pCbvDataBegin, &g_constantBufferData, sizeof(g_constantBufferData));
}

// 描画
VOID OnRender()
{
	// コマンドアロケータをリセット
	g_commandAllocator->Reset();

	// コマンドリストをリセット
	g_commandList->Reset(g_commandAllocator.Get(), g_pipelineState.Get());

	// コマンドリストにシェーダ表示領域(ビューポート)を把握する上で必要なウィンドウ状態を設定
	g_commandList->SetGraphicsRootSignature(g_rootSignature.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { g_cbvHeap.Get() };
	g_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);		// 定数バッファヒープをセット
	g_commandList->SetGraphicsRootDescriptorTable(0, g_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	g_commandList->RSSetViewports(1, &g_viewport);
	g_commandList->RSSetScissorRects(1, &g_scissorRect);

	// バックバッファをレンダーターゲットとして使用
	g_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	// リソースバリアとは、GPU側で扱うリソースの状況を同期させる機能。
	// マルチスレッドを前提とした動きなので、GPU側の動作も複数のアクセスが同時に行われることを想定した機能だということ。

	// レンダーターゲットビューのハンドルを作成
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart(), g_frameIndex, g_rtvDescriptorSize);

	// レンダーターゲットをセット
	g_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// バックバッファに描画(コマンドを記録する)
	const FLOAT	clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };      // 青っぽい色
	g_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	g_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_commandList->IASetVertexBuffers(0, 1, &g_vertexBufferView);
	g_commandList->DrawInstanced(3, 1, 0, 0);

	// バックバッファを表示
	g_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// コマンドリストをクローズ
	g_commandList->Close();

	// コマンドリストを実行
	ID3D12CommandList* ppCommandLists[] = { g_commandList.Get() };
	g_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// フレームを最終出力
	g_swapChain->Present(1, 0);

	// フレーム後処理
	WaitForPreviousFrame();
}

// フレーム後処理
VOID WaitForPreviousFrame()
{
	const UINT64 fence = g_fenceValue;
	g_commandQueue->Signal(g_fence.Get(), fence);
	g_fenceValue++;

	// 前のフレームが終了するまで待機
	if (g_fence->GetCompletedValue() < fence)
	{
		g_fence->SetEventOnCompletion(fence, g_fenceEvent);
		WaitForSingleObject(g_fenceEvent, INFINITE);
	}

	// バックバッファのインデックスを格納
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();
}
// コマンド消化を判断し待ちを解除する関数。

// 解放
VOID OnDestroy()
{
	WaitForPreviousFrame();
	CloseHandle(g_fenceEvent);
}
