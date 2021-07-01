#include <windows.h>
#include <wrl.h>
#include <memory>
#include <list>
#include <vector>

// DirectX11,10のコードセット
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <directxmath.h>

// ライブラリ
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

#define WINDOW_CLASS    L"シェーダ版ブレンドモードテスト [L:次・J:戻・I:濃・M:薄・K:回転スイッチ]"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

// ブレンドモード種別
enum BlendMode {
    NONE,               // ブレンドなし
    NORMAL,             // 通常(アルファブレンド)
    DARK, 			    // 比較(暗)
    MULTIPLE,		    // 乗算
    BURNCOLOR,		    // 焼き込みカラー
    BURNLINEAR,		    // 焼き込みリニア
    COLORDARK,          // カラー比較(暗)
    LIGHT,			    // 比較(明)
    SCREEN,			    // スクリーン
    DODGECOLOR,		    // 覆い焼きカラー
    DODGELINEAR,	    // 覆い焼きリニア(加算)
    COLORLIGHT,         // カラー比較(明)
    OVERLAY,		    // オーバーレイ
    SOFTLIGHT,          // ソフトライト
    HARDLIGHT,		    // ハードライト
    VIVIDLIGHT,		    // ビビッドライト
    LINEARLIGHT,	    // リニアライト
    PINLIGHT,		    // ピンライト
    HARDMIX,		    // ハードミックス
    ABSOLUTENESS,	    // 差の絶対値
    EXCLUSION,		    // 除外
    SUBTRACTION,	    // 減算
    DIVISION,		    // 除算
    ADDITION,		    // 加算
    MONOCHROME,         // モノクロ
    BRIGHTEXTRACT,      // ブライトエクストラクト
    COLORKEYALPHA,      // カラーキーアルファ
    COLORTONE,          // カラートーン
    GRAYSCALE,          // グレースケール
    INVERT,             // 反転色
    MOSAIC,             // モザイク
    SEPIA,              // セピア
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
    {NONE,          "VSMain",   "PSMain"},
    {NORMAL,        "VSMain",   "PSNormal"},
    {DARK,          "VSBlend",  "PSDark"},
    {MULTIPLE,      "VSBlend",  "PSMultiple"},
    {BURNCOLOR,     "VSBlend",  "PSBurnColor"},
    {BURNLINEAR,    "VSBlend",  "PSBurnLinear"},
    {COLORDARK,     "VSBlend",  "PSColorDark"},
    {LIGHT,         "VSBlend",  "PSLight"},
    {SCREEN,        "VSBlend",  "PSScreen"},
    {DODGECOLOR,    "VSBlend",  "PSDodgeColor"},
    {DODGELINEAR,   "VSBlend",  "PSDodgeLinear"},
    {COLORLIGHT,    "VSBlend",  "PSColorLight"},
    {OVERLAY,       "VSBlend",  "PSOverlay"},
    {SOFTLIGHT,     "VSBlend",  "PSSoftLight"},
    {HARDLIGHT,     "VSBlend",  "PSHardLight"},
    {VIVIDLIGHT,    "VSBlend",  "PSVividLight"},
    {LINEARLIGHT,   "VSBlend",  "PSLinearLight"},
    {PINLIGHT,      "VSBlend",  "PSPinLight"},
    {HARDMIX,       "VSBlend",  "PSHardMix"},
    {ABSOLUTENESS,  "VSBlend",  "PSAbsoluteness"},
    {EXCLUSION,     "VSBlend",  "PSExclusion"},
    {SUBTRACTION,   "VSBlend",  "PSSubtraction"},
    {DIVISION,      "VSBlend",  "PSDivision"},
    {ADDITION,      "VSBlend",  "PSAddition"},
    {MONOCHROME,    "VSBlend",  "PSMonochrome"},
    {BRIGHTEXTRACT, "VSBlend",  "PSBrightExtract"},
    {COLORKEYALPHA, "VSBlend",  "PSColorKeyAlpha"},
    {COLORTONE,     "VSBlend",  "PSColorTone"},
    {GRAYSCALE,     "VSBlend",  "PSGrayScale"},
    {INVERT,        "VSBlend",  "PSInvert"},
    {MOSAIC,        "VSBlend",  "PSMosaic"},
    {SEPIA,         "VSBlend",  "PSSepia"},
};

// 画像オブジェクト
enum Object
{
    LANDSCAPE,
    HEART,
    OBJ_NUMMAX,
};

// 画像ファイル
static const LPCWSTR file[OBJ_NUMMAX] =
{
    L"landscape.jpg",
    L"heart.png",
};

// ブレンドオブジェクトデータ
struct BlendObjectData
{
    Object obj;
    BlendMode mode;
    BOOL blendFlag;
};

// 使用するブレンドオブジェクトリスト
static const BlendObjectData blendObjectList[] =
{
    {HEART,     NONE,           FALSE},
    {HEART,     NORMAL,         FALSE},
    {HEART,     DARK,           TRUE},
    {HEART,     MULTIPLE,       TRUE},
    {HEART,     BURNCOLOR,      TRUE},
    {HEART,     BURNLINEAR,     TRUE},
    {HEART,     COLORDARK,      TRUE},
    {HEART,     LIGHT,          TRUE},
    {HEART,     SCREEN,         TRUE},
    {HEART,     DODGECOLOR,     TRUE},
    {HEART,     DODGELINEAR,    TRUE},
    {HEART,     OVERLAY,        TRUE},
    {HEART,     SOFTLIGHT,      TRUE},
    {HEART,     HARDLIGHT,      TRUE},
    {HEART,     VIVIDLIGHT,     TRUE},
    {HEART,     LINEARLIGHT,    TRUE},
    {HEART,     PINLIGHT,       TRUE},
    {HEART,     ABSOLUTENESS,   TRUE},
    {HEART,     EXCLUSION,      TRUE},
    {HEART,     SUBTRACTION,    TRUE},
    {HEART,     DIVISION,       TRUE},
    {HEART,     ADDITION,       TRUE},
};

// プリント出力リスト
static const std::vector<LPCSTR> stringList
{
    "ブレンドなし",
    "通常(アルファブレンド)",
    "比較(暗)",
    "乗算",
    "焼き込みカラー",
    "焼き込みリニア",
    "カラー比較(暗)",
    "比較(明)",
    "スクリーン",
    "覆い焼きカラー",
    "覆い焼きリニア(加算)",
    "カラー比較(明)",
    "オーバーレイ",
    "ソフトライト",
    "ハードライト",
    "ビビッドライト",
    "リニアライト",
    "ピンライト",
    "ハードミックス",
    "差の絶対値",
    "除外",
    "減算",
    "除算",
    "加算",
    "モノクロ",
    "ブライトエクストラクト",
    "カラーキーアルファ",
    "カラートーン",
    "グレースケール",
    "反転色",
    "モザイク",
    "セピア",
};

// ブレンド切替変数
static size_t g_blendNum = sizeof(blendObjectList) / sizeof(struct BlendObjectData);

// コンソール出力フラグ
static BOOL g_outputFlag = FALSE;

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
VOID OnKeyDown(UINT8 key);          // キー押下

// usingディレクティブ
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// パイプラインオブジェクト
ComPtr<ID3D11Device> g_device;                                              // デバイスインターフェイス
ComPtr<ID3D11DeviceContext> g_context;                                      // コンテキスト
ComPtr<IDXGISwapChain> g_swapChain;                                         // スワップチェインインターフェイス
ComPtr<ID3D11InputLayout> g_layout[BLEND_NUMMAX > NORMAL ? 2 : 1];          // インプットレイアウト
ComPtr<ID3D11VertexShader> g_vertexShader[BLEND_NUMMAX > NORMAL ? 2 : 1];   // 頂点シェーダ
ComPtr<ID3D11PixelShader> g_pixelShader[BLEND_NUMMAX];                      // ピクセルシェーダ
ComPtr<ID3D11Buffer> g_vertexBuffer;                                        // 頂点バッファ
ComPtr<ID3D11Buffer> g_indexBuffer;                                         // インデックスバッファ
ComPtr<ID3D11Buffer> g_constantBuffer;                                      // 定数バッファ
ComPtr<ID3D11SamplerState> g_sampler;                                       // テクスチャサンプラ
ComPtr<ID3D11ShaderResourceView> g_shaderResourceView[OBJ_NUMMAX];          // テクスチャリソース

// 頂点構造体
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
};

// 定数構造体
struct ConstantBuffer
{
    XMMATRIX m_world;   // ワールド行列の方向ベクトル
    FLOAT m_alpha;      // アルファ値
};

// マトリックス
static XMMATRIX g_world[OBJ_NUMMAX];    // ワールド行列の方向ベクトル

// アルファ値
static FLOAT g_alpha = 1.0f;

// 膨張変数
static FLOAT g_expX = 0.0f;
static FLOAT g_expY = 0.0f;

// Y軸回転変数
static FLOAT g_rotateY = 0.0f;

// 回転フラグ
static BOOL g_rotateFlag = FALSE;

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
    }

    // ターゲット初期化
    HRESULT Init(Target target)
    {
        switch (target) {
        case BACK_BUFFER:       // バックバッファ
            // バックバッファ使用時はスワップチェインと紐づけ
            g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(m_layerBuffer.GetAddressOf()));

            // バックバッファを作成
            if (FAILED(g_device->CreateRenderTargetView(m_layerBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf())))
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
            if (FAILED(g_device->CreateTexture2D(&rtDesc, nullptr, m_layerBuffer.GetAddressOf())))
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
            if (FAILED(g_device->CreateRenderTargetView(m_layerBuffer.Get(), &rtvDesc, m_renderTargetView.GetAddressOf())))
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
            if (FAILED(g_device->CreateShaderResourceView(m_layerBuffer.Get(), &srvDesc, m_shaderResourceView.GetAddressOf())))
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
        g_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
    }

    // レイヤークリア
    inline VOID ClearRenderTargetView() const
    {
        g_context->ClearRenderTargetView(m_renderTargetView.Get(), m_clearColor);
    }

    // シェーダリソースビューアドレス取得
    inline ID3D11ShaderResourceView** GetAddressSRV()
    {
        return m_shaderResourceView.GetAddressOf();
    }

private:
    ComPtr<ID3D11Texture2D> m_layerBuffer;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
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
    case WM_KEYDOWN:    // キー押下時
        OnKeyDown(static_cast<UINT8>(wParam));
        return 0;

    case WM_DESTROY:    // 終了時
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

    // 初期ブレンドをプリント
    printf("0: %s\n", stringList[0]);

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
            g_swapChain.GetAddressOf(),
            g_device.GetAddressOf(),
            nullptr,
            g_context.GetAddressOf()
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
    // PSMainを通す場合は、アルファ値0.0fでも定数バッファで上書きするので、アルファブレンドは無効になる。

    // ブレンドステートを作成
    ComPtr<ID3D11BlendState> blendState;
    if (FAILED(g_device->CreateBlendState(&blendDesc, blendState.GetAddressOf())))
    {
        MessageBox(NULL, L"ブレンドステートを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // ブレンドをセット
    FLOAT blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
    g_context->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);

    // ラスタライザの設定
    D3D11_RASTERIZER_DESC rasDesc;
    ZeroMemory(&rasDesc, sizeof(rasDesc));
    rasDesc.FillMode = D3D11_FILL_SOLID;        // ソリッド
    rasDesc.CullMode = D3D11_CULL_NONE;         // カリングなし：裏表描画

    // ラスタライザステートを作成
    ComPtr<ID3D11RasterizerState> rasState;
    if (FAILED(g_device->CreateRasterizerState(&rasDesc, rasState.GetAddressOf())))
    {
        MessageBox(NULL, L"ラスタライザステートを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // ラスタライザをセット
    g_context->RSSetState(rasState.Get());

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

    ComPtr<ID3DBlob> vertexShader[BLEND_NUMMAX > NORMAL ? 2 : 1], pixelShader[BLEND_NUMMAX];

    // 両方のシェーダをロードしコンパイル
    for (UINT i = 0; i < BLEND_NUMMAX; i++)
    {
        // 頂点シェーダ
        if (i + 1 <= (BLEND_NUMMAX > NORMAL ? 2 : 1))
        {
            if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, initShaderData[i].vsEntry, "vs_4_0", NULL, NULL, NULL, vertexShader[i].GetAddressOf(), NULL, NULL)))
            {
                MessageBox(NULL, L"頂点シェーダを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            // カプセル化
            g_device->CreateVertexShader(vertexShader[i]->GetBufferPointer(), vertexShader[i]->GetBufferSize(), NULL, g_vertexShader[i].GetAddressOf());

            // 頂点インプットレイアウトを作成
            if (FAILED(g_device->CreateInputLayout(inputElementDescs, numElements, vertexShader[i]->GetBufferPointer(), vertexShader[i]->GetBufferSize(), g_layout[i].GetAddressOf())))
            {
                MessageBox(NULL, L"頂点インプットレイアウトの定義が間違っています。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            // 頂点インプットレイアウトをセット
            g_context->IASetInputLayout(g_layout[i].Get());
        }

        // ピクセルシェーダ
        if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, initShaderData[i].psEntry, "ps_4_0", NULL, NULL, NULL, pixelShader[i].GetAddressOf(), NULL, NULL)))
        {
            MessageBox(NULL, L"ピクセルシェーダを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }

        // カプセル化
        g_device->CreatePixelShader(pixelShader[i]->GetBufferPointer(), pixelShader[i]->GetBufferSize(), NULL, g_pixelShader[i].GetAddressOf());
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
    if (FAILED(g_device->CreateBuffer(&bufferDesc, &initData, g_vertexBuffer.GetAddressOf())))
    {
        MessageBox(NULL, L"頂点バッファ1を作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // 表示する頂点バッファを選択
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_context->IASetVertexBuffers(0, 1, g_vertexBuffer.GetAddressOf(), &stride, &offset);

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
    if (FAILED(g_device->CreateBuffer(&bufferDesc, &initData, g_indexBuffer.GetAddressOf())))
    {
        MessageBox(NULL, L"インデックスバッファを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // 表示するインデックスバッファを選択
    g_context->IASetIndexBuffer(g_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    // 使用するプリミティブタイプを設定
    g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 定数情報の追加
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(ConstantBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    // 定数バッファを作成
    if (FAILED(g_device->CreateBuffer(&bufferDesc, nullptr, g_constantBuffer.GetAddressOf())))
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
    if (FAILED(g_device->CreateSamplerState(&samplerDesc, g_sampler.GetAddressOf())))
    {
        MessageBox(NULL, L"サンプラステートを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // テクスチャの読み込み
    for (UINT i = 0; i < OBJ_NUMMAX; i++)
    {
        if (FAILED(D3DX11CreateShaderResourceViewFromFile(g_device.Get(), file[i], NULL, NULL, g_shaderResourceView[i].GetAddressOf(), NULL)))
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
    // 膨張
    g_expX += 0.0005f;                  // X軸0.0005ずつ膨張
    g_expY += 0.0008f;                  // Y軸0.0008ずつ膨張
    if (g_expX > 0.5f) g_expX = 0.5f;   // 上限0.5f
    if (g_expY > 0.8f) g_expY = 0.8f;   // 上限0.8f
    XMMATRIX scale = XMMatrixScaling(g_expX, g_expY, 0.0f);
    // ブレンド切替の時は膨張値をリセット

    // 回転
    XMMATRIX rotate = XMMatrixRotationY(g_rotateY += g_rotateFlag ? 0.0005f : 0.0f);
    // 回転フラグ/オンの時はY軸0.0005ずつ回転
    // 回転フラグ/オフの時はそのままの回転値でストップ
    // ブレンド切替の時は回転値をリセット

    // パラメータの受け渡し
    g_world[HEART] = scale * rotate;    // 各マトリックスの積

    // アルファ限界値
    if (g_alpha > 1.0f) g_alpha = 1.0f;         // 上限1.0f
    if (g_alpha < 0.25f) g_alpha = 0.2f;        // 下限0.2f
}

// 描画
VOID OnRender()
{
    // 共通バッファをセット
    g_context->VSSetConstantBuffers(0, 1, g_constantBuffer.GetAddressOf());
    g_context->PSSetConstantBuffers(0, 1, g_constantBuffer.GetAddressOf());
    g_context->PSSetSamplers(0, 1, g_sampler.GetAddressOf());

    size_t blendObjNumMax = sizeof(blendObjectList) / sizeof(struct BlendObjectData);
    size_t listNum = g_blendNum % blendObjNumMax;



    // レンダーターゲット1をセット
    g_renderTarget[_RENDER_TARGET1]->SetRenderTarget();
    g_renderTarget[_RENDER_TARGET1]->ClearRenderTargetView();

    // シェーダオブジェクトをセット
    g_context->VSSetShader(g_vertexShader[NONE].Get(), nullptr, 0);
    g_context->PSSetShader(g_pixelShader[NONE].Get(), nullptr, 0);

    // テクスチャ1をシェーダに登録
    g_context->PSSetShaderResources(0, 1, g_shaderResourceView[LANDSCAPE].GetAddressOf());

    // 変数1代入
    ConstantBuffer cBuffer;
    cBuffer.m_world = XMMatrixTranspose(g_world[LANDSCAPE]);
    cBuffer.m_alpha = 1.0f;

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer.Get(), 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをRT1に描画
    g_context->DrawIndexed(6, 0, 0);



    // レンダーターゲット2をセット
    g_renderTarget[_RENDER_TARGET2]->SetRenderTarget();
    g_renderTarget[_RENDER_TARGET2]->ClearRenderTargetView();

    // シェーダオブジェクトをセット
    g_context->PSSetShader(g_pixelShader[blendObjectList[listNum == NONE ? NONE : NORMAL].mode].Get(), nullptr, 0);

    // テクスチャ2をシェーダに登録
    g_context->PSSetShaderResources(0, 1, g_shaderResourceView[blendObjectList[listNum == NONE ? NONE : NORMAL].obj].GetAddressOf());

    // 変数2代入
    cBuffer.m_world = XMMatrixTranspose(g_world[blendObjectList[listNum == NONE ? NONE : NORMAL].obj]);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer.Get(), 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをRT2に描画
    g_context->DrawIndexed(6, 0, 0);



    // バックバッファをセット
    g_renderTarget[_BACK_BUFFER]->SetRenderTarget();
    g_renderTarget[_BACK_BUFFER]->ClearRenderTargetView();


    // シェーダオブジェクトをセット
    g_context->PSSetShader(g_pixelShader[NONE].Get(), nullptr, 0);

    // RT1を下のレイヤーとして先に描画
    g_context->PSSetShaderResources(0, 1, g_renderTarget[_RENDER_TARGET1]->GetAddressSRV());

    // 変数1代入
    cBuffer.m_world = XMMatrixTranspose(g_world[LANDSCAPE]);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer.Get(), 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをバックバッファに描画
    g_context->DrawIndexed(6, 0, 0);
    
    // シェーダオブジェクトをセット
    g_context->VSSetShader(g_vertexShader[blendObjectList[listNum].blendFlag].Get(), nullptr, 0);
    if (blendObjectList[listNum].mode == NONE) g_context->PSSetShader(g_pixelShader[NORMAL].Get(), nullptr, 0);
    else g_context->PSSetShader(g_pixelShader[blendObjectList[listNum].mode].Get(), nullptr, 0);

    if (blendObjectList[listNum].mode > NORMAL)
    {
        // RT1とRT2をシェーダに登録し合成
        g_context->PSSetShaderResources(0, 1, g_renderTarget[_RENDER_TARGET1]->GetAddressSRV());
        g_context->PSSetShaderResources(1, 1, g_renderTarget[_RENDER_TARGET2]->GetAddressSRV());
    }
    else g_context->PSSetShaderResources(0, 1, g_renderTarget[_RENDER_TARGET2]->GetAddressSRV());

    // 変数2代入
    cBuffer.m_alpha = g_alpha;

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer.Get(), 0, nullptr, &cBuffer, 0, 0);

    // インデックスバッファをバックバッファに描画
    g_context->DrawIndexed(6, 0, 0);
    


    // 両極端に達したらリセット
    if (g_blendNum == 0 || g_blendNum == blendObjNumMax * 2) g_blendNum = blendObjNumMax;

    // コンソール出力
    if (g_outputFlag)
    {
        // プリント
        printf("%d: %s\n", listNum, stringList[blendObjectList[listNum].mode]);

        // コンソール出力フラグ/オフ
        g_outputFlag = FALSE;
    }



    // フレームを最終出力
    g_swapChain->Present(0, 0);     // フリップ
}

// キー押下
VOID OnKeyDown(UINT8 key)
{
    switch (key)
    {
    case 'L':       // 次
        g_blendNum++;
        g_outputFlag = TRUE;        // コンソール出力フラグ/オン
        g_expX = g_expY = 0.0f;     // エクスパンションリセット
        g_rotateFlag = FALSE;       // 回転フラグ/オフ
        g_rotateY = 0.0f;           // 回転値リセット
        break;
    case 'J':       // 戻
        g_blendNum--;
        g_outputFlag = TRUE;        // コンソール出力フラグ/オン
        g_expX = g_expY = 0.0f;     // エクスパンションリセット
        g_rotateFlag = FALSE;       // 回転フラグ/オフ
        g_rotateY = 0.0f;           // 回転値リセット
        break;
    case 'I':       // 濃
        g_alpha += 0.2f;
        break;
    case 'M':       // 薄
        g_alpha -= 0.2f;
        break;
    case 'K':       // 回転スイッチ
        g_rotateFlag = !g_rotateFlag;     // 回転フラグ反転
        break;
    }
}
