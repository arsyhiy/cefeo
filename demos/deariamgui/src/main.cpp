#include <windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <vector>
#include <cstring>
#include <stdexcept>
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

static const int NUM_FRAMES_IN_FLIGHT = 3;
static const int NUM_BACK_BUFFERS = 3;

HWND g_hWnd = nullptr;
UINT g_Width = 1280, g_Height = 800;

ComPtr<ID3D12Device> g_pd3dDevice;
ComPtr<IDXGISwapChain3> g_pSwapChain;
ComPtr<ID3D12CommandQueue> g_pd3dCommandQueue;
ComPtr<ID3D12DescriptorHeap> g_pd3dRtvDescHeap;
ComPtr<ID3D12DescriptorHeap> g_pd3dSrvDescHeap;
ComPtr<ID3D12CommandAllocator> g_pd3dCommandAllocators[NUM_FRAMES_IN_FLIGHT];
ComPtr<ID3D12GraphicsCommandList> g_pd3dCommandList;
ComPtr<ID3D12Fence> g_fence;
UINT64 g_fenceLastSignaledValue = 0;
HANDLE g_fenceEvent = nullptr;
UINT g_rtvDescriptorSize = 0;
UINT g_frameIndex = 0;
ComPtr<ID3D12Resource> g_mainRenderTargetResource[NUM_BACK_BUFFERS];
D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS];

// REQUIRED: declare this as extern, per new ImGui backend policy!
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            WaitForLastSubmittedFrame();
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam),
                DXGI_FORMAT_UNKNOWN, 0);
            g_frameIndex = g_pSwapChain->GetCurrentBackBufferIndex();
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

static void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) { throw std::runtime_error("HRESULT failed!"); }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      _T("ImGuiDX12App"), NULL };
    ::RegisterClassEx(&wc);
    g_hWnd = ::CreateWindow(wc.lpszClassName, _T("ImGui DX12"), WS_OVERLAPPEDWINDOW, 100, 100, g_Width, g_Height,
                            NULL, NULL, wc.hInstance, NULL);

    if (!CreateDeviceD3D(g_hWnd))
    {
        MessageBoxA(nullptr, "Failed to create D3D12 device!", "Fatal Error", MB_ICONERROR | MB_OK);
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(g_hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(g_hWnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(g_hWnd)) {
        MessageBoxA(nullptr, "ImGui_ImplWin32_Init failed!", "Fatal Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = g_pd3dDevice.Get();
    init_info.NumFramesInFlight = NUM_FRAMES_IN_FLIGHT;
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    init_info.SrvDescriptorHeap = g_pd3dSrvDescHeap.Get();
    init_info.CommandQueue = g_pd3dCommandQueue.Get();
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

    if (!ImGui_ImplDX12_Init(&init_info))
    {
        MessageBoxA(nullptr, "ImGui_ImplDX12_Init failed!", "Fatal Error", MB_ICONERROR | MB_OK);
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Надёжный буфер редактора: std::vector<char>, всегда нуль-терминирован!
    std::vector<char> editor_buffer(4096, 0);
    const char* hello = "// Добро пожаловать!\n";
    std::strncpy(editor_buffer.data(), hello, editor_buffer.size() - 1);

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Редактор");
        // Размер всегда >= strlen+1, всегда есть \0
        ImGui::InputTextMultiline("##editor", editor_buffer.data(), editor_buffer.size(), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight()*30));
        ImGui::End();

        ImGui::Render();

        g_pd3dCommandAllocators[g_frameIndex]->Reset();
        g_pd3dCommandList->Reset(g_pd3dCommandAllocators[g_frameIndex].Get(), NULL);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = g_mainRenderTargetResource[g_frameIndex].Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_pd3dCommandList->ResourceBarrier(1, &barrier);

        const float clear_color[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[g_frameIndex], FALSE, NULL);
        g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[g_frameIndex], clear_color, 0, NULL);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList.Get());

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_pd3dCommandList->Close();

        ID3D12CommandList* commandLists[] = { g_pd3dCommandList.Get() };
        g_pd3dCommandQueue->ExecuteCommandLists(1, commandLists);

        g_pSwapChain->Present(1, 0);

        g_frameIndex = g_pSwapChain->GetCurrentBackBufferIndex();

        WaitForLastSubmittedFrame();
    }

    WaitForLastSubmittedFrame();
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(g_hWnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// --- DX12 RENDER FUNCTIONS BELOW ---

bool CreateDeviceD3D(HWND hWnd)
{
    UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    ThrowIfFailed(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&g_pd3dDevice)));

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;
        ThrowIfFailed(g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)));
    }

    {
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.BufferCount = NUM_BACK_BUFFERS;
        desc.Width = g_Width;
        desc.Height = g_Height;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.Stereo = FALSE;

        ComPtr<IDXGISwapChain1> swapChain1;
        ThrowIfFailed(factory->CreateSwapChainForHwnd(g_pd3dCommandQueue.Get(), hWnd, &desc,
            nullptr, nullptr, &swapChain1));
        ThrowIfFailed(swapChain1.As(&g_pSwapChain));
        g_frameIndex = g_pSwapChain->GetCurrentBackBufferIndex();
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = NUM_BACK_BUFFERS;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        ThrowIfFailed(g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)));

        g_rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            g_mainRenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += g_rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)));
    }

    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        ThrowIfFailed(g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&g_pd3dCommandAllocators[i])));

    ThrowIfFailed(g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        g_pd3dCommandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&g_pd3dCommandList)));
    ThrowIfFailed(g_pd3dCommandList->Close());

    ThrowIfFailed(g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)));

    g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (g_fenceEvent == nullptr)
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));

    CreateRenderTarget();
    return true;
}

void CreateRenderTarget()
{
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        ThrowIfFailed(g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_mainRenderTargetResource[i])));
        g_pd3dDevice->CreateRenderTargetView(g_mainRenderTargetResource[i].Get(), nullptr,
            g_mainRenderTargetDescriptor[i]);
    }
}

void CleanupRenderTarget()
{
    WaitForLastSubmittedFrame();
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        if (g_mainRenderTargetResource[i]) g_mainRenderTargetResource[i].Reset();
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) g_pSwapChain.Reset();
    if (g_pd3dCommandQueue) g_pd3dCommandQueue.Reset();
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_pd3dCommandAllocators[i]) g_pd3dCommandAllocators[i].Reset();
    if (g_pd3dCommandList) g_pd3dCommandList.Reset();
    if (g_pd3dRtvDescHeap) g_pd3dRtvDescHeap.Reset();
    if (g_pd3dSrvDescHeap) g_pd3dSrvDescHeap.Reset();
    if (g_fence) g_fence.Reset();
    if (g_fenceEvent) CloseHandle(g_fenceEvent);
    if (g_pd3dDevice) g_pd3dDevice.Reset();
}

void WaitForLastSubmittedFrame()
{
    UINT64 fenceValue = g_fenceLastSignaledValue;
    if (fenceValue == 0)
        return;

    g_fenceLastSignaledValue++;
    if (g_fence->GetCompletedValue() >= fenceValue)
        return;

    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}
