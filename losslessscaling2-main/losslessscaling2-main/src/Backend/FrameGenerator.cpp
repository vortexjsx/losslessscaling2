#include "FrameGenerator.h"
#include <d3dcompiler.h>
#include <iostream>

#pragma comment(lib, "d3dcompiler.lib")

bool FrameGenerator::Initialize(ID3D11Device* device) {
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompileFromFile(
        L"Shaders/FrameGen.hlsl", nullptr, nullptr, "CSMain", "cs_5_0", 
        D3DCOMPILE_ENABLE_STRICTNESS, 0, &shaderBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "[SHADER ERRO] FrameGen.hlsl: " << (char*)errorBlob->GetBufferPointer() << std::endl;
        }
        return false;
    }

    hr = device->CreateComputeShader(
        shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &computeShader);
    
    return SUCCEEDED(hr);
}

void FrameGenerator::GenerateFrame(ID3D11DeviceContext* context, 
                                   ID3D11ShaderResourceView* frameAnterior, 
                                   ID3D11ShaderResourceView* frameAtual, 
                                   ID3D11UnorderedAccessView* frameGeradoOutput,
                                   UINT width, UINT height) {
    
    context->CSSetShader(computeShader.Get(), nullptr, 0);

    ID3D11ShaderResourceView* srvs[] = { frameAnterior, frameAtual };
    context->CSSetShaderResources(0, 2, srvs);
    context->CSSetUnorderedAccessViews(0, 1, &frameGeradoOutput, nullptr);

    UINT threadGroupX = (width + 7) / 8;
    UINT threadGroupY = (height + 7) / 8;
    context->Dispatch(threadGroupX, threadGroupY, 1);

    ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr };
    context->CSSetShaderResources(0, 2, nullSRVs);
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
}