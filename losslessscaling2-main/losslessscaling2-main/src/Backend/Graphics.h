#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class Graphics {
public:
    Graphics() = default;
    ~Graphics() = default;

    bool Initialize(HWND hWnd);
    void ClearScreen(float r, float g, float b);
    void Present();

    ID3D11Device* GetDevice() const { return device.Get(); }
    ID3D11DeviceContext* GetContext() const { return context.Get(); }

private:
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11RenderTargetView> renderTargetView;
};