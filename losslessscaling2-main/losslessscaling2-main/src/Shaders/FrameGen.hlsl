Texture2D<float4> FrameAnterior : register(t0);
Texture2D<float4> FrameAtual    : register(t1);
RWTexture2D<float4> OutputFrame : register(u0);
SamplerState SamplerLinear : register(s0);

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pos = dispatchThreadID.xy;
    float2 res;
    FrameAtual.GetDimensions(res.x, res.y);
    float2 uv = (float2)pos / res;

    const int RANGE = 10; 
    float3 target = FrameAtual[pos].rgb;
    float2 bestVec = float2(0, 0);
    float minDiff = 1000.0;

    for(int y = -RANGE; y <= RANGE; y += 2) {
        for(int x = -RANGE; x <= RANGE; x += 2) {
            float3 p = FrameAnterior[pos + int2(x, y)].rgb;
            
            float diff = length(target - p);
            
            diff += length(float2(x, y)) * 0.05; 

            if(diff < minDiff) {
                minDiff = diff;
                bestVec = float2(x, y);
            }
        }
    }

    float2 mHalf = (bestVec * 0.5) / res;
    
    float4 colA = FrameAnterior.SampleLevel(SamplerLinear, uv + mHalf, 0);
    float4 colB = FrameAtual.SampleLevel(SamplerLinear, uv - mHalf, 0);
    float confidence = saturate(1.2 - minDiff); 
    
    float4 result = lerp(colB, (colA + colB) * 0.5, confidence);

    float4 center = result;
    float4 neighbor = FrameAtual.SampleLevel(SamplerLinear, uv + float2(1.0/res.x, 0), 0);
    result = center + (center - neighbor) * 0.2;

    OutputFrame[pos] = float4(result.rgb, 1.0);
}