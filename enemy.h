//=============================================================================
//
// �G�l�~�[���� [enemy.h]
// Author : ��c�@�P
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define ENEMY_MAX		(66)		// �G�l�~�[��Max�l��


//*****************************************************************************
// �\���̒�`
//*****************************************************************************

struct ENEMY
{

	short       loop;
	XMFLOAT3	pos;			// �|���S���̍��W

	XMFLOAT3	rot;			// �|���S���̉�]��
	XMFLOAT3	scl;			// �|���S���̊g��k��
	BOOL		use;			// true:�g���Ă���  false:���g�p
	BOOL		markedForDeath;	// use��false�ɂ���
	float		w, h;			// ���ƍ���
	float		countAnim;		// �A�j���[�V�����J�E���g
	int			patternAnim;	// �A�j���[�V�����p�^�[���i���o�[
	int			texNo;			// �e�N�X�`���ԍ�
	int         cnt;			
	XMFLOAT3	move;			// �ړ����x


	float		time;			// ���`��ԗp
	float		timeAlive;		// �����Ă��鎞��
	float		delay;			// ���`��Ԃ̎��ԍ�
	int			tblNo;			// �s���f�[�^�̃e�[�u���ԍ�
	int			tblMax;			// ���̃e�[�u���̃f�[�^��

	//INTERPOLATION_DATA* tbl_adr;			// �A�j���f�[�^�̃e�[�u���擪�A�h���X
	//int					tbl_size;			// �o�^�����e�[�u���̃��R�[�h����
	//float				move_time;			// ���s����
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);
void DrawPlayerHp(void);
void AddCount(int add);
void SetEnemies(BOOL startR, int enemyAmount, XMFLOAT3 pos, float space);


ENEMY* GetEnemy(void);



