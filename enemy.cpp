//=============================================================================
//
// �G�l�~�[���� [enemy.cpp]
// Author : ��c�@�P
//
//=============================================================================
#include "enemy.h"
#include "bg.h"
#include "player.h"
#include "fade.h"
#include "collision.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(120/2)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(120/2)	// 
#define TEXTURE_MAX					(2)		// �e�N�X�`���̐�

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iX)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// �A�j���p�^�[���̃e�N�X�`�����������iY)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// �A�j���[�V�����p�^�[����
#define ANIM_WAIT					(4)		// �A�j���[�V�����̐؂�ւ��Wait�l
#define GAME_COUNT					(3500)	// �t���[���J�E���g

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;				// ���_���
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/22286029.png",
};


static BOOL		g_Load = TRUE;			// ���������s�������̃t���O
static ENEMY	g_Enemy[ENEMY_MAX];		// �G�l�~�[�\����
static int		g_EnemyCnt = ENEMY_MAX;
static int		g_Count;				// �J�E���g


//=============================================================================
// ����������
//=============================================================================
HRESULT InitEnemy(void)
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


	// �G�l�~�[�\���̂̏�����
	g_EnemyCnt = 0;
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		g_Enemy[i].loop = 0;
		g_Enemy[i].use = FALSE;
		g_Enemy[i].pos = XMFLOAT3(100.0f + (i % 6) * 100.0f, 1.0f, 0.0f);	// ���S�_����\��
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Enemy[i].w = TEXTURE_WIDTH;
		g_Enemy[i].h = TEXTURE_HEIGHT;
		g_Enemy[i].texNo = 0;
		g_Enemy[i].markedForDeath = FALSE;

		g_Enemy[i].countAnim = 0;
		g_Enemy[i].patternAnim = 0;

		g_Enemy[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// �ړ���

		g_Enemy[i].time = 0.0f;			// ���`��ԗp�̃^�C�}�[���N���A
		g_Enemy[i].timeAlive = 0.0f;	// TRUE(�g���Ă��鎞��)�ɂȂ��Ă��鎞�Ԃ����肠
		g_Enemy[i].delay = 0;			// ���ԍ����N���A
		g_Enemy[i].tblNo = 0;			// �Đ�����s���f�[�^�e�[�u��No���Z�b�g
		g_Enemy[i].tblMax = 0;			// �Đ�����s���f�[�^�e�[�u���̃��R�[�h�����Z�b�g

		g_EnemyCnt++;
	}


	g_Count = 0;
	g_Load = TRUE;
	return S_OK;
}


//=============================================================================
// �I������
//=============================================================================
void UninitEnemy(void)
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

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateEnemy(void)
{


	g_EnemyCnt = 0;	// �����Ă�G�l�~�[�̐�

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Count == 10 + i * 50)		// �J�E���g
		{
			if (i < 500)					// �g����G�l�~�[�̐���(�����̐��܂Ŏg����)
			{
				g_Enemy[i].use = TRUE;
			}

		}

		// �����Ă�G�l�~�[��������������
		if (g_Enemy[i].use == TRUE)
		{
			g_Enemy[i].cnt++;
			g_Enemy[i].rot.z += 0.00f;

			// �n�`�Ƃ̓����蔻��p�ɍ��W�̃o�b�N�A�b�v������Ă���
			XMFLOAT3 pos_old = g_Enemy[i].pos;

			// �A�j���[�V����
			g_Enemy[i].countAnim += 1.0f;
			if (g_Enemy[i].countAnim > ANIM_WAIT)
			{
				g_Enemy[i].countAnim = 0.0f;
				// �p�^�[���̐؂�ւ�
				g_Enemy[i].patternAnim = (g_Enemy[i].patternAnim + 1) % ANIM_PATTERN_NUM;
			}


			// �G�l�~�[�̓��쏈��

			if (i < 6)  //�G�̓��쏈��(��{�I�ɂU�̂��j
			{
				int t = g_Enemy[i].cnt;
				int pattern_1 = 100;		//�ォ�炫�Ď~�܂�܂ł̃t���[����
				int wait = 100;				//�~�܂�t���[����

				if (t < pattern_1)
				{
					g_Enemy[i].pos.x -= 1.0f;		//�E�Ɉړ����Ă���
					g_Enemy[i].pos.y += 2.5f;		//���Ɉړ����Ă���
				}
				else if(t == pattern_1)
				{
					g_Enemy[i].pos.y += 0; //��~�M��
				}
				else if (t > pattern_1 + wait)
				{
					g_Enemy[i].pos.x += 1.0f;		//���Ɉړ����Ă���
					g_Enemy[i].pos.y += 2.5f;		//���Ɉړ����Ă���
				}
			}

			else if (i < 12)  //�G�̓��쏈��
			{
				int t = g_Enemy[i].cnt;
				int pattern_2 = 100;		//�ォ�炫�Ď~�܂�܂ł̃t���[����
				int wait = 100;				//�~�܂�t���[����

				if (t < pattern_2)
				{
					g_Enemy[i].pos.x += 1.0f;
					g_Enemy[i].pos.y += 2.5f;		//�E���Ɉړ����Ă���
				}
				if (t == pattern_2)
				{
					g_Enemy[i].pos.y += 0;			//��~�M��
				}
				if (t > pattern_2 + wait)
				{
					g_Enemy[i].pos.x -= 1.0f;
					g_Enemy[i].pos.y += 2.5f;		//�����Ɉړ����Ă���
				}
			}

			else if (i < 18)  //�G�̓��쏈��
			{
				int t = g_Enemy[i].cnt;
				int pattern_3 = 100;		//�ォ�炫�Ď~�܂�܂ł̃t���[����
				int wait = 100;				//�~�܂�t���[����

				if (t < pattern_3)
				{
					g_Enemy[i].pos.y += 2.5f;		//������
				}
				if (t == pattern_3)
				{
					g_Enemy[i].pos.y += 0;			//��~�M��
				}
				if (t > pattern_3 + wait)
				{
					g_Enemy[i].pos.y += 2.5f;		//������
				}
			}

			else if (i < 24)  //�G�̓��쏈��
			{
				int t = g_Enemy[i].cnt;
				int pattern_4 = 200;		//�ォ�炫�Ď~�܂�܂ł̃t���[����

				if (t < pattern_4)
				{
					g_Enemy[i].pos.y += 4.0f;		//������
				}

			}

			else if (i < 30)  //�G�̓��쏈��
			{
				int t = g_Enemy[i].cnt;
				int pattern_5 = 100;		//�ォ�炫�Ď~�܂�܂ł̃t���[����
				int wait = 100;				//�~�܂�t���[����

				if (t < pattern_5)
				{
					g_Enemy[i].pos.y += 2.5f;		//������
				}
				if (t == pattern_5)
				{
					g_Enemy[i].pos.y += 0;			//��~�M��
				}
				if (t > pattern_5 + wait)
				{
					g_Enemy[i].pos.y -= 2.5f;		//�㏸
				}

			}

			else if (i < 36)  //�G�̓��쏈��
			{
				int t = g_Enemy[i].cnt;
				int pattern_6 = 100;		//�ォ�炫�Ď~�܂�܂ł̃t���[����
				int wait = 100;				//�~�܂�t���[����

				if (t < pattern_6)
				{
					g_Enemy[i].pos.y += 2.5f;		//������
				}
				if (t == pattern_6)
				{
					g_Enemy[i].pos.y += 0;			//��~�M��
				}
				if (t > pattern_6 + wait)
				{
					g_Enemy[i].pos.x -= 2.5f;		//�E�Ɉړ�
				}

			}

			else if (i < 42)  //�G�̓��쏈��
			{
				int t = g_Enemy[i].cnt;
				int pattern_7 = 100;		//�ォ�炫�Ď~�܂�܂ł̃t���[����
				int wait = 100;				//�~�܂�t���[����

				if (t < pattern_7)
				{
					g_Enemy[i].pos.y += 2.5f;		//������
				}

				if (t == pattern_7)
				{
					g_Enemy[i].pos.y += 0;			//��~�M��
				}

				if (t > pattern_7 + wait)
				{
					g_Enemy[i].pos.x += 2.5f;		//���Ɉړ�
				}

			}

			else if (i < 48)  //�G�̓��쏈��
			{
				int t = g_Enemy[i].cnt;
				int pattern_8 = 100;		//�ォ�炫�Ď~�܂�܂ł̃t���[����
				int wait = 100;				//�~�܂�t���[����

				if (t < pattern_8)
				{
					g_Enemy[i].pos.x -= 1.0f;		//�E�Ɉړ����Ă���
					g_Enemy[i].pos.y += 2.5f;		//���Ɉړ����Ă���
				}

				if (t == pattern_8)
				{
					g_Enemy[i].pos.y += 0;			//��~�M��
				}

				if (t > pattern_8 + wait)
				{
					g_Enemy[i].pos.y += 2.5f;		//������
				}

			}

			else if (i < 54)  //�G�̓��쏈��
			{
				int t = g_Enemy[i].cnt;
				int pattern_9 = 100;		//�ォ�炫�Ď~�܂�܂ł̃t���[����
				int wait = 100;				//�~�܂�t���[����

				if (t < pattern_9)
				{
					g_Enemy[i].pos.x += 1.0f;		//���Ɉړ����Ă���
					g_Enemy[i].pos.y += 2.5f;		//���Ɉړ����Ă���
				}

				if (t == pattern_9)
				{
					g_Enemy[i].pos.y += 0;			//��~�M��
				}

				if (t > pattern_9 + wait)
				{
					g_Enemy[i].pos.y += 2.5f;		//������
				}
			}

			else if (i < 60)  //�G�̓��쏈��
			{
				int t = g_Enemy[i].cnt;
				int pattern_10 = 250;		//�ォ�炫�Ď~�܂�܂ł̃t���[����

				if (t < pattern_10)
				{
					g_Enemy[i].pos.x += 1.0f;		//���Ɉړ����Ă���
					g_Enemy[i].pos.y += 2.5f;		//���Ɉړ����Ă���
				}

			}

			else if (i < 66)  //�G�̓��쏈��
			{
				int t = g_Enemy[i].cnt;
				int pattern_10 = 250;		//�ォ�炫�Ď~�܂�܂ł̃t���[����

				if (t < pattern_10)
				{
					g_Enemy[i].pos.x -= 1.0f;		//�E�Ɉړ����Ă���
					g_Enemy[i].pos.y += 2.5f;		//���Ɉړ����Ă���
				}

			}

			// �ړ����I�������v���C���[�Ƃ̓����蔻��
			{
				PLAYER* player = GetPlayer();

				// �v���C���[�̐��������蔻����s��
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// �����Ă�v���C���[�Ɠ����蔻�������
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Enemy[i].pos, g_Enemy[i].w, g_Enemy[i].h,
							player[j].pos, player[j].w, player[j].h);
						// �������Ă���H
						if (ans == TRUE)
						{
							// �����������̏���
						//	player[j].use = FALSE;	// �f�o�b�O�ňꎞ�I�ɖ��G�ɂ���
						}
					}
				}
			}


			g_EnemyCnt++;		// �����Ă��G�̐�
		}
	}

	if (g_Count >= 3500)	//3600�t���[��(��1��)��������Q�[�����U���g�ɍs��
	{
		SetFade(FADE_OUT, MODE_RESULT);

	}

	//�f�o�b�N�p(�R�����g�C������ƃ^�C�g�����烊�U���g�ɍs����)
	// �G�l�~�[�S�ł����烊�U���g��ʂɍs��
	/*if (g_EnemyCnt >= 0)
	{
		SetFade(FADE_OUT, MODE_RESULT);
	}*/

#ifdef _DEBUG	// �f�o�b�O����\������


#endif

}



//=============================================================================
// �`�揈��
//=============================================================================
void DrawEnemy(void)
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

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Enemy[i].use == TRUE)			// ���̃G�l�~�[���g���Ă���H
		{									// Yes
			// �e�N�X�`���ݒ�
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Enemy[i].texNo]);

			//�G�l�~�[�̈ʒu��e�N�X�`���[���W�𔽉f
			float px = g_Enemy[i].pos.x - bg->pos.x;	// �G�l�~�[�̕\���ʒuX
			float py = g_Enemy[i].pos.y - bg->pos.y;	// �G�l�~�[�̕\���ʒuY
			float pw = g_Enemy[i].w;		// �G�l�~�[�̕\����
			float ph = g_Enemy[i].h;		// �G�l�~�[�̕\������

			// �A�j���[�V�����p
			//float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// �e�N�X�`���̕�
			//float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// �e�N�X�`���̍���
			//float tx = (float)(g_Player[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// �e�N�X�`���̍���X���W
			//float ty = (float)(g_Player[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// �e�N�X�`���̍���Y���W

			float tw = 1.0f;	// �e�N�X�`���̕�
			float th = 1.0f;	// �e�N�X�`���̍���
			float tx = 0.0f;	// �e�N�X�`���̍���X���W
			float ty = 0.0f;	// �e�N�X�`���̍���Y���W

			// �P���̃|���S���̒��_�ƃe�N�X�`�����W��ݒ�
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Enemy[i].rot.z);

			// �|���S���`��
			GetDeviceContext()->Draw(4, 0);
		}
	}



}




//=============================================================================
// Enemy�\���̂̐擪�A�h���X���擾
//=============================================================================
ENEMY* GetEnemy(void)
{
	return &g_Enemy[0];
}


void Straight(XMFLOAT3 startPos, XMFLOAT3 endPos, float speed)	//�G�l�~�[�̓���Ǘ�
{



}


//=============================================================================
// �J�E���g�����Z����
// ����:add :�ǉ�����_���B�}�C�i�X���\
//=============================================================================
void AddCount(int add)
{
	g_Count += add;
	if (g_Count > GAME_COUNT)
	{
		g_Count = GAME_COUNT;
	}

}




