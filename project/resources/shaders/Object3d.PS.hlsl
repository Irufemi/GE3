
/*テクスチャを貼ろう*/

#include "./Object3d.hlsli"

/*三角形の色を変えよう*/

struct Material
{
	float32_t4 color;
	
	/*LambertianReflectance*/
	
	int32_t enableLighting;
	
	int32_t hasTexture;
	
	 // 0=Lightingなし, 1=Lambert, 2=HalfLambert
	int32_t lightingMode;
	
	float padding;
	
	/*UVTransform*/
	
	///Materialの拡張
	
	float32_t4x4 uvTransform;
	
};
ConstantBuffer<Material> gMaterial : register(b0);
struct PixelShaderOutput
{
	float32_t4 color : SV_TARGET0;
};

/*テクスチャを貼ろう*/

///Textureを使う

Texture2D<float32_t4> gTexture : register(t0); //SRVのregisterはt
SamplerState gSampler : register(s0); //Samplerのregisterはs

/*LambertianReflectance*/

struct DirectionalLight
{
	 //!< ライトの色
	float32_t4 color;
    //!< ライトの向き
	float32_t3 direction;
    //!< 輝度
	float intensity;
};
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

/*テクスチャを貼ろう*/

PixelShaderOutput main(VertexShaderOutput input)
{
	PixelShaderOutput output;
	
	/*UVTransform*/
	
	///Materialを拡張する
	
	float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
	float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
	
	/*2値抜き*/
		
	/// disxard
		
	// textureのα値が0.5以下の時にPixelを棄却
	if (textureColor.a <= 0.5)
	{
		discard;
	}
		
	// textureのα値が0の時にPixelを棄却
	if (textureColor.a == 0.0)
	{
		discard;
	}
	
	/*テクスチャを貼ろう*/
	
	///Lightingの計算を行う
	
	if (gMaterial.enableLighting != 0) //Lightingする場合
	{
	
		float cos = 1.0f;
	
		if (gMaterial.lightingMode == 0)
		{
			if (gMaterial.hasTexture == 1)
			{
				output.color = gMaterial.color * textureColor;
			}
			else
			{
				output.color = gMaterial.color;
				output.color.a = 1.0f;
			}
		}
		else
		{
			if (gMaterial.lightingMode == 1)
			{
				cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
				output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
			}
			else if (gMaterial.lightingMode == 2)
			{
		
			/*HalfLambert*/
		
			///HalfLambertを実装する
		
			//HalfLambert
				float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
				cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
		
			}
			
			if (gMaterial.hasTexture == 1)
			{
				/*BlendMode*/
		
				/// PixelShaderを書き換える
		
				output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
				output.color.a = gMaterial.color.a * textureColor.a;
			}
			else
			{
				output.color.rgb = gMaterial.color.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
				output.color.a = 1.0f;
			}
			
		}
		
		
		
		
		/*2値抜き*/
		
		/// disxard
		
		// output.aolorのα値が0の時にPixelを棄却
		if (output.color.a == 0.0)
		{
			discard;
		}
	}
	else
	{ 
	
		if (gMaterial.hasTexture == 1)
		{
			output.color = gMaterial.color * textureColor;
		}
		else
		{
			output.color = gMaterial.color;
			output.color.a = 1.0f;
		}
		
	}
	
	return output;
}


