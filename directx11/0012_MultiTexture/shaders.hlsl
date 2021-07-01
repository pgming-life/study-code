//================================================================================
// �萔�o�b�t�@�ϐ�
//================================================================================
cbuffer ConstantBuffer : register(b0)
{
	matrix world;
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
	float2 uv : TEXCOORD0;			// ��{�FUV�l
};

//================================================================================
// ���_�V�F�[�_
//================================================================================
VS_OUTPUT vsMain(VS_INPUT input)
{
	VS_OUTPUT output;

	// �s��ϊ�
	output.position = mul(input.position, world);

	output.uv = input.uv;

	return output;
}

// �e�N�X�`�����
Texture2D       g_destTexture : register(t0);	// ��{�F
Texture2D       g_srcTexture : register(t1);	// �����F
SamplerState    g_sampler : register(s0);		// �T���v��

// �s�N�Z���V�F�[�_���̓p�����[�^
typedef VS_OUTPUT PS_INPUT;		// �u������

//================================================================================
// �s�N�Z���V�F�[�_
//================================================================================
// �ʏ�o��
float4 psMain(PS_INPUT input) : SV_TARGET
{
	// ���̂܂܏o��
	return g_destTexture.Sample(g_sampler, input.uv);
}

// �}���`�o��
float4 psMulti(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);

	// ��Z����
	return dest * src * 2;
	// �F���Â��̂�2�{�ɂ��ďo��
}
