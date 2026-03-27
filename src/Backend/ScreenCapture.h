#pragma once
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr;

enum class CaptureResult {
    Success,
    NoUpdate,
    Error,
    AccessLost
};

class ScreenCapture {
public:
    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    CaptureResult AcquireNextFrame();
    CaptureResult CaptureWindowFast();

    bool InitializeWindowCapture(ID3D11Device* device, ID3D11DeviceContext* context, HWND targetHWnd);
    CaptureResult CaptureWindow();

    ID3D11Texture2D* GetTexture() const { return m_capturedTexture.Get(); }
    UINT GetWidth() const { return width; }
    UINT GetHeight() const { return height; }

private:
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGIOutputDuplication> m_deskDupl;
    ComPtr<ID3D11Texture2D> m_capturedTexture;
    
    HWND m_targetHWnd = nullptr;
    UINT width = 0;
    UINT height = 0;
};