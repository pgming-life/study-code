#include <windows.h>
#include <wrl.h>
#include <wincodec.h>
#include <string>
#include <vector>

// DirectX11のコードセット
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

// ライブラリ
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define WINDOW_CLASS    L"WICテクスチャとシェーダリソース化による左上座標2D描画"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

#define SAFE_RELEASE(x) if(x) x->Release();

// ファイル名
static LPCWSTR fileName = L"red_smoke.png";

// 画像サイズ(ピクセル単位のFLOAT型)
static FLOAT g_imageWidth = 0.f;
static FLOAT g_imageHeight = 0.f;

// シェーダ本文
static const std::string shaderCode = "\
//================================================================================\n\
// 定数バッファ変数n\n\
//================================================================================\n\
cbuffer ConstantBuffer : register(b0)\n\
{\n\
	matrix			g_wp;			// ワールド行列・プロジェクション行列\n\
	Texture2D       g_texture;		// 基本色\n\
	SamplerState    g_sampler;		// サンプラ\n\
};\n\
\n\
//================================================================================\n\
// 入力パラメータ\n\
//================================================================================\n\
struct VS_INPUT		// 頂点シェーダ\n\
{\n\
	float4 position : POSITION;		// 頂点座標\n\
	float2 uv : TEXCOORD0;			// テクセル(UV座標)\n\
};\n\
\n\
//================================================================================\n\
// 出力パラメータ\n\
//================================================================================\n\
struct VS_OUTPUT	// 頂点シェーダ\n\
{\n\
	float4 position : SV_POSITION;\n\
	float2 uv : TEXCOORD0;			// 基本色UV値\n\
};\n\
\n\
//================================================================================\n\
// 頂点シェーダ\n\
//================================================================================\n\
VS_OUTPUT VSMain(VS_INPUT input)\n\
{\n\
	VS_OUTPUT output;\n\
\n\
	// 行列変換\n\
	output.position = mul(input.position, g_wp);\n\
\n\
	output.uv = input.uv;\n\
\n\
	return output;\n\
}\n\
\n\
// ピクセルシェーダ入力パラメータ\n\
typedef VS_OUTPUT PS_INPUT;		// 置き換え\n\
\n\
//================================================================================\n\
// ピクセルシェーダ\n\
//================================================================================\n\
float4 PSMain(PS_INPUT input) : SV_TARGET\n\
{\n\
	// そのまま出力\n\
	return g_texture.Sample(g_sampler, input.uv);\n\
}\n\
\n";
// シェーダ本文は単なるテキストなので、
// 文字列として読み込めることができるためリソース化することができる。

// プロトタイプ宣言
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // ウィンドウプロシージャ
HRESULT OnInit(HWND hWnd);          // 初期化
HRESULT InitDevice(HWND hWnd);      // デバイス関連初期化
HRESULT InitView();                 // ビュー関連初期化
HRESULT InitShader();               // シェーダ関連初期化
HRESULT InitTexture();              // テクスチャ関連初期化
HRESULT InitBuffer();               // バッファ関連初期化
VOID InitMatrix();                  // マトリックス関連初期化
VOID OnUpdate();                    // 更新
VOID OnRender();                    // 描画
VOID OnDestroy();                   // メモリ解放

// usingディレクティブ
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// パイプラインオブジェクト
ID3D11Device* g_device;                             // デバイスインターフェイス
ID3D11DeviceContext* g_context;                     // コンテキスト
IDXGISwapChain* g_swapChain;                        // スワップチェインインターフェイス
ID3D11RenderTargetView* g_renderTargetView;         // レンダーターゲットビュー
ID3D11InputLayout* g_layout;                        // インプットレイアウト
ID3D11VertexShader* g_vertexShader;                 // 頂点シェーダ
ID3D11PixelShader* g_pixelShader;                   // ピクセルシェーダ
ID3D11Buffer* g_vertexBuffer;                       // 頂点バッファ
ID3D11Buffer* g_constantBuffer;						// 定数バッファ
ID3D11ShaderResourceView* g_shaderResourceView;     // テクスチャリソース

// 頂点構造体
struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
};

// 定数バッファ
struct ConstantBuffer
{
	XMMATRIX m_WP;
};

// マトリックス
static XMMATRIX g_world;		// ワールド行列の方向ベクトル
static XMMATRIX g_projection;	// プロジェクション行列の方向ベクトル

// XY軸方向移動変数
static FLOAT x = 0.f;
static FLOAT y = 0.f;

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

	MSG	msg;
	ZeroMemory(&msg, sizeof(msg));
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
				OnUpdate();     // 更新
				OnRender();     // 描画
			}
		}
	}

	// WM_QUITメッセージの部分をWindowsに返す
	return static_cast<char>(msg.wParam);
}

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
	case WM_DESTROY:    // 終了時
		OnDestroy();    // メモリ解放
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

	// シェーダ関連初期化
	if (FAILED(InitShader())) return E_FAIL;

	// テクスチャ関連初期化
	if (FAILED(InitTexture())) return E_FAIL;

	// バッファ関連初期化
	if (FAILED(InitBuffer())) return E_FAIL;

	// マトリックス関連初期化
	InitMatrix();

	return S_OK;
}

// デバイス関連初期化
HRESULT InitDevice(HWND hWnd)
{
	// ドライバー種別を定義
	std::vector<D3D_DRIVER_TYPE> driverTypes
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
		D3D_DRIVER_TYPE_SOFTWARE,
	};

	// スワップチェインの作成
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferDesc.Width = WINDOW_WIDTH;
	swapChainDesc.BufferDesc.Height = WINDOW_HEIGHT;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE;

	// ドライバー種別を上から検証し選択
	// 選択したデバイスを用いて描画する
	HRESULT hr;
	for (size_t i = 0; i < driverTypes.size(); i++)
	{
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			driverTypes[i],
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&g_swapChain,
			&g_device,
			nullptr,
			&g_context
		);
		if (SUCCEEDED(hr)) break;
	}
	if (FAILED(hr))
	{
		MessageBox(NULL, L"DirectX11に対応していないデバイスです。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	return S_OK;
}

// ビュー関連初期化
HRESULT InitView()
{
	// 表示領域を作成
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(viewport));
	viewport.Width = WINDOW_WIDTH;
	viewport.Height = WINDOW_HEIGHT;
	viewport.MinDepth = D3D11_MIN_DEPTH;    // 0.0f
	viewport.MaxDepth = D3D11_MAX_DEPTH;    // 1.0f
	g_context->RSSetViewports(1, &viewport);

	// バックバッファを作成
	ID3D11Texture2D* backBuffer;
	g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	g_device->CreateRenderTargetView(backBuffer, nullptr, &g_renderTargetView);
	SAFE_RELEASE(backBuffer);

	// レンダーターゲットをバックバッファに設定
	g_context->OMSetRenderTargets(1, &g_renderTargetView, nullptr);

	// ブレンドの設定
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;         // 通常(アルファブレンド)有効
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;    // ・・
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	// ブレンドステートを作成
	ID3D11BlendState* blendState;
	if (FAILED(g_device->CreateBlendState(&blendDesc, &blendState)))
	{
		MessageBox(NULL, L"ブレンドステートを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// ブレンドをセット
	FLOAT blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
	g_context->OMSetBlendState(blendState, blendFactor, 0xffffffff);

	SAFE_RELEASE(blendState);

	return S_OK;
}

// シェーダ関連初期化
HRESULT InitShader()
{
	ID3DBlob* vertexShader, * pixelShader;

	// 両方のシェーダをロードしコンパイル
	if (FAILED(D3DCompile(shaderCode.c_str(), shaderCode.length(), NULL, NULL, NULL, "VSMain", "vs_4_0", NULL, NULL, &vertexShader, NULL)))
	{
		MessageBox(NULL, L"頂点シェーダを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}
	if (FAILED(D3DCompile(shaderCode.c_str(), shaderCode.length(), NULL, NULL, NULL, "PSMain", "ps_4_0", NULL, NULL, &pixelShader, NULL)))
	{
		MessageBox(NULL, L"ピクセルシェーダを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// カプセル化
	g_device->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), NULL, &g_vertexShader);
	g_device->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), NULL, &g_pixelShader);

	// 頂点インプットレイアウトを定義
	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// インプットレイアウトのサイズ
	UINT numElements = sizeof(inputElementDescs) / sizeof(inputElementDescs[0]);

	// 頂点インプットレイアウトを作成
	if (FAILED(g_device->CreateInputLayout(inputElementDescs, numElements, vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), &g_layout)))
	{
		MessageBox(NULL, L"頂点インプットレイアウトの定義が間違っています。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);

	// 頂点インプットレイアウトをセット
	g_context->IASetInputLayout(g_layout);

	return S_OK;
}

// テクスチャ関連初期化
HRESULT InitTexture()
{
	// WIC使用のための呼び出し
	CoInitialize(NULL);

	// ファクトリを作成
	IWICImagingFactory* factory;
	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)(&factory))))
	{
		MessageBox(NULL, L"ファクトリを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// テクスチャの読み込み
	IWICBitmapDecoder* decoder;
	if (FAILED(factory->CreateDecoderFromFilename(fileName, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)))
	{
		MessageBox(NULL, L"テクスチャを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// デコードフレームを取得
	IWICBitmapFrameDecode* frame;
	if (FAILED(decoder->GetFrame(0, &frame)))
	{
		MessageBox(NULL, L"デコードフレームを取得できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// フォーマットコンバータを作成
	IWICFormatConverter* formatConv;
	if (FAILED(factory->CreateFormatConverter(&formatConv)))
	{
		MessageBox(NULL, L"フォーマットコンバータを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// フォーマット初期化
	if (FAILED(formatConv->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeMedianCut)))
	{
		MessageBox(NULL, L"フォーマットの初期化に失敗しました。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// 画像サイズを取得
	UINT imageWidth;
	UINT imageHeight;
	if (FAILED(formatConv->GetSize(&imageWidth, &imageHeight)))
	{
		MessageBox(NULL, L"画像サイズを取得できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// 画像サイズを格納
	g_imageWidth = static_cast<FLOAT>(imageWidth);
	g_imageHeight = static_cast<FLOAT>(imageHeight);

	// テクスチャリソースの設定
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = imageWidth;
	textureDesc.Height = imageHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureDesc.MiscFlags = 0;

	// テクスチャリソースを作成
	ID3D11Texture2D* textureRsrc;
	if (FAILED(g_device->CreateTexture2D(&textureDesc, NULL, &textureRsrc)))
	{
		MessageBox(NULL, L"テクスチャリソースを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// テクスチャリソースにテクスチャ情報をコピー
	D3D11_MAPPED_SUBRESOURCE mappedSubrsrc;
	ZeroMemory(&mappedSubrsrc, sizeof(mappedSubrsrc));
	g_context->Map(textureRsrc, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubrsrc);
	formatConv->CopyPixels(NULL, g_imageWidth * 4, g_imageWidth * g_imageHeight * 4, (BYTE*)mappedSubrsrc.pData);
	g_context->Unmap(textureRsrc, 0);

	// シェーダリソースビューの設定
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// シェーダリソースビューを作成
	if (FAILED(g_device->CreateShaderResourceView(textureRsrc, &srvDesc, &g_shaderResourceView)))
	{
		MessageBox(NULL, L"シェーダリソースビューを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	SAFE_RELEASE(factory);
	SAFE_RELEASE(decoder);
	SAFE_RELEASE(frame);
	SAFE_RELEASE(formatConv);
	SAFE_RELEASE(textureRsrc);

	return S_OK;
}

// バッファ関連初期化
HRESULT InitBuffer()
{
	// 四角形のジオメトリを定義
	Vertex vertices[] =
	{
		{{0.0f,			0.0f,		   0.0f}, {0.0f, 0.0f}},
		{{g_imageWidth, 0.0f,		   0.0f}, {1.0f, 0.0f}},
		{{0.0f,			g_imageHeight, 0.0f}, {0.0f, 1.0f}},
		{{g_imageWidth, g_imageHeight, 0.0f}, {1.0f, 1.0f}},
	};
	// 頂点座標はTranslationに対応するためにピクセル座標に変換。
	// Scalingは行列を使わず、頂点そのものに対してスケールするのでUV値はいじらない。
	// なので、ターゲットを定義するその都度頂点情報を更新する。
	// WICで画像サイズを読み込んで頂点を設定するので必ずジオメトリ定義は後処理になる。

	// バッファを作成
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));

	// 頂点バッファの設定
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;                 // デフォルトアクセス
	bufferDesc.ByteWidth = sizeof(Vertex) * 4;              // サイズはVertex構造体×4
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;        // 頂点バッファを使用
	bufferDesc.CPUAccessFlags = 0;                          // CPUのバッファへのアクセス拒否

	// リソースの設定
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;

	// 頂点バッファを作成
	if (FAILED(g_device->CreateBuffer(&bufferDesc, &initData, &g_vertexBuffer)))
	{
		MessageBox(NULL, L"頂点バッファを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// 表示する頂点バッファを選択
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_context->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, &offset);

	// 使用するプリミティブタイプを設定
	g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 定数バッファの設定
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// 定数バッファを作成
	if (FAILED(g_device->CreateBuffer(&cbDesc, nullptr, &g_constantBuffer)))
	{
		MessageBox(NULL, L"定数バッファを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	return S_OK;
}

// マトリックス関連初期化
VOID InitMatrix()
{
	// ワールドマトリックスの初期化
	g_world = XMMatrixIdentity();       // 引数なしで全ての軸で0.0f(移動なし)

	// プロジェクションマトリックスの初期化(射影行列変換)
	g_projection = XMMatrixOrthographicOffCenterLH(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 0.0f, 1.0f);
	// 正射影変換行列を設定しこの正行列によって平行投影する(遠くに行っても同じ大きさ)
	// 左上(0, 0)を基準とした2D座標にするために頂点ジオメトリをWINDOW_WIDTHとWINDOW_HEIGHTで設定
	// 奥行方向: 2D描画用(Z軸は0～1)
}

// 更新
VOID OnUpdate()
{
	const FLOAT speedX = WINDOW_WIDTH / 30000.f;	// X軸スピード
	const FLOAT speedY = WINDOW_HEIGHT / 30000.f;	// Y軸スピード
	const FLOAT offsetBounds = WINDOW_WIDTH;		// ウィンドウ幅オフセット域

	// ウィンドウ幅のオフセット域を出ると左上からやり直し
	if (x > offsetBounds)
	{
		x = -g_imageWidth;
		y = -g_imageHeight;
	}

	// パラメータの受け渡し
	D3D11_MAPPED_SUBRESOURCE cData;
	ConstantBuffer cBuffer;
	g_world = XMMatrixTranslation(x += speedX, y += speedY, 0.f);		// 右下方向にピクセル単位移動
	cBuffer.m_WP = XMMatrixTranspose(g_world * g_projection);

	// GPU(シェーダ側)へ転送する
	g_context->Map(g_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cData);
	memcpy_s(cData.pData, cData.RowPitch, (void*)(&cBuffer), sizeof(cBuffer));
	g_context->Unmap(g_constantBuffer, 0);
}

// 描画
VOID OnRender()
{
	// レンダーターゲットビューを指定した色でクリア
	FLOAT clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };       // ブルー
	g_context->ClearRenderTargetView(g_renderTargetView, clearColor);

	// シェーダオブジェクトををセット
	g_context->VSSetShader(g_vertexShader, nullptr, 0);
	g_context->PSSetShader(g_pixelShader, nullptr, 0);
	g_context->VSSetConstantBuffers(0, 1, &g_constantBuffer);

	// テクスチャをシェーダに登録
	g_context->PSSetShaderResources(0, 1, &g_shaderResourceView);

	// 頂点バッファをバックバッファに描画
	g_context->Draw(4, 0);

	// フレームを最終出力
	g_swapChain->Present(0, 0);     // フリップ
}

// メモリ解放
VOID OnDestroy()
{
	SAFE_RELEASE(g_device);
	SAFE_RELEASE(g_context);
	SAFE_RELEASE(g_swapChain);
	SAFE_RELEASE(g_renderTargetView);
	SAFE_RELEASE(g_layout);
	SAFE_RELEASE(g_vertexShader);
	SAFE_RELEASE(g_pixelShader);
	SAFE_RELEASE(g_vertexBuffer);
	SAFE_RELEASE(g_constantBuffer);
	SAFE_RELEASE(g_shaderResourceView);
}
