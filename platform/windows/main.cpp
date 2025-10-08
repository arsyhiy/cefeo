#include <windows.h>
#include <d3d11.h>
#include <iostream>
#include <fstream>
#include "editor.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

// DirectX objects
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

Editor* editor = nullptr;
HWND hwnd = nullptr; // Declare hwnd globally

// Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}





void openFile(Editor* editor) {
    OPENFILENAME ofn;
    char szFile[260];

    // File open dialog setup
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text files\0*.TXT\0All files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile[0] = '\0';
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = "Open File";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        FILE* file = fopen(ofn.lpstrFile, "r");
        if (file == nullptr) {
            std::cerr << "Failed to open file!" << std::endl;
            return;
        }

        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Initialize buffer
        editor->buffer.data = (char*)realloc(editor->buffer.data, fileSize + 1);
        if (editor->buffer.data == nullptr) {
            std::cerr << "Memory allocation failed!" << std::endl;
            fclose(file);
            return;
        }

        fread(editor->buffer.data, 1, fileSize, file);
        editor->buffer.data[fileSize] = '\0';
        editor->buffer.size = fileSize;
        fclose(file);
    }
}

void saveFile(Editor* editor) {
    OPENFILENAME ofn;
    char szFile[260];

    // File save dialog setup
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text files\0*.TXT\0All files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile[0] = '\0';
    ofn.lpstrTitle = "Save File";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE) {
        FILE* file = fopen(ofn.lpstrFile, "w");
        if (file == nullptr) {
            std::cerr << "Failed to open file for saving!" << std::endl;
            return;
        }

        fwrite(editor->buffer.data, 1, editor->buffer.size, file);
        fclose(file);
    }
}





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // Create window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0, 0,
                      GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
                      "Cefeo", nullptr };
    RegisterClassEx(&wc);
    hwnd = CreateWindow(wc.lpszClassName, "Cefeo Editor", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720,
        nullptr, nullptr, wc.hInstance, nullptr);

    // DirectX 11 setup
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = 1280;
    scd.BufferDesc.Height = 720;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &scd, &g_pSwapChain, &g_pd3dDevice, nullptr, &g_pd3dDeviceContext);

    CreateRenderTarget();

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // Create editor
    editor = new Editor();
    if (editor == nullptr) {
        std::cout << "Editor initialization failed!" << std::endl;
        return -1;
    }
    initEditor(editor);

    // Main loop
    MSG msg;
    bool done = false;
    while (!done) {
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Rendering editor
        ImGui::Begin("Cefeo Editor");

        if (ImGui::Button("Open File")) {
            openFile(editor);
        }

        if (ImGui::Button("Save File")) {
            saveFile(editor);
        }

        if (editor != nullptr && editor->buffer.data != nullptr) {
            ImGui::InputTextMultiline("##editor", editor->buffer.data, editor->buffer.size, ImVec2(-FLT_MIN, -FLT_MIN));
        }
        else {
            std::cout << "Null pointer detected!" << std::endl;
        }

        ImGui::End();

        // Render
        ImGui::Render();
        const float clear_color[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    // Cleanup
    freeEditor(editor);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupRenderTarget();
    g_pSwapChain->Release();
    g_pd3dDeviceContext->Release();
    g_pd3dDevice->Release();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
