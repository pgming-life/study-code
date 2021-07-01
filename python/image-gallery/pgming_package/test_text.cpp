#include <windows.h>
#include <wrl.h>

// DirectX12�̃R�[�h�Z�b�g
#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"

// ���C�u����
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define WINDOW_CLASS	L"������(�u���[���)"
#define WINDOW_TITLE	WINDOW_CLASS
#define WINDOW_WIDTH	750
#define WINDOW_HEIGHT	500

// �v���g�^�C�v�錾
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // �E�B���h�E�v���V�[�W��
HRESULT OnInit(HWND hWnd);      // ������
VOID OnRender();                // �`��
VOID WaitForPreviousFrame();    // �t���[���㏈��
VOID OnDestroy();				// ���

// using�f�B���N�e�B�u
using Microsoft::WRL::ComPtr;
// ComPtr�̓X�}�[�g�|�C���^�Ȃ̂ł��������������������K�v���Ȃ��B

// �t���[���J�E���g�͍Œ�2����(�t�����g�o�b�t�@�E�o�b�N�o�b�t�@)
static const UINT g_frameCount = 2;

// �p�C�v���C���I�u�W�F�N�g
ComPtr<ID3D12Device>				g_device;                           // �f�o�C�X�C���^�[�t�F�C�X
ComPtr<IDXGISwapChain3>				g_swapChain;                        // �X���b�v�`�F�C���C���^�[�t�F�C�X
ComPtr<ID3D12Resource>				g_renderTargets[g_frameCount];      // �����_�[�^�[�Q�b�g���\�[�X
ComPtr<ID3D12CommandAllocator>		g_commandAllocator;                 // �R�}���h�A���P�[�^(�R�}���h���X�g�̃������m�ۂ��Ǘ�)
ComPtr<ID3D12CommandQueue>			g_commandQueue;                     // �R�}���h�L���[(GPU�ɑ΂��ăR�}���h�o�b�t�@�̎��s�˗����s��)
ComPtr<ID3D12DescriptorHeap>		g_rtvHeap;                          // �f�B�X�N���v�^�q�[�v(�����_�[�^�[�Q�b�g�r���[)
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

// GPU�����I�u�W�F�N�g
static UINT			g_frameIndex = 0;       // �t���[���C���f�b�N�X
static HANDLE		g_fenceEvent;           // �t�F���X�n���h��
ComPtr<ID3D12Fence>	g_fence;                // �t�F���X(GPU�Ɠ������Ď��s�����҂����s��)
static UINT64		g_fenceValue;           // �t�F���X�l

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

		// �f�B�X�N���v�^�q�[�v���쐬
		if (FAILED(g_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(g_rtvHeap.GetAddressOf()))))
		{
			MessageBox(NULL, L"�f�B�X�N���v�^�q�[�v���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
			return E_FAIL;
		}

		// �f�B�X�N���v�^�̃T�C�Y���擾
		g_rtvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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

	// �R�}���h�A���P�[�^�[���쐬
	if (FAILED(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(g_commandAllocator.GetAddressOf()))))
	{
		MessageBox(NULL, L"�R�}���h�A���P�[�^���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �R�}���h���X�g���쐬
	if (FAILED(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator.Get(), nullptr, IID_PPV_ARGS(g_commandList.GetAddressOf()))))
	{
		MessageBox(NULL, L"�R�}���h���X�g���쐬�ł��܂���ł����B", WINDOW_TITLE, MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	// �R�}���h���X�g���N���[�Y
	g_commandList->Close();
	// �R�}���h���X�g�͋L�^��Ԃō쐬����邪�A����͏��������ł����ɉ�������Ȃ��̂ł����ɕ���B

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
	}

	return S_OK;
}

// �`��
VOID OnRender()
{
	// �R�}���h�A���P�[�^�����Z�b�g
	g_commandAllocator->Reset();

	// �R�}���h���X�g�����Z�b�g
	g_commandList->Reset(g_commandAllocator.Get(), g_pipelineState.Get());

	// �o�b�N�o�b�t�@�������_�[�^�[�Q�b�g�Ƃ��Ďg�p
	g_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	// ���\�[�X�o���A�Ƃ́AGPU���ň������\�[�X�̏󋵂𓯊�������@�\�B
	// �}���`�X���b�h��O��Ƃ��������Ȃ̂ŁAGPU���̓���������̃A�N�Z�X�������ɍs���邱�Ƃ�z�肵���@�\���Ƃ������ƁB

	// �����_�[�^�[�Q�b�g�r���[�̃n���h�����쐬
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart(), g_frameIndex, g_rtvDescriptorSize);

	// �o�b�N�o�b�t�@�ɕ`��(�R�}���h���L�^)
	const FLOAT	clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };		// ���ۂ��F
	g_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

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
	if (g_fence->GetCompletedValue() < fence) {
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
