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

#define WINDOW_CLASS    L"文字の描画"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

// フォント情報
static const FLOAT g_fontSize = 350.f;       // 350px
static const LPCWSTR g_fontChar = L"驫";     // Shift-JIS漢字の中で最も画数が多い漢字

// プロトタイプ宣言
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // ウィンドウプロシージャ
HRESULT OnInit(HWND hWnd);          // 初期化
HRESULT InitDevice(HWND hWnd);      // デバイス関連初期化
HRESULT InitView();                 // ビュー関連初期化
HRESULT InitShader();               // シェーダ関連初期化
HRESULT InitBuffer();               // バッファ関連初期化
HRESULT InitTexture();              // テクスチャ関連初期化
VOID InitMatrix();                  // マトリックス関連初期化
VOID OnRender();                    // 描画

// usingディレクティブ
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// パイプラインオブジェクト
ComPtr<ID3D11Device> g_device;                              // デバイスインターフェイス
ComPtr<ID3D11DeviceContext> g_context;                      // コンテキスト
ComPtr<IDXGISwapChain> g_swapChain;                         // スワップチェインインターフェイス
ComPtr<ID3D11RenderTargetView> g_renderTargetView;          // レンダーターゲットビュー
ComPtr<ID3D11InputLayout> g_layout;                         // インプットレイアウト
ComPtr<ID3D11VertexShader> g_vertexShader;                  // 頂点シェーダ
ComPtr<ID3D11PixelShader> g_pixelShader;                    // ピクセルシェーダ
ComPtr<ID3D11Buffer> g_vertexBuffer;                        // 頂点バッファ
ComPtr<ID3D11Buffer> g_indexBuffer;                         // インデックスバッファ
ComPtr<ID3D11Buffer> g_constantBuffer;                      // 定数バッファ
ComPtr<ID3D11SamplerState> g_sampler;                       // テクスチャサンプラ

// 頂点構造体
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
};

// 定数構造体
struct ConstantBuffer
{
    XMMATRIX m_WP;
};

// フォントテキストクラス
class FontText
{
public:
    // コンストラクタ
    FontText() : m_layerBuffer(nullptr), m_shaderResourceView(nullptr)
    {
    }

    // デストラクタ
    ~FontText()
    {
    }

    // １文字の生成
    HRESULT InitChar(LPCWSTR c)
    {
        // フォントハンドルの設定
        UINT fontSize = 64;
        UINT fontWeight = 1000;
        LPCSTR font = "ＭＳ ゴシック";
        LOGFONT lf =
        {
            fontSize, 0, 0, 0,
            fontWeight, 0, 0, 0,
            SHIFTJIS_CHARSET,
            OUT_TT_ONLY_PRECIS,
            CLIP_DEFAULT_PRECIS,
            PROOF_QUALITY,
            DEFAULT_PITCH | FF_MODERN,
            (WCHAR)font,
        };

        // フォントハンドルを生成
        HFONT hFont = CreateFontIndirectW(&lf);

        // 現在のウィンドウに適用
        HDC hdc = GetDC(NULL);
        HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, hFont));
        // デバイスにフォントを持たせないとGetGlyphOutlineW関数はエラーとなる。

        // 出力する文字(一文字だけ)
        UINT code = static_cast<UINT>(*c);

        // 17階調のグレーのグリフビットマップ
        const UINT gradFlag = GGO_GRAY4_BITMAP;

        // ビットマップの設定
        GLYPHMETRICS gm;
        TEXTMETRIC tm;
        GetTextMetrics(hdc, &tm);
        CONST MAT2 mat = { {0,1},{0,0},{0,0},{0,1} };

        // フォントビットマップを取得
        DWORD size = GetGlyphOutlineW(hdc, code, gradFlag, &gm, 0, NULL, &mat);
        BYTE* pMono = new BYTE[size];
        GetGlyphOutlineW(hdc, code, gradFlag, &gm, size, pMono, &mat);

        // フォントの幅と高さ
        INT fontWidth = gm.gmCellIncX;
        INT fontHeight = tm.tmHeight;

        // レンダーターゲットの設定
        D3D11_TEXTURE2D_DESC rtDesc;
        ZeroMemory(&rtDesc, sizeof(rtDesc));
        rtDesc.Width = fontWidth;
        rtDesc.Height = fontHeight;
        rtDesc.MipLevels = 1;
        rtDesc.ArraySize = 1;
        rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtDesc.SampleDesc.Count = 1;
        rtDesc.SampleDesc.Quality = 0;
        rtDesc.Usage = D3D11_USAGE_DYNAMIC;
        rtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        rtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        rtDesc.MiscFlags = 0;

        // フォント用テクスチャを作成
        if (FAILED(g_device->CreateTexture2D(&rtDesc, nullptr, m_layerBuffer.GetAddressOf())))
        {
            MessageBox(NULL, L"フォント用テクスチャを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }

        // フォント用テクスチャリソースにテクスチャ情報をコピー
        D3D11_MAPPED_SUBRESOURCE mappedSubrsrc;
        g_context->Map(m_layerBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubrsrc);
        BYTE* pBits = static_cast<BYTE*>(mappedSubrsrc.pData);
        INT iOfs_x = gm.gmptGlyphOrigin.x;
        INT iOfs_y = tm.tmAscent - gm.gmptGlyphOrigin.y;
        INT iBmp_w = gm.gmBlackBoxX + (4 - (gm.gmBlackBoxX % 4)) % 4;
        INT iBmp_h = gm.gmBlackBoxY;
        INT Level = 17;
        INT x, y;
        DWORD Alpha, Color;
        memset(pBits, 0, mappedSubrsrc.RowPitch * tm.tmHeight);
        for (y = iOfs_y; y < iOfs_y + iBmp_h; y++)
        {
            for (x = iOfs_x; x < iOfs_x + iBmp_w; x++)
            {
                Alpha = (255 * pMono[x - iOfs_x + iBmp_w * (y - iOfs_y)]) / (Level - 1);
                Color = 0x00ffffff | (Alpha << 24);
                memcpy(static_cast<BYTE*>(pBits) + mappedSubrsrc.RowPitch * y + 4 * x, &Color, sizeof(DWORD));
            }
        }
        g_context->Unmap(m_layerBuffer.Get(), 0);
        // フォント情報の書き込み
        // iOfs_x, iOfs_y : 書き出し位置(左上)
        // iBmp_w, iBmp_h : フォントビットマップの幅高
        // Level : α値の段階 (GGO_GRAY4_BITMAPなので17段階)

        // メモリ解放
        delete[] pMono;

        // シェーダリソースビューの設定
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = rtDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = rtDesc.MipLevels;

        // シェーダリソースビューを作成
        if (FAILED(g_device->CreateShaderResourceView(m_layerBuffer.Get(), &srvDesc, m_shaderResourceView.GetAddressOf())))
        {
            MessageBox(NULL, L"シェーダリソースビューを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }
    }

    // シェーダリソースビューアドレス取得
    inline ID3D11ShaderResourceView** GetAddressSRV()
    {
        return m_shaderResourceView.GetAddressOf();
    }

private:
    ComPtr<ID3D11Texture2D> m_layerBuffer;
    ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
};
std::unique_ptr<FontText> g_fontText;

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
                // 描画
                OnRender();
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

    // バックバッファを作成
    ComPtr<ID3D11Texture2D> backBuffer;
    g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    g_device->CreateRenderTargetView(backBuffer.Get(), nullptr, g_renderTargetView.GetAddressOf());

    // レンダーターゲットをバックバッファに設定
    g_context->OMSetRenderTargets(1, g_renderTargetView.GetAddressOf(), nullptr);

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
    ComPtr<ID3D11BlendState> blendState;
    if (FAILED(g_device->CreateBlendState(&blendDesc, blendState.GetAddressOf())))
    {
        MessageBox(NULL, L"ブレンドステートを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // ブレンドをセット
    FLOAT blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
    g_context->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);

    return S_OK;
}

// シェーダ関連初期化
HRESULT InitShader()
{
    ComPtr<ID3DBlob> vertexShader, pixelShader;

    // 両方のシェーダをロードしコンパイル
    if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, "vsMain", "vs_4_0", NULL, NULL, NULL, vertexShader.GetAddressOf(), NULL, NULL)))
    {
        MessageBox(NULL, L"頂点シェーダを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }
    if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, "psMain", "ps_4_0", NULL, NULL, NULL, pixelShader.GetAddressOf(), NULL, NULL)))
    {
        MessageBox(NULL, L"ピクセルシェーダを読み込めませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // カプセル化
    g_device->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), NULL, g_vertexShader.GetAddressOf());
    g_device->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), NULL, g_pixelShader.GetAddressOf());

    // 頂点インプットレイアウトを定義
    D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    // インプットレイアウトのサイズ
    UINT numElements = sizeof(inputElementDescs) / sizeof(inputElementDescs[0]);

    // 頂点インプットレイアウトを作成
    if (FAILED(g_device->CreateInputLayout(inputElementDescs, numElements, vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), g_layout.GetAddressOf())))
    {
        MessageBox(NULL, L"頂点インプットレイアウトの定義が間違っています。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // 頂点インプットレイアウトをセット
    g_context->IASetInputLayout(g_layout.Get());

    return S_OK;
}

// バッファ関連初期化
HRESULT InitBuffer()
{
    // 四角形のジオメトリを定義
    Vertex vertices[] =
    {
        {{0.0f,       0.0f,       0.0f}, {0.0f, 0.0f}},
        {{g_fontSize, 0.0f,       0.0f}, {1.0f, 0.0f}},
        {{0.0f,       g_fontSize, 0.0f}, {0.0f, 1.0f}},
        {{g_fontSize, g_fontSize, 0.0f}, {1.0f, 1.0f}},
    };
    // フォント用ポリゴン。
    // 頂点座標はTranslationに対応するためにピクセル座標に変換。
    // Scalingは行列を使わず、頂点そのものに対してスケールするのでUV値はいじらない。
    // なので、ターゲットを定義するその都度頂点情報を更新する。

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
        MessageBox(NULL, L"頂点バッファを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
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

    // フォントテキスト初期化
    g_fontText.reset(new FontText());
    g_fontText->InitChar(g_fontChar);

    return S_OK;
}

// マトリックス関連初期化
VOID InitMatrix()
{
    // ワールドマトリックスの初期化
    XMMATRIX world = XMMatrixIdentity();       // 引数なしで全ての軸で0.0f(移動なし)

    // プロジェクションマトリックスの初期化(射影行列変換)
    XMMATRIX projection = XMMatrixOrthographicOffCenterLH(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 0.0f, 1.0f);
    // 正射影変換行列を設定しこの正行列によって平行投影する(遠くに行っても同じ大きさ)
    // 左上(0, 0)を基準とした2D座標にするために頂点ジオメトリをWINDOW_WIDTHとWINDOW_HEIGHTで設定
    // 奥行方向: 2D描画用(Z軸は0～1)

    // 変数代入
    ConstantBuffer cBuffer;
    cBuffer.m_WP = XMMatrixTranspose(world * projection);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer.Get(), 0, nullptr, &cBuffer, 0, 0);
}

// 描画
VOID OnRender()
{
    // レンダーターゲットビューを指定した色でクリア
    const FLOAT clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };       // ブルー
    g_context->ClearRenderTargetView(g_renderTargetView.Get(), clearColor);

    // GPUバッファをセット
    g_context->VSSetShader(g_vertexShader.Get(), nullptr, 0);
    g_context->PSSetShader(g_pixelShader.Get(), nullptr, 0);
    g_context->VSSetConstantBuffers(0, 1, g_constantBuffer.GetAddressOf());

    // テクスチャをシェーダに登録
    g_context->PSSetSamplers(0, 1, g_sampler.GetAddressOf());
    g_context->PSSetShaderResources(0, 1, g_fontText->GetAddressSRV());

    // インデックスバッファをバックバッファに描画
    g_context->DrawIndexed(6, 0, 0);

    // フレームを最終出力
    g_swapChain->Present(0, 0);     // フリップ
}
