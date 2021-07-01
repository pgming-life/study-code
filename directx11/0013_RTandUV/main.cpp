#include <windows.h>
#include <wrl.h>
#include <memory>
#include <vector>

// DirectX11,10のコードセット
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <directxmath.h>

// ライブラリ
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

#define WINDOW_CLASS    L"レンダーターゲットとUV値変換"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

#define SAFE_RELEASE(x) if(x) x->Release();

// ブレンドモード種別
enum BlendMode
{
    NONE,           // ブレンドなし
    MULTIPLE,       // 乗算合成
    BLEND_NUMMAX,
};

// シェーダデータ
struct ShaderData
{
    BlendMode mode;
    LPCSTR vsEntry;
    LPCSTR psEntry;
};

// シェーダデータ初期化リスト
static const ShaderData initShaderData[BLEND_NUMMAX] =
{
    {NONE,      "vsMain",   "psMain"},
    {MULTIPLE,  "vsBlend",  "psMultiple"},
};

// 画像オブジェクト
enum Object
{
    SKY,
    DUST,
    OBJ_NUMMAX,
};

// 画像ファイル
static const LPCWSTR file[OBJ_NUMMAX] =
{
    L"sky.jpg",
    L"dust.png",
};

// オブジェクトデータ
struct ObjectData
{
    Object obj;
    BlendMode mode;
    BOOL blendFlag;
};

// オブジェクトリスト
static const ObjectData objectList[] =
{
    {SKY,   NONE,       FALSE},
    {DUST,  MULTIPLE,   TRUE},
};

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
ID3D11Device* g_device;                                         // デバイスインターフェイス
ID3D11DeviceContext* g_context;                                 // コンテキスト
IDXGISwapChain* g_swapChain;                                    // スワップチェインインターフェイス
ID3D11InputLayout* g_layout[BLEND_NUMMAX >= 2 ? 2 : 1];         // インプットレイアウト
ID3D11VertexShader* g_vertexShader[BLEND_NUMMAX >= 2 ? 2 : 1];  // 頂点シェーダ
ID3D11PixelShader* g_pixelShader[BLEND_NUMMAX];                 // ピクセルシェーダ
ID3D11Buffer* g_vertexBuffer;                                   // 頂点バッファ
ID3D11Buffer* g_indexBuffer;                                    // インデックスバッファ
ID3D11Buffer* g_constantBuffer;                                 // 定数バッファ
ID3D11SamplerState* g_sampler;                                  // テクスチャサンプラ
ID3D11ShaderResourceView* g_shaderResourceView[OBJ_NUMMAX];     // テクスチャリソース

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
XMMATRIX g_world[OBJ_NUMMAX];   // ワールド行列の方向ベクトル

// Y軸回転変数
static FLOAT y = 0;

// 使用するターゲットのリスト
enum TargetList
{
    _BACK_BUFFER,
    _RENDER_TARGET1,
    _RENDER_TARGET2,
    _RENDER_TARGET3,
    _TARGET_NUMMAX,
};
// 今回はレンダーターゲット3は使わないが、、
// オブジェクトが増えてくると使う機会があるかもしれない。

// レンダーターゲットクラス
class RenderTarget
{
public:
    // ターゲット種別
    enum Target
    {
        BACK_BUFFER,        // バックバッファ
        RENDER_TARGET,      // レンダーターゲット
        TARGET_NUMMAX,
    };

    // コンストラクタ
    RenderTarget() : m_renderTargetView(nullptr), m_shaderResourceView(nullptr)
    {
        SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    // デストラクタ
    ~RenderTarget()
    {
        SAFE_RELEASE(m_layerBuffer);
        SAFE_RELEASE(m_renderTargetView);
        SAFE_RELEASE(m_shaderResourceView);
    }

    // ターゲット初期化
    HRESULT Init(Target target)
    {
        switch (target) {
        case BACK_BUFFER:       // バックバッファ
            // バックバッファ使用時はスワップチェインと紐づけ
            g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_layerBuffer));

            // バックバッファを作成
            if (FAILED(g_device->CreateRenderTargetView(m_layerBuffer, nullptr, &m_renderTargetView)))
            {
                MessageBox(NULL, L"バックバッファを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            break;

        case RENDER_TARGET:     // レンダーターゲット
            // レンダーターゲットの設定
            D3D11_TEXTURE2D_DESC rtDesc;
            ZeroMemory(&rtDesc, sizeof(rtDesc));
            rtDesc.Width = WINDOW_WIDTH;
            rtDesc.Height = WINDOW_HEIGHT;
            rtDesc.MipLevels = 1;
            rtDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
            rtDesc.SampleDesc.Count = 1;
            rtDesc.Usage = D3D11_USAGE_DEFAULT;
            rtDesc.ArraySize = 1;
            rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            rtDesc.CPUAccessFlags = 0;

            // レンダーターゲットを作成
            if (FAILED(g_device->CreateTexture2D(&rtDesc, nullptr, &m_layerBuffer)))
            {
                MessageBox(NULL, L"レンダーターゲットを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            // レンダーターゲットビューの設定
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            ZeroMemory(&rtvDesc, sizeof(rtvDesc));
            rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

            // レンダーターゲットビューを作成
            if (FAILED(g_device->CreateRenderTargetView(m_layerBuffer, &rtvDesc, &m_renderTargetView)))
            {
                MessageBox(NULL, L"レンダーターゲットビューを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            // シェーダリソースビューの設定
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            ZeroMemory(&srvDesc, sizeof(srvDesc));
            srvDesc.Format = rtvDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            // シェーダリソースビューを作成
            if (FAILED(g_device->CreateShaderResourceView(m_layerBuffer, &srvDesc, &m_shaderResourceView)))
            {
                MessageBox(NULL, L"シェーダリソースビューを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }
        }

        return S_OK;
    }

    // 背景色セット
    inline VOID SetClearColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a)
    {
        m_clearColor = { r, g, b, a };
    }

    // ターゲットセット
    inline VOID SetRenderTarget() const
    {
        g_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
    }

    // レイヤークリア
    inline VOID ClearRenderTargetView() const
    {
        g_context->ClearRenderTargetView(m_renderTargetView, m_clearColor);
    }

    // シェーダリソースビューアドレス取得
    inline ID3D11ShaderResourceView** GetAddressSRV()
    {
        return &m_shaderResourceView;
    }

private:
    ID3D11Texture2D* m_layerBuffer;
    ID3D11RenderTargetView* m_renderTargetView;
    ID3D11ShaderResourceView* m_shaderResourceView;
    D3DXCOLOR m_clearColor;
};
std::unique_ptr<RenderTarget> g_renderTarget[_TARGET_NUMMAX];

// ターゲットデータ
struct TargetData
{
    RenderTarget::Target target;
    D3DXCOLOR color;
};

// ターゲットデータ初期化リスト
TargetData initTargetData[] =
{
    {RenderTarget::BACK_BUFFER,     {0.0f, 0.0f, 1.0f, 1.0f}},      // ブルー
    {RenderTarget::RENDER_TARGET,   {0.0f, 1.0f, 0.0f, 1.0f}},      // グリーン
    {RenderTarget::RENDER_TARGET,   {0.0f, 0.0f, 0.0f, 0.0f}},      // 透明
    {RenderTarget::RENDER_TARGET,   {0.0f, 0.0f, 0.0f, 0.0f}},      // 透明
};

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

    // ターゲットリスト初期化
    for (UINT i = 0; i < _TARGET_NUMMAX; i++)
    {
        g_renderTarget[i].reset(new RenderTarget());
        if (FAILED(g_renderTarget[i]->Init(initTargetData[i].target))) return E_FAIL;
        g_renderTarget[i]->SetClearColor(
            initTargetData[i].color.r,
            initTargetData[i].color.g,
            initTargetData[i].color.b,
            initTargetData[i].color.a
        );
    }

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

    SAFE_RELEASE(blendState);
    SAFE_RELEASE(rasState);

    return S_OK;
}

// シェーダ関連初期化
HRESULT InitShader()
{
    // 頂点インプットレイアウトを定義
    D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    // インプットレイアウトのサイズ
    UINT numElements = sizeof(inputElementDescs) / sizeof(inputElementDescs[0]);

    ID3DBlob* vertexShader[BLEND_NUMMAX >= 2 ? 2 : 1], * pixelShader[BLEND_NUMMAX];

    // 両方のシェーダをロードしコンパイル
    for (UINT i = 0; i < BLEND_NUMMAX; i++)
    {
        // 頂点シェーダ
        if (i + 1 <= (BLEND_NUMMAX >= 2 ? 2 : 1))
        {
            if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, initShaderData[i].vsEntry, "vs_4_0", NULL, NULL, NULL, &vertexShader[i], NULL, NULL)))
            {
                MessageBox(NULL, L"頂点シェーダを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            // カプセル化
            g_device->CreateVertexShader(vertexShader[i]->GetBufferPointer(), vertexShader[i]->GetBufferSize(), NULL, &g_vertexShader[i]);

            // 頂点インプットレイアウトを作成
            if (FAILED(g_device->CreateInputLayout(inputElementDescs, numElements, vertexShader[i]->GetBufferPointer(), vertexShader[i]->GetBufferSize(), &g_layout[i])))
            {
                MessageBox(NULL, L"頂点インプットレイアウトの定義が間違っています。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            // 頂点インプットレイアウトをセット
            g_context->IASetInputLayout(g_layout[i]);

            SAFE_RELEASE(vertexShader[i]);
        }

        // ピクセルシェーダ
        if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, initShaderData[i].psEntry, "ps_4_0", NULL, NULL, NULL, &pixelShader[i], NULL, NULL)))
        {
            MessageBox(NULL, L"ピクセルシェーダを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }

        // カプセル化
        g_device->CreatePixelShader(pixelShader[i]->GetBufferPointer(), pixelShader[i]->GetBufferSize(), NULL, &g_pixelShader[i]);

        SAFE_RELEASE(pixelShader[i]);
    }

    return S_OK;
}

// バッファ関連初期化
HRESULT InitBuffer()
{
    // 四角形のジオメトリを定義    
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
    for (UINT i = 0; i < OBJ_NUMMAX; i++)
    {
        if (FAILED(D3DX11CreateShaderResourceViewFromFile(g_device, file[i], NULL, NULL, &g_shaderResourceView[i], NULL)))
        {
            MessageBox(NULL, L"テクスチャを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }
    }

    return S_OK;
}

// マトリックス関連初期化
VOID InitMatrix()
{
    // ワールドマトリックスの初期化
    for (UINT i = 0; i < OBJ_NUMMAX; i++)
        g_world[i] = XMMatrixIdentity();    // 引数なしで全ての軸で0.0f(移動なし)
}

// 更新
VOID OnUpdate()
{
    // パラメータの受け渡し
    XMMATRIX scale = XMMatrixScaling(0.55f, 0.8f, 0.0f);    // X軸0.55倍, Y軸0.8倍に縮小
    XMMATRIX rotate = XMMatrixRotationY(y += 0.0005f);      // Y軸0.0005ずつ回転
    g_world[DUST] = scale * rotate;                         // 各マトリックスの積
}

// 描画
VOID OnRender()
{
    // 共通バッファをセット
    g_context->VSSetConstantBuffers(0, 1, &g_constantBuffer);
    g_context->PSSetSamplers(0, 1, &g_sampler);



    // レンダーターゲット1をセット
    g_renderTarget[_RENDER_TARGET1]->SetRenderTarget();
    g_renderTarget[_RENDER_TARGET1]->ClearRenderTargetView();

    // シェーダオブジェクトをセット
    g_context->VSSetShader(g_vertexShader[objectList[SKY].blendFlag], nullptr, 0);
    g_context->PSSetShader(g_pixelShader[objectList[SKY].mode], nullptr, 0);

    // テクスチャ1をシェーダに登録
    g_context->PSSetShaderResources(0, 1, &g_shaderResourceView[objectList[SKY].obj]);

    // 変数1代入
    ConstantBuffer cBuffer;
    cBuffer.m_world = XMMatrixTranspose(g_world[objectList[SKY].obj]);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをRT1に描画
    g_context->DrawIndexed(6, 0, 0);



    // レンダーターゲット2をセット
    g_renderTarget[_RENDER_TARGET2]->SetRenderTarget();
    g_renderTarget[_RENDER_TARGET2]->ClearRenderTargetView();

    // テクスチャ2をシェーダに登録
    g_context->PSSetShaderResources(0, 1, &g_shaderResourceView[objectList[DUST].obj]);

    // 変数2代入
    cBuffer.m_world = XMMatrixTranspose(g_world[objectList[DUST].obj]);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをRT2に描画
    g_context->DrawIndexed(6, 0, 0);



    // バックバッファをセット
    g_renderTarget[_BACK_BUFFER]->SetRenderTarget();
    g_renderTarget[_BACK_BUFFER]->ClearRenderTargetView();


    // RT1を下のレイヤーとして先に描画
    g_context->PSSetShaderResources(0, 1, g_renderTarget[_RENDER_TARGET1]->GetAddressSRV());

    // 変数1代入
    cBuffer.m_world = XMMatrixTranspose(g_world[objectList[SKY].obj]);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをバックバッファに描画
    g_context->DrawIndexed(6, 0, 0);


    // シェーダオブジェクトをセット
    g_context->VSSetShader(g_vertexShader[objectList[DUST].blendFlag], nullptr, 0);
    g_context->PSSetShader(g_pixelShader[objectList[DUST].mode], nullptr, 0);

    // RT1とRT2をシェーダに登録し合成
    g_context->PSSetShaderResources(0, 1, g_renderTarget[_RENDER_TARGET1]->GetAddressSRV());
    g_context->PSSetShaderResources(1, 1, g_renderTarget[_RENDER_TARGET2]->GetAddressSRV());

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
    SAFE_RELEASE(g_vertexBuffer);
    SAFE_RELEASE(g_indexBuffer);
    SAFE_RELEASE(g_constantBuffer);
    SAFE_RELEASE(g_sampler);

    for (UINT i = 0; i < BLEND_NUMMAX; i++)
    {
        if (i + 1 <= (BLEND_NUMMAX >= 2 ? 2 : 1))
        {
            SAFE_RELEASE(g_layout[i]);
            SAFE_RELEASE(g_vertexShader[i]);
        }
        if (i + 1 <= OBJ_NUMMAX) SAFE_RELEASE(g_shaderResourceView[i]);
        SAFE_RELEASE(g_pixelShader[i]);
    }
}
