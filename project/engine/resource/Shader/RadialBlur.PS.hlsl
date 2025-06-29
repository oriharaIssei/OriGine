#include "FullScreen.hlsli"

/// ==========================================
// struct
/// ==========================================
struct RadialBlurParam
{
    float2 center; // Center of the radial blur
    float kBulerWidth;
};
ConstantBuffer<RadialBlurParam> gRadialBlurParams : register(b0); // Radial blur parameters
Texture2D<float4> gTexture : register(t0); // input texture
SamplerState gSampler : register(s0); // input sampler

/// ------------------------------------------------------------------
// main
/// ------------------------------------------------------------------
PixelShaderOutput main(VertexShaderOutput input)
{
    const int MAX_SAMPLES = 16; // Maximum number of samples for radial blur
    
    PixelShaderOutput output;
    // doesn't Normalize. 
    float2 direction = input.texCoords - gRadialBlurParams.center;
    float3 outputColor = float3(0.0f, 0.0f, 0.0f);

    for (int samplerIdx = 0; samplerIdx < MAX_SAMPLES; samplerIdx++)
    {
        // Calculate the sample offset based on the sample index
        float2 sampleOffset = direction * (gRadialBlurParams.kBulerWidth * float(samplerIdx));
        // Sample the texture at the offset position
        outputColor += gTexture.Sample(gSampler, input.texCoords + sampleOffset).rgb;
    }

    outputColor *= rcp(float(MAX_SAMPLES)); // Average the color by dividing by the number of samples)

    output.color.rgb = outputColor;
    output.color.a = 1.0f; // Set alpha to 1.0
    
    return output;
}
