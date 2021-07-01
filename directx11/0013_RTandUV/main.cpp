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

#define WINDOW_CLASS    L"�����_�[�^�[�Q�b�g��UV�l�ϊ�"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

#define SAFE_RELEASE(x) if(x) x->Release();

// �u�����h���[�h���
enum BlendMode
{
    NONE,           // �u�����h�Ȃ�
    MULTIPLE,       // ��Z����
    BLEND_NUMMAX,
};

// �V�F�[�_�f�[�^
struct ShaderData
{
    BlendMode mode;
    LPCSTR vsEntry;
    LPCSTR psEntry;
};

// �V�F�[�_�f�[�^���������X�g
static const ShaderData initShaderData[BLEND_NUMMAX] =
{
    {NONE,      "vsMain",   "psMain"},
    {MULTIPLE,  "vsBlend",  "psMultiple"},
};

// �摜�I�u�W�F�N�g
enum Object
{
    SKY,
    DUST,
    OBJ_NUMMAX,
};

// �摜�t�@�C��
static const LPCWSTR file[OBJ_NUMMAX] =
{
    L"sky.jpg",
    L"dust.png",
};

// �I�u�W�F�N�g�f�[�^
struct ObjectData
{
    Object obj;
    BlendMode mode;
    BOOL blendFlag;
};

// �I�u�W�F�N�g���X�g
static const ObjectData objectList[] =
{
    {SKY,   NONE,       FALSE},
    {DUST,  MULTIPLE,   TRUE},
};

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

// using�f�B���N�e�B�u
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// �p�C�v���C���I�u�W�F�N�g
ID3D11Device* g_device;                                         // �f�o�C�X�C���^�[�t�F�C�X
ID3D11DeviceContext* g_context;                                 // �R���e�L�X�g
IDXGISwapChain* g_swapChain;                                    // �X���b�v�`�F�C���C���^�[�t�F�C�X
ID3D11InputLayout* g_layout[BLEND_NUMMAX >= 2 ? 2 : 1];         // �C���v�b�g���C�A�E�g
ID3D11VertexShader* g_vertexShader[BLEND_NUMMAX >= 2 ? 2 : 1];  // ���_�V�F�[�_
ID3D11PixelShader* g_pixelShader[BLEND_NUMMAX];                 // �s�N�Z���V�F�[�_
ID3D11Buffer* g_vertexBuffer;                                   // ���_�o�b�t�@
ID3D11Buffer* g_indexBuffer;                                    // �C���f�b�N�X�o�b�t�@
ID3D11Buffer* g_constantBuffer;                                 // �萔�o�b�t�@
ID3D11SamplerState* g_sampler;                                  // �e�N�X�`���T���v��
ID3D11ShaderResourceView* g_shaderResourceView[OBJ_NUMMAX];     // �e�N�X�`�����\�[�X

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
XMMATRIX g_world[OBJ_NUMMAX];   // ���[���h�s��̕����x�N�g��

// Y����]�ϐ�
static FLOAT y = 0;

// �g�p����^�[�Q�b�g�̃��X�g
enum TargetList
{
    _BACK_BUFFER,
    _RENDER_TARGET1,
    _RENDER_TARGET2,
    _RENDER_TARGET3,
    _TARGET_NUMMAX,
};
// ����̓����_�[�^�[�Q�b�g3�͎g��Ȃ����A�A
// �I�u�W�F�N�g�������Ă���Ǝg���@����邩������Ȃ��B

// �����_�[�^�[�Q�b�g�N���X
class RenderTarget
{
public:
    // �^�[�Q�b�g���
    enum Target
    {
        BACK_BUFFER,        // �o�b�N�o�b�t�@
        RENDER_TARGET,      // �����_�[�^�[�Q�b�g
        TARGET_NUMMAX,
    };

    // �R���X�g���N�^
    RenderTarget() : m_renderTargetView(nullptr), m_shaderResourceView(nullptr)
    {
        SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    // �f�X�g���N�^
    ~RenderTarget()
    {
        SAFE_RELEASE(m_layerBuffer);
        SAFE_RELEASE(m_renderTargetView);
        SAFE_RELEASE(m_shaderResourceView);
    }

    // �^�[�Q�b�g������
    HRESULT Init(Target target)
    {
        switch (target) {
        case BACK_BUFFER:       // �o�b�N�o�b�t�@
            // �o�b�N�o�b�t�@�g�p���̓X���b�v�`�F�C���ƕR�Â�
            g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_layerBuffer));

            // �o�b�N�o�b�t�@���쐬
            if (FAILED(g_device->CreateRenderTargetView(m_layerBuffer, nullptr, &m_renderTargetView)))
            {
                MessageBox(NULL, L"�o�b�N�o�b�t�@���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            break;

        case RENDER_TARGET:     // �����_�[�^�[�Q�b�g
            // �����_�[�^�[�Q�b�g�̐ݒ�
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

            // �����_�[�^�[�Q�b�g���쐬
            if (FAILED(g_device->CreateTexture2D(&rtDesc, nullptr, &m_layerBuffer)))
            {
                MessageBox(NULL, L"�����_�[�^�[�Q�b�g���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            // �����_�[�^�[�Q�b�g�r���[�̐ݒ�
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            ZeroMemory(&rtvDesc, sizeof(rtvDesc));
            rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

            // �����_�[�^�[�Q�b�g�r���[���쐬
            if (FAILED(g_device->CreateRenderTargetView(m_layerBuffer, &rtvDesc, &m_renderTargetView)))
            {
                MessageBox(NULL, L"�����_�[�^�[�Q�b�g�r���[���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            // �V�F�[�_���\�[�X�r���[�̐ݒ�
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            ZeroMemory(&srvDesc, sizeof(srvDesc));
            srvDesc.Format = rtvDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            // �V�F�[�_���\�[�X�r���[���쐬
            if (FAILED(g_device->CreateShaderResourceView(m_layerBuffer, &srvDesc, &m_shaderResourceView)))
            {
                MessageBox(NULL, L"�V�F�[�_���\�[�X�r���[���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }
        }

        return S_OK;
    }

    // �w�i�F�Z�b�g
    inline VOID SetClearColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a)
    {
        m_clearColor = { r, g, b, a };
    }

    // �^�[�Q�b�g�Z�b�g
    inline VOID SetRenderTarget() const
    {
        g_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
    }

    // ���C���[�N���A
    inline VOID ClearRenderTargetView() const
    {
        g_context->ClearRenderTargetView(m_renderTargetView, m_clearColor);
    }

    // �V�F�[�_���\�[�X�r���[�A�h���X�擾
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

// �^�[�Q�b�g�f�[�^
struct TargetData
{
    RenderTarget::Target target;
    D3DXCOLOR color;
};

// �^�[�Q�b�g�f�[�^���������X�g
TargetData initTargetData[] =
{
    {RenderTarget::BACK_BUFFER,     {0.0f, 0.0f, 1.0f, 1.0f}},      // �u���[
    {RenderTarget::RENDER_TARGET,   {0.0f, 1.0f, 0.0f, 1.0f}},      // �O���[��
    {RenderTarget::RENDER_TARGET,   {0.0f, 0.0f, 0.0f, 0.0f}},      // ����
    {RenderTarget::RENDER_TARGET,   {0.0f, 0.0f, 0.0f, 0.0f}},      // ����
};

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

    // �^�[�Q�b�g���X�g������
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

    // �u�����h�̐ݒ�
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;         // �ʏ�(�A���t�@�u�����h)�L��
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;    // �E�E
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    // �u�����h�X�e�[�g���쐬
    ID3D11BlendState* blendState;
    if (FAILED(g_device->CreateBlendState(&blendDesc, &blendState)))
    {
        MessageBox(NULL, L"�u�����h�X�e�[�g���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �u�����h���Z�b�g
    FLOAT blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
    g_context->OMSetBlendState(blendState, blendFactor, 0xffffffff);

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

    SAFE_RELEASE(blendState);
    SAFE_RELEASE(rasState);

    return S_OK;
}

// �V�F�[�_�֘A������
HRESULT InitShader()
{
    // ���_�C���v�b�g���C�A�E�g���`
    D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    // �C���v�b�g���C�A�E�g�̃T�C�Y
    UINT numElements = sizeof(inputElementDescs) / sizeof(inputElementDescs[0]);

    ID3DBlob* vertexShader[BLEND_NUMMAX >= 2 ? 2 : 1], * pixelShader[BLEND_NUMMAX];

    // �����̃V�F�[�_�����[�h���R���p�C��
    for (UINT i = 0; i < BLEND_NUMMAX; i++)
    {
        // ���_�V�F�[�_
        if (i + 1 <= (BLEND_NUMMAX >= 2 ? 2 : 1))
        {
            if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, initShaderData[i].vsEntry, "vs_4_0", NULL, NULL, NULL, &vertexShader[i], NULL, NULL)))
            {
                MessageBox(NULL, L"���_�V�F�[�_��ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            // �J�v�Z����
            g_device->CreateVertexShader(vertexShader[i]->GetBufferPointer(), vertexShader[i]->GetBufferSize(), NULL, &g_vertexShader[i]);

            // ���_�C���v�b�g���C�A�E�g���쐬
            if (FAILED(g_device->CreateInputLayout(inputElementDescs, numElements, vertexShader[i]->GetBufferPointer(), vertexShader[i]->GetBufferSize(), &g_layout[i])))
            {
                MessageBox(NULL, L"���_�C���v�b�g���C�A�E�g�̒�`���Ԉ���Ă��܂��B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
                return E_FAIL;
            }

            // ���_�C���v�b�g���C�A�E�g���Z�b�g
            g_context->IASetInputLayout(g_layout[i]);

            SAFE_RELEASE(vertexShader[i]);
        }

        // �s�N�Z���V�F�[�_
        if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, initShaderData[i].psEntry, "ps_4_0", NULL, NULL, NULL, &pixelShader[i], NULL, NULL)))
        {
            MessageBox(NULL, L"�s�N�Z���V�F�[�_��ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }

        // �J�v�Z����
        g_device->CreatePixelShader(pixelShader[i]->GetBufferPointer(), pixelShader[i]->GetBufferSize(), NULL, &g_pixelShader[i]);

        SAFE_RELEASE(pixelShader[i]);
    }

    return S_OK;
}

// �o�b�t�@�֘A������
HRESULT InitBuffer()
{
    // �l�p�`�̃W�I���g�����`    
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
    for (UINT i = 0; i < OBJ_NUMMAX; i++)
    {
        if (FAILED(D3DX11CreateShaderResourceViewFromFile(g_device, file[i], NULL, NULL, &g_shaderResourceView[i], NULL)))
        {
            MessageBox(NULL, L"�e�N�X�`����ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }
    }

    return S_OK;
}

// �}�g���b�N�X�֘A������
VOID InitMatrix()
{
    // ���[���h�}�g���b�N�X�̏�����
    for (UINT i = 0; i < OBJ_NUMMAX; i++)
        g_world[i] = XMMatrixIdentity();    // �����Ȃ��őS�Ă̎���0.0f(�ړ��Ȃ�)
}

// �X�V
VOID OnUpdate()
{
    // �p�����[�^�̎󂯓n��
    XMMATRIX scale = XMMatrixScaling(0.55f, 0.8f, 0.0f);    // X��0.55�{, Y��0.8�{�ɏk��
    XMMATRIX rotate = XMMatrixRotationY(y += 0.0005f);      // Y��0.0005����]
    g_world[DUST] = scale * rotate;                         // �e�}�g���b�N�X�̐�
}

// �`��
VOID OnRender()
{
    // ���ʃo�b�t�@���Z�b�g
    g_context->VSSetConstantBuffers(0, 1, &g_constantBuffer);
    g_context->PSSetSamplers(0, 1, &g_sampler);



    // �����_�[�^�[�Q�b�g1���Z�b�g
    g_renderTarget[_RENDER_TARGET1]->SetRenderTarget();
    g_renderTarget[_RENDER_TARGET1]->ClearRenderTargetView();

    // �V�F�[�_�I�u�W�F�N�g���Z�b�g
    g_context->VSSetShader(g_vertexShader[objectList[SKY].blendFlag], nullptr, 0);
    g_context->PSSetShader(g_pixelShader[objectList[SKY].mode], nullptr, 0);

    // �e�N�X�`��1���V�F�[�_�ɓo�^
    g_context->PSSetShaderResources(0, 1, &g_shaderResourceView[objectList[SKY].obj]);

    // �ϐ�1���
    ConstantBuffer cBuffer;
    cBuffer.m_world = XMMatrixTranspose(g_world[objectList[SKY].obj]);

    // GPU(�V�F�[�_��)�֓]������
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // �C���f�b�N�X�o�b�t�@��RT1�ɕ`��
    g_context->DrawIndexed(6, 0, 0);



    // �����_�[�^�[�Q�b�g2���Z�b�g
    g_renderTarget[_RENDER_TARGET2]->SetRenderTarget();
    g_renderTarget[_RENDER_TARGET2]->ClearRenderTargetView();

    // �e�N�X�`��2���V�F�[�_�ɓo�^
    g_context->PSSetShaderResources(0, 1, &g_shaderResourceView[objectList[DUST].obj]);

    // �ϐ�2���
    cBuffer.m_world = XMMatrixTranspose(g_world[objectList[DUST].obj]);

    // GPU(�V�F�[�_��)�֓]������
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // �C���f�b�N�X�o�b�t�@��RT2�ɕ`��
    g_context->DrawIndexed(6, 0, 0);



    // �o�b�N�o�b�t�@���Z�b�g
    g_renderTarget[_BACK_BUFFER]->SetRenderTarget();
    g_renderTarget[_BACK_BUFFER]->ClearRenderTargetView();


    // RT1�����̃��C���[�Ƃ��Đ�ɕ`��
    g_context->PSSetShaderResources(0, 1, g_renderTarget[_RENDER_TARGET1]->GetAddressSRV());

    // �ϐ�1���
    cBuffer.m_world = XMMatrixTranspose(g_world[objectList[SKY].obj]);

    // GPU(�V�F�[�_��)�֓]������
    g_context->UpdateSubresource(g_constantBuffer, 0, nullptr, &cBuffer, 0, 0);

    // �C���f�b�N�X�o�b�t�@���o�b�N�o�b�t�@�ɕ`��
    g_context->DrawIndexed(6, 0, 0);


    // �V�F�[�_�I�u�W�F�N�g���Z�b�g
    g_context->VSSetShader(g_vertexShader[objectList[DUST].blendFlag], nullptr, 0);
    g_context->PSSetShader(g_pixelShader[objectList[DUST].mode], nullptr, 0);

    // RT1��RT2���V�F�[�_�ɓo�^������
    g_context->PSSetShaderResources(0, 1, g_renderTarget[_RENDER_TARGET1]->GetAddressSRV());
    g_context->PSSetShaderResources(1, 1, g_renderTarget[_RENDER_TARGET2]->GetAddressSRV());

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
