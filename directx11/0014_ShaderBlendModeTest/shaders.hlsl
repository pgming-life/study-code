//================================================================================
// �萔�o�b�t�@�ϐ�
//================================================================================
cbuffer ConstantBuffer : register(b0)
{
	matrix world;		// ���[���h�s��
	float alpha;        // �A���t�@�l
};

//================================================================================
// ���̓p�����[�^
//================================================================================
struct VS_INPUT		// ���_�V�F�[�_
{
	float4 position : POSITION;		// ���_���W
	float2 uv : TEXCOORD0;			// �e�N�Z��(UV���W)
};

//================================================================================
// �o�̓p�����[�^
//================================================================================
struct VS_OUTPUT	// ���_�V�F�[�_
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

//================================================================================
// ���_�V�F�[�_
//================================================================================
// �u�����h�Ȃ�
VS_OUTPUT VSMain(VS_INPUT input)
{
	VS_OUTPUT output;

	// �s��ϊ�
	output.position = mul(input.position, world);

	// ���̂܂܏o��
	output.uv = input.uv;

	return output;
}

// �u�����h����
VS_OUTPUT VSBlend(VS_INPUT input)
{
	VS_OUTPUT output;

	// �s��ϊ�
	output.position = mul(input.position, world);

	// UV�l�ϊ�
	input.uv.x = (output.position.x + 1.0) / 2;
	input.uv.y = (-output.position.y + 1.0) / 2;
	//input.uv.x = output.position.x;       // X���W�ϊ�
	//input.uv.y = -output.position.y;		// Y���W�ϊ�
	//input.uv.x = input.uv.x + 1.0;		// X���W����
	//input.uv.y = input.uv.y + 1.0;		// Y���W����
	//input.uv.x = input.uv.x / 2;			// ������
	//input.uv.y = input.uv.x / 2;			// ��������
	output.uv = input.uv;

	return output;
}

// �e�N�X�`�����
Texture2D       g_destTexture : register(t0);	// ��{�F�e�N�X�`��
Texture2D       g_srcTexture : register(t1);	// �����F�e�N�X�`��
SamplerState    g_sampler : register(s0);		// �T���v��

// �s�N�Z���V�F�[�_���̓p�����[�^
typedef VS_OUTPUT PS_INPUT;		// �u������

//================================================================================
// �s�N�Z���V�F�[�_
//================================================================================
// �u�����h�Ȃ�
float4 PSMain(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F

	float4 result;
	result.rgb = dest.rgb;
	result.a = alpha;

	return result;
}

// �ʏ�(�A���t�@�u�����h)
float4 PSNormal(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F

	float4 result;
	result.rgb = dest.rgb;
	result.a = dest.a * alpha;

	return result;
}

// ��r(��)
float4 PSDark(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = min(dest.rgb, src.rgb);
	result.a = src.a * alpha;

	return result;
}

// ��Z
float4 PSMultiple(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = dest.rgb * src.rgb;
	result.a = src.a * alpha;

	return result;
}

// �Ă����݃J���[
float4 PSBurnColor(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = 1.0 - (1.0 - dest.rgb) / src.rgb;
	result.a = src.a * alpha;

	return result;
}

// �Ă����݃��j�A
float4 PSBurnLinear(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = dest.rgb + src.rgb <= 1.0 ? 0.0 : dest.rgb + src.rgb - 1.0;
	result.a = src.a * alpha;

	return result;
}

// �J���[��r(��)
float4 PSColorDark(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = (src.r + src.g + src.b > dest.r + dest.g + dest.b) ? dest.rgb : src.rgb;
	result.a = src.a * alpha;

	return result;
}

// ��r(��)
float4 PSLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = max(dest.rgb, src.rgb);
	result.a = src.a * alpha;

	return result;
}

// �X�N���[��
float4 PSScreen(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = dest.rgb + src.rgb - dest.rgb * src.rgb;
	result.a = src.a * alpha;

	return result;
}

// �����Ă��J���[
float4 PSDodgeColor(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = dest.rgb / (1.0 - src.rgb);
	result.a = src.a * alpha;

	return result;
}

// �����Ă����j�A(���Z)
float4 PSDodgeLinear(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = src.rgb + dest.rgb;
	result.a = src.a * alpha;

	return result;
}

// �J���[��r(��)
float4 PSColorLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = (src.r + src.g + src.b > dest.r + dest.g + dest.b) ? src.rgb : dest.rgb;
	result.a = src.a * alpha;

	return result;
}

// �I�[�o�[���C
float4 PSOverlay(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);	// ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

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

// �\�t�g���C�g
float4 PSSoftLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);	// ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

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

// �n�[�h���C�g
float4 PSHardLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

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

// �r�r�b�h���C�g
float4 PSVividLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

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

// ���j�A���C�g
float4 PSLinearLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

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

// �s�����C�g
float4 PSPinLight(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

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

// �n�[�h�~�b�N�X
float4 PSHardMix(PS_INPUT input) : SV_TARGET
{
	// ������
	return g_destTexture.Sample(g_sampler, input.uv);
}

// ���̐�Βl
float4 PSAbsoluteness(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = abs(src.rgb - dest.rgb);
	result.a = src.a * alpha;

	return result;
}

// ���O
float4 PSExclusion(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = dest.rgb + src.rgb - 2 * dest.rgb * src.rgb;
	result.a = src.a * alpha;

	return result;
}

// ���Z
float4 PSSubtraction(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = dest.rgb - src.rgb;
	result.a = src.a * alpha;

	return result;
}

// ���Z
float4 PSDivision(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = dest.rgb / src.rgb;
	result.a = src.a * alpha;

	return result;
}

// ���Z
float4 PSAddition(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);    // ��{�F
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);      // �����F

	float4 result;
	result.rgb = dest.rgb + src.rgb;
	result.a = src.a * alpha;

	return result;
}

// ���m�N��
float4 PSMonochrome(PS_INPUT input) : SV_TARGET
{
	// ������
	return g_destTexture.Sample(g_sampler, input.uv);
}

// �u���C�g�G�N�X�g���N�g
float4 PSBrightExtract(PS_INPUT input) : SV_TARGET
{
	// ������
	return g_destTexture.Sample(g_sampler, input.uv);
}

// �J���[�L�[�A���t�@
float4 PSColorKeyAlpha(PS_INPUT input) : SV_TARGET
{
	// ������
	return g_destTexture.Sample(g_sampler, input.uv);
}

// �J���[�g�[��
float4 PSColorTone(PS_INPUT input) : SV_TARGET
{
	// ������
	return g_destTexture.Sample(g_sampler, input.uv);
}

// �O���[�X�P�[��
float4 PSGrayScale(PS_INPUT input) : SV_TARGET
{
	// ������
	return g_destTexture.Sample(g_sampler, input.uv);
}

// ���]�F
float4 PSInvert(PS_INPUT input) : SV_TARGET
{
	// ������
	return g_destTexture.Sample(g_sampler, input.uv);
}

// ���U�C�N
float4 PSMosaic(PS_INPUT input) : SV_TARGET
{
	// ������
	return g_destTexture.Sample(g_sampler, input.uv);
}

// �Z�s�A
float4 PSSepia(PS_INPUT input) : SV_TARGET
{
	// ������
	return g_destTexture.Sample(g_sampler, input.uv);
}
