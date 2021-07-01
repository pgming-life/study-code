#include <windows.h>
#include <wrl.h>
#include <memory>
#include <vector>

// DirectX11のコードセット
#include <d3d11.h>
#include <d3dx11.h>
#include <directxmath.h>

// ライブラリ
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

#define WINDOW_CLASS    L"アルファブレンディング+α [L:次・J:戻]"
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
VOID OnKeyDown(UINT8 key);          // キー押下

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
    XMMATRIX m_world;
};

// マトリックス
XMMATRIX g_world1;          // ワールド行列の方向ベクトル1
XMMATRIX g_world2;          // ワールド行列の方向ベクトル2

// Y軸回転変数
static FLOAT y = 0;

// コンソールクラス
class Console
{
public:
    Console()
    {
        // コンソール起動
        AllocConsole();

        // 新規ファイルをオープンしストリームと紐づけ
        freopen_s(&m_fp, "CONOUT$", "w", stdout);
        freopen_s(&m_fp, "CONIN$", "r", stdin);
    }

    ~Console()
    {
        // ファイルクローズ
        fclose(m_fp);
    }

private:
    FILE* m_fp;
};

// ブレンドモードクラス
class BlendMode
{
public:
    // ブレンドモード種別
    enum Mode
    {
        NONE,           // ブレンドなし
        NORMAL,         // 通常(アルファブレンド)
        ADDITION,       // 加算
        ADDITIONALPHA,  // 加算(透過あり)
        SUBTRACTION,    // 減算
        SCREEN,         // スクリーン
        BLEND_NUMMAX,
    };

    // コンストラクタ
    BlendMode() : m_blendState(nullptr)
    {
    }

    // デストラクタ
    ~BlendMode()
    {
        SAFE_RELEASE(m_blendState);
    }

    // ブレンドモード初期化
    HRESULT Init(Mode mode)
    {
        // ブレンドの設定
        D3D11_BLEND_DESC blendDesc;
        ZeroMemory(&blendDesc, sizeof(blendDesc));
        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;
        blendDesc.RenderTarget[0].BlendEnable = TRUE;

        // Dest=基本色(下レイヤー), Src=合成色(上レイヤー)
        switch (mode)
        {
        case NORMAL:        // 通常(アルファブレンド)
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            // Src * SrcA + Dest * (1 - SrcA)
            break;

        case ADDITION:      // 加算
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            // Src * 1 + Dest * 1
            break;

        case ADDITIONALPHA: // 加算(透過あり)
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            // Src * SrcA + Dest * 1
            break;

        case SUBTRACTION:   // 減算
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_COLOR;
            // Src * 0 + Dest * (1 - Src)
            break;

        case SCREEN:        // スクリーン
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            // Src * (1 - Dest) + Dest * 1
            break;

        case NONE:          // なし
        default:            // 保険処理
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
            // Src * 1 + Dest * 0
        }

        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        // ブレンドステートを作成
        if (FAILED(g_device->CreateBlendState(&blendDesc, &m_blendState)))
        {
            MessageBox(NULL, L"ブレンドステートを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }

        return S_OK;
    }

    // ブレンドステート取得
    inline ID3D11BlendState* GetBlendState()
    {
        return m_blendState;
    }

private:
    ID3D11BlendState* m_blendState;
};
std::unique_ptr<BlendMode> g_blendMode[BlendMode::BLEND_NUMMAX];

// 使用するブレンドのリスト
static const std::vector<BlendMode::Mode> blendList
{
    BlendMode::NONE,            // ブレンドなし
    BlendMode::NORMAL,          // 通常(アルファブレンド)
    BlendMode::ADDITION,        // 加算
    BlendMode::ADDITIONALPHA,   // 加算(透過あり)
    BlendMode::SUBTRACTION,     // 減算
    BlendMode::SCREEN,          // スクリーン
};

// プリント出力リスト
static const std::vector<LPCSTR> stringList
{
    "ブレンドなし",
    "通常(アルファブレンド)",
    "加算",
    "加算(透過あり)",
    "減算",
    "スクリーン",
};

// ブレンド切替変数
static size_t g_blendNum = blendList.size();

// コンソール出力フラグ
static BOOL g_outputFlag = FALSE;

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
    case WM_KEYDOWN:    // キー押下時
        OnKeyDown(static_cast<UINT8>(wParam));
        return 0;

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
    // コンソールウィンドウを生成
    Console console;

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
    g_context->OMSetRenderTargets(1, &g_renderTargetView, nullptr);

    // ブレンドリスト初期化
    for (size_t i = 0; i < blendList.size(); i++)
    {
        g_blendMode[i].reset(new BlendMode());
        if (FAILED(g_blendMode[i]->Init(blendList[i]))) return E_FAIL;
    }

    // 初期ブレンドをプリント
    printf("0: %s\n", stringList[0]);

    // ラスタライザの設定
    D3D11_RASTERIZER_DESC rasDesc;
    ZeroMemory(&rasDesc, sizeof(rasDesc));
    rasDesc.FillMode = D3D11_FILL_SOLID;        // ソリッド
    rasDesc.CullMode = D3D11_CULL_NONE;         // カリングなし：裏表描画

    // ラスタライザステートを作成
    ID3D11RasterizerState* rasState;
    if (FAILED(g_device->CreateRasterizerState(&rasDesc, &rasState)))
    {
        MessageBox(NULL, L"ラスタライザステートを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // ラスタライザをセット
    g_context->RSSetState(rasState);

    SAFE_RELEASE(rasState);

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
    // 四角形のジオメトリを定義
    // 画面全体表示
    Vertex vertices[] =
    {
        {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
    };

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
        MessageBox(NULL, L"頂点バッファ1を作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // 表示する頂点バッファを選択
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_context->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, &offset);

    // 四角形のインデックスを定義
    WORD index[] =
    {
        0, 1, 2,
        2, 1, 3
    };

    // インデックス情報の追加
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;                 // デフォルトアクセス
    bufferDesc.ByteWidth = sizeof(WORD) * 6;                // サイズはインデックスの数 6
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
    if (FAILED(D3DX11CreateShaderResourceViewFromFile(g_device, L"sky.jpg", NULL, NULL, &g_shaderResourceView1, NULL)))
    {
        MessageBox(NULL, L"テクスチャ1を読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }
    if (FAILED(D3DX11CreateShaderResourceViewFromFile(g_device, L"color_ink.png", NULL, NULL, &g_shaderResourceView2, NULL)))
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
}

// 更新
VOID OnUpdate()
{
    // パラメータの受け渡し
    XMMATRIX scale = XMMatrixScaling(0.5f, 0.5f, 0.0f);     // X軸Y軸を1/2に縮小
    XMMATRIX translate = XMMatrixRotationY(y += 0.0005);    // Y軸0.0005ずつ回転
    g_world2 = scale * translate;                           // 各マトリックスの積
}

// 描画
VOID OnRender()
{
    // レンダーターゲットビューを指定した色でクリア
    FLOAT clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };       // ブルー
    g_context->ClearRenderTargetView(g_renderTargetView, clearColor);

    // GPUバッファをセット
    g_context->VSSetShader(g_vertexShader, nullptr, 0);
    g_context->PSSetShader(g_pixelShader, nullptr, 0);
    g_context->VSSetConstantBuffers(0, 1, &g_constantBuffer);



    // ブレンドなしをセット
    FLOAT blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
    g_context->OMSetBlendState(g_blendMode[BlendMode::NONE]->GetBlendState(), blendFactor, 0xffffffff);

    // テクスチャ1をシェーダに登録
    g_context->PSSetSamplers(0, 1, &g_sampler);
    g_context->PSSetShaderResources(0, 1, &g_shaderResourceView1);

    // 変数1代入
    ConstantBuffer cBuffer;
    cBuffer.m_world = XMMatrixTranspose(g_world1);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをバックバッファに描画
    g_context->DrawIndexed(6, 0, 0);




    size_t listNum = g_blendNum % blendList.size();

    // ブレンドをセット
    g_context->OMSetBlendState(g_blendMode[listNum]->GetBlendState(), blendFactor, 0xffffffff);

    // 両極端に達したらリセット
    if (g_blendNum == 0 || g_blendNum == blendList.size() * 2) g_blendNum = blendList.size();

    // コンソール出力
    if (g_outputFlag)
    {
        // プリント
        printf("%d: %s\n", listNum, stringList[blendList[listNum]]);

        // コンソール出力フラグ/オフ
        g_outputFlag = FALSE;
    }

    // テクスチャ2をシェーダに登録
    g_context->PSSetShaderResources(0, 1, &g_shaderResourceView2);

    // 変数2代入
    cBuffer.m_world = XMMatrixTranspose(g_world2);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをバックバッファに描画
    g_context->DrawIndexed(6, 0, 0);



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
    SAFE_RELEASE(g_indexBuffer);
    SAFE_RELEASE(g_constantBuffer);
    SAFE_RELEASE(g_sampler);
    SAFE_RELEASE(g_shaderResourceView1);
    SAFE_RELEASE(g_shaderResourceView2);
}

VOID OnKeyDown(UINT8 key)
{
    switch (key)
    {
    case 'L':       // 次
        g_blendNum++;
        g_outputFlag = TRUE;    // コンソール出力フラグ/オン
        break;
    case 'J':       // 戻
        g_blendNum--;
        g_outputFlag = TRUE;    // コンソール出力フラグ/オン
        break;
    }
}
