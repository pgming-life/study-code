#include <windows.h>
#include <wrl.h>
#include <vector>

// DirectX11�̃R�[�h�Z�b�g
#include <d3d11.h>
#include <directxmath.h>

// ���C�u����
#pragma comment(lib, "d3d11.lib")

#define WINDOW_CLASS    L"�u���[��ʂŏ�����"
#define WINDOW_TITLE    WINDOW_CLASS
#define WINDOW_WIDTH    750
#define WINDOW_HEIGHT   500

// �v���g�^�C�v�錾
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);    // �E�B���h�E�v���V�[�W��
HRESULT OnInit(HWND hWnd);      // ������
void OnRender();                // �`��

// using�f�B���N�e�B�u
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// �p�C�v���C���I�u�W�F�N�g
ComPtr<ID3D11Device>            g_device;
ComPtr<ID3D11DeviceContext>     g_context;
ComPtr<IDXGISwapChain>          g_swapChain;
ComPtr<ID3D11RenderTargetView>  g_renderTargetView;

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

    // �����_�[�^�[�Q�b�g�r���[���w�肵���F�ŃN���A
    FLOAT clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };       // �u���[
    g_context->ClearRenderTargetView(g_renderTargetView.Get(), clearColor);

    return S_OK;
}

// �`��
void OnRender()
{
    // �t���[�����ŏI�o��
    g_swapChain->Present(0, 0);     // �t���b�v
}
