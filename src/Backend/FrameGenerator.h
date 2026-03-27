#pragma once
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class FrameGenerator {
public:
    bool Initialize(ID3D11Device* device);
    void GenerateFrame(ID3D11DeviceContext* context, 
                       ID3D11ShaderResourceView* frameAnterior, 
                       ID3D11ShaderResourceView* frameAtual, 
                       ID3D11UnorderedAccessView* frameGeradoOutput,
                       UINT width, UINT height);

private:
    ComPtr<ID3D11ComputeShader> computeShader;
};