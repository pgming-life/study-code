//================================================================================
// 定数バッファ変数
//================================================================================
cbuffer ConstantBuffer : register(b0)
{
	matrix world;		// ワールド行列
	float alpha;        // アルファ値
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
VS_OUTPUT VSMain(VS_INPUT input)
{
	VS_OUTPUT output;

	// 行列変換
	output.position = mul(input.position, world);

	// そのまま出力
	output.uv = input.uv;

	return output;
}

// ブレンドあり
VS_OUTPUT VSBlend(VS_INPUT input)
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
float4 PSMain(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色

	float4 result;
	result.rgb = dest.rgb;
	result.a = alpha;

	return result;
}

// 通常(アルファブレンド)
float4 PSNormal(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色

	float4 result;
	result.rgb = dest.rgb;
	result.a = dest.a * alpha;

	return result;
}

// 比較(暗)
float4 PSDark(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = min(dest.rgb, src.rgb);
	result.a = src.a * alpha;

	return result;
}

// 乗算
float4 PSMultiple(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = dest.rgb * src.rgb;
	result.a = src.a * alpha;

	return result;
}

// 焼き込みカラー
float4 PSBurnColor(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = 1.0 - (1.0 - dest.rgb) / src.rgb;
	result.a = src.a * alpha;

	return result;
}

// 焼き込みリニア
float4 PSBurnLinear(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = dest.rgb + src.rgb <= 1.0 ? 0.0 : dest.rgb + src.rgb - 1.0;
	result.a = src.a * alpha;

	return result;
}

// カラー比較(暗)
float4 PSColorDark(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = (src.r + src.g + src.b > dest.r + dest.g + dest.b) ? dest.rgb : src.rgb;
	result.a = src.a * alpha;

	return result;
}

// 比較(明)
float4 PSLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = max(dest.rgb, src.rgb);
	result.a = src.a * alpha;

	return result;
}

// スクリーン
float4 PSScreen(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = dest.rgb + src.rgb - dest.rgb * src.rgb;
	result.a = src.a * alpha;

	return result;
}

// 覆い焼きカラー
float4 PSDodgeColor(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = dest.rgb / (1.0 - src.rgb);
	result.a = src.a * alpha;

	return result;
}

// 覆い焼きリニア(加算)
float4 PSDodgeLinear(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = src.rgb + dest.rgb;
	result.a = src.a * alpha;

	return result;
}

// カラー比較(明)
float4 PSColorLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = (src.r + src.g + src.b > dest.r + dest.g + dest.b) ? src.rgb : dest.rgb;
	result.a = src.a * alpha;

	return result;
}

// オーバーレイ
float4 PSOverlay(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);	// 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	if (dest.r < 0.5) result.r = dest.r * src.r * 2;
	else result.r = 2 * (dest.r + src.r - dest.r * src.r) - 1.0;
	if (dest.g < 0.5) result.g = dest.g * src.g * 2;
	else result.g = 2 * (dest.g + src.g - dest.g * src.g) - 1.0;
	if (dest.b < 0.5) result.b = dest.b * src.b * 2;
	else result.b = 2 * (dest.b + src.b - dest.b * src.b) - 1.0;
	result.a = src.a * alpha;

	return result;
}

// ソフトライト
float4 PSSoftLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);	// 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	if (src.r < 0.5) result.r = 2 * dest.r * src.r + pow(dest.r, dest.r) * (1.0f - 2 * src.r);
	else result.r = 2 * dest.r * (1.0 - src.r) + sqrt(dest.r) * (2 * src.r - 1.0);
	if (src.g < 0.5) result.g = 2 * dest.g * src.g + pow(dest.g, dest.g) * (1.0f - 2 * src.g);
	else result.g = 2 * dest.g * (1.0 - src.g) + sqrt(dest.g) * (2 * src.g - 1.0);
	if (src.b < 0.5) result.b = 2 * dest.b * src.b + pow(dest.b, dest.b) * (1.0f - 2 * src.b);
	else result.b = 2 * dest.b * (1.0 - src.b) + sqrt(dest.b) * (2 * src.b - 1.0);
	result.a = src.a * alpha;

	return result;
}

// ハードライト
float4 PSHardLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	if (dest.r > 0.5) result.r = dest.r * src.r * 2;
	else result.r = 2 * (dest.r + src.r - dest.r * src.r) - 1.0;
	if (dest.g > 0.5) result.g = dest.g * src.g * 2;
	else result.g = 2 * (dest.g + src.g - dest.g * src.g) - 1.0;
	if (dest.b > 0.5) result.b = dest.b * src.b * 2;
	else result.b = 2 * (dest.b + src.b - dest.b * src.b) - 1.0;
	result.a = src.a * alpha;

	return result;
}

// ビビッドライト
float4 PSVividLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	if (src.r < 0.5) result.r = 1.0 - (1.0 - dest.r) / (2 * src.r);
	else result.r = dest.r / (1.0 - 2 * (src.r - 0.5));
	if (src.g < 0.5) result.g = 1.0 - (1.0 - dest.g) / (2 * src.g);
	else result.g = dest.g / (1.0 - 2 * (src.g - 0.5));
	if (src.b < 0.5) result.b = 1.0 - (1.0 - dest.b) / (2 * src.b);
	else result.b = dest.b / (1.0 - 2 * (src.b - 0.5));
	result.a = src.a * alpha;

	return result;
}

// リニアライト
float4 PSLinearLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	if (src.r < 0.5) result.r = dest.r + 2 * src.r - 1.0;
	else result.r = dest.r + 2 * (src.r - 0.5);
	if (src.g < 0.5) result.g = dest.g + 2 * src.g - 1.0;
	else result.g = dest.g + 2 * (src.g - 0.5);
	if (src.b < 0.5) result.b = dest.b + 2 * src.b - 1.0;
	else result.b = dest.b + 2 * (src.b - 0.5);
	result.a = src.a * alpha;

	return result;
}

// ピンライト
float4 PSPinLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	if (src.r < 0.5) result.r = min(dest.r, 2 * src.r);
	else result.r = max(dest.r, 2 * (src.r - 0.5));
	if (src.g < 0.5) result.g = min(dest.g, 2 * src.g);
	else result.g = max(dest.g, 2 * (src.g - 0.5));
	if (src.b < 0.5) result.b = min(dest.b, 2 * src.b);
	else result.b = max(dest.b, 2 * (src.b - 0.5));
	result.a = src.a * alpha;

	return result;
}

// ハードミックス
float4 PSHardMix(PS_INPUT input) : SV_TARGET
{
	// 未実装
	return g_destTexture.Sample(g_sampler, input.uv);
}

// 差の絶対値
float4 PSAbsoluteness(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = abs(src.rgb - dest.rgb);
	result.a = src.a * alpha;

	return result;
}

// 除外
float4 PSExclusion(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = dest.rgb + src.rgb - 2 * dest.rgb * src.rgb;
	result.a = src.a * alpha;

	return result;
}

// 減算
float4 PSSubtraction(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = dest.rgb - src.rgb;
	result.a = src.a * alpha;

	return result;
}

// 除算
float4 PSDivision(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = dest.rgb / src.rgb;
	result.a = src.a * alpha;

	return result;
}

// 加算
float4 PSAddition(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // 基本色
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // 合成色

	float4 result;
	result.rgb = dest.rgb + src.rgb;
	result.a = src.a * alpha;

	return result;
}

// モノクロ
float4 PSMonochrome(PS_INPUT input) : SV_TARGET
{
	// 未実装
	return g_destTexture.Sample(g_sampler, input.uv);
}

// ブライトエクストラクト
float4 PSBrightExtract(PS_INPUT input) : SV_TARGET
{
	// 未実装
	return g_destTexture.Sample(g_sampler, input.uv);
}

// カラーキーアルファ
float4 PSColorKeyAlpha(PS_INPUT input) : SV_TARGET
{
	// 未実装
	return g_destTexture.Sample(g_sampler, input.uv);
}

// カラートーン
float4 PSColorTone(PS_INPUT input) : SV_TARGET
{
	// 未実装
	return g_destTexture.Sample(g_sampler, input.uv);
}

// グレースケール
float4 PSGrayScale(PS_INPUT input) : SV_TARGET
{
	// 未実装
	return g_destTexture.Sample(g_sampler, input.uv);
}

// 反転色
float4 PSInvert(PS_INPUT input) : SV_TARGET
{
	// 未実装
	return g_destTexture.Sample(g_sampler, input.uv);
}

// モザイク
float4 PSMosaic(PS_INPUT input) : SV_TARGET
{
	// 未実装
	return g_destTexture.Sample(g_sampler, input.uv);
}

// セピア
float4 PSSepia(PS_INPUT input) : SV_TARGET
{
	// 未実装
	return g_destTexture.Sample(g_sampler, input.uv);
}
