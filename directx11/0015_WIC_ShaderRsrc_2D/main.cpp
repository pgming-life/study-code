#include <windows.h>
#include <wrl.h>
#include <wincodec.h>
#include <string>
#include <vector>

// DirectX11�̃R�[�h�Z�b�g
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

// ���C�u����
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define WINDOW_CLASS    L"WIC�e�N�X�`���ƃV�F�[�_���\�[�X���ɂ�鍶����W2D�`��"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

#define SAFE_RELEASE(x) if(x) x->Release();

// �t�@�C����
static LPCWSTR fileName = L"red_smoke.png";

// �摜�T�C�Y(�s�N�Z���P�ʂ�FLOAT�^)
static FLOAT g_imageWidth = 0.f;
static FLOAT g_imageHeight = 0.f;

// �V�F�[�_�{��
static const std::string shaderCode = "\
//================================================================================\n\
// �萔�o�b�t�@�ϐ�n\n\
//================================================================================\n\
cbuffer ConstantBuffer : register(b0)\n\
{\n\
	matrix			g_wp;			// ���[���h�s��E�v���W�F�N�V�����s��\n\
	Texture2D       g_texture;		// ��{�F\n\
	SamplerState    g_sampler;		// �T���v��\n\
};\n\
\n\
//================================================================================\n\
// ���̓p�����[�^\n\
//================================================================================\n\
struct VS_INPUT		// ���_�V�F�[�_\n\
{\n\
	float4 position : POSITION;		// ���_���W\n\
	float2 uv : TEXCOORD0;			// �e�N�Z��(UV���W)\n\
};\n\
\n\
//================================================================================\n\
// �o�̓p�����[�^\n\
//================================================================================\n\
struct VS_OUTPUT	// ���_�V�F�[�_\n\
{\n\
	float4 position : SV_POSITION;\n\
	float2 uv : TEXCOORD0;			// ��{�FUV�l\n\
};\n\
\n\
//================================================================================\n\
// ���_�V�F�[�_\n\
//================================================================================\n\
VS_OUTPUT VSMain(VS_INPUT input)\n\
{\n\
	VS_OUTPUT output;\n\
\n\
	// �s��ϊ�\n\
	output.position = mul(input.position, g_wp);\n\
\n\
	output.uv = input.uv;\n\
\n\
	return output;\n\
}\n\
\n\
// �s�N�Z���V�F�[�_���̓p�����[�^\n\
typedef VS_OUTPUT PS_INPUT;		// �u������\n\
\n\
//================================================================================\n\
// �s�N�Z���V�F�[�_\n\
//================================================================================\n\
float4 PSMain(PS_INPUT input) : SV_TARGET\n\
{\n\
	// ���̂܂܏o��\n\
	return g_texture.Sample(g_sampler, input.uv);\n\
}\n\
\n";
// �V�F�[�_�{���͒P�Ȃ�e�L�X�g�Ȃ̂ŁA
// ������Ƃ��ēǂݍ��߂邱�Ƃ��ł��邽�߃��\�[�X�����邱�Ƃ��ł���B

// �v���g�^�C�v�錾
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // �E�B���h�E�v���V�[�W��
HRESULT OnInit(HWND hWnd);          // ������
HRESULT InitDevice(HWND hWnd);      // �f�o�C�X�֘A������
HRESULT InitView();                 // �r���[�֘A������
HRESULT InitShader();               // �V�F�[�_�֘A������
HRESULT InitTexture();              // �e�N�X�`���֘A������
HRESULT InitBuffer();               // �o�b�t�@�֘A������
VOID InitMatrix();                  // �}�g���b�N�X�֘A������
VOID OnUpdate();                    // �X�V
VOID OnRender();                    // �`��
VOID OnDestroy();                   // ���������

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
ID3D11Buffer* g_constantBuffer;						// �萔�o�b�t�@
ID3D11ShaderResourceView* g_shaderResourceView;     // �e�N�X�`�����\�[�X

// ���_�\����
struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
};

// �萔�o�b�t�@
struct ConstantBuffer
{
	XMMATRIX m_WP;
};

// �}�g���b�N�X
static XMMATRIX g_world;		// ���[���h�s��̕����x�N�g��
static XMMATRIX g_projection;	// �v���W�F�N�V�����s��̕����x�N�g��

// XY�������ړ��ϐ�
static FLOAT x = 0.f;
static FLOAT y = 0.f;

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

	// �e�N�X�`���֘A������
	if (FAILED(InitTexture())) return E_FAIL;

	// �o�b�t�@�֘A������
	if (FAILED(InitBuffer())) return E_FAIL;

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

	SAFE_RELEASE(blendState);

	return S_OK;
}

// �V�F�[�_�֘A������
HRESULT InitShader()
{
	ID3DBlob* vertexShader, * pixelShader;

	// �����̃V�F�[�_�����[�h���R���p�C��
	if (FAILED(D3DCompile(shaderCode.c_str(), shaderCode.length(), NULL, NULL, NULL, "VSMain", "vs_4_0", NULL, NULL, &vertexShader, NULL)))
	{
		MessageBox(NULL, L"���_�V�F�[�_��ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}
	if (FAILED(D3DCompile(shaderCode.c_str(), shaderCode.length(), NULL, NULL, NULL, "PSMain", "ps_4_0", NULL, NULL, &pixelShader, NULL)))
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

// �e�N�X�`���֘A������
HRESULT InitTexture()
{
	// WIC�g�p�̂��߂̌Ăяo��
	CoInitialize(NULL);

	// �t�@�N�g�����쐬
	IWICImagingFactory* factory;
	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)(&factory))))
	{
		MessageBox(NULL, L"�t�@�N�g�����쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �e�N�X�`���̓ǂݍ���
	IWICBitmapDecoder* decoder;
	if (FAILED(factory->CreateDecoderFromFilename(fileName, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)))
	{
		MessageBox(NULL, L"�e�N�X�`����ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �f�R�[�h�t���[�����擾
	IWICBitmapFrameDecode* frame;
	if (FAILED(decoder->GetFrame(0, &frame)))
	{
		MessageBox(NULL, L"�f�R�[�h�t���[�����擾�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �t�H�[�}�b�g�R���o�[�^���쐬
	IWICFormatConverter* formatConv;
	if (FAILED(factory->CreateFormatConverter(&formatConv)))
	{
		MessageBox(NULL, L"�t�H�[�}�b�g�R���o�[�^���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �t�H�[�}�b�g������
	if (FAILED(formatConv->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeMedianCut)))
	{
		MessageBox(NULL, L"�t�H�[�}�b�g�̏������Ɏ��s���܂����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �摜�T�C�Y���擾
	UINT imageWidth;
	UINT imageHeight;
	if (FAILED(formatConv->GetSize(&imageWidth, &imageHeight)))
	{
		MessageBox(NULL, L"�摜�T�C�Y���擾�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �摜�T�C�Y���i�[
	g_imageWidth = static_cast<FLOAT>(imageWidth);
	g_imageHeight = static_cast<FLOAT>(imageHeight);

	// �e�N�X�`�����\�[�X�̐ݒ�
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = imageWidth;
	textureDesc.Height = imageHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureDesc.MiscFlags = 0;

	// �e�N�X�`�����\�[�X���쐬
	ID3D11Texture2D* textureRsrc;
	if (FAILED(g_device->CreateTexture2D(&textureDesc, NULL, &textureRsrc)))
	{
		MessageBox(NULL, L"�e�N�X�`�����\�[�X���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �e�N�X�`�����\�[�X�Ƀe�N�X�`�������R�s�[
	D3D11_MAPPED_SUBRESOURCE mappedSubrsrc;
	ZeroMemory(&mappedSubrsrc, sizeof(mappedSubrsrc));
	g_context->Map(textureRsrc, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubrsrc);
	formatConv->CopyPixels(NULL, g_imageWidth * 4, g_imageWidth * g_imageHeight * 4, (BYTE*)mappedSubrsrc.pData);
	g_context->Unmap(textureRsrc, 0);

	// �V�F�[�_���\�[�X�r���[�̐ݒ�
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// �V�F�[�_���\�[�X�r���[���쐬
	if (FAILED(g_device->CreateShaderResourceView(textureRsrc, &srvDesc, &g_shaderResourceView)))
	{
		MessageBox(NULL, L"�V�F�[�_���\�[�X�r���[���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	SAFE_RELEASE(factory);
	SAFE_RELEASE(decoder);
	SAFE_RELEASE(frame);
	SAFE_RELEASE(formatConv);
	SAFE_RELEASE(textureRsrc);

	return S_OK;
}

// �o�b�t�@�֘A������
HRESULT InitBuffer()
{
	// �l�p�`�̃W�I���g�����`
	Vertex vertices[] =
	{
		{{0.0f,			0.0f,		   0.0f}, {0.0f, 0.0f}},
		{{g_imageWidth, 0.0f,		   0.0f}, {1.0f, 0.0f}},
		{{0.0f,			g_imageHeight, 0.0f}, {0.0f, 1.0f}},
		{{g_imageWidth, g_imageHeight, 0.0f}, {1.0f, 1.0f}},
	};
	// ���_���W��Translation�ɑΉ����邽�߂Ƀs�N�Z�����W�ɕϊ��B
	// Scaling�͍s����g�킸�A���_���̂��̂ɑ΂��ăX�P�[������̂�UV�l�͂�����Ȃ��B
	// �Ȃ̂ŁA�^�[�Q�b�g���`���邻�̓s�x���_�����X�V����B
	// WIC�ŉ摜�T�C�Y��ǂݍ���Œ��_��ݒ肷��̂ŕK���W�I���g����`�͌㏈���ɂȂ�B

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
		MessageBox(NULL, L"���_�o�b�t�@���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �\�����钸�_�o�b�t�@��I��
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_context->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, &offset);

	// �g�p����v���~�e�B�u�^�C�v��ݒ�
	g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �萔�o�b�t�@�̐ݒ�
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// �萔�o�b�t�@���쐬
	if (FAILED(g_device->CreateBuffer(&cbDesc, nullptr, &g_constantBuffer)))
	{
		MessageBox(NULL, L"�萔�o�b�t�@���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	return S_OK;
}

// �}�g���b�N�X�֘A������
VOID InitMatrix()
{
	// ���[���h�}�g���b�N�X�̏�����
	g_world = XMMatrixIdentity();       // �����Ȃ��őS�Ă̎���0.0f(�ړ��Ȃ�)

	// �v���W�F�N�V�����}�g���b�N�X�̏�����(�ˉe�s��ϊ�)
	g_projection = XMMatrixOrthographicOffCenterLH(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, 0.0f, 1.0f);
	// ���ˉe�ϊ��s���ݒ肵���̐��s��ɂ���ĕ��s���e����(�����ɍs���Ă������傫��)
	// ����(0, 0)����Ƃ���2D���W�ɂ��邽�߂ɒ��_�W�I���g����WINDOW_WIDTH��WINDOW_HEIGHT�Őݒ�
	// ���s����: 2D�`��p(Z����0�`1)
}

// �X�V
VOID OnUpdate()
{
	const FLOAT speedX = WINDOW_WIDTH / 30000.f;	// X���X�s�[�h
	const FLOAT speedY = WINDOW_HEIGHT / 30000.f;	// Y���X�s�[�h
	const FLOAT offsetBounds = WINDOW_WIDTH;		// �E�B���h�E���I�t�Z�b�g��

	// �E�B���h�E���̃I�t�Z�b�g����o��ƍ��ォ���蒼��
	if (x > offsetBounds)
	{
		x = -g_imageWidth;
		y = -g_imageHeight;
	}

	// �p�����[�^�̎󂯓n��
	D3D11_MAPPED_SUBRESOURCE cData;
	ConstantBuffer cBuffer;
	g_world = XMMatrixTranslation(x += speedX, y += speedY, 0.f);		// �E�������Ƀs�N�Z���P�ʈړ�
	cBuffer.m_WP = XMMatrixTranspose(g_world * g_projection);

	// GPU(�V�F�[�_��)�֓]������
	g_context->Map(g_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cData);
	memcpy_s(cData.pData, cData.RowPitch, (void*)(&cBuffer), sizeof(cBuffer));
	g_context->Unmap(g_constantBuffer, 0);
}

// �`��
VOID OnRender()
{
	// �����_�[�^�[�Q�b�g�r���[���w�肵���F�ŃN���A
	FLOAT clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };       // �u���[
	g_context->ClearRenderTargetView(g_renderTargetView, clearColor);

	// �V�F�[�_�I�u�W�F�N�g�����Z�b�g
	g_context->VSSetShader(g_vertexShader, nullptr, 0);
	g_context->PSSetShader(g_pixelShader, nullptr, 0);
	g_context->VSSetConstantBuffers(0, 1, &g_constantBuffer);

	// �e�N�X�`�����V�F�[�_�ɓo�^
	g_context->PSSetShaderResources(0, 1, &g_shaderResourceView);

	// ���_�o�b�t�@���o�b�N�o�b�t�@�ɕ`��
	g_context->Draw(4, 0);

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
	SAFE_RELEASE(g_constantBuffer);
	SAFE_RELEASE(g_shaderResourceView);
}
