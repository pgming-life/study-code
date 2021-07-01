//================================================================================
// 定数バッファ変数
//================================================================================
cbuffer ConstantBuffer : register(b0)
{
	matrix world;
};

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

	// 行列変換
	output.position = mul(input.position, world);

	output.uv = input.uv;

	return output;
}

// テクスチャ情報
Texture2D       g_destTexture : register(t0);	// 基本色
Texture2D       g_srcTexture : register(t1);	// 合成色
SamplerState    g_sampler : register(s0);		// サンプラ

// ピクセルシェーダ入力パラメータ
typedef VS_OUTPUT PS_INPUT;		// 置き換え

//================================================================================
// ピクセルシェーダ
//================================================================================
// 通常出力
float4 psMain(PS_INPUT input) : SV_TARGET
{
	// そのまま出力
	return g_destTexture.Sample(g_sampler, input.uv);
}

// マルチ出力
float4 psMulti(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);

	// 乗算合成
	return dest * src * 2;
	// 色が暗いので2倍にして出力
}
