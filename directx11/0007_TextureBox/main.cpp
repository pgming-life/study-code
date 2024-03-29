#include <windows.h>
#include <wrl.h>
#include <vector>

// DirectX11のコードセット
#include <d3d11.h>
#include <d3dx11.h>
#include <directxmath.h>

// ライブラリ
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

#define WINDOW_CLASS    L"テクスチャボックス"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

#define SAFE_RELEASE(x) if(x) x->Release();

// プロトタイプ宣言
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // ウィンドウプロシージャ
HRESULT OnInit(HWND hWnd);          // 初期化
HRESULT InitDevice(HWND hWnd);      // デバイス関連初期化
HRESULT InitView();                 // ビュー関連初期化
HRESULT InitShader();               // シェーダ関連初期化
HRESULT InitBuffer();               // バッファ関連初期化
HRESULT InitTexture();              // テクスチャ関連初期化
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
ID3D11DepthStencilView* g_depthStencilView;         // 深度ステンシルビュー
ID3D11InputLayout* g_layout;                        // インプットレイアウト
ID3D11VertexShader* g_vertexShader;                 // 頂点シェーダ
ID3D11PixelShader* g_pixelShader;                   // ピクセルシェーダ
ID3D11Buffer* g_vertexBuffer1;                      // 頂点バッファ1
ID3D11Buffer* g_vertexBuffer2;                      // 頂点バッファ2
ID3D11Buffer* g_indexBuffer;                        // インデックスバッファ
ID3D11Buffer* g_constantBuffer;                     // 定数バッファ
ID3D11SamplerState* g_sampler;                      // テクスチャサンプラ
ID3D11ShaderResourceView* g_shaderResourceView1;    // テクスチャリソース1
ID3D11ShaderResourceView* g_shaderResourceView2;    // テクスチャリソース2

// 頂点構造体
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
};

// 定数構造体
struct ConstantBuffer
{
    XMMATRIX m_WVP;
};

// マトリックス
XMMATRIX g_world1;          // ワールド行列の方向ベクトル1
XMMATRIX g_world2;          // ワールド行列の方向ベクトル2
XMMATRIX g_world3;          // ワールド行列の方向ベクトル3
XMMATRIX g_view;            // ビュー行列の方向ベクトル
XMMATRIX g_projection;      // プロジェクション行列の方向ベクトル

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

    // バッファ関連初期化
    if (FAILED(InitBuffer())) return E_FAIL;

    // テクスチャ関連初期化
    if (FAILED(InitTexture())) return E_FAIL;

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
    //g_context->OMSetRenderTargets(1, &g_renderTargetView, nullptr);

    // 深度ステンシルの設定
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = WINDOW_WIDTH;
    descDepth.Height = WINDOW_HEIGHT;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    // 深度ステンシルを作成
    ID3D11Texture2D* depthStencil;
    if (FAILED(g_device->CreateTexture2D(&descDepth, nullptr, &depthStencil)))
    {
        MessageBox(NULL, L"深度ステンシルを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // 深度ステンシルビュー(Zバッファ)の設定
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    // 深度ステンシルビューを作成
    if (FAILED(g_device->CreateDepthStencilView(depthStencil, &descDSV, &g_depthStencilView)))
    {
        MessageBox(NULL, L"深度ステンシルビューを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    SAFE_RELEASE(depthStencil);

    // レンダーターゲットをバックバッファに設定(深度ステンシル/オン)
    g_context->OMSetRenderTargets(1, &g_renderTargetView, g_depthStencilView);

    return S_OK;
}

// シェーダ関連初期化
HRESULT InitShader()
{
    ID3DBlob* vertexShader, * pixelShader;

    // 両方のシェーダをロードしコンパイル
    if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, "vsMain", "vs_4_0", NULL, NULL, NULL, &vertexShader, NULL, NULL)))
    {
        MessageBox(NULL, L"頂点シェーダを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }
    if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, "psMain", "ps_4_0", NULL, NULL, NULL, &pixelShader, NULL, NULL)))
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

// バッファ関連初期化
HRESULT InitBuffer()
{
    // 立方体1のジオメトリを定義
    Vertex vertices1[] =
    {
        {{-1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f}, {1.0f, 1.0f}},

        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 1.0f}},
        {{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},

        {{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f}},

        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 1.0f}},
        {{ 1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}},

        {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},

        {{-1.0f, -1.0f,  1.0f}, {1.0f, 1.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f}},
        {{-1.0f,  1.0f,  1.0f}, {1.0f, 0.0f}},
    };

    // 立方体2のジオメトリを定義
    Vertex vertices2[] =
    {
        {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},

        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},

        {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},

        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},

        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}},

        {{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
    };

    // バッファを作成
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));

    // 頂点バッファの設定
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;                 // デフォルトアクセス
    bufferDesc.ByteWidth = sizeof(Vertex) * 24;             // サイズはVertex構造体×24
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;        // 頂点バッファを使用
    bufferDesc.CPUAccessFlags = 0;                          // CPUのバッファへのアクセス拒否

    // リソースの設定
    D3D11_SUBRESOURCE_DATA initData;
    ZeroMemory(&initData, sizeof(initData));
    initData.pSysMem = vertices1;

    // 頂点バッファを作成
    if (FAILED(g_device->CreateBuffer(&bufferDesc, &initData, &g_vertexBuffer1)))
    {
        MessageBox(NULL, L"頂点バッファ1を作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // 追加
    initData.pSysMem = vertices2;
    if (FAILED(g_device->CreateBuffer(&bufferDesc, &initData, &g_vertexBuffer2)))
    {
        MessageBox(NULL, L"頂点バッファ2を作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // 四角形のインデックスを定義
    WORD index[] =
    {
        3, 1, 0,
        2, 1, 3,

        6, 4, 5,
        7, 4, 6,

        11, 9, 8,
        10, 9, 11,

        14, 12, 13,
        15, 12, 14,

        19, 17, 16,
        18, 17, 19,

        22, 20, 21,
        23, 20, 22,
    };

    // インデックス情報の追加
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;                 // デフォルトアクセス
    bufferDesc.ByteWidth = sizeof(WORD) * 36;               // サイズはインデックスの数 36
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;         // インデックスバッファを使用
    bufferDesc.CPUAccessFlags = 0;                          // CPUのバッファへのアクセス拒否
    initData.pSysMem = index;

    // インデックスバッファを作成
    if (FAILED(g_device->CreateBuffer(&bufferDesc, &initData, &g_indexBuffer)))
    {
        MessageBox(NULL, L"インデックスバッファを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // 表示するインデックスバッファを選択
    g_context->IASetIndexBuffer(g_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // 使用するプリミティブタイプを設定
    g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 定数情報の追加
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(ConstantBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    // 定数バッファを作成
    if (FAILED(g_device->CreateBuffer(&bufferDesc, nullptr, &g_constantBuffer)))
    {
        MessageBox(NULL, L"定数バッファを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    return S_OK;
}

// テクスチャ関連初期化
HRESULT InitTexture()
{
    // サンプラの設定
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 1.0f;		// ホワイト
    samplerDesc.BorderColor[1] = 1.0f;		// ..
    samplerDesc.BorderColor[2] = 1.0f;		// ..
    samplerDesc.BorderColor[3] = 1.0f;		// ..
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    // サンプラステートを作成
    if (FAILED(g_device->CreateSamplerState(&samplerDesc, &g_sampler)))
    {
        MessageBox(NULL, L"サンプラステートを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // テクスチャの読み込み
    if (FAILED(D3DX11CreateShaderResourceViewFromFile(g_device, L"wood.jpg", NULL, NULL, &g_shaderResourceView1, NULL)))
    {
        MessageBox(NULL, L"テクスチャ1を読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }
    if (FAILED(D3DX11CreateShaderResourceViewFromFile(g_device, L"stone.jpg", NULL, NULL, &g_shaderResourceView2, NULL)))
    {
        MessageBox(NULL, L"テクスチャ2を読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    return S_OK;
}

// マトリックス関連初期化
VOID InitMatrix()
{
    // ワールドマトリックスの初期化
    g_world1 = XMMatrixIdentity();       // 引数なしで全ての軸で0.0f(移動なし)
    g_world2 = XMMatrixIdentity();       // 引数なしで全ての軸で0.0f(移動なし)

    // ビューマトリックスの初期化
    XMVECTOR eye = XMVectorSet(0.0f, 3.0f, -5.0f, 0.0f);    // カメラの座標を示すベクトル
    XMVECTOR focus = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);   // 焦点(ターゲット)の座標を示すベクトル
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);      // カメラの上方向を示すベクトル
    g_view = XMMatrixLookAtLH(eye, focus, up);

    // プロジェクションマトリックスの初期化(射影行列変換)
    g_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, static_cast<FLOAT>(WINDOW_WIDTH) / static_cast<FLOAT>(WINDOW_HEIGHT), 0.1f, 100.0f);
    // 視野角90°(XM_PI(π)を2で割るので90°)
    // ビュー空間X:Yのアスペクト比
    // Z軸0.1fから100.0fまでの間の空間
}

// 更新
VOID OnUpdate()
{
    // 時間の更新
    static float t = 0.0f;
    static ULONGLONG timeStart = 0;
    ULONGLONG timeCur = GetTickCount64();
    if (timeStart == 0) timeStart = timeCur;
    t = (timeCur - timeStart) / 1000.0f;

    // パラメータの受け渡し1
    XMMATRIX scale1 = XMMatrixScaling(0.8f, 0.8f, 0.8f);                                // 0.8倍に拡大
    XMMATRIX rotate1 = XMMatrixRotationX(t * 2.0f) * XMMatrixRotationY(t * 1.5f);       // X軸回転とY軸回転
    XMMATRIX translate1 = XMMatrixTranslation(0.0f, -1.0f, 0.0f);                       // Y軸-1.0f移動
    g_world1 = scale1 * rotate1 * translate1;                                           // 各マトリックスの積

    // パラメータの受け渡し2
    XMMATRIX scale2 = XMMatrixScaling(2.0f, 2.0f, 2.0f);                                // 2.0倍に拡大
    XMMATRIX rotate2 = XMMatrixRotationX(t * 2.0f) * XMMatrixRotationY(t * 1.5f);       // X軸回転とY軸回転
    XMMATRIX translate2 = XMMatrixTranslation(0.0f, 3.0f, 0.0f);                        // Y軸3.0f移動
    g_world2 = scale2 * rotate2 * translate2;                                           // 各マトリックスの積
    
    // パラメータの受け渡し3
    XMMATRIX scale3 = XMMatrixScaling(0.5f, 0.5f, 0.5f);            // 0.5倍に拡大
    XMMATRIX rotate3 = XMMatrixRotationZ(t * 2.5f);                 // Z軸回転
    XMMATRIX translate3 = XMMatrixTranslation(-3.0f, 3.0f, 0.0f);   // X軸-3.0f,Y軸3.0f移動
    XMMATRIX orbit3 = XMMatrixRotationY(t * 1.1f);                  // 軌道回転速
    g_world3 = scale3 * rotate3 * translate3 * orbit3;              // 各マトリックスの積
}

// 描画
VOID OnRender()
{
    // レンダーターゲットビューを指定した色でクリア
    FLOAT clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };       // ブルー
    g_context->ClearRenderTargetView(g_renderTargetView, clearColor);

    // 深度バッファを1.0(最大深度)にクリア
    g_context->ClearDepthStencilView(g_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // GPUバッファをセット
    g_context->VSSetShader(g_vertexShader, nullptr, 0);
    g_context->PSSetShader(g_pixelShader, nullptr, 0);
    g_context->VSSetConstantBuffers(0, 1, &g_constantBuffer);

    
    
    // テクスチャ1をシェーダに登録
    g_context->PSSetSamplers(0, 1, &g_sampler);
    g_context->PSSetShaderResources(0, 1, &g_shaderResourceView1);

    // 表示する頂点バッファを選択
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_context->IASetVertexBuffers(0, 1, &g_vertexBuffer1, &stride, &offset);

    // 変数1代入
    ConstantBuffer cBuffer;
    cBuffer.m_WVP = XMMatrixTranspose(g_world1 * g_view * g_projection);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをバックバッファに描画
    g_context->DrawIndexed(36, 0, 0);



    // テクスチャ2をシェーダに登録
    g_context->PSSetShaderResources(0, 1, &g_shaderResourceView2);

    // 表示する頂点バッファを選択
    g_context->IASetVertexBuffers(0, 1, &g_vertexBuffer2, &stride, &offset);

    // 変数2代入
    cBuffer.m_WVP = XMMatrixTranspose(g_world2 * g_view * g_projection);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをバックバッファに描画
    g_context->DrawIndexed(36, 0, 0);



    // 変数3代入
    cBuffer.m_WVP = XMMatrixTranspose(g_world3 * g_view * g_projection);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをバックバッファに描画
    g_context->DrawIndexed(36, 0, 0);



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
    SAFE_RELEASE(g_depthStencilView);
    SAFE_RELEASE(g_layout);
    SAFE_RELEASE(g_vertexShader);
    SAFE_RELEASE(g_pixelShader);
    SAFE_RELEASE(g_vertexBuffer1);
    SAFE_RELEASE(g_vertexBuffer2);
    SAFE_RELEASE(g_indexBuffer);
    SAFE_RELEASE(g_constantBuffer);
    SAFE_RELEASE(g_sampler);
    SAFE_RELEASE(g_shaderResourceView1);
    SAFE_RELEASE(g_shaderResourceView2);
}
