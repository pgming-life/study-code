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

#define WINDOW_CLASS    L"ブレンドモードテスト [L:次・J:戻]"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

#define SAFE_RELEASE(x) if(x) x->Release();

// プロトタイプ宣言
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // ウィンドウプロシージャ
HRESULT OnInit(HWND hWnd);          // 初期化
HRESULT InitDevice(HWND hWnd);      // デバイス関連初期化
VOID InitView();                    // ビュー関連初期化
HRESULT InitShader();               // シェーダ関連初期化
HRESULT InitBuffer();               // バッファ関連初期化
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

// 頂点構造体
struct Vertex
{
    XMFLOAT3 position;
    D3DXCOLOR color;
};

// 表示領域の寸法(アスペクト比)
static const FLOAT g_aspectRatio = static_cast<FLOAT>(WINDOW_WIDTH) / static_cast<FLOAT>(WINDOW_HEIGHT);

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
    enum Mode {
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

    // コンストラクタ
    BlendMode() : m_vertexBuffer(nullptr)
    {
    }

    // デストラクタ
    ~BlendMode()
    {
        SAFE_RELEASE(m_vertexBuffer);
    }

    // ブレンドモード初期化
    HRESULT Init(Mode mode)
    {
        // 赤っぽい(基本色)
        D3DXCOLOR dest =
        {
            244 / 255.f,    // Rt=244
            24 / 255.f,     // Gt=24
            24 / 255.f,     // Bt=24
            1.0f,           // At=255
        };

        // 緑っぽい(合成色)
        D3DXCOLOR src =
        {
            25 / 255.f,     // Rb=25
            220 / 255.f,    // Gb=220
            140 / 255.f,    // Bb=140
            1.0f,           // Ab=255
        };

        // 合成結果
        D3DXCOLOR result;   // Ro, Go, Bo, Ao

        switch (mode)
        {
        case NORMAL:        // 通常(アルファブレンド)
            result = { src.r, src.g, src.b, src.a };
            // Ro = Rt = 25
            // Go = Gt = 220
            // Bo = Bt = 140
            // オーバーラップ部分は常に上のレイヤーになる。
            // 透過はなし。シェーダの方では実装予定。
            break;

        case DARK:          // 比較(暗)
            result =
            {
                min(dest.r, src.r),
                min(dest.g, src.g),
                min(dest.b, src.b),
                src.a,
            };
            // Ro = Rt = 25
            // Go = Gb = 24
            // Bo = Bb = 24
            // レイヤー間で低い値を採用。
            break;

        case MULTIPLE:      // 乗算
            result =
            {
                dest.r * src.r,
                dest.g * src.g,
                dest.b * src.b,
                src.a,
            };
            // Ro = Rt×Rb÷255 = 25 * 244 / 255 = 23.92 ≒ 24
            // Go = Gt×Gb÷255 = 24 * 220 / 255 = 20.71 ≒ 21
            // Bo = Bt×Bb÷255 = 24 * 140 / 255 = 13.18 ≒ 13
            // 上下のレイヤーの値を乗算して255で割る。
            break;

        case BURNCOLOR:     // 焼き込みカラー
            result =
            {
                1.0f - (1.0f - dest.r) / src.r,
                1.0f - (1.0f - dest.g) / src.g,
                1.0f - (1.0f - dest.b) / src.b,
                src.a,
            };
            // Overwrap = 255 - (255 - Bottom) * 255 / Top
            // Ro = 255 - (255 - 244) * 255 / 25 = 142.8 ≒ 143
            // Go = 255 - (255 - 24) * 255 / 220 = -12.75 ≒ 0
            // Bo = 255 - (255 - 24) * 255 / 140 = -165.75 ≒ 0
            // Topが0，もしくは、結果がマイナスの場合は、0，となる。
            // 上記では0.0fを下回るものは自動で0.0fになる。
            break;

        case BURNLINEAR:    // 焼き込みリニア
            result =
            {
                dest.r + src.r <= 1.0f ? 0.0f : dest.r + src.r - 1.0f,
                dest.g + src.g <= 1.0f ? 0.0f : dest.g + src.g - 1.0f,
                dest.b + src.b <= 1.0f ? 0.0f : dest.b + src.b - 1.0f,
                src.a,
            };
            // 二枚のレイヤーの各RGBの和が，
            // 255以下：0
            // 255以上：二枚の値の和 - 255
            break;

        case COLORDARK:     // カラー比較(暗)
            if (src.r + src.g + src.b > dest.r + dest.g + dest.b)
                result = { dest.r, dest.g, dest.b, src.a };
            else if (src.r + src.g + src.b <= dest.r + dest.g + dest.b)
                result = { src.r, src.g, src.b, src.a };
            // 25 + 220 + 140 = 385 > 244 + 24 + 24 = 292
            // 二つのレイヤーのRGBの和を比較して，低い方の値を示す。
            break;

        case LIGHT:         // 比較(明)
            result =
            {
                max(dest.r, src.r),
                max(dest.g, src.g),
                max(dest.b, src.b),
                src.a,
            };
            // Ro = Rb = 244
            // Go = Gt = 220
            // Bo = Bt = 140
            // レイヤー間で高い値を採用。
            break;

        case SCREEN:        // スクリーン
            result =
            {
                dest.r + src.r - dest.r * src.r,
                dest.g + src.g - dest.g * src.g,
                dest.b + src.b - dest.b * src.b,
                src.a,
            };
            // Overwrap = Bottom + Top - Bottom * Top / 255
            // Ro = 244 + 25 - 244 * 25 / 255 = 245.08 ≒ 245
            // Go = 24 + 220 - 24 * 220 / 255 = 223.29 ≒ 223
            // Bo = 24 + 140 - 24 * 140 / 255 = 150.82 ≒ 151
            break;

        case DODGECOLOR:    // 覆い焼きカラー
            result =
            {
                dest.r / (1.0f - src.r),
                dest.g / (1.0f - src.g),
                dest.b / (1.0f - src.b),
                src.a,
            };
            // Overwrap = Bottom * 255 / (255 - Top)
            // Ro = 244 * 255 / (255 - 25) = 270.5 ≒ 255
            // Go = 24 * 255 / (255 - 220) = 174.86 ≒ 175
            // Bo = 24 * 255 / (255 - 140) = 53.22 ≒ 53
            // 結果が255以上の場合は、255となる。
            // 上記では1.0fを超えるものは自動で1.0fになる。
            break;

        case DODGELINEAR:   // 覆い焼きリニア(加算)
            result =
            {
                src.r + dest.r,
                src.g + dest.g,
                src.b + dest.b,
                src.a,
            };
            // Ro = 25 + 244 = 269 ≒ 255
            // Go = 220 + 24 = 244
            // Bo = 140 + 24 = 164
            // 各色を単純加算し、255以上は255となる。
            // 上記では1.0fを超えるものは自動で1.0fになる。
            // 加算合成と同じ？
            break;

        case COLORLIGHT:    // カラー比較(明)
            if (src.r + src.g + src.b > dest.r + dest.g + dest.b)
                result = { src.r, src.g, src.b, src.a };
            else if (src.r + src.g + src.b <= dest.r + dest.g + dest.b)
                result = { dest.r, dest.g, dest.b, src.a };
            // 25 + 220 + 140 = 385 > 244 + 24 + 24 = 292
            // 二つのレイヤーのRGBの和を比較して，高い方の値を示す。
            break;

        case OVERLAY:       // オーバーレイ
            if (dest.r < 0.5f) result.r = dest.r * src.r * 2;
            else result.r = 2 * (dest.r + src.r - dest.r * src.r) - 1.0f;
            if (dest.g < 0.5f) result.g = dest.g * src.g * 2;
            else result.g = 2 * (dest.g + src.g - dest.g * src.g) - 1.0f;
            if (dest.b < 0.5f) result.b = dest.b * src.b * 2;
            else result.b = 2 * (dest.b + src.b - dest.b * src.b) - 1.0f;
            result.a = src.a;
            // 下の色の値で条件が変わり，
            // Bottom < 128 の場合
            // Bottom * Top * 2 / 255
            // Bottom >= 128 の場合
            // 2 * (Bottom + Top - Bottom * Top / 255) - 255
            // したがって、
            // Ro = 2 * (244 + 25 - 244 * 25 / 255) / 255 - 255 = 235.16 ≒ 235
            // Go = 24 * 220 * 2 / 255 = 41.41 ≒ 41
            // Bo = 24 * 140 * 2 / 255 = 26.35 ≒ 26
            break;

        case SOFTLIGHT:     // ソフトライト
            if (src.r < 0.5f) result.r = 2 * dest.r * src.r + powf(dest.r, 2) * (1.0f - 2 * src.r);
            else result.r = 2 * dest.r * (1.0f - src.r) + sqrtf(dest.r) * (2 * src.r - 1.0f);
            if (src.g < 0.5f) result.g = 2 * dest.g * src.g + powf(dest.g, 2) * (1.0f - 2 * src.g);
            else result.g = 2 * dest.g * (1.0f - src.g) + sqrtf(dest.g) * (2 * src.g - 1.0f);
            if (src.b < 0.5f) result.b = 2 * dest.b * src.b + powf(dest.b, 2) * (1.0f - 2 * src.b);
            else result.b = 2 * dest.b * (1.0f - src.b) + sqrtf(dest.b) * (2 * src.b - 1.0f);
            result.a = src.a;
            break;

        case HARDLIGHT:     // ハードライト
            if (dest.r > 0.5f) result.r = dest.r * src.r * 2;
            else result.r = 2 * (dest.r + src.r - dest.r * src.r) - 1.0f;
            if (dest.g > 0.5f) result.g = dest.g * src.g * 2;
            else result.g = 2 * (dest.g + src.g - dest.g * src.g) - 1.0f;
            if (dest.b > 0.5f) result.b = dest.b * src.b * 2;
            else result.b = 2 * (dest.b + src.b - dest.b * src.b) - 1.0f;
            result.a = src.a;
            // 下の色の値で条件が変わり，
            // Bottom > 128 の場合
            // Bottom * Top * 2 / 255
            // Bottom <= 128 の場合
            // 2 * (Bottom + Top - Bottom * Top / 255) - 255
            // したがって
            // Ro = 244 * 25 * 2 / 255 = 47.84 ≒ 48
            // Go = 2 * (24 + 220 - 24 * 220 / 255) / 255 - 255 = 191.59 ≒ 192
            // Bo = 2 * (24 + 140 - 24 * 140 / 255) / 255 - 255 = 46.65 ≒ 47
            // オーバーレイの条件と逆転している。
            // また、Boがこれだけ小数点以下切り下げ(Photoshopでは46）
            break;

        case VIVIDLIGHT:    // ビビッドライト
            if (src.r < 0.5f) result.r = 1.0f - (1.0f - dest.r) / (2 * src.r);
            else result.r = dest.r / (1.0f - 2 * (src.r - 0.5f));
            if (src.g < 0.5f) result.g = 1.0f - (1.0f - dest.g) / (2 * src.g);
            else result.g = dest.g / (1.0f - 2 * (src.g - 0.5f));
            if (src.b < 0.5f) result.b = 1.0f - (1.0f - dest.b) / (2 * src.b);
            else result.b = dest.b / (1.0f - 2 * (src.b - 0.5f));
            result.a = src.a;
            break;

        case LINEARLIGHT:   // リニアライト
            if (src.r < 0.5f) result.r = dest.r + 2 * src.r - 1.0f;
            else result.r = dest.r + 2 * (src.r - 0.5f);
            if (src.g < 0.5f) result.g = dest.g + 2 * src.g - 1.0f;
            else result.g = dest.g + 2 * (src.g - 0.5f);
            if (src.b < 0.5f) result.b = dest.b + 2 * src.b - 1.0f;
            else result.b = dest.b + 2 * (src.b - 0.5f);
            result.a = src.a;
            break;

        case PINLIGHT:      // ピンライト
            if (src.r < 0.5f) result.r = min(dest.r, 2 * src.r);
            else result.r = max(dest.r, 2 * (src.r - 0.5f));
            if (src.g < 0.5f) result.g = min(dest.g, 2 * src.g);
            else result.g = max(dest.g, 2 * (src.g - 0.5f));
            if (src.b < 0.5f) result.b = min(dest.b, 2 * src.b);
            else result.b = max(dest.b, 2 * (src.b - 0.5f));
            result.a = src.a;
            break;

        case HARDMIX:       // ハードミックス
            break;

        case ABSOLUTENESS:  // 差の絶対値
            result =
            {
                fabsf(src.r - dest.r),
                fabsf(src.g - dest.g),
                fabsf(src.b - dest.b),
                src.a,
            };
            // Ro =｜25-244｜= 219
            // Go = ｜220 - 24｜ = 196
            // Bo = ｜140 - 24｜ = 116
            // 差分の絶対値。
            break;

        case EXCLUSION:     // 除外
            result =
            {
                dest.r + src.r - 2 * dest.r * src.r,
                dest.g + src.g - 2 * dest.g * src.g,
                dest.b + src.b - 2 * dest.b * src.b,
                src.a,
            };
            break;

        case SUBTRACTION:   // 減算
            result =
            {
                dest.r - src.r,
                dest.g - src.g,
                dest.b - src.b,
                src.a,
            };
            break;

        case DIVISION:      // 除算
            result =
            {
                dest.r / src.r,
                dest.g / src.g,
                dest.b / src.b,
                src.a,
            };
            break;

        case ADDITION:      // 加算
            result =
            {
                dest.r + src.r,
                dest.g + src.g,
                dest.b + src.b,
                src.a,
            };
            // Ro = 25 + 244 = 269 ≒ 255
            // Go = 220 + 24 = 196
            // Bo = 140 + 24 = 164
            // 上下のレイヤーの加算。
            // 上記では1.0fを超えるものは自動で1.0fになる。
            break;
        default:            // 保険処理
            result = { 1.0f, 1.0f, 1.0f, 1.0f };      // 白
        }

        // 三角形のジオメトリを定義
        Vertex vertices[] =
        {
            {{ 0.0f,   0.25f * g_aspectRatio, 0.0f}, result},	// 上(合成結果)
            {{ 0.25f, -0.25f * g_aspectRatio, 0.0f}, src},		// 右(緑っぽい)
            {{-0.25f, -0.25f * g_aspectRatio, 0.0f}, dest},		// 左(赤っぽい)
        };

        // バッファを作成
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));

        // 頂点バッファの設定
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;                 // CPUおよびGPUによる書き込みアクセス
        bufferDesc.ByteWidth = sizeof(Vertex) * 3;              // サイズはVertex構造体×3
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;        // 頂点バッファを使用
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;     // CPUがバッファに書き込むことを許可

        // 頂点バッファを作成
        if (FAILED(g_device->CreateBuffer(&bufferDesc, nullptr, &m_vertexBuffer)))
        {
            MessageBox(NULL, L"頂点バッファを作成できませんでした。", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }

        // 頂点バッファに頂点データをコピー
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        g_context->Map(m_vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedResource);   // バッファのマッピング
        memcpy(mappedResource.pData, vertices, sizeof(vertices));                               // データをコピー
        g_context->Unmap(m_vertexBuffer, NULL);

        return S_OK;
    }

    // 頂点バッファアドレスのゲッター
    inline ID3D11Buffer** GetAddressVertexBuffer()
    {
        return &m_vertexBuffer;
    }

private:
    ID3D11Buffer* m_vertexBuffer;       // 頂点バッファ
};
std::unique_ptr<BlendMode> g_blendMode[BlendMode::BLEND_NUMMAX];

// 使用するブレンドのリスト
static const std::vector<BlendMode::Mode> blendList
{
    BlendMode::NORMAL,          // 通常(アルファブレンド)
    BlendMode::DARK,            // 比較(暗)
    BlendMode::MULTIPLE,        // 乗算
    BlendMode::BURNCOLOR,       // 焼き込みカラー
    BlendMode::BURNLINEAR,      // 焼き込みリニア
    BlendMode::COLORDARK,       // カラー比較(暗)
    BlendMode::LIGHT,           // 比較(明)
    BlendMode::SCREEN,          // スクリーン
    BlendMode::DODGECOLOR,      // 覆い焼きカラー
    BlendMode::DODGELINEAR,     // 覆い焼きリニア(加算)
    BlendMode::COLORLIGHT,      // カラー比較(明)
    BlendMode::OVERLAY,         // オーバーレイ
    BlendMode::SOFTLIGHT,       // ソフトライト
    BlendMode::HARDLIGHT,       // ハードライト
    BlendMode::VIVIDLIGHT,      // ビビッドライト
    BlendMode::LINEARLIGHT,     // リニアライト
    BlendMode::PINLIGHT,        // ピンライト
    BlendMode::ABSOLUTENESS,    // 差の絶対値
    BlendMode::EXCLUSION,       // 除外
    BlendMode::SUBTRACTION,     // 減算
    BlendMode::DIVISION,        // 除算
    BlendMode::ADDITION,        // 加算
};

// プリント出力リスト
static const std::vector<LPCSTR> stringList
{
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
    InitView();

    // シェーダ関連初期化
    if (FAILED(InitShader())) return E_FAIL;

    // バッファ関連初期化
    if (FAILED(InitBuffer())) return E_FAIL;

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
VOID InitView()
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
    // ブレンドリスト初期化
    for (size_t i = 0; i < blendList.size(); i++)
    {
        g_blendMode[i].reset(new BlendMode());
        if (FAILED(g_blendMode[i]->Init(blendList[i]))) return E_FAIL;
    }

    // 初期ブレンドをプリント
    printf("0: %s\n", stringList[0]);

    // 使用するプリミティブタイプを設定
    g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return S_OK;
}

// 更新
VOID OnUpdate()
{
    size_t listNum = g_blendNum % blendList.size();

    // 表示する頂点バッファを選択
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_context->IASetVertexBuffers(0, 1, g_blendMode[listNum]->GetAddressVertexBuffer(), &stride, &offset);

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
}

// 描画
VOID OnRender()
{
    // レンダーターゲットビューを指定した色でクリア
    FLOAT clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };       // ブルー
    g_context->ClearRenderTargetView(g_renderTargetView, clearColor);

    // シェーダオブジェクトをセット
    g_context->VSSetShader(g_vertexShader, nullptr, 0);
    g_context->PSSetShader(g_pixelShader, nullptr, 0);

    // 頂点バッファをバックバッファに描画
    g_context->Draw(3, 0);

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
