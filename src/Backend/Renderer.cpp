#include "Renderer.h"
#include <d3dcompiler.h>
#include <iostream>

bool Renderer::Initialize(ID3D11Device* device) {
    const char* shaderCode =
        "struct VS_OUT { float4 pos : SV_POSITION; float2 tex : TEXCOORD; };"
        "VS_OUT VSMain(uint id : SV_VertexID) {"
        "    VS_OUT o;" 
        "    o.tex = float2((id << 1) & 2, id & 2);"
        "    o.pos = float4(o.tex * float2(2, -2) + float2(-1, 1), 0, 1);"
        "    return o;"
        "}"
        "Texture2D tex : register(t0); SamplerState samp : register(s0);"
        "float4 PSMain(VS_OUT i) : SV_Target { return tex.Sample(samp, i.tex); }";

    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    HRESULT hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) return false;

    hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) return false;

    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vs);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_ps);

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState(&sampDesc, &m_sampler);

    return true;
}

void Renderer::RenderToScreen(ID3D11DeviceContext* context, ID3D11ShaderResourceView* textureSRV) {
    if (!textureSRV) return;
    
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_vs.Get(), nullptr, 0);
    context->PSSetShader(m_ps.Get(), nullptr, 0);
    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
    context->PSSetShaderResources(0, 1, &textureSRV);
    context->Draw(3, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(0, 1, &nullSRV);
}