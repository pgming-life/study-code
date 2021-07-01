#include <windows.h>
#include <wrl.h>

// DirectX12のコードセット
#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"

// ライブラリ
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define WINDOW_CLASS	L"初期化(ブルー画面)"
#define WINDOW_TITLE	WINDOW_CLASS
#define WINDOW_WIDTH	750
#define WINDOW_HEIGHT	500

// プロトタイプ宣言
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // ウィンドウプロシージャ
HRESULT OnInit(HWND hWnd);      // 初期化
VOID OnRender();                // 描画
VOID WaitForPreviousFrame();    // フレーム後処理
VOID OnDestroy();				// 解放

// usingディレクティブ
using Microsoft::WRL::ComPtr;
// ComPtrはスマートポインタなのでいちいちメモリ解放する必要がない。

// フレームカウントは最低2から(フロントバッファ・バックバッファ)
static const UINT g_frameCount = 2;

// パイプラインオブジェクト
ComPtr<ID3D12Device>				g_device;                           // デバイスインターフェイス
ComPtr<IDXGISwapChain3>				g_swapChain;                        // スワップチェインインターフェイス
ComPtr<ID3D12Resource>				g_renderTargets[g_frameCount];      // レンダーターゲットリソース
ComPtr<ID3D12CommandAllocator>		g_commandAllocator;                 // コマンドアロケータ(コマンドリストのメモリ確保を管理)
ComPtr<ID3D12CommandQueue>			g_commandQueue;                     // コマンドキュー(GPUに対してコマンドバッファの実行依頼を行う)
ComPtr<ID3D12DescriptorHeap>		g_rtvHeap;                          // ディスクリプタヒープ(レンダーターゲットビュー)
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

// GPU同期オブジェクト
static UINT			g_frameIndex = 0;       // フレームインデックス
static HANDLE		g_fenceEvent;           // フェンスハンドル
ComPtr<ID3D12Fence>	g_fence;                // フェンス(GPUと同期して実行完了待ちを行う)
static UINT64		g_fenceValue;           // フェンス値

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
				// 描画
				OnRender();
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

		// ディスクリプタヒープを作成
		if (FAILED(g_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(g_rtvHeap.GetAddressOf()))))
		{
			MessageBox(NULL, L"ディスクリプタヒープを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}

		// ディスクリプタのサイズを取得
		g_rtvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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

	// コマンドアロケーターを作成
	if (FAILED(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(g_commandAllocator.GetAddressOf()))))
	{
		MessageBox(NULL, L"コマンドアロケータを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// コマンドリストを作成
	if (FAILED(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator.Get(), nullptr, IID_PPV_ARGS(g_commandList.GetAddressOf()))))
	{
		MessageBox(NULL, L"コマンドリストを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// コマンドリストをクローズ
	g_commandList->Close();
	// コマンドリストは記録状態で作成されるが、今回は初期化内でそこに何も入れないのですぐに閉じる。

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
	}

	return S_OK;
}

// 描画
VOID OnRender()
{
	// コマンドアロケータをリセット
	g_commandAllocator->Reset();

	// コマンドリストをリセット
	g_commandList->Reset(g_commandAllocator.Get(), g_pipelineState.Get());

	// バックバッファをレンダーターゲットとして使用
	g_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	// リソースバリアとは、GPU側で扱うリソースの状況を同期させる機能。
	// マルチスレッドを前提とした動きなので、GPU側の動作も複数のアクセスが同時に行われることを想定した機能だということ。

	// レンダーターゲットビューのハンドルを作成
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart(), g_frameIndex, g_rtvDescriptorSize);

	// バックバッファに描画(コマンドを記録)
	const FLOAT	clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };		// 青っぽい色
	g_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

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
	if (g_fence->GetCompletedValue() < fence) {
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
