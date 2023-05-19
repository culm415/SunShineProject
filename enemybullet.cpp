//=============================================================================
//
// �o���b�g���� [bullet.cpp]
// Author : 
//
//=============================================================================
#include "enemy.h"
#include "enemy_bullet.h"
#include "collision.h"
#include "score.h"
#include "bg.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(40/2)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(40/2)	// 
#define TEXTURE_MAX					(1)		// �e�N�X�`���̐�

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iY)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// �A�j���[�V�����p�^�[����
#define ANIM_WAIT					(4)		// �A�j���[�V�����̐؂�ւ��Wait�l


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char* g_TexturName[] = {
	"data/TEXTURE/bullet00.png",
};

static BOOL		g_Load = FALSE;			// ���������s�������̃t���O
static ENEMYBULLET	g_EnemyBullet[ENEMY_BULLET_MAX];	// �o���b�g�\����


//=============================================================================
// ����������
//=============================================================================
HRESULT InitEnemyBullet(void)
{
	ID3D11Device* pDevice = GetDevice();

	//�e�N�X�`������
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// �o���b�g�\���̂̏�����
	for (int i = 0; i < ENEMY_BULLET_MAX; i++)
	{
		g_EnemyBullet[i].use = FALSE;			// ���g�p�i���˂���Ă��Ȃ��e�j
		g_EnemyBullet[i].w = TEXTURE_WIDTH;
		g_EnemyBullet[i].h = TEXTURE_HEIGHT;
		g_EnemyBullet[i].pos = XMFLOAT3(300, 300.0f, 0.0f);
		g_EnemyBullet[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_EnemyBullet[i].texNo = 0;

		g_EnemyBullet[i].countAnim = 0;
		g_EnemyBullet[i].patternAnim = 0;

		g_EnemyBullet[i].move = XMFLOAT3(0.0f, +ENEMY_BULLET_SPEED, 0.0f);	// �ړ��ʂ�������
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitEnemyBullet(void)
{
	if (g_Load == FALSE) return;

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i])
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateEnemyBullet(void)
{
	int bulletCount = 0;				// ���������o���b�g�̐�

	for (int i = 0; i < ENEMY_BULLET_MAX; i++)
	{
		if (g_EnemyBullet[i].use == TRUE)	// ���̃o���b�g���g���Ă���H
		{								// Yes
			// �A�j���[�V����  
			g_EnemyBullet[i].countAnim++;
			if ((g_EnemyBullet[i].countAnim % ANIM_WAIT) == 0)
			{
				// �p�^�[���̐؂�ւ�
				g_EnemyBullet[i].patternAnim = (g_EnemyBullet[i].patternAnim + 1) % ANIM_PATTERN_NUM;
			}

			// �o���b�g�̈ړ�����
			XMVECTOR pos = XMLoadFloat3(&g_EnemyBullet[i].pos);
			XMVECTOR move = XMLoadFloat3(&g_EnemyBullet[i].move);
			pos += move;
			XMStoreFloat3(&g_EnemyBullet[i].pos, pos);

			// ��ʊO�܂Ői�񂾁H
			BG* bg = GetBG();
			if (g_EnemyBullet[i].pos.y < (-g_EnemyBullet[i].h / 2))		// �����̑傫�����l�����ĉ�ʊO�����肵�Ă���
			{
				g_EnemyBullet[i].use = false;
			}
			if (g_EnemyBullet[i].pos.y > (bg->h + g_EnemyBullet[i].h / 2))	// �����̑傫�����l�����ĉ�ʊO�����肵�Ă���
			{
				g_EnemyBullet[i].use = false;
			}

			// �����蔻�菈��
			{
				ENEMY* enemy = GetEnemy();

				// �G�l�~�[�̐��������蔻����s��
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// �����Ă�G�l�~�[�Ɠ����蔻�������
					if (enemy[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_EnemyBullet[i].pos, g_EnemyBullet[i].w, g_EnemyBullet[i].h,
							enemy[j].pos, enemy[j].w, enemy[j].h);
						// �������Ă���H
						if (ans == TRUE)
						{
							// �����������̏���

							enemy[j].use = FALSE;
							AddScore(100);
						}
					}
				}
			}


			bulletCount++;
		}
	}


}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawEnemyBullet(void)
{
	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �}�g���N�X�ݒ�
	SetWorldViewProjection2D();

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �}�e���A���ݒ�
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	BG* bg = GetBG();

	for (int i = 0; i < ENEMY_BULLET_MAX; i++)
	{
		if (g_EnemyBullet[i].use == TRUE)		// ���̃o���b�g���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_EnemyBullet[i].texNo]);

			//�o���b�g�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_EnemyBullet[i].pos.x - bg->pos.x;	// �o���b�g�̕\���ʒuX
			float py = g_EnemyBullet[i].pos.y - bg->pos.y;	// �o���b�g�̕\���ʒuY
			float pw = g_EnemyBullet[i].w;		// �o���b�g�̕\����
			float ph = g_EnemyBullet[i].h;		// �o���b�g�̕\������

			float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// �e�N�X�`���̕�
			float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// �e�N�X�`���̍���
			float tx = (float)(g_EnemyBullet[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// �e�N�X�`���̍���X���W
			float ty = (float)(g_EnemyBullet[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer,
				px, py, pw, ph,
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_EnemyBullet[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// �o���b�g�\���̂̐擪�A�h���X���擾
//=============================================================================
ENEMYBULLET* GetEnemyBullet(void)
{
	return &g_EnemyBullet[0];
}


//=============================================================================
// �o���b�g�̔��ːݒ�
//=============================================================================
void SetEnemyBullet(XMFLOAT3 pos)
{
	// �������g�p�̒e�����������甭�˂��Ȃ�( =����ȏ㌂�ĂȂ����Ď� )
	for (int i = 0; i < ENEMY_BULLET_MAX; i++)
	{
		if (g_EnemyBullet[i].use == FALSE)		// ���g�p��Ԃ̃o���b�g��������
		{
			g_EnemyBullet[i].use = TRUE;			// �g�p��Ԃ֕ύX����
			g_EnemyBullet[i].pos = pos;			// ���W���Z�b�g
			return;							// 1���Z�b�g�����̂ŏI������
		}
	}
}
