//================================================================================
// 定数バッファ変数
//================================================================================
cbuffer ConstantBuffer : register(b0)
{
	matrix world;		// ワールド行列
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
	float2 uv : TEXCOORD0;
};

//================================================================================
// 頂点シェーダ
//================================================================================
// ブレンドなし
VS_OUTPUT vsMain(VS_INPUT input)
{
	VS_OUTPUT output;

	// 行列変換
	output.position = mul(input.position, world);

	// そのまま出力
	output.uv = input.uv;

	return output;
}

// ブレンドあり
VS_OUTPUT vsBlend(VS_INPUT input)
{
	VS_OUTPUT output;

	// 行列変換
	output.position = mul(input.position, world);

	// UV値変換
	input.uv.x = (output.position.x + 1.0) / 2;
	input.uv.y = (-output.position.y + 1.0) / 2;
	//input.uv.x = output.position.x;       // X座標変換
	//input.uv.y = -output.position.y;		// Y座標変換
	//input.uv.x = input.uv.x + 1.0;		// X座標調整
	//input.uv.y = input.uv.y + 1.0;		// Y座標調整
	//input.uv.x = input.uv.x / 2;			// 幅調整
	//input.uv.y = input.uv.x / 2;			// 高さ調整
	output.uv = input.uv;

	return output;
}

// テクスチャ情報
Texture2D       g_destTexture : register(t0);	// 基本色テクスチャ
Texture2D       g_srcTexture : register(t1);	// 合成色テクスチャ
SamplerState    g_sampler : register(s0);		// サンプラ

// ピクセルシェーダ入力パラメータ
typedef VS_OUTPUT PS_INPUT;		// 置き換え

//================================================================================
// ピクセルシェーダ
//================================================================================
// ブレンドなし
float4 psMain(PS_INPUT input) : SV_TARGET
{
	// そのまま出力
	return g_destTexture.Sample(g_sampler, input.uv);
}

// 乗算 * 2
float4 psMultiple(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);

	return dest * src * 2;
}
