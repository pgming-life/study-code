#include <windows.h>
#include <wrl.h>
#include <memory>
#include <vector>

// DirectX11�̃R�[�h�Z�b�g
#include <d3d11.h>
#include <d3dx11.h>
#include <directxmath.h>

// ���C�u����
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

#define WINDOW_CLASS    L"�A���t�@�u�����f�B���O+�� [L:���EJ:��]"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

#define SAFE_RELEASE(x) if(x) x->Release();

// �v���g�^�C�v�錾
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // �E�B���h�E�v���V�[�W��
HRESULT OnInit(HWND hWnd);          // ������
HRESULT InitDevice(HWND hWnd);      // �f�o�C�X�֘A������
HRESULT InitView();                 // �r���[�֘A������
HRESULT InitShader();               // �V�F�[�_�֘A������
HRESULT InitBuffer();               // �o�b�t�@�֘A������
HRESULT InitTexture();              // �e�N�X�`���֘A������
VOID InitMatrix();                  // �}�g���b�N�X�֘A������
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
ID3D11Buffer* g_vertexBuffer;                       // ���_�o�b�t�@
ID3D11Buffer* g_indexBuffer;                        // �C���f�b�N�X�o�b�t�@
ID3D11Buffer* g_constantBuffer;                     // �萔�o�b�t�@
ID3D11SamplerState* g_sampler;                      // �e�N�X�`���T���v��
ID3D11ShaderResourceView* g_shaderResourceView1;    // �e�N�X�`�����\�[�X1
ID3D11ShaderResourceView* g_shaderResourceView2;    // �e�N�X�`�����\�[�X2

// ���_�\����
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
};

// �萔�\����
struct ConstantBuffer
{
    XMMATRIX m_world;
};

// �}�g���b�N�X
XMMATRIX g_world1;          // ���[���h�s��̕����x�N�g��1
XMMATRIX g_world2;          // ���[���h�s��̕����x�N�g��2

// Y����]�ϐ�
static FLOAT y = 0;

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
    enum Mode
    {
        NONE,           // �u�����h�Ȃ�
        NORMAL,         // �ʏ�(�A���t�@�u�����h)
        ADDITION,       // ���Z
        ADDITIONALPHA,  // ���Z(���߂���)
        SUBTRACTION,    // ���Z
        SCREEN,         // �X�N���[��
        BLEND_NUMMAX,
    };

    // �R���X�g���N�^
    BlendMode() : m_blendState(nullptr)
    {
    }

    // �f�X�g���N�^
    ~BlendMode()
    {
        SAFE_RELEASE(m_blendState);
    }

    // �u�����h���[�h������
    HRESULT Init(Mode mode)
    {
        // �u�����h�̐ݒ�
        D3D11_BLEND_DESC blendDesc;
        ZeroMemory(&blendDesc, sizeof(blendDesc));
        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;
        blendDesc.RenderTarget[0].BlendEnable = TRUE;

        // Dest=��{�F(�����C���[), Src=�����F(�ヌ�C���[)
        switch (mode)
        {
        case NORMAL:        // �ʏ�(�A���t�@�u�����h)
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            // Src * SrcA + Dest * (1 - SrcA)
            break;

        case ADDITION:      // ���Z
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            // Src * 1 + Dest * 1
            break;

        case ADDITIONALPHA: // ���Z(���߂���)
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            // Src * SrcA + Dest * 1
            break;

        case SUBTRACTION:   // ���Z
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_COLOR;
            // Src * 0 + Dest * (1 - Src)
            break;

        case SCREEN:        // �X�N���[��
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            // Src * (1 - Dest) + Dest * 1
            break;

        case NONE:          // �Ȃ�
        default:            // �ی�����
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
            // Src * 1 + Dest * 0
        }

        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        // �u�����h�X�e�[�g���쐬
        if (FAILED(g_device->CreateBlendState(&blendDesc, &m_blendState)))
        {
            MessageBox(NULL, L"�u�����h�X�e�[�g���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }

        return S_OK;
    }

    // �u�����h�X�e�[�g�擾
    inline ID3D11BlendState* GetBlendState()
    {
        return m_blendState;
    }

private:
    ID3D11BlendState* m_blendState;
};
std::unique_ptr<BlendMode> g_blendMode[BlendMode::BLEND_NUMMAX];

// �g�p����u�����h�̃��X�g
static const std::vector<BlendMode::Mode> blendList
{
    BlendMode::NONE,            // �u�����h�Ȃ�
    BlendMode::NORMAL,          // �ʏ�(�A���t�@�u�����h)
    BlendMode::ADDITION,        // ���Z
    BlendMode::ADDITIONALPHA,   // ���Z(���߂���)
    BlendMode::SUBTRACTION,     // ���Z
    BlendMode::SCREEN,          // �X�N���[��
};

// �v�����g�o�̓��X�g
static const std::vector<LPCSTR> stringList
{
    "�u�����h�Ȃ�",
    "�ʏ�(�A���t�@�u�����h)",
    "���Z",
    "���Z(���߂���)",
    "���Z",
    "�X�N���[��",
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
    if (FAILED(InitView())) return E_FAIL;

    // �V�F�[�_�֘A������
    if (FAILED(InitShader())) return E_FAIL;

    // �o�b�t�@�֘A������
    if (FAILED(InitBuffer())) return E_FAIL;

    // �e�N�X�`���֘A������
    if (FAILED(InitTexture())) return E_FAIL;

    // �}�g���b�N�X�֘A������
    InitMatrix();

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
HRESULT InitView()
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

    // �u�����h���X�g������
    for (size_t i = 0; i < blendList.size(); i++)
    {
        g_blendMode[i].reset(new BlendMode());
        if (FAILED(g_blendMode[i]->Init(blendList[i]))) return E_FAIL;
    }

    // �����u�����h���v�����g
    printf("0: %s\n", stringList[0]);

    // ���X�^���C�U�̐ݒ�
    D3D11_RASTERIZER_DESC rasDesc;
    ZeroMemory(&rasDesc, sizeof(rasDesc));
    rasDesc.FillMode = D3D11_FILL_SOLID;        // �\���b�h
    rasDesc.CullMode = D3D11_CULL_NONE;         // �J�����O�Ȃ��F���\�`��

    // ���X�^���C�U�X�e�[�g���쐬
    ID3D11RasterizerState* rasState;
    if (FAILED(g_device->CreateRasterizerState(&rasDesc, &rasState)))
    {
        MessageBox(NULL, L"���X�^���C�U�X�e�[�g���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // ���X�^���C�U���Z�b�g
    g_context->RSSetState(rasState);

    SAFE_RELEASE(rasState);

    return S_OK;
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
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
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
    // �l�p�`�̃W�I���g�����`
    // ��ʑS�̕\��
    Vertex vertices[] =
    {
        {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
    };

    // �o�b�t�@���쐬
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));

    // ���_�o�b�t�@�̐ݒ�
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;                 // �f�t�H���g�A�N�Z�X
    bufferDesc.ByteWidth = sizeof(Vertex) * 4;              // �T�C�Y��Vertex�\���́~4
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;        // ���_�o�b�t�@���g�p
    bufferDesc.CPUAccessFlags = 0;                          // CPU�̃o�b�t�@�ւ̃A�N�Z�X����

    // ���\�[�X�̐ݒ�
    D3D11_SUBRESOURCE_DATA initData;
    ZeroMemory(&initData, sizeof(initData));
    initData.pSysMem = vertices;

    // ���_�o�b�t�@���쐬
    if (FAILED(g_device->CreateBuffer(&bufferDesc, &initData, &g_vertexBuffer)))
    {
        MessageBox(NULL, L"���_�o�b�t�@1���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �\�����钸�_�o�b�t�@��I��
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_context->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, &offset);

    // �l�p�`�̃C���f�b�N�X���`
    WORD index[] =
    {
        0, 1, 2,
        2, 1, 3
    };

    // �C���f�b�N�X���̒ǉ�
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;                 // �f�t�H���g�A�N�Z�X
    bufferDesc.ByteWidth = sizeof(WORD) * 6;                // �T�C�Y�̓C���f�b�N�X�̐� 6
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;         // �C���f�b�N�X�o�b�t�@���g�p
    bufferDesc.CPUAccessFlags = 0;                          // CPU�̃o�b�t�@�ւ̃A�N�Z�X����
    initData.pSysMem = index;

    // �C���f�b�N�X�o�b�t�@���쐬
    if (FAILED(g_device->CreateBuffer(&bufferDesc, &initData, &g_indexBuffer)))
    {
        MessageBox(NULL, L"�C���f�b�N�X�o�b�t�@���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �\������C���f�b�N�X�o�b�t�@��I��
    g_context->IASetIndexBuffer(g_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // �g�p����v���~�e�B�u�^�C�v��ݒ�
    g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // �萔���̒ǉ�
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(ConstantBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    // �萔�o�b�t�@���쐬
    if (FAILED(g_device->CreateBuffer(&bufferDesc, nullptr, &g_constantBuffer)))
    {
        MessageBox(NULL, L"�萔�o�b�t�@���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    return S_OK;
}

// �e�N�X�`���֘A������
HRESULT InitTexture()
{
    // �T���v���̐ݒ�
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 1.0f;		// �z���C�g
    samplerDesc.BorderColor[1] = 1.0f;		// ..
    samplerDesc.BorderColor[2] = 1.0f;		// ..
    samplerDesc.BorderColor[3] = 1.0f;		// ..
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // �T���v���X�e�[�g���쐬
    if (FAILED(g_device->CreateSamplerState(&samplerDesc, &g_sampler)))
    {
        MessageBox(NULL, L"�T���v���X�e�[�g���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �e�N�X�`���̓ǂݍ���
    if (FAILED(D3DX11CreateShaderResourceViewFromFile(g_device, L"sky.jpg", NULL, NULL, &g_shaderResourceView1, NULL)))
    {
        MessageBox(NULL, L"�e�N�X�`��1��ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }
    if (FAILED(D3DX11CreateShaderResourceViewFromFile(g_device, L"color_ink.png", NULL, NULL, &g_shaderResourceView2, NULL)))
    {
        MessageBox(NULL, L"�e�N�X�`��2��ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    return S_OK;
}

// �}�g���b�N�X�֘A������
VOID InitMatrix()
{
    // ���[���h�}�g���b�N�X�̏�����
    g_world1 = XMMatrixIdentity();       // �����Ȃ��őS�Ă̎���0.0f(�ړ��Ȃ�)
    g_world2 = XMMatrixIdentity();       // �����Ȃ��őS�Ă̎���0.0f(�ړ��Ȃ�)
}

// �X�V
VOID OnUpdate()
{
    // �p�����[�^�̎󂯓n��
    XMMATRIX scale = XMMatrixScaling(0.5f, 0.5f, 0.0f);     // X��Y����1/2�ɏk��
    XMMATRIX translate = XMMatrixRotationY(y += 0.0005);    // Y��0.0005����]
    g_world2 = scale * translate;                           // �e�}�g���b�N�X�̐�
}

// �`��
VOID OnRender()
{
    // �����_�[�^�[�Q�b�g�r���[���w�肵���F�ŃN���A
    FLOAT clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };       // �u���[
    g_context->ClearRenderTargetView(g_renderTargetView, clearColor);

    // GPU�o�b�t�@���Z�b�g
    g_context->VSSetShader(g_vertexShader, nullptr, 0);
    g_context->PSSetShader(g_pixelShader, nullptr, 0);
    g_context->VSSetConstantBuffers(0, 1, &g_constantBuffer);



    // �u�����h�Ȃ����Z�b�g
    FLOAT blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
    g_context->OMSetBlendState(g_blendMode[BlendMode::NONE]->GetBlendState(), blendFactor, 0xffffffff);

    // �e�N�X�`��1���V�F�[�_�ɓo�^
    g_context->PSSetSamplers(0, 1, &g_sampler);
    g_context->PSSetShaderResources(0, 1, &g_shaderResourceView1);

    // �ϐ�1���
    ConstantBuffer cBuffer;
    cBuffer.m_world = XMMatrixTranspose(g_world1);

    // GPU(�V�F�[�_��)�֓]������
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // �C���f�b�N�X�o�b�t�@���o�b�N�o�b�t�@�ɕ`��
    g_context->DrawIndexed(6, 0, 0);




    size_t listNum = g_blendNum % blendList.size();

    // �u�����h���Z�b�g
    g_context->OMSetBlendState(g_blendMode[listNum]->GetBlendState(), blendFactor, 0xffffffff);

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

    // �e�N�X�`��2���V�F�[�_�ɓo�^
    g_context->PSSetShaderResources(0, 1, &g_shaderResourceView2);

    // �ϐ�2���
    cBuffer.m_world = XMMatrixTranspose(g_world2);

    // GPU(�V�F�[�_��)�֓]������
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // �C���f�b�N�X�o�b�t�@���o�b�N�o�b�t�@�ɕ`��
    g_context->DrawIndexed(6, 0, 0);



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
