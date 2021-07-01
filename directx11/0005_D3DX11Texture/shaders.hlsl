//================================================================================
// 入力パラメータ
//================================================================================
struct VS_INPUT		// 頂点シェーダ
{
	float4 position : POSITION;		// 頂点座標
	float2 uv : TEXCOORD0;			// テクセル(UV座標)
};

//================================================================================
// 出力パラメータ
//================================================================================
struct VS_OUTPUT	// 頂点シェーダ
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;			// 基本色UV値
};

//================================================================================
// 頂点シェーダ
//================================================================================
VS_OUTPUT vsMain(VS_INPUT input)
{
	VS_OUTPUT output;

	// 加工なしでそのまま出力
	output.position = input.position;
	output.uv = input.uv;

	return output;
}

// テクスチャ情報
Texture2D       g_texture : register(t0);		// 基本色
SamplerState    g_sampler : register(s0);		// サンプラ

// ピクセルシェーダ入力パラメータ
typedef VS_OUTPUT PS_INPUT;		// 置き換え

//================================================================================
// ピクセルシェーダ
//================================================================================
float4 psMain(PS_INPUT input) : SV_TARGET
{
	// そのまま出力
	return g_texture.Sample(g_sampler, input.uv);
}
