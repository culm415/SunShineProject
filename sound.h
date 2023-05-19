//=============================================================================
//
// サウンド処理 [sound.h]
// Author : 岩田　輝

//
//=============================================================================
#pragma once

#include <windows.h>
#include "xaudio2.h"						// サウンド処理で必要

//*****************************************************************************
// サウンドファイル
//*****************************************************************************
enum 
{
	SOUND_LABEL_BGM_gamemode,	// BGM1
	SOUND_LABEL_BGM_result,	// BGM2
	SOUND_LABEL_SE_bomb000,		// 爆発音
	SOUND_LABEL_SE_shot,		// 

	SOUND_LABEL_BGM_title,		// BGM Maou

	SOUND_LABEL_MAX,
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
bool InitSound(HWND hWnd);
void UninitSound(void);
void PlaySound(int label);
void StopSound(int label);
void StopSound(void);

