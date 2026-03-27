#include "Graphics.h"
#include <iostream>
#include <iomanip>

#pragma comment(lib, "d3d11.lib")

bool Graphics::Initialize(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.BufferDesc.Width = 0;
    scd.BufferDesc.Height = 0;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        nullptr, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &context);

    if (FAILED(hr) && (createDeviceFlags & D3D11_CREATE_DEVICE_DEBUG)) {
        std::cout << "[AVISO] Falha ao iniciar DX11 com Debug Layer. Tentando modo normal..." << std::endl;
        createDeviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
            nullptr, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &context);
    }

    if (FAILED(hr)) {
        std::cerr << "[ERRO] D3D11CreateDeviceAndSwapChain falhou com HRESULT: 0x" 
                  << std::hex << std::setw(8) << std::setfill('0') << hr << std::endl;
        return false;
    }

    ComPtr<ID3D11Texture2D> backBuffer;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr)) return false;

    hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);
    if (FAILED(hr)) return false;

    context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);

    RECT rc;
    GetClientRect(hWnd, &rc);
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)(rc.right - rc.left);
    vp.Height = (FLOAT)(rc.bottom - rc.top);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context->RSSetViewports(1, &vp);

    return true;
}

void Graphics::ClearScreen(float r, float g, float b) {
    float color[4] = { r, g, b, 1.0f };
    context->ClearRenderTargetView(renderTargetView.Get(), color);
}

void Graphics::Present() {
    swapChain->Present(0, 0); 
}