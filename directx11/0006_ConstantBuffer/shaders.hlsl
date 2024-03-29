//================================================================================
// 定数バッファ変数
//================================================================================
cbuffer ConstantBuffer : register(b0)
{
	matrix wvp;
};

//================================================================================
// 入力パラメータ
//================================================================================
struct VS_INPUT		// 頂点シェーダ
{
	float4 position : POSITION;
	float4 color : COLOR;
};

//================================================================================
// 出力パラメータ
//================================================================================
struct VS_OUTPUT	// 頂点シェーダ
{
	float4 position : SV_POSITION;
	float4 color: COLOR;
};

//================================================================================
// 頂点シェーダ
//================================================================================
VS_OUTPUT vsMain(VS_INPUT input)
{
	VS_OUTPUT output;

	// 行列変換
	output.position = mul(input.position, wvp);

	output.color = input.color;

	return output;
}

// ピクセルシェーダ入力パラメータ
typedef VS_OUTPUT PS_INPUT;		// 置き換え

//================================================================================
// ピクセルシェーダ
//================================================================================
float4 psMain(PS_INPUT input) : SV_TARGET
{
	// そのまま出力
	return input.color;
}
