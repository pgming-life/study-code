//================================================================================
// 定数バッファ変数
//================================================================================
cbuffer ConstantBuffer : register(b0)
{
	matrix wp;
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
VS_OUTPUT VSMain(VS_INPUT input)
{
	VS_OUTPUT output;

	// 行列変換
	output.position = mul(input.position, wp);

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
// デフォルト
float4 PSMain(PS_INPUT input) : SV_TARGET
{
	// そのまま出力
	return g_texture.Sample(g_sampler, input.uv);
}

// レッド
float4 PSRed(PS_INPUT input) : SV_TARGET
{
	float4 color = g_texture.Sample(g_sampler, input.uv);

	color.r = 1.0;
	color.g = 0.0;
	color.b = 0.0;

	return color;
}

// ライム
float4 PSLime(PS_INPUT input) : SV_TARGET
{
	float4 color = g_texture.Sample(g_sampler, input.uv);

	color.r = 0.0;
	color.g = 1.0;
	color.b = 0.0;

	return color;
}

// アクア
float4 PSAqua(PS_INPUT input) : SV_TARGET
{
	float4 color = g_texture.Sample(g_sampler, input.uv);

	color.r = 0.0;
	color.g = 1.0;
	color.b = 1.0;

	return color;
}

// イエロー
float4 PSYellow(PS_INPUT input) : SV_TARGET
{
	float4 color = g_texture.Sample(g_sampler, input.uv);

	color.r = 1.0;
	color.g = 1.0;
	color.b = 0.0;

	return color;
}

// パープル
float4 PSPurple(PS_INPUT input) : SV_TARGET
{
	float4 color = g_texture.Sample(g_sampler, input.uv);

	color.r = 1.0;
	color.g = 0.0;
	color.b = 1.0;

	return color;
}

// オレンジ
float4 PSOrange(PS_INPUT input) : SV_TARGET
{
	float4 color = g_texture.Sample(g_sampler, input.uv);

	color.r = 1.0;
	color.g = 0.5;
	color.b = 0.0;

	return color;
}
