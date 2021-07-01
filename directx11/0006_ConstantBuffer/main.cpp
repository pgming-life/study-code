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

#define WINDOW_CLASS    L"定数バッファ"
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
ID3D11Buffer* g_indexBuffer;                        // インデックスバッファ
ID3D11Buffer* g_constantBuffer;                     // 定数バッファ

// 頂点構造体
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT4 color;
};

// 定数構造体
struct ConstantBuffer
{
    XMMATRIX m_WVP;
};

// マトリックス
XMMATRIX g_world;           // ワールド行列の方向ベクトル
XMMATRIX g_view;            // ビュー行列の方向ベクトル
XMMATRIX g_projection;      // プロジェクション行列の方向ベクトル

// Y軸回転変数
static FLOAT y = 0;

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
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
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
        { {-1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },    // 左上-赤
        { { 1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },    // 右上-黄
        { {-1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },    // 左下-青
        { { 1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },    // 右下-緑
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
        MessageBox(NULL, L"頂点バッファを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
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

// マトリックス関連初期化
VOID InitMatrix()
{
    // ワールドマトリックスの初期化
    g_world = XMMatrixIdentity();       // 引数なしで全ての軸で0.0f(移動なし)

    // ビューマトリックスの初期化
    XMVECTOR eye = XMVectorSet(0.0f, 1.0f, -2.0f, 0.0f);    // カメラの座標を示すベクトル
    XMVECTOR focus = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);   // 焦点(ターゲット)の座標を示すベクトル
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);      // カメラの上方向を示すベクトル
    g_view = XMMatrixLookAtLH(eye, focus, up);

    // プロジェクションマトリックスの初期化(射影行列変換)
    g_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(WINDOW_WIDTH) / static_cast<FLOAT>(WINDOW_HEIGHT), 0.1f, 100.0f);
    // 視野角45°(XM_PI(π)を4で割るので45°)
    // ビュー空間X:Yのアスペクト比
    // Z軸0.1fから100.0fまでの間の空間
}

// 更新
VOID OnUpdate()
{
    // パラメータの受け渡し
    ConstantBuffer cBuffer;
    XMMATRIX scale = XMMatrixScaling(0.5f, 0.5f, 0.0f);     // X軸Y軸を1/2に縮小
    XMMATRIX translate = XMMatrixRotationY(y += 0.0002);    // Y軸0.0002ずつ回転
    g_world = scale * translate;                            // 各マトリックスの積
    cBuffer.m_WVP = XMMatrixTranspose(g_world * g_view * g_projection);

    // GPU(シェーダ側)へ転送する
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);
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
}
