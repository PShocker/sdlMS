Texture2D u_texture : register(t0, space2);
SamplerState u_sampler : register(s0, space2);

struct PSInput {
    float4 v_color : COLOR0;
    float2 v_uv : TEXCOORD0;
};

struct PSOutput {
    float4 o_color : SV_Target;
};

PSOutput main(PSInput input) {
    PSOutput output;
    
    // 获取纹理尺寸
    uint width, height;
    u_texture.GetDimensions(width, height);
    
    // 计算1像素的UV偏移量
    float2 pixelSize = float2(1.0 / width, 1.0 / height);
    
    // 采样中心点
    float4 centerColor = u_texture.Sample(u_sampler, input.v_uv);
    float centerAlpha = centerColor.a;
    
    // 如果中心点完全不透明，直接返回原色
    if (centerAlpha ==1.0) {
        output.o_color = centerColor * input.v_color;
        return output;
    }
    
    // 采样周围4个像素
    float alphaSum = 0.0;
    
    // 描边宽度
    int outlineRadius = 10;
    int curlineRadius = 0;
    
    // 采样四个方向的像素
    for (int i = 1; i <= outlineRadius; i++) {
        float2 offset = pixelSize * i;
        
        // 上
        alphaSum += u_texture.Sample(u_sampler, input.v_uv + float2(0, offset.y)).a;
        // 下
        alphaSum += u_texture.Sample(u_sampler, input.v_uv + float2(0, -offset.y)).a;
        // 左
        alphaSum += u_texture.Sample(u_sampler, input.v_uv + float2(-offset.x, 0)).a;
        // 右
        alphaSum += u_texture.Sample(u_sampler, input.v_uv + float2(offset.x, 0)).a;

        // 找到描边像素是第几层
        if(alphaSum>0&&curlineRadius==0){
            curlineRadius=i;
            break;
        }
    }
    
    // 描边颜色
    // 渐变
    // float4 outlineColor = float4( input.v_uv.x*5, input.v_uv.x*8,1-input.v_uv.y*8, 1.0);
    // 非渐变
    float4 outlineColor = float4( 1.0, 0.0,0, 1.0);
    
    // 描边逻辑：如果中心透明但周围有不透明像素，则显示描边
    if (alphaSum > 0.0) {
        // 描边
        // outlineColor.a=(1.0/curlineRadius);
        // 透明度按照0.15随层数递减
        outlineColor.a=1.0-curlineRadius*0.15;
        output.o_color = outlineColor * input.v_color;
    } else {
        // 正常纹理
        output.o_color = centerColor * input.v_color;
    }
    
    return output;
}