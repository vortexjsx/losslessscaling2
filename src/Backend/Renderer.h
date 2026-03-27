#pragma once
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class Renderer {
public:
    bool Initialize(ID3D11Device* device);
    void RenderToScreen(ID3D11DeviceContext* context, ID3D11ShaderResourceView* textureSRV);

private:
    ComPtr<ID3D11VertexShader> m_vs;
    ComPtr<ID3D11PixelShader> m_ps;
    ComPtr<ID3D11SamplerState> m_sampler;
};