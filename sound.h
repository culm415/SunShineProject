//=============================================================================
//
// �T�E���h���� [sound.h]
// Author : ��c�@�P

//
//=============================================================================
#pragma once

#include <windows.h>
#include "xaudio2.h"						// �T�E���h�����ŕK�v

//*****************************************************************************
// �T�E���h�t�@�C��
//*****************************************************************************
enum 
{
	SOUND_LABEL_BGM_gamemode,	// BGM1
	SOUND_LABEL_BGM_result,	// BGM2
	SOUND_LABEL_SE_bomb000,		// ������
	SOUND_LABEL_SE_shot,		// 

	SOUND_LABEL_BGM_title,		// BGM Maou

	SOUND_LABEL_MAX,
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
bool InitSound(HWND hWnd);
void UninitSound(void);
void PlaySound(int label);
void StopSound(int label);
void StopSound(void);

