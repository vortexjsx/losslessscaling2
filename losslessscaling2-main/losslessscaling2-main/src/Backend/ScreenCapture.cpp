#include "ScreenCapture.h"

bool ScreenCapture::Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
    m_device = device;
    m_context = context;
    ComPtr<IDXGIDevice> dxgiDevice;
    if (FAILED(device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice))) return false;
    ComPtr<IDXGIAdapter> dxgiAdapter;
    if (FAILED(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter))) return false;
    ComPtr<IDXGIOutput> dxgiOutput;
    if (FAILED(dxgiAdapter->EnumOutputs(0, &dxgiOutput))) return false;
    ComPtr<IDXGIOutput1> dxgiOutput1;
    if (FAILED(dxgiOutput.As(&dxgiOutput1))) return false;
    if (FAILED(dxgiOutput1->DuplicateOutput(device, &m_deskDupl))) return false;
    DXGI_OUTDUPL_DESC duplDesc;
    m_deskDupl->GetDesc(&duplDesc);
    width = duplDesc.ModeDesc.Width;
    height = duplDesc.ModeDesc.Height;
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_capturedTexture))) return false;
    return true;
}


CaptureResult ScreenCapture::AcquireNextFrame() {
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    ComPtr<IDXGIResource> desktopResource;

    HRESULT hr = m_deskDupl->AcquireNextFrame(16, &frameInfo, &desktopResource);

    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        return CaptureResult::NoUpdate;
    }
    if (hr == DXGI_ERROR_ACCESS_LOST) {
        return CaptureResult::Error;
    }
    if (FAILED(hr)) {
        return CaptureResult::Error; 
    }

    ComPtr<ID3D11Texture2D> desktopTexture;
    if (SUCCEEDED(desktopResource.As(&desktopTexture))) {
        m_context->CopyResource(m_capturedTexture.Get(), desktopTexture.Get());
    }

    m_deskDupl->ReleaseFrame();
    return CaptureResult::Success;
}

bool ScreenCapture::InitializeWindowCapture(ID3D11Device* device, ID3D11DeviceContext* context, HWND targetHWnd) {
    m_device = device;
    m_context = context;
    m_targetHWnd = targetHWnd;

    RECT rect;
    GetClientRect(targetHWnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    width = (width / 8) * 8;
    height = (height / 8) * 8;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    return SUCCEEDED(device->CreateTexture2D(&desc, nullptr, &m_capturedTexture));
}

CaptureResult ScreenCapture::CaptureWindow() {
    if (!m_targetHWnd || !IsWindow(m_targetHWnd)) return CaptureResult::Error;

    HDC hdcWindow = GetDC(m_targetHWnd);
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP hbm = CreateCompatibleBitmap(hdcWindow, width, height);
    SelectObject(hdcMem, hbm);

    PrintWindow(m_targetHWnd, hdcMem, PW_RENDERFULLCONTENT);

    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), (LONG)width, -(LONG)height, 1, 32, BI_RGB };
    std::vector<uint8_t> pixels(width * height * 4);
    
    GetDIBits(hdcMem, hbm, 0, height, pixels.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    m_context->UpdateSubresource(m_capturedTexture.Get(), 0, nullptr, pixels.data(), width * 4, 0);

    DeleteObject(hbm);
    DeleteDC(hdcMem);
    ReleaseDC(m_targetHWnd, hdcWindow);

    return CaptureResult::Success;
}

CaptureResult ScreenCapture::CaptureWindowFast() {
    if (!m_targetHWnd || !IsWindow(m_targetHWnd)) return CaptureResult::Error;

    POINT pt = { 0, 0 };
    if (!ClientToScreen(m_targetHWnd, &pt)) return CaptureResult::NoUpdate;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    ComPtr<IDXGIResource> desktopResource;
    HRESULT hr = m_deskDupl->AcquireNextFrame(0, &frameInfo, &desktopResource);
    
    if (hr == DXGI_ERROR_WAIT_TIMEOUT) return CaptureResult::NoUpdate;
    if (hr == DXGI_ERROR_ACCESS_LOST) return CaptureResult::AccessLost; 
    if (FAILED(hr)) return CaptureResult::Error;

    ComPtr<ID3D11Texture2D> desktopTexture;
    if (SUCCEEDED(desktopResource.As(&desktopTexture))) {
        D3D11_TEXTURE2D_DESC desc;
        desktopTexture->GetDesc(&desc);

        int left = max(0, (int)pt.x);
        int top = max(0, (int)pt.y);
        int right = min((int)desc.Width, (int)(left + width));
        int bottom = min((int)desc.Height, (int)(top + height));

        if (right <= left || bottom <= top) {
            m_deskDupl->ReleaseFrame();
            return CaptureResult::NoUpdate;
        }

        D3D11_BOX sourceRegion;
        sourceRegion.left = left;
        sourceRegion.top = top;
        sourceRegion.right = right;
        sourceRegion.bottom = bottom;
        sourceRegion.front = 0;
        sourceRegion.back = 1;

        if ((sourceRegion.right - sourceRegion.left) == (int)width && 
            (sourceRegion.bottom - sourceRegion.top) == (int)height) 
        {
            m_context->CopySubresourceRegion(
                m_capturedTexture.Get(), 0, 0, 0, 0, 
                desktopTexture.Get(), 0, &sourceRegion
            );
        }
    }

    m_deskDupl->ReleaseFrame();
    return CaptureResult::Success;
}