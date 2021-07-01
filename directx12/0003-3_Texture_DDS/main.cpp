#include <windows.h>
#include <wrl.h>

// DirectX12�̃R�[�h�Z�b�g
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "d3dx12.h"
#include "DDSTextureLoader12.h"

// ���C�u����
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define WINDOW_CLASS	L"�e�N�X�`��(DDS)"
#define WINDOW_TITLE	WINDOW_CLASS
#define WINDOW_WIDTH	750
#define WINDOW_HEIGHT	500

// �v���g�^�C�v�錾
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // �E�B���h�E�v���V�[�W��
HRESULT OnInit(HWND hWnd);      // ������
HRESULT InitDevice(HWND hWnd);  // �f�o�C�X�֘A������
HRESULT InitView();             // �r���[�֘A������
HRESULT InitTraffic();          // �g���t�B�b�N�֘A������
HRESULT InitShader();           // �V�F�[�_�֘A������
HRESULT InitBuffer();           // �o�b�t�@�֘A������
HRESULT InitFence();            // �t�F���X�֘A������
VOID OnRender();                // �`��
VOID WaitForPreviousFrame();    // �t���[���㏈��
VOID OnDestroy();				// ���

// using�f�B���N�e�B�u
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// �t���[���J�E���g�͍Œ�2����(�t�����g�o�b�t�@�E�o�b�N�o�b�t�@)
static const UINT	g_frameCount = 2;

// ���_�\����
struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
};

// �p�C�v���C���I�u�W�F�N�g
ComPtr<ID3D12Device>				g_device;                           // �f�o�C�X�C���^�[�t�F�C�X
ComPtr<IDXGISwapChain3>				g_swapChain;                        // �X���b�v�`�F�C���C���^�[�t�F�C�X
ComPtr<ID3D12Resource>				g_renderTargets[g_frameCount];      // �����_�[�^�[�Q�b�g���\�[�X
ComPtr<ID3D12CommandAllocator>		g_commandAllocator;                 // �R�}���h�A���P�[�^(�R�}���h���X�g�̃������m�ۂ��Ǘ�)
ComPtr<ID3D12CommandQueue>			g_commandQueue;                     // �R�}���h�L���[(GPU�ɑ΂��ăR�}���h�o�b�t�@�̎��s�˗����s��)
ComPtr<ID3D12RootSignature>			g_rootSignature;                    // ���[�g�V�O�l�`��(�`�惊�\�[�X�̑Ή��t��)
ComPtr<ID3D12DescriptorHeap>		g_rtvHeap;                          // �f�B�X�N���v�^�q�[�v(�����_�[�^�[�Q�b�g�r���[)
ComPtr<ID3D12DescriptorHeap>		g_srvHeap;							// �f�B�X�N���v�^�q�[�v(�V�F�[�_���\�[�X�r���[)
ComPtr<ID3D12PipelineState>			g_pipelineState;                    // �p�C�v���C���X�e�[�g
ComPtr<ID3D12GraphicsCommandList>	g_commandList;                      // �R�}���h���X�g�C���^�[�t�F�C�X(�R�}���h���X�g�̐����ƊǗ�)
static UINT							g_rtvDescriptorSize = 0;            // �f�B�X�N���v�^�T�C�Y
// �p�C�v���C���X�e�[�g�Ƃ́A�`��p�C�v���C���̗�����w�肷��@�\�B
// �E�`��Ɏg�p����e��V�F�[�_�R�[�h�̐ݒ�
// �E���X�^���C�Y�̐ݒ�
// �E�u�����h���@�̐ݒ�
// �E���_���C�A�E�g�̐ݒ�
// �E�g�p����RootSignature�̐ݒ�
// �E�[�x�X�e���V���̐ݒ�
// ���̂悤�ȕ`��̈�A�̗�����w�肷��B

// �V�F�[�_�\���̈�(�r���[�|�[�g)��c�������ŕK�v�ȃE�B���h�E��Ԃ��i�[
static CD3DX12_VIEWPORT	g_viewport(0.0f, 0.0f, static_cast<FLOAT>(WINDOW_WIDTH), static_cast<FLOAT>(WINDOW_HEIGHT));
static CD3DX12_RECT		g_scissorRect(0, 0, static_cast<LONG>(WINDOW_WIDTH), static_cast<LONG>(WINDOW_HEIGHT));

// GPU�����I�u�W�F�N�g
static UINT			g_frameIndex = 0;       // �t���[���C���f�b�N�X
static HANDLE		g_fenceEvent;           // �t�F���X�n���h��
ComPtr<ID3D12Fence>	g_fence;                // �t�F���X(GPU�Ɠ������Ď��s�����҂����s��)
static UINT64		g_fenceValue;           // �t�F���X�l

// ���_�V�F�[�_���\�[�X
ComPtr<ID3D12Resource>		        g_vertexBuffer;         // ���_�o�b�t�@
static D3D12_VERTEX_BUFFER_VIEW	    g_vertexBufferView;     // ���_�o�b�t�@�r���[
ComPtr<ID3D12Resource>				g_texture;				// �e�N�X�`�����\�[�X

// �V�F�[�_�\���̈�̐��@(�r���[�|�[�g�̃A�X�y�N�g��)
static const FLOAT g_aspectRatio = static_cast<FLOAT>(WINDOW_WIDTH) / static_cast<FLOAT>(WINDOW_HEIGHT);

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

	MSG	msg = {};
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
	return static_cast<INT8>(msg.wParam);
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg) {
	case WM_DESTROY:    // �I����
		OnDestroy();    // ���
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

	// �g���t�B�b�N�֘A������
	if (FAILED(InitTraffic())) return E_FAIL;

	// �V�F�[�_�֘A������
	if (FAILED(InitShader())) return E_FAIL;

	// �o�b�t�@�֘A������
	if (FAILED(InitBuffer())) return E_FAIL;

	return S_OK;
}

// �f�o�C�X�֘A������
HRESULT InitDevice(HWND hWnd)
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// DirectX12�̃f�o�b�O���C���[��L��
	{
		ComPtr<ID3D12Debug>	debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()))))
		{
			debugController->EnableDebugLayer();

			// �ǉ��̃f�o�b�O���C���[��L��
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	// �t�@�N�g�����쐬
	ComPtr<IDXGIFactory4> factory;
	if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(factory.GetAddressOf()))))
	{
		MessageBox(NULL, L"�t�@�N�g�����쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// DirectX12���T�|�[�g���闘�p�\�ȃn�[�h�E�F�A�A�_�v�^���������擾
	HRESULT hr;
	ComPtr<IDXGIAdapter1> hardwareAdapter = nullptr;
	ComPtr<IDXGIAdapter1> adapter;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, adapter.GetAddressOf()); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 adapterDesc;
		adapter->GetDesc1(&adapterDesc);

		// �ǂ��炩��FALSE�Ȃ�FALSE�ŃX���[
		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

		// �A�_�v�^��DirectX12�ɑΉ����Ă��邩�m�F
		hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			// �f�o�C�X���쐬
			if (FAILED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(g_device.GetAddressOf()))))
			{
				MessageBox(NULL, L"�I�������n�[�h�E�F�A�f�o�C�X��DirectX12�ɑΉ����Ă��܂���B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
				return E_FAIL;
			}
			break;
		}
	}

	// �֘A�t������
	hardwareAdapter = adapter.Detach();

	// �n�[�h�E�F�A�őΉ��ł��Ȃ��ꍇ��WARP�f�o�C�X(�\�t�g�E�F�A)�̕���p����
	if (FAILED(hr))
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		factory->EnumWarpAdapter(IID_PPV_ARGS(warpAdapter.GetAddressOf()));
		if (FAILED(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(g_device.GetAddressOf()))))
		{
			MessageBox(NULL, L"�I������WARP�f�o�C�X�܂ł���DirectX12�ɑΉ����Ă��܂���B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
	}

	// �R�}���h�L���[�̐ݒ�
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// �R�}���h�L���[���쐬
	if (FAILED(g_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(g_commandQueue.GetAddressOf()))))
	{
		MessageBox(NULL, L"�R�}���h�L���[���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �X���b�v�`�F�C���̐ݒ�
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = g_frameCount;                       // �o�b�N�o�b�t�@�̐�
	swapChainDesc.Width = WINDOW_WIDTH;
	swapChainDesc.Height = WINDOW_HEIGHT;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;       // �t�����g�o�b�t�@�ƃo�b�N�o�b�t�@�̓���ւ����@
	swapChainDesc.SampleDesc.Count = 1;
	// �X���b�v�`�F�C���Ƃ́A�����_�����O���ʂ��o�͂��邽�߂̃I�u�W�F�N�g�B
	// �R�Â����r�f�I�A�_�v�^��E�B���h�E�ɑ΂��ă����_�����O���ʂ��o�͂���B

	// �X���b�v�`�F�C�����쐬
	ComPtr<IDXGISwapChain1> swapChain;
	if (FAILED(factory->CreateSwapChainForHwnd(g_commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf())))
	{
		MessageBox(NULL, L"�X���b�v�`�F�C�����쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// Alt+Enter�ɂ��S��ʑJ�ڂ��ł��Ȃ��悤�ɂ���
	if (FAILED(factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER)))
	{
		MessageBox(NULL, L"��ʂ̐ݒ肪�ł��܂���B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �X���b�v�`�F�C�����L���X�g
	swapChain.As(&g_swapChain);

	// �o�b�N�o�b�t�@�̃C���f�b�N�X���i�[
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();

	return S_OK;
}

// �r���[�֘A������
HRESULT InitView()
{
	{
		// �����_�[�^�[�Q�b�g�r���[�p�̃f�B�X�N���v�^�q�[�v�̐ݒ�
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = g_frameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		// �f�B�X�N���v�^�Ƃ́AGPU�ƃ��\�[�X�̋��n�����s�������̂��́B
		// DX11�܂ł͉B������Ă����B
		// �f�B�X�N���v�^�ɂ�3�̊T�O�����݂���B
		// Descriptor�F�e�N�X�`���Ȃǂ̃��\�[�X��GPU�ƕR�Â���B
		// DescriptorHeap�FDescriptorHeap����Descriptor���쐬����B�Ǘ��ł���Descriptor�̎�ނ␔�͎��O�Ɏw��B
		// DescriptorTable�FGPU��Ŏg�p����Descriptor�̐���z�u�𐧌䂷��BDescriptorTable��RootSignature�Őݒ肷��B

		// RTV�f�B�X�N���v�^�q�[�v���쐬
		if (FAILED(g_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(g_rtvHeap.GetAddressOf()))))
		{
			MessageBox(NULL, L"RTV�f�B�X�N���v�^�q�[�v���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}

		// �f�B�X�N���v�^�̃T�C�Y���擾
		g_rtvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// �V�F�[�_���\�[�X�r���[�p�̃f�B�X�N���v�^�q�[�v�̐ݒ�
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		// SRV�f�B�X�N���v�^�q�[�v���쐬
		if (FAILED(g_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(g_srvHeap.GetAddressOf()))))
		{
			MessageBox(NULL, L"SRV�f�B�X�N���v�^�q�[�v���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
	}
	// �f�B�X�N���v�^�q�[�v�Ƃ����̂́AGPU��ɍ����f�X�N���v�^��ۑ����邽�߂̔z��B
	// GPU��������ɑ��݂���A�l�X�ȃf�[�^��o�b�t�@�̎�ނ�ʒu�A�傫���������\���̂̂悤�Ȃ��́B
	// ���炩�̃f�[�^�z��Ƃ��ĕ\����Ă���Ƃ������ƂɂȂ�B
	// ���̂悤�ɖ����I�ɋ�؂邱�Ƃɂ���āA���̒��̍\���̂̂悤�Ȕz�񂩂�f�[�^���Q�Ƃ��₷�����Ă���B

	{
		// �t���[�����\�[�X�̃n���h�����擾
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// �t���[���o�b�t�@�ƃo�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g�r���[���쐬
		for (UINT i = 0; i < g_frameCount; i++)
		{
			// RTV�o�b�t�@���擾
			if (FAILED(g_swapChain->GetBuffer(i, IID_PPV_ARGS(g_renderTargets[i].GetAddressOf()))))
			{
				MessageBox(NULL, L"RTV�o�b�t�@���擾�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
				return E_FAIL;
			}

			// �����_�[�^�[�Q�b�g�r���[���쐬
			g_device->CreateRenderTargetView(g_renderTargets[i].Get(), nullptr, rtvHandle);

			// �n���h���̃I�t�Z�b�g
			rtvHandle.Offset(1, g_rtvDescriptorSize);
		}
	}

	return S_OK;
}

// �g���t�B�b�N�֘A������
HRESULT InitTraffic()
{
	// �R�}���h�A���P�[�^�[���쐬
	if (FAILED(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(g_commandAllocator.GetAddressOf()))))
	{
		MessageBox(NULL, L"�R�}���h�A���P�[�^���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// ���[�g�V�O�l�`�����쐬
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		// ���i�K�ŃT�|�[�g����ł������o�[�W�����B
		// CheckFeatureSupport�����������ꍇ�A�Ԃ����HighestVersion�͂�����傫���Ȃ邱�Ƃ͂Ȃ��B

		// �T�|�[�g�o�[�W�����`�F�b�N
		if (FAILED(g_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

		// �f�B�X�N���v�^��Ԃ̐ݒ�
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		// ���[�g�p�����[�^�̐ݒ�
		CD3DX12_ROOT_PARAMETER1 rootParam[1];
		rootParam[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		// �T���v���̐ݒ�
		D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.ShaderRegister = 0;
		samplerDesc.RegisterSpace = 0;
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// ���[�g�����̐ݒ�
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC vrsDesc;
		vrsDesc.Init_1_1(_countof(rootParam), rootParam, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// ���[�g�V�O�l�`�����쐬
		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3DX12SerializeVersionedRootSignature(&vrsDesc, featureData.HighestVersion, signature.GetAddressOf(), error.GetAddressOf());
		if (FAILED(g_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&g_rootSignature))))
		{
			MessageBox(NULL, L"���[�g�V�O�l�`�����쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		// ���[�g�V�O�l�`���Ƃ́A���[�g�����Ƃ������Ƃŕ`�惊�\�[�X�̑Ή��t�����s���Ă���B
		// �`�惊�\�[�X�Ƃ͒萔�o�b�t�@�A�e�N�X�`���A�T���v���Ƃ��̂��ƁB
		// �������V�F�[�_�ɓn���Ă���B
		// �������̂��߂ɂ̓v���O�������Ń��������C�A�E�g���œK��������K�v������炵���B
	}

	return S_OK;
}

// �V�F�[�_�֘A������
HRESULT InitShader()
{
	// �V�F�[�_�̃R���p�C���ƃ��[�h���܂ރp�C�v���C���X�e�[�g���쐬
	{
		ComPtr<ID3DBlob> vertexShader, pixelShader;

#if defined(_DEBUG)
		// �O���t�B�b�N�f�o�b�O�c�[���ɂ��V�F�[�_�[�̃f�o�b�O��L��
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		// �����̃V�F�[�_�����[�h���R���p�C��
		if (FAILED(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, vertexShader.GetAddressOf(), nullptr)))
		{
			MessageBox(NULL, L"���_�V�F�[�_��ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		if (FAILED(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, pixelShader.GetAddressOf(), nullptr)))
		{
			MessageBox(NULL, L"�s�N�Z���V�F�[�_��ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}

		// ���_�C���v�b�g���C�A�E�g���`
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};

		// �O���t�B�b�N�p�C�v���C���X�e�[�g�̐ݒ�
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
		gpsDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		gpsDesc.pRootSignature = g_rootSignature.Get();
		gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		gpsDesc.DepthStencilState.DepthEnable = FALSE;
		gpsDesc.DepthStencilState.StencilEnable = FALSE;
		gpsDesc.SampleMask = UINT_MAX;
		gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		gpsDesc.NumRenderTargets = 1;
		gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		gpsDesc.SampleDesc.Count = 1;

		// �O���t�B�b�N�p�C�v���C���X�e�[�g���쐬
		if (FAILED(g_device->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(g_pipelineState.GetAddressOf()))))
		{
			MessageBox(NULL, L"�O���t�B�b�N�p�C�v���C���X�e�[�g���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
	}

	return S_OK;
}

// �o�b�t�@�֘A������
HRESULT InitBuffer()
{
	// �R�}���h���X�g���쐬
	if (FAILED(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator.Get(), nullptr, IID_PPV_ARGS(g_commandList.GetAddressOf()))))
	{
		MessageBox(NULL, L"�R�}���h���X�g���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// ���_�o�b�t�@���쐬
	{
		/*
		// �E�B���h�E�̉��̒������ő�Ƃ���0.25�{�l�p�`�̃W�I���g�����`
		Vertex vertices[] =
		{
			{{-0.25f,  0.25f * g_aspectRatio, 0.0f}, {0.0f, 0.0f}},
			{{ 0.25f,  0.25f * g_aspectRatio, 0.0f}, {1.0f, 0.0f}},
			{{-0.25f, -0.25f * g_aspectRatio, 0.0f}, {0.0f, 1.0f}},
			{{ 0.25f, -0.25f * g_aspectRatio, 0.0f}, {1.0f, 1.0f}},
		};
		*/

		// ��ʑS�̕\��
		Vertex vertices[] =
		{
			{{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}},
			{{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}},
			{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
			{{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
		};

		// ���_�o�b�t�@�T�C�Y
		const UINT vertexBufferSize = sizeof(vertices);

		// �R�~�b�g���\�[�X���쐬
		if (FAILED(g_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(g_vertexBuffer.GetAddressOf()))))
		{
			MessageBox(NULL, L"���_�o�b�t�@�R�~�b�g���\�[�X���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		// �R�~�b�g���\�[�X�Ƃ����q�[�v�𗘗p���ăV�F�[�_�ɃA�b�v���[�h����B

		// ���_�o�b�t�@�ɒ��_�f�[�^���R�s�[
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// CPU��̂��̃��\�[�X����o�b�t�@��ǂݍ��܂Ȃ��ݒ�
		g_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		memcpy(pVertexDataBegin, vertices, sizeof(vertices));
		g_vertexBuffer->Unmap(0, nullptr);

		// ���_�o�b�t�@�r���[��������
		g_vertexBufferView.BufferLocation = g_vertexBuffer->GetGPUVirtualAddress();
		g_vertexBufferView.StrideInBytes = sizeof(Vertex);
		g_vertexBufferView.SizeInBytes = vertexBufferSize;
	}

	// �e�N�X�`���A�b�v���[�h�q�[�v���m��
	ComPtr<ID3D12Resource> textureUploadHeap;
	// ComPtr��CPU�I�u�W�F�N�g�����A���̃��\�[�X�͂�����Q�Ƃ���R�}���h���X�g��GPU�ł̎��s����������܂ŃX�R�[�v���ɗ��܂�K�v������B
	// ���̃��\�b�h�̍Ō��GPU���t���b�V�����āA���\�[�X�������ɔj�󂳂�Ȃ��悤�ɂ���B

	// �e�N�X�`�����쐬
	{
		// DDS�e�N�X�`���̓ǂݍ���
		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresouceData;
		if (FAILED(LoadDDSTextureFromFile(g_device.Get(), L"texture.dds", g_texture.GetAddressOf(), ddsData, subresouceData)))
		{
			MessageBox(NULL, L"�e�N�X�`����ǂݍ��߂܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}

		// �e�N�X�`�����\�[�X�̐ݒ�ɑ΂��ăe�N�X�`�����\�[�X����
		D3D12_RESOURCE_DESC textureDesc = g_texture->GetDesc();

		// �T�C�Y���擾
		const UINT subresoucesize = static_cast<UINT>(subresouceData.size());
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(g_texture.Get(), 0, subresoucesize);

		// GPU�A�b�v���[�h�o�b�t�@���쐬
		if (FAILED(g_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(textureUploadHeap.GetAddressOf()))))
		{
			MessageBox(NULL, L"GPU�A�b�v���[�h�o�b�t�@���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}

		// �e�N�X�`���f�[�^�X�V
		UpdateSubresources(g_commandList.Get(), g_texture.Get(), textureUploadHeap.Get(), 0, 0, subresoucesize, &subresouceData[0]);
		g_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		// �V�F�[�_���\�[�X�r���[�̐ݒ�
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = subresoucesize;

		// �V�F�[�_���\�[�X�r���[���쐬
		g_device->CreateShaderResourceView(g_texture.Get(), &srvDesc, g_srvHeap->GetCPUDescriptorHandleForHeapStart());

		// ���\�[�X���A�����[�h
		g_commandList->DiscardResource(textureUploadHeap.Get(), nullptr);
	}

	// �R�}���h���X�g����Ď��s������GPU�Z�b�g�A�b�v���J�n
	g_commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { g_commandList.Get() };
	g_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// �t�F���X�֘A������
	if (FAILED(InitFence())) return E_FAIL;
	// InitBuffer��S_OK�̖߂�l��Ԃ��O�Ƀt�F���X���쐬���t���[���҂����s���K�v������B
	// �łȂ��ƁA�R�}���h���X�g�ɋL�^�������̂����s���邽�߂̏���GPU�Z�b�g�A�b�v��ێ��ł��Ȃ��̂ŗ�O����������B

	return S_OK;
}

// �t�F���X�֘A������
HRESULT InitFence()
{
	{
		// �t�F���X���쐬
		if (FAILED(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(g_fence.GetAddressOf()))))
		{
			MessageBox(NULL, L"�t�F���X���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		// �t�F���X�Ƃ́A�����I�u�W�F�N�g�Ƃ��ă��\�[�X��GPU�ɃA�b�v���[�h�����܂őҋ@������́B

		// �t�F���X�l��1�ɐݒ�
		g_fenceValue = 1;

		// �t���[�������Ɏg�p����C�x���g�n���h�����쐬
		g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (g_fenceEvent == nullptr)
		{
			MessageBox(NULL, L"�t�F���X�C�x���g�n���h�����쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
		// �҂ۂ�Windows��Event�𗘗p

		// �t���[���㏈��
		WaitForPreviousFrame();
		// �R�}���h���X�g�����s�����̂�҂B
		// ���C�����[�v�œ����R�}���h���X�g���ė��p���Ă��邪�A
		// ���̂Ƃ���́A�Z�b�g�A�b�v����������̂�҂��Ă��瑱�s����B
	}

	return S_OK;
}

// �`�揈��
void OnRender()
{
	// �R�}���h�A���P�[�^�����Z�b�g
	g_commandAllocator->Reset();

	// �R�}���h���X�g�����Z�b�g
	g_commandList->Reset(g_commandAllocator.Get(), g_pipelineState.Get());

	// �R�}���h���X�g�ɃV�F�[�_�\���̈�(�r���[�|�[�g)��c�������ŕK�v�ȃE�B���h�E��Ԃ�ݒ�
	g_commandList->SetGraphicsRootSignature(g_rootSignature.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { g_srvHeap.Get() };
	g_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	g_commandList->SetGraphicsRootDescriptorTable(0, g_srvHeap->GetGPUDescriptorHandleForHeapStart());
	g_commandList->RSSetViewports(1, &g_viewport);
	g_commandList->RSSetScissorRects(1, &g_scissorRect);

	// �o�b�N�o�b�t�@�������_�[�^�[�Q�b�g�Ƃ��Ďg�p
	g_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	// ���\�[�X�o���A�Ƃ́AGPU���ň������\�[�X�̏󋵂𓯊�������@�\�B
	// �}���`�X���b�h��O��Ƃ��������Ȃ̂ŁAGPU���̓���������̃A�N�Z�X�������ɍs���邱�Ƃ�z�肵���@�\���Ƃ������ƁB

	// �����_�[�^�[�Q�b�g�r���[�̃n���h�����쐬
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart(), g_frameIndex, g_rtvDescriptorSize);

	// �����_�[�^�[�Q�b�g���Z�b�g
	g_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// �o�b�N�o�b�t�@�ɕ`��(�R�}���h���L�^����)
	const FLOAT	clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };      // ���ۂ��F
	g_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	g_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	g_commandList->IASetVertexBuffers(0, 1, &g_vertexBufferView);
	g_commandList->DrawInstanced(4, 2, 0, 0);

	// �o�b�N�o�b�t�@��\��
	g_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// �R�}���h���X�g���N���[�Y
	g_commandList->Close();

	// �R�}���h���X�g�����s
	ID3D12CommandList* ppCommandLists[] = { g_commandList.Get() };
	g_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// �t���[�����ŏI�o��
	g_swapChain->Present(1, 0);

	// �t���[���㏈��
	WaitForPreviousFrame();
}

// �t���[���㏈��
VOID WaitForPreviousFrame()
{
	const UINT64 fence = g_fenceValue;
	g_commandQueue->Signal(g_fence.Get(), fence);
	g_fenceValue++;

	// �O�̃t���[�����I������܂őҋ@
	if (g_fence->GetCompletedValue() < fence)
	{
		g_fence->SetEventOnCompletion(fence, g_fenceEvent);
		WaitForSingleObject(g_fenceEvent, INFINITE);
	}

	// �o�b�N�o�b�t�@�̃C���f�b�N�X���i�[
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();
}
// �R�}���h�����𔻒f���҂�����������֐��B

// ���
VOID OnDestroy()
{
	WaitForPreviousFrame();
	CloseHandle(g_fenceEvent);
}
