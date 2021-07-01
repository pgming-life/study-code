#include <windows.h>
#include <wrl.h>
#include <memory>
#include <vector>

// DirectX11,10�̃R�[�h�Z�b�g
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <directxmath.h>

// ���C�u����
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

#define WINDOW_CLASS    L"�u�����h���[�h�e�X�g [L:���EJ:��]"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

#define SAFE_RELEASE(x) if(x) x->Release();

// �v���g�^�C�v�錾
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // �E�B���h�E�v���V�[�W��
HRESULT OnInit(HWND hWnd);          // ������
HRESULT InitDevice(HWND hWnd);      // �f�o�C�X�֘A������
VOID InitView();                    // �r���[�֘A������
HRESULT InitShader();               // �V�F�[�_�֘A������
HRESULT InitBuffer();               // �o�b�t�@�֘A������
VOID OnUpdate();                    // �X�V
VOID OnRender();                    // �`��
VOID OnDestroy();                   // ���������
VOID OnKeyDown(UINT8 key);          // �L�[����

// using�f�B���N�e�B�u
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// �p�C�v���C���I�u�W�F�N�g
ID3D11Device* g_device;                             // �f�o�C�X�C���^�[�t�F�C�X
ID3D11DeviceContext* g_context;                     // �R���e�L�X�g
IDXGISwapChain* g_swapChain;                        // �X���b�v�`�F�C���C���^�[�t�F�C�X
ID3D11RenderTargetView* g_renderTargetView;         // �����_�[�^�[�Q�b�g�r���[
ID3D11InputLayout* g_layout;                        // �C���v�b�g���C�A�E�g
ID3D11VertexShader* g_vertexShader;                 // ���_�V�F�[�_
ID3D11PixelShader* g_pixelShader;                   // �s�N�Z���V�F�[�_

// ���_�\����
struct Vertex
{
    XMFLOAT3 position;
    D3DXCOLOR color;
};

// �\���̈�̐��@(�A�X�y�N�g��)
static const FLOAT g_aspectRatio = static_cast<FLOAT>(WINDOW_WIDTH) / static_cast<FLOAT>(WINDOW_HEIGHT);

// �R���\�[���N���X
class Console
{
public:
    Console()
    {
        // �R���\�[���N��
        AllocConsole();

        // �V�K�t�@�C�����I�[�v�����X�g���[���ƕR�Â�
        freopen_s(&m_fp, "CONOUT$", "w", stdout);
        freopen_s(&m_fp, "CONIN$", "r", stdin);
    }

    ~Console()
    {
        // �t�@�C���N���[�Y
        fclose(m_fp);
    }

private:
    FILE* m_fp;
};

// �u�����h���[�h�N���X
class BlendMode
{
public:
    // �u�����h���[�h���
    enum Mode {
        NORMAL,             // �ʏ�(�A���t�@�u�����h)
        DARK, 			    // ��r(��)
        MULTIPLE,		    // ��Z
        BURNCOLOR,		    // �Ă����݃J���[
        BURNLINEAR,		    // �Ă����݃��j�A
        COLORDARK,          // �J���[��r(��)
        LIGHT,			    // ��r(��)
        SCREEN,			    // �X�N���[��
        DODGECOLOR,		    // �����Ă��J���[
        DODGELINEAR,	    // �����Ă����j�A(���Z)
        COLORLIGHT,         // �J���[��r(��)
        OVERLAY,		    // �I�[�o�[���C
        SOFTLIGHT,          // �\�t�g���C�g
        HARDLIGHT,		    // �n�[�h���C�g
        VIVIDLIGHT,		    // �r�r�b�h���C�g
        LINEARLIGHT,	    // ���j�A���C�g
        PINLIGHT,		    // �s�����C�g
        HARDMIX,		    // �n�[�h�~�b�N�X
        ABSOLUTENESS,	    // ���̐�Βl
        EXCLUSION,		    // ���O
        SUBTRACTION,	    // ���Z
        DIVISION,		    // ���Z
        ADDITION,		    // ���Z
        MONOCHROME,         // ���m�N��
        BRIGHTEXTRACT,      // �u���C�g�G�N�X�g���N�g
        COLORKEYALPHA,      // �J���[�L�[�A���t�@
        COLORTONE,          // �J���[�g�[��
        GRAYSCALE,          // �O���[�X�P�[��
        INVERT,             // ���]�F
        MOSAIC,             // ���U�C�N
        SEPIA,              // �Z�s�A
        BLEND_NUMMAX,
    };

    // �R���X�g���N�^
    BlendMode() : m_vertexBuffer(nullptr)
    {
    }

    // �f�X�g���N�^
    ~BlendMode()
    {
        SAFE_RELEASE(m_vertexBuffer);
    }

    // �u�����h���[�h������
    HRESULT Init(Mode mode)
    {
        // �Ԃ��ۂ�(��{�F)
        D3DXCOLOR dest =
        {
            244 / 255.f,    // Rt=244
            24 / 255.f,     // Gt=24
            24 / 255.f,     // Bt=24
            1.0f,           // At=255
        };

        // �΂��ۂ�(�����F)
        D3DXCOLOR src =
        {
            25 / 255.f,     // Rb=25
            220 / 255.f,    // Gb=220
            140 / 255.f,    // Bb=140
            1.0f,           // Ab=255
        };

        // ��������
        D3DXCOLOR result;   // Ro, Go, Bo, Ao

        switch (mode)
        {
        case NORMAL:        // �ʏ�(�A���t�@�u�����h)
            result = { src.r, src.g, src.b, src.a };
            // Ro = Rt = 25
            // Go = Gt = 220
            // Bo = Bt = 140
            // �I�[�o�[���b�v�����͏�ɏ�̃��C���[�ɂȂ�B
            // ���߂͂Ȃ��B�V�F�[�_�̕��ł͎����\��B
            break;

        case DARK:          // ��r(��)
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
            // ���C���[�ԂŒႢ�l���̗p�B
            break;

        case MULTIPLE:      // ��Z
            result =
            {
                dest.r * src.r,
                dest.g * src.g,
                dest.b * src.b,
                src.a,
            };
            // Ro = Rt�~Rb��255 = 25 * 244 / 255 = 23.92 �� 24
            // Go = Gt�~Gb��255 = 24 * 220 / 255 = 20.71 �� 21
            // Bo = Bt�~Bb��255 = 24 * 140 / 255 = 13.18 �� 13
            // �㉺�̃��C���[�̒l����Z����255�Ŋ���B
            break;

        case BURNCOLOR:     // �Ă����݃J���[
            result =
            {
                1.0f - (1.0f - dest.r) / src.r,
                1.0f - (1.0f - dest.g) / src.g,
                1.0f - (1.0f - dest.b) / src.b,
                src.a,
            };
            // Overwrap = 255 - (255 - Bottom) * 255 / Top
            // Ro = 255 - (255 - 244) * 255 / 25 = 142.8 �� 143
            // Go = 255 - (255 - 24) * 255 / 220 = -12.75 �� 0
            // Bo = 255 - (255 - 24) * 255 / 140 = -165.75 �� 0
            // Top��0�C�������́A���ʂ��}�C�i�X�̏ꍇ�́A0�C�ƂȂ�B
            // ��L�ł�0.0f���������͎̂�����0.0f�ɂȂ�B
            break;

        case BURNLINEAR:    // �Ă����݃��j�A
            result =
            {
                dest.r + src.r <= 1.0f ? 0.0f : dest.r + src.r - 1.0f,
                dest.g + src.g <= 1.0f ? 0.0f : dest.g + src.g - 1.0f,
                dest.b + src.b <= 1.0f ? 0.0f : dest.b + src.b - 1.0f,
                src.a,
            };
            // �񖇂̃��C���[�̊eRGB�̘a���C
            // 255�ȉ��F0
            // 255�ȏ�F�񖇂̒l�̘a - 255
            break;

        case COLORDARK:     // �J���[��r(��)
            if (src.r + src.g + src.b > dest.r + dest.g + dest.b)
                result = { dest.r, dest.g, dest.b, src.a };
            else if (src.r + src.g + src.b <= dest.r + dest.g + dest.b)
                result = { src.r, src.g, src.b, src.a };
            // 25 + 220 + 140 = 385 > 244 + 24 + 24 = 292
            // ��̃��C���[��RGB�̘a���r���āC�Ⴂ���̒l�������B
            break;

        case LIGHT:         // ��r(��)
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
            // ���C���[�Ԃō����l���̗p�B
            break;

        case SCREEN:        // �X�N���[��
            result =
            {
                dest.r + src.r - dest.r * src.r,
                dest.g + src.g - dest.g * src.g,
                dest.b + src.b - dest.b * src.b,
                src.a,
            };
            // Overwrap = Bottom + Top - Bottom * Top / 255
            // Ro = 244 + 25 - 244 * 25 / 255 = 245.08 �� 245
            // Go = 24 + 220 - 24 * 220 / 255 = 223.29 �� 223
            // Bo = 24 + 140 - 24 * 140 / 255 = 150.82 �� 151
            break;

        case DODGECOLOR:    // �����Ă��J���[
            result =
            {
                dest.r / (1.0f - src.r),
                dest.g / (1.0f - src.g),
                dest.b / (1.0f - src.b),
                src.a,
            };
            // Overwrap = Bottom * 255 / (255 - Top)
            // Ro = 244 * 255 / (255 - 25) = 270.5 �� 255
            // Go = 24 * 255 / (255 - 220) = 174.86 �� 175
            // Bo = 24 * 255 / (255 - 140) = 53.22 �� 53
            // ���ʂ�255�ȏ�̏ꍇ�́A255�ƂȂ�B
            // ��L�ł�1.0f�𒴂�����͎̂�����1.0f�ɂȂ�B
            break;

        case DODGELINEAR:   // �����Ă����j�A(���Z)
            result =
            {
                src.r + dest.r,
                src.g + dest.g,
                src.b + dest.b,
                src.a,
            };
            // Ro = 25 + 244 = 269 �� 255
            // Go = 220 + 24 = 244
            // Bo = 140 + 24 = 164
            // �e�F��P�����Z���A255�ȏ��255�ƂȂ�B
            // ��L�ł�1.0f�𒴂�����͎̂�����1.0f�ɂȂ�B
            // ���Z�����Ɠ����H
            break;

        case COLORLIGHT:    // �J���[��r(��)
            if (src.r + src.g + src.b > dest.r + dest.g + dest.b)
                result = { src.r, src.g, src.b, src.a };
            else if (src.r + src.g + src.b <= dest.r + dest.g + dest.b)
                result = { dest.r, dest.g, dest.b, src.a };
            // 25 + 220 + 140 = 385 > 244 + 24 + 24 = 292
            // ��̃��C���[��RGB�̘a���r���āC�������̒l�������B
            break;

        case OVERLAY:       // �I�[�o�[���C
            if (dest.r < 0.5f) result.r = dest.r * src.r * 2;
            else result.r = 2 * (dest.r + src.r - dest.r * src.r) - 1.0f;
            if (dest.g < 0.5f) result.g = dest.g * src.g * 2;
            else result.g = 2 * (dest.g + src.g - dest.g * src.g) - 1.0f;
            if (dest.b < 0.5f) result.b = dest.b * src.b * 2;
            else result.b = 2 * (dest.b + src.b - dest.b * src.b) - 1.0f;
            result.a = src.a;
            // ���̐F�̒l�ŏ������ς��C
            // Bottom < 128 �̏ꍇ
            // Bottom * Top * 2 / 255
            // Bottom >= 128 �̏ꍇ
            // 2 * (Bottom + Top - Bottom * Top / 255) - 255
            // ���������āA
            // Ro = 2 * (244 + 25 - 244 * 25 / 255) / 255 - 255 = 235.16 �� 235
            // Go = 24 * 220 * 2 / 255 = 41.41 �� 41
            // Bo = 24 * 140 * 2 / 255 = 26.35 �� 26
            break;

        case SOFTLIGHT:     // �\�t�g���C�g
            if (src.r < 0.5f) result.r = 2 * dest.r * src.r + powf(dest.r, 2) * (1.0f - 2 * src.r);
            else result.r = 2 * dest.r * (1.0f - src.r) + sqrtf(dest.r) * (2 * src.r - 1.0f);
            if (src.g < 0.5f) result.g = 2 * dest.g * src.g + powf(dest.g, 2) * (1.0f - 2 * src.g);
            else result.g = 2 * dest.g * (1.0f - src.g) + sqrtf(dest.g) * (2 * src.g - 1.0f);
            if (src.b < 0.5f) result.b = 2 * dest.b * src.b + powf(dest.b, 2) * (1.0f - 2 * src.b);
            else result.b = 2 * dest.b * (1.0f - src.b) + sqrtf(dest.b) * (2 * src.b - 1.0f);
            result.a = src.a;
            break;

        case HARDLIGHT:     // �n�[�h���C�g
            if (dest.r > 0.5f) result.r = dest.r * src.r * 2;
            else result.r = 2 * (dest.r + src.r - dest.r * src.r) - 1.0f;
            if (dest.g > 0.5f) result.g = dest.g * src.g * 2;
            else result.g = 2 * (dest.g + src.g - dest.g * src.g) - 1.0f;
            if (dest.b > 0.5f) result.b = dest.b * src.b * 2;
            else result.b = 2 * (dest.b + src.b - dest.b * src.b) - 1.0f;
            result.a = src.a;
            // ���̐F�̒l�ŏ������ς��C
            // Bottom > 128 �̏ꍇ
            // Bottom * Top * 2 / 255
            // Bottom <= 128 �̏ꍇ
            // 2 * (Bottom + Top - Bottom * Top / 255) - 255
            // ����������
            // Ro = 244 * 25 * 2 / 255 = 47.84 �� 48
            // Go = 2 * (24 + 220 - 24 * 220 / 255) / 255 - 255 = 191.59 �� 192
            // Bo = 2 * (24 + 140 - 24 * 140 / 255) / 255 - 255 = 46.65 �� 47
            // �I�[�o�[���C�̏����Ƌt�]���Ă���B
            // �܂��ABo�����ꂾ�������_�ȉ��؂艺��(Photoshop�ł�46�j
            break;

        case VIVIDLIGHT:    // �r�r�b�h���C�g
            if (src.r < 0.5f) result.r = 1.0f - (1.0f - dest.r) / (2 * src.r);
            else result.r = dest.r / (1.0f - 2 * (src.r - 0.5f));
            if (src.g < 0.5f) result.g = 1.0f - (1.0f - dest.g) / (2 * src.g);
            else result.g = dest.g / (1.0f - 2 * (src.g - 0.5f));
            if (src.b < 0.5f) result.b = 1.0f - (1.0f - dest.b) / (2 * src.b);
            else result.b = dest.b / (1.0f - 2 * (src.b - 0.5f));
            result.a = src.a;
            break;

        case LINEARLIGHT:   // ���j�A���C�g
            if (src.r < 0.5f) result.r = dest.r + 2 * src.r - 1.0f;
            else result.r = dest.r + 2 * (src.r - 0.5f);
            if (src.g < 0.5f) result.g = dest.g + 2 * src.g - 1.0f;
            else result.g = dest.g + 2 * (src.g - 0.5f);
            if (src.b < 0.5f) result.b = dest.b + 2 * src.b - 1.0f;
            else result.b = dest.b + 2 * (src.b - 0.5f);
            result.a = src.a;
            break;

        case PINLIGHT:      // �s�����C�g
            if (src.r < 0.5f) result.r = min(dest.r, 2 * src.r);
            else result.r = max(dest.r, 2 * (src.r - 0.5f));
            if (src.g < 0.5f) result.g = min(dest.g, 2 * src.g);
            else result.g = max(dest.g, 2 * (src.g - 0.5f));
            if (src.b < 0.5f) result.b = min(dest.b, 2 * src.b);
            else result.b = max(dest.b, 2 * (src.b - 0.5f));
            result.a = src.a;
            break;

        case HARDMIX:       // �n�[�h�~�b�N�X
            break;

        case ABSOLUTENESS:  // ���̐�Βl
            result =
            {
                fabsf(src.r - dest.r),
                fabsf(src.g - dest.g),
                fabsf(src.b - dest.b),
                src.a,
            };
            // Ro =�b25-244�b= 219
            // Go = �b220 - 24�b = 196
            // Bo = �b140 - 24�b = 116
            // �����̐�Βl�B
            break;

        case EXCLUSION:     // ���O
            result =
            {
                dest.r + src.r - 2 * dest.r * src.r,
                dest.g + src.g - 2 * dest.g * src.g,
                dest.b + src.b - 2 * dest.b * src.b,
                src.a,
            };
            break;

        case SUBTRACTION:   // ���Z
            result =
            {
                dest.r - src.r,
                dest.g - src.g,
                dest.b - src.b,
                src.a,
            };
            break;

        case DIVISION:      // ���Z
            result =
            {
                dest.r / src.r,
                dest.g / src.g,
                dest.b / src.b,
                src.a,
            };
            break;

        case ADDITION:      // ���Z
            result =
            {
                dest.r + src.r,
                dest.g + src.g,
                dest.b + src.b,
                src.a,
            };
            // Ro = 25 + 244 = 269 �� 255
            // Go = 220 + 24 = 196
            // Bo = 140 + 24 = 164
            // �㉺�̃��C���[�̉��Z�B
            // ��L�ł�1.0f�𒴂�����͎̂�����1.0f�ɂȂ�B
            break;
        default:            // �ی�����
            result = { 1.0f, 1.0f, 1.0f, 1.0f };      // ��
        }

        // �O�p�`�̃W�I���g�����`
        Vertex vertices[] =
        {
            {{ 0.0f,   0.25f * g_aspectRatio, 0.0f}, result},	// ��(��������)
            {{ 0.25f, -0.25f * g_aspectRatio, 0.0f}, src},		// �E(�΂��ۂ�)
            {{-0.25f, -0.25f * g_aspectRatio, 0.0f}, dest},		// ��(�Ԃ��ۂ�)
        };

        // �o�b�t�@���쐬
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));

        // ���_�o�b�t�@�̐ݒ�
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;                 // CPU�����GPU�ɂ�鏑�����݃A�N�Z�X
        bufferDesc.ByteWidth = sizeof(Vertex) * 3;              // �T�C�Y��Vertex�\���́~3
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;        // ���_�o�b�t�@���g�p
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;     // CPU���o�b�t�@�ɏ������ނ��Ƃ�����

        // ���_�o�b�t�@���쐬
        if (FAILED(g_device->CreateBuffer(&bufferDesc, nullptr, &m_vertexBuffer)))
        {
            MessageBox(NULL, L"���_�o�b�t�@���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }

        // ���_�o�b�t�@�ɒ��_�f�[�^���R�s�[
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        g_context->Map(m_vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedResource);   // �o�b�t�@�̃}�b�s���O
        memcpy(mappedResource.pData, vertices, sizeof(vertices));                               // �f�[�^���R�s�[
        g_context->Unmap(m_vertexBuffer, NULL);

        return S_OK;
    }

    // ���_�o�b�t�@�A�h���X�̃Q�b�^�[
    inline ID3D11Buffer** GetAddressVertexBuffer()
    {
        return &m_vertexBuffer;
    }

private:
    ID3D11Buffer* m_vertexBuffer;       // ���_�o�b�t�@
};
std::unique_ptr<BlendMode> g_blendMode[BlendMode::BLEND_NUMMAX];

// �g�p����u�����h�̃��X�g
static const std::vector<BlendMode::Mode> blendList
{
    BlendMode::NORMAL,          // �ʏ�(�A���t�@�u�����h)
    BlendMode::DARK,            // ��r(��)
    BlendMode::MULTIPLE,        // ��Z
    BlendMode::BURNCOLOR,       // �Ă����݃J���[
    BlendMode::BURNLINEAR,      // �Ă����݃��j�A
    BlendMode::COLORDARK,       // �J���[��r(��)
    BlendMode::LIGHT,           // ��r(��)
    BlendMode::SCREEN,          // �X�N���[��
    BlendMode::DODGECOLOR,      // �����Ă��J���[
    BlendMode::DODGELINEAR,     // �����Ă����j�A(���Z)
    BlendMode::COLORLIGHT,      // �J���[��r(��)
    BlendMode::OVERLAY,         // �I�[�o�[���C
    BlendMode::SOFTLIGHT,       // �\�t�g���C�g
    BlendMode::HARDLIGHT,       // �n�[�h���C�g
    BlendMode::VIVIDLIGHT,      // �r�r�b�h���C�g
    BlendMode::LINEARLIGHT,     // ���j�A���C�g
    BlendMode::PINLIGHT,        // �s�����C�g
    BlendMode::ABSOLUTENESS,    // ���̐�Βl
    BlendMode::EXCLUSION,       // ���O
    BlendMode::SUBTRACTION,     // ���Z
    BlendMode::DIVISION,        // ���Z
    BlendMode::ADDITION,        // ���Z
};

// �v�����g�o�̓��X�g
static const std::vector<LPCSTR> stringList
{
    "�ʏ�(�A���t�@�u�����h)",
    "��r(��)",
    "��Z",
    "�Ă����݃J���[",
    "�Ă����݃��j�A",
    "�J���[��r(��)",
    "��r(��)",
    "�X�N���[��",
    "�����Ă��J���[",
    "�����Ă����j�A(���Z)",
    "�J���[��r(��)",
    "�I�[�o�[���C",
    "�\�t�g���C�g",
    "�n�[�h���C�g",
    "�r�r�b�h���C�g",
    "���j�A���C�g",
    "�s�����C�g",
    "�n�[�h�~�b�N�X",
    "���̐�Βl",
    "���O",
    "���Z",
    "���Z",
    "���Z",
    "���m�N��",
    "�u���C�g�G�N�X�g���N�g",
    "�J���[�L�[�A���t�@",
    "�J���[�g�[��",
    "�O���[�X�P�[��",
    "���]�F",
    "���U�C�N",
    "�Z�s�A",
};

// �u�����h�ؑ֕ϐ�
static size_t g_blendNum = blendList.size();

// �R���\�[���o�̓t���O
static BOOL g_outputFlag = FALSE;

// ���C���֐�
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, INT nCmdShow)
{
    // �E�B���h�E�N���X��������
    WNDCLASSEX	windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = WINDOW_CLASS;
    RegisterClassEx(&windowClass);

    // �E�B���h�E�̃T�C�Y�����߂�
    RECT windowRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // �E�B���h�E�n���h�����쐬
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
    if (SUCCEEDED(OnInit(hWnd)))   // DirectX�̏�����
    {
        // �E�B���h�E�̕\��
        ShowWindow(hWnd, SW_SHOW);

        // ���C�����[�v
        while (msg.message != WM_QUIT)
        {
            // �L���[���̃��b�Z�[�W������
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                OnUpdate();     // �X�V
                OnRender();     // �`��
            }
        }
    }

    // WM_QUIT���b�Z�[�W�̕�����Windows�ɕԂ�
    return static_cast<char>(msg.wParam);
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    switch (nMsg)
    {
    case WM_KEYDOWN:    // �L�[������
        OnKeyDown(static_cast<UINT8>(wParam));
        return 0;

    case WM_DESTROY:    // �I����
        OnDestroy();    // ���������
        PostQuitMessage(0);
        return 0;
    }

    // switch�����������Ȃ��������b�Z�[�W������
    return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

// ������
HRESULT OnInit(HWND hWnd)
{
    // �R���\�[���E�B���h�E�𐶐�
    Console console;

    // �f�o�C�X�֘A������
    if (FAILED(InitDevice(hWnd))) return E_FAIL;

    // �r���[�֘A������
    InitView();

    // �V�F�[�_�֘A������
    if (FAILED(InitShader())) return E_FAIL;

    // �o�b�t�@�֘A������
    if (FAILED(InitBuffer())) return E_FAIL;

    return S_OK;
}

// �f�o�C�X�֘A������
HRESULT InitDevice(HWND hWnd)
{
    // �h���C�o�[��ʂ��`
    std::vector<D3D_DRIVER_TYPE> driverTypes
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
        D3D_DRIVER_TYPE_SOFTWARE,
    };

    // �X���b�v�`�F�C���̍쐬
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

    // �h���C�o�[��ʂ��ォ�猟�؂��I��
    // �I�������f�o�C�X��p���ĕ`�悷��
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
        MessageBox(NULL, L"DirectX11�ɑΉ����Ă��Ȃ��f�o�C�X�ł��B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    return S_OK;
}

// �r���[�֘A������
VOID InitView()
{
    // �\���̈���쐬
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(viewport));
    viewport.Width = WINDOW_WIDTH;
    viewport.Height = WINDOW_HEIGHT;
    viewport.MinDepth = D3D11_MIN_DEPTH;    // 0.0f
    viewport.MaxDepth = D3D11_MAX_DEPTH;    // 1.0f
    g_context->RSSetViewports(1, &viewport);

    // �o�b�N�o�b�t�@���쐬
    ID3D11Texture2D* backBuffer;
    g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    g_device->CreateRenderTargetView(backBuffer, nullptr, &g_renderTargetView);
    SAFE_RELEASE(backBuffer);

    // �����_�[�^�[�Q�b�g���o�b�N�o�b�t�@�ɐݒ�
    g_context->OMSetRenderTargets(1, &g_renderTargetView, nullptr);
}

// �V�F�[�_�֘A������
HRESULT InitShader()
{
    ID3DBlob* vertexShader, * pixelShader;

    // �����̃V�F�[�_�����[�h���R���p�C��
    if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, "vsMain", "vs_4_0", NULL, NULL, NULL, &vertexShader, NULL, NULL)))
    {
        MessageBox(NULL, L"���_�V�F�[�_��ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }
    if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, "psMain", "ps_4_0", NULL, NULL, NULL, &pixelShader, NULL, NULL)))
    {
        MessageBox(NULL, L"�s�N�Z���V�F�[�_��ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �J�v�Z����
    g_device->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), NULL, &g_vertexShader);
    g_device->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), NULL, &g_pixelShader);

    // ���_�C���v�b�g���C�A�E�g���`
    D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    // �C���v�b�g���C�A�E�g�̃T�C�Y
    UINT numElements = sizeof(inputElementDescs) / sizeof(inputElementDescs[0]);

    // ���_�C���v�b�g���C�A�E�g���쐬
    if (FAILED(g_device->CreateInputLayout(inputElementDescs, numElements, vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), &g_layout)))
    {
        MessageBox(NULL, L"���_�C���v�b�g���C�A�E�g�̒�`���Ԉ���Ă��܂��B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    SAFE_RELEASE(vertexShader);
    SAFE_RELEASE(pixelShader);

    // ���_�C���v�b�g���C�A�E�g���Z�b�g
    g_context->IASetInputLayout(g_layout);

    return S_OK;
}

// �o�b�t�@�֘A������
HRESULT InitBuffer()
{
    // �u�����h���X�g������
    for (size_t i = 0; i < blendList.size(); i++)
    {
        g_blendMode[i].reset(new BlendMode());
        if (FAILED(g_blendMode[i]->Init(blendList[i]))) return E_FAIL;
    }

    // �����u�����h���v�����g
    printf("0: %s\n", stringList[0]);

    // �g�p����v���~�e�B�u�^�C�v��ݒ�
    g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return S_OK;
}

// �X�V
VOID OnUpdate()
{
    size_t listNum = g_blendNum % blendList.size();

    // �\�����钸�_�o�b�t�@��I��
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_context->IASetVertexBuffers(0, 1, g_blendMode[listNum]->GetAddressVertexBuffer(), &stride, &offset);

    // ���ɒ[�ɒB�����烊�Z�b�g
    if (g_blendNum == 0 || g_blendNum == blendList.size() * 2) g_blendNum = blendList.size();

    // �R���\�[���o��
    if (g_outputFlag)
    {
        // �v�����g
        printf("%d: %s\n", listNum, stringList[blendList[listNum]]);

        // �R���\�[���o�̓t���O/�I�t
        g_outputFlag = FALSE;
    }
}

// �`��
VOID OnRender()
{
    // �����_�[�^�[�Q�b�g�r���[���w�肵���F�ŃN���A
    FLOAT clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };       // �u���[
    g_context->ClearRenderTargetView(g_renderTargetView, clearColor);

    // �V�F�[�_�I�u�W�F�N�g���Z�b�g
    g_context->VSSetShader(g_vertexShader, nullptr, 0);
    g_context->PSSetShader(g_pixelShader, nullptr, 0);

    // ���_�o�b�t�@���o�b�N�o�b�t�@�ɕ`��
    g_context->Draw(3, 0);

    // �t���[�����ŏI�o��
    g_swapChain->Present(0, 0);     // �t���b�v
}

// ���������
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
    case 'L':       // ��
        g_blendNum++;
        g_outputFlag = TRUE;    // �R���\�[���o�̓t���O/�I��
        break;
    case 'J':       // ��
        g_blendNum--;
        g_outputFlag = TRUE;    // �R���\�[���o�̓t���O/�I��
        break;
    }
}
