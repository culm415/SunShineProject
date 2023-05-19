//=============================================================================
//
// エネミー処理 [enemy.h]
// Author : 岩田　輝
//
//=============================================================================
#pragma once

#include "main.h"
#include "renderer.h"
#include "debugproc.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define ENEMY_MAX		(66)		// エネミーのMax人数


//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct ENEMY
{

	short       loop;
	XMFLOAT3	pos;			// ポリゴンの座標

	XMFLOAT3	rot;			// ポリゴンの回転量
	XMFLOAT3	scl;			// ポリゴンの拡大縮小
	BOOL		use;			// true:使っている  false:未使用
	BOOL		markedForDeath;	// useをfalseにする
	float		w, h;			// 幅と高さ
	float		countAnim;		// アニメーションカウント
	int			patternAnim;	// アニメーションパターンナンバー
	int			texNo;			// テクスチャ番号
	int         cnt;			
	XMFLOAT3	move;			// 移動速度


	float		time;			// 線形補間用
	float		timeAlive;		// 生きている時間
	float		delay;			// 線形補間の時間差
	int			tblNo;			// 行動データのテーブル番号
	int			tblMax;			// そのテーブルのデータ数

	//INTERPOLATION_DATA* tbl_adr;			// アニメデータのテーブル先頭アドレス
	//int					tbl_size;			// 登録したテーブルのレコード総数
	//float				move_time;			// 実行時間
};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitEnemy(void);
void UninitEnemy(void);
void UpdateEnemy(void);
void DrawEnemy(void);
void DrawPlayerHp(void);
void AddCount(int add);
void SetEnemies(BOOL startR, int enemyAmount, XMFLOAT3 pos, float space);


ENEMY* GetEnemy(void);



