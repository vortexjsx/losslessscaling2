#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <wrl/client.h>
#include <windows.h>
#include "UI/Window.h"
#include "UI/WindowPicker.h"
#include "Backend/Graphics.h"
#include "Backend/ScreenCapture.h"
#include "Backend/FrameGenerator.h"
#include "Backend/Renderer.h"
#include <thread>

using Microsoft::WRL::ComPtr;

#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

void CreateTexture(ID3D11Device* device, UINT w, UINT h, ComPtr<ID3D11Texture2D>& tex, ComPtr<ID3D11ShaderResourceView>& srv, ComPtr<ID3D11UnorderedAccessView>& uav) {
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

    device->CreateTexture2D(&desc, nullptr, &tex);
    device->CreateShaderResourceView(tex.Get(), nullptr, &srv);
    device->CreateUnorderedAccessView(tex.Get(), nullptr, &uav);
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    std::cout << "        PICA SCALING      " << std::endl;

    auto windows = WindowPicker::FindWindows();
    if (windows.empty()) {
        std::cout << "Nenhuma janela encontrada!" << std::endl;
        return -1;
    }

    for (size_t i = 0; i < windows.size(); i++) {
        std::wstring ws = windows[i].title;
        std::string s(ws.begin(), ws.end());
        std::cout << i << ": " << s << std::endl;
    }

    std::cout << "\nSelecione o numero da janela: ";
    size_t choice;
    std::cin >> choice;
    if (choice >= windows.size()) return -1;
    HWND targetHWnd = windows[choice].hwnd;

    AppWindow ui;
    ui.Create(1280, 720, GetModuleHandle(nullptr));
    SetWindowDisplayAffinity(ui.GetHWND(), WDA_EXCLUDEFROMCAPTURE);

    auto graphics = std::make_unique<Graphics>();
    graphics->Initialize(ui.GetHWND());

    auto capture = std::make_unique<ScreenCapture>();
    capture->InitializeWindowCapture(graphics->GetDevice(), graphics->GetContext(), targetHWnd);

    auto frameGen = std::make_unique<FrameGenerator>();
    frameGen->Initialize(graphics->GetDevice());

    auto renderer = std::make_unique<Renderer>();
    renderer->Initialize(graphics->GetDevice());

    UINT width = capture->GetWidth();
    UINT height = capture->GetHeight();
    ComPtr<ID3D11Texture2D> frameATex, frameBTex, generatedTex;
    ComPtr<ID3D11ShaderResourceView> frameASRV, frameBSRV, generatedSRV;
    ComPtr<ID3D11UnorderedAccessView> generatedUAV;

    CreateTexture(graphics->GetDevice(), width, height, frameATex, frameASRV, ComPtr<ID3D11UnorderedAccessView>());
    CreateTexture(graphics->GetDevice(), width, height, frameBTex, frameBSRV, ComPtr<ID3D11UnorderedAccessView>());
    CreateTexture(graphics->GetDevice(), width, height, generatedTex, generatedSRV, generatedUAV);

    std::cout << "[LOG] Rodando!" << std::endl;

    std::chrono::time_point<std::chrono::high_resolution_clock> frameStartTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> frameEndTime;

const double targetFPS = 120.0;
const double interval = 1000.0 / targetFPS;
auto nextFrameTime = std::chrono::high_resolution_clock::now();

    bool hasFirstFrame = false;
    int fpsCounter = 0;
    auto lastFpsUpdate = std::chrono::high_resolution_clock::now();


while (ui.IsOpen()) {
        ui.ProcessMessages();

        if (IsIconic(targetHWnd)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        RECT gameRect;
        if (GetClientRect(targetHWnd, &gameRect)) {
            POINT pt = { 0, 0 };
            ClientToScreen(targetHWnd, &pt);
            
            SetWindowPos(ui.GetHWND(), HWND_TOPMOST, 
                         pt.x, pt.y, 
                         gameRect.right - gameRect.left, gameRect.bottom - gameRect.top, 
                         SWP_NOACTIVATE);
        }

        if (capture->CaptureWindowFast() == CaptureResult::Success) {
            if (!hasFirstFrame) {
                graphics->GetContext()->CopyResource(frameATex.Get(), capture->GetTexture());
                hasFirstFrame = true;
            } else {
                graphics->GetContext()->CopyResource(frameBTex.Get(), capture->GetTexture());

                frameGen->GenerateFrame(graphics->GetContext(), frameASRV.Get(), frameBSRV.Get(), generatedUAV.Get(), width, height);

                renderer->RenderToScreen(graphics->GetContext(), generatedSRV.Get());
                graphics->Present(); 
                fpsCounter++;

                renderer->RenderToScreen(graphics->GetContext(), frameBSRV.Get());
                graphics->Present(); 
                fpsCounter++;

                graphics->GetContext()->CopyResource(frameATex.Get(), frameBTex.Get());
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = now - lastFpsUpdate;
        if (elapsed.count() >= 1.0f) {
            std::cout << "Pica Scaling | FPS Gerado: " << fpsCounter << " | Resolucao: " << width << "x" << height << "\r"; // \r reescreve a mesma linha
            fpsCounter = 0;
            lastFpsUpdate = now;
        }
    }

    return 0;
}