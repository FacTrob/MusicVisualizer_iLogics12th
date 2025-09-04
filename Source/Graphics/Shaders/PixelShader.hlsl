cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 Color;
    float Time;
    float3 Padding;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float4 Color : COLOR;
};

float4 main(PS_INPUT input) : SV_Target
{
    // Simple solid color output with white lines
    float4 finalColor = float4(1.0f, 1.0f, 1.0f, 1.0f); // White color for shapes
    
    // Add some edge detection for line rendering
    float2 uv = input.Tex;
    
    // Calculate distance from edge for antialiasing
    float2 distToEdge = min(uv, 1.0f - uv);
    float edgeDistance = min(distToEdge.x, distToEdge.y);
    
    // Smooth edge falloff
    float alpha = smoothstep(0.0f, 0.02f, edgeDistance);
    
    // For wireframe rendering, invert alpha
    alpha = 1.0f - alpha;
    
    // Mix with input color for potential background effects
    finalColor.rgb = lerp(input.Color.rgb, finalColor.rgb, 0.8f);
    finalColor.a = alpha;
    
    return finalColor;
}