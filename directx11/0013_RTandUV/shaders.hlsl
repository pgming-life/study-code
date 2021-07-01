//================================================================================
// �萔�o�b�t�@�ϐ�
//================================================================================
cbuffer ConstantBuffer : register(b0)
{
	matrix world;		// ���[���h�s��
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
VS_OUTPUT vsMain(VS_INPUT input)
{
	VS_OUTPUT output;

	// �s��ϊ�
	output.position = mul(input.position, world);

	// ���̂܂܏o��
	output.uv = input.uv;

	return output;
}

// �u�����h����
VS_OUTPUT vsBlend(VS_INPUT input)
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
float4 psMain(PS_INPUT input) : SV_TARGET
{
	// ���̂܂܏o��
	return g_destTexture.Sample(g_sampler, input.uv);
}

// ��Z * 2
float4 psMultiple(PS_INPUT input) : SV_TARGET
{
	float4 dest = g_destTexture.Sample(g_sampler, input.uv);
	float4 src = g_srcTexture.Sample(g_sampler, input.uv);

	return dest * src * 2;
}
