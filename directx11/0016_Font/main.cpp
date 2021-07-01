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

#define WINDOW_CLASS    L"�����̕`��"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

// �t�H���g���
static const FLOAT g_fontSize = 350.f;       // 350px
static const LPCWSTR g_fontChar = L"�";     // Shift-JIS�����̒��ōł��搔����������

// �v���g�^�C�v�錾
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // �E�B���h�E�v���V�[�W��
HRESULT OnInit(HWND hWnd);          // ������
HRESULT InitDevice(HWND hWnd);      // �f�o�C�X�֘A������
HRESULT InitView();                 // �r���[�֘A������
HRESULT InitShader();               // �V�F�[�_�֘A������
HRESULT InitBuffer();               // �o�b�t�@�֘A������
HRESULT InitTexture();              // �e�N�X�`���֘A������
VOID InitMatrix();                  // �}�g���b�N�X�֘A������
VOID OnRender();                    // �`��

// using�f�B���N�e�B�u
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// �p�C�v���C���I�u�W�F�N�g
ComPtr<ID3D11Device> g_device;                              // �f�o�C�X�C���^�[�t�F�C�X
ComPtr<ID3D11DeviceContext> g_context;                      // �R���e�L�X�g
ComPtr<IDXGISwapChain> g_swapChain;                         // �X���b�v�`�F�C���C���^�[�t�F�C�X
ComPtr<ID3D11RenderTargetView> g_renderTargetView;          // �����_�[�^�[�Q�b�g�r���[
ComPtr<ID3D11InputLayout> g_layout;                         // �C���v�b�g���C�A�E�g
ComPtr<ID3D11VertexShader> g_vertexShader;                  // ���_�V�F�[�_
ComPtr<ID3D11PixelShader> g_pixelShader;                    // �s�N�Z���V�F�[�_
ComPtr<ID3D11Buffer> g_vertexBuffer;                        // ���_�o�b�t�@
ComPtr<ID3D11Buffer> g_indexBuffer;                         // �C���f�b�N�X�o�b�t�@
ComPtr<ID3D11Buffer> g_constantBuffer;                      // �萔�o�b�t�@
ComPtr<ID3D11SamplerState> g_sampler;                       // �e�N�X�`���T���v��

// ���_�\����
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
};

// �萔�\����
struct ConstantBuffer
{
    XMMATRIX m_WP;
};

// �t�H���g�e�L�X�g�N���X
class FontText
{
public:
    // �R���X�g���N�^
    FontText() : m_layerBuffer(nullptr), m_shaderResourceView(nullptr)
    {
    }

    // �f�X�g���N�^
    ~FontText()
    {
    }

    // �P�����̐���
    HRESULT InitChar(LPCWSTR c)
    {
        // �t�H���g�n���h���̐ݒ�
        UINT fontSize = 64;
        UINT fontWeight = 1000;
        LPCSTR font = "�l�r �S�V�b�N";
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

        // �t�H���g�n���h���𐶐�
        HFONT hFont = CreateFontIndirectW(&lf);

        // ���݂̃E�B���h�E�ɓK�p
        HDC hdc = GetDC(NULL);
        HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, hFont));
        // �f�o�C�X�Ƀt�H���g���������Ȃ���GetGlyphOutlineW�֐��̓G���[�ƂȂ�B

        // �o�͂��镶��(�ꕶ������)
        UINT code = static_cast<UINT>(*c);

        // 17�K���̃O���[�̃O���t�r�b�g�}�b�v
        const UINT gradFlag = GGO_GRAY4_BITMAP;

        // �r�b�g�}�b�v�̐ݒ�
        GLYPHMETRICS gm;
        TEXTMETRIC tm;
        GetTextMetrics(hdc, &tm);
        CONST MAT2 mat = { {0,1},{0,0},{0,0},{0,1} };

        // �t�H���g�r�b�g�}�b�v���擾
        DWORD size = GetGlyphOutlineW(hdc, code, gradFlag, &gm, 0, NULL, &mat);
        BYTE* pMono = new BYTE[size];
        GetGlyphOutlineW(hdc, code, gradFlag, &gm, size, pMono, &mat);

        // �t�H���g�̕��ƍ���
        INT fontWidth = gm.gmCellIncX;
        INT fontHeight = tm.tmHeight;

        // �����_�[�^�[�Q�b�g�̐ݒ�
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

        // �t�H���g�p�e�N�X�`�����쐬
        if (FAILED(g_device->CreateTexture2D(&rtDesc, nullptr, m_layerBuffer.GetAddressOf())))
        {
            MessageBox(NULL, L"�t�H���g�p�e�N�X�`�����쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }

        // �t�H���g�p�e�N�X�`�����\�[�X�Ƀe�N�X�`�������R�s�[
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
        // �t�H���g���̏�������
        // iOfs_x, iOfs_y : �����o���ʒu(����)
        // iBmp_w, iBmp_h : �t�H���g�r�b�g�}�b�v�̕���
        // Level : ���l�̒i�K (GGO_GRAY4_BITMAP�Ȃ̂�17�i�K)

        // ���������
        delete[] pMono;

        // �V�F�[�_���\�[�X�r���[�̐ݒ�
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = rtDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = rtDesc.MipLevels;

        // �V�F�[�_���\�[�X�r���[���쐬
        if (FAILED(g_device->CreateShaderResourceView(m_layerBuffer.Get(), &srvDesc, m_shaderResourceView.GetAddressOf())))
        {
            MessageBox(NULL, L"�V�F�[�_���\�[�X�r���[���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return E_FAIL;
        }
    }

    // �V�F�[�_���\�[�X�r���[�A�h���X�擾
    inline ID3D11ShaderResourceView** GetAddressSRV()
    {
        return m_shaderResourceView.GetAddressOf();
    }

private:
    ComPtr<ID3D11Texture2D> m_layerBuffer;
    ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
};
std::unique_ptr<FontText> g_fontText;

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
                // �`��
                OnRender();
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
            g_swapChain.GetAddressOf(),
            g_device.GetAddressOf(),
            nullptr,
            g_context.GetAddressOf()
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
    ComPtr<ID3D11Texture2D> backBuffer;
    g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    g_device->CreateRenderTargetView(backBuffer.Get(), nullptr, g_renderTargetView.GetAddressOf());

    // �����_�[�^�[�Q�b�g���o�b�N�o�b�t�@�ɐݒ�
    g_context->OMSetRenderTargets(1, g_renderTargetView.GetAddressOf(), nullptr);

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
    ComPtr<ID3D11BlendState> blendState;
    if (FAILED(g_device->CreateBlendState(&blendDesc, blendState.GetAddressOf())))
    {
        MessageBox(NULL, L"�u�����h�X�e�[�g���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �u�����h���Z�b�g
    FLOAT blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
    g_context->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);

    return S_OK;
}

// �V�F�[�_�֘A������
HRESULT InitShader()
{
    ComPtr<ID3DBlob> vertexShader, pixelShader;

    // �����̃V�F�[�_�����[�h���R���p�C��
    if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, "vsMain", "vs_4_0", NULL, NULL, NULL, vertexShader.GetAddressOf(), NULL, NULL)))
    {
        MessageBox(NULL, L"���_�V�F�[�_��ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }
    if (FAILED(D3DX11CompileFromFile(L"shaders.hlsl", NULL, NULL, "psMain", "ps_4_0", NULL, NULL, NULL, pixelShader.GetAddressOf(), NULL, NULL)))
    {
        MessageBox(NULL, L"�s�N�Z���V�F�[�_��ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �J�v�Z����
    g_device->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), NULL, g_vertexShader.GetAddressOf());
    g_device->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), NULL, g_pixelShader.GetAddressOf());

    // ���_�C���v�b�g���C�A�E�g���`
    D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    // �C���v�b�g���C�A�E�g�̃T�C�Y
    UINT numElements = sizeof(inputElementDescs) / sizeof(inputElementDescs[0]);

    // ���_�C���v�b�g���C�A�E�g���쐬
    if (FAILED(g_device->CreateInputLayout(inputElementDescs, numElements, vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), g_layout.GetAddressOf())))
    {
        MessageBox(NULL, L"���_�C���v�b�g���C�A�E�g�̒�`���Ԉ���Ă��܂��B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // ���_�C���v�b�g���C�A�E�g���Z�b�g
    g_context->IASetInputLayout(g_layout.Get());

    return S_OK;
}

// �o�b�t�@�֘A������
HRESULT InitBuffer()
{
    // �l�p�`�̃W�I���g�����`
    Vertex vertices[] =
    {
        {{0.0f,       0.0f,       0.0f}, {0.0f, 0.0f}},
        {{g_fontSize, 0.0f,       0.0f}, {1.0f, 0.0f}},
        {{0.0f,       g_fontSize, 0.0f}, {0.0f, 1.0f}},
        {{g_fontSize, g_fontSize, 0.0f}, {1.0f, 1.0f}},
    };
    // �t�H���g�p�|���S���B
    // ���_���W��Translation�ɑΉ����邽�߂Ƀs�N�Z�����W�ɕϊ��B
    // Scaling�͍s����g�킸�A���_���̂��̂ɑ΂��ăX�P�[������̂�UV�l�͂�����Ȃ��B
    // �Ȃ̂ŁA�^�[�Q�b�g���`���邻�̓s�x���_�����X�V����B

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
    if (FAILED(g_device->CreateBuffer(&bufferDesc, &initData, g_vertexBuffer.GetAddressOf())))
    {
        MessageBox(NULL, L"���_�o�b�t�@���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �\�����钸�_�o�b�t�@��I��
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_context->IASetVertexBuffers(0, 1, g_vertexBuffer.GetAddressOf(), &stride, &offset);

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
    if (FAILED(g_device->CreateBuffer(&bufferDesc, &initData, g_indexBuffer.GetAddressOf())))
    {
        MessageBox(NULL, L"�C���f�b�N�X�o�b�t�@���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �\������C���f�b�N�X�o�b�t�@��I��
    g_context->IASetIndexBuffer(g_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    // �g�p����v���~�e�B�u�^�C�v��ݒ�
    g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // �萔���̒ǉ�
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(ConstantBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    // �萔�o�b�t�@���쐬
    if (FAILED(g_device->CreateBuffer(&bufferDesc, nullptr, g_constantBuffer.GetAddressOf())))
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
    if (FAILED(g_device->CreateSamplerState(&samplerDesc, g_sampler.GetAddressOf())))
    {
        MessageBox(NULL, L"�T���v���X�e�[�g���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �t�H���g�e�L�X�g������
    g_fontText.reset(new FontText());
    g_fontText->InitChar(g_fontChar);

    return S_OK;
}

// �}�g���b�N�X�֘A������
VOID InitMatrix()
{
    // ���[���h�}�g���b�N�X�̏�����
    XMMATRIX world = XMMatrixIdentity();       // �����Ȃ��őS�Ă̎���0.0f(�ړ��Ȃ�)

    // �v���W�F�N�V�����}�g���b�N�X�̏�����(�ˉe�s��ϊ�)
    XMMATRIX projection = XMMatrixOrthographicOffCenterLH(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 0.0f, 1.0f);
    // ���ˉe�ϊ��s���ݒ肵���̐��s��ɂ���ĕ��s���e����(�����ɍs���Ă������傫��)
    // ����(0, 0)����Ƃ���2D���W�ɂ��邽�߂ɒ��_�W�I���g����WINDOW_WIDTH��WINDOW_HEIGHT�Őݒ�
    // ���s����: 2D�`��p(Z����0�`1)

    // �ϐ����
    ConstantBuffer cBuffer;
    cBuffer.m_WP = XMMatrixTranspose(world * projection);

    // GPU(�V�F�[�_��)�֓]������
    g_context->UpdateSubresource(g_constantBuffer.Get(), 0, nullptr, &cBuffer, 0, 0);
}

// �`��
VOID OnRender()
{
    // �����_�[�^�[�Q�b�g�r���[���w�肵���F�ŃN���A
    const FLOAT clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };       // �u���[
    g_context->ClearRenderTargetView(g_renderTargetView.Get(), clearColor);

    // GPU�o�b�t�@���Z�b�g
    g_context->VSSetShader(g_vertexShader.Get(), nullptr, 0);
    g_context->PSSetShader(g_pixelShader.Get(), nullptr, 0);
    g_context->VSSetConstantBuffers(0, 1, g_constantBuffer.GetAddressOf());

    // �e�N�X�`�����V�F�[�_�ɓo�^
    g_context->PSSetSamplers(0, 1, g_sampler.GetAddressOf());
    g_context->PSSetShaderResources(0, 1, g_fontText->GetAddressSRV());

    // �C���f�b�N�X�o�b�t�@���o�b�N�o�b�t�@�ɕ`��
    g_context->DrawIndexed(6, 0, 0);

    // �t���[�����ŏI�o��
    g_swapChain->Present(0, 0);     // �t���b�v
}
