//=============================================================================
//
// エネミー処理 [enemy.cpp]
// Author : 岩田　輝
//
//=============================================================================
#include "enemy.h"
#include "bg.h"
#include "player.h"
#include "fade.h"
#include "collision.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(120/2)	// キャラサイズ
#define TEXTURE_HEIGHT				(120/2)	// 
#define TEXTURE_MAX					(2)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値
#define GAME_COUNT					(3500)	// フレームカウント

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/22286029.png",
};


static BOOL		g_Load = TRUE;			// 初期化を行ったかのフラグ
static ENEMY	g_Enemy[ENEMY_MAX];		// エネミー構造体
static int		g_EnemyCnt = ENEMY_MAX;
static int		g_Count;				// カウント


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{
	ID3D11Device* pDevice = GetDevice();

	//テクスチャ生成
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


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);


	// エネミー構造体の初期化
	g_EnemyCnt = 0;
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		g_Enemy[i].loop = 0;
		g_Enemy[i].use = FALSE;
		g_Enemy[i].pos = XMFLOAT3(100.0f + (i % 6) * 100.0f, 1.0f, 0.0f);	// 中心点から表示
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Enemy[i].w = TEXTURE_WIDTH;
		g_Enemy[i].h = TEXTURE_HEIGHT;
		g_Enemy[i].texNo = 0;
		g_Enemy[i].markedForDeath = FALSE;

		g_Enemy[i].countAnim = 0;
		g_Enemy[i].patternAnim = 0;

		g_Enemy[i].move = XMFLOAT3(4.0f, 0.0f, 0.0f);		// 移動量

		g_Enemy[i].time = 0.0f;			// 線形補間用のタイマーをクリア
		g_Enemy[i].timeAlive = 0.0f;	// TRUE(使われている時間)になっている時間をくりあ
		g_Enemy[i].delay = 0;			// 時間差をクリア
		g_Enemy[i].tblNo = 0;			// 再生する行動データテーブルNoをセット
		g_Enemy[i].tblMax = 0;			// 再生する行動データテーブルのレコード数をセット

		g_EnemyCnt++;
	}


	g_Count = 0;
	g_Load = TRUE;
	return S_OK;
}


//=============================================================================
// 終了処理
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
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{


	g_EnemyCnt = 0;	// 生きてるエネミーの数

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Count == 10 + i * 50)		// カウント
		{
			if (i < 500)					// 使えるエネミーの制限(数字の数まで使える)
			{
				g_Enemy[i].use = TRUE;
			}

		}

		// 生きてるエネミーだけ処理をする
		if (g_Enemy[i].use == TRUE)
		{
			g_Enemy[i].cnt++;
			g_Enemy[i].rot.z += 0.00f;

			// 地形との当たり判定用に座標のバックアップを取っておく
			XMFLOAT3 pos_old = g_Enemy[i].pos;

			// アニメーション
			g_Enemy[i].countAnim += 1.0f;
			if (g_Enemy[i].countAnim > ANIM_WAIT)
			{
				g_Enemy[i].countAnim = 0.0f;
				// パターンの切り替え
				g_Enemy[i].patternAnim = (g_Enemy[i].patternAnim + 1) % ANIM_PATTERN_NUM;
			}


			// エネミーの動作処理

			if (i < 6)  //敵の動作処理(基本的に６体ずつ）
			{
				int t = g_Enemy[i].cnt;
				int pattern_1 = 100;		//上からきて止まるまでのフレーム数
				int wait = 100;				//止まるフレーム数

				if (t < pattern_1)
				{
					g_Enemy[i].pos.x -= 1.0f;		//右に移動していく
					g_Enemy[i].pos.y += 2.5f;		//下に移動していく
				}
				else if(t == pattern_1)
				{
					g_Enemy[i].pos.y += 0; //停止信号
				}
				else if (t > pattern_1 + wait)
				{
					g_Enemy[i].pos.x += 1.0f;		//左に移動していく
					g_Enemy[i].pos.y += 2.5f;		//下に移動していく
				}
			}

			else if (i < 12)  //敵の動作処理
			{
				int t = g_Enemy[i].cnt;
				int pattern_2 = 100;		//上からきて止まるまでのフレーム数
				int wait = 100;				//止まるフレーム数

				if (t < pattern_2)
				{
					g_Enemy[i].pos.x += 1.0f;
					g_Enemy[i].pos.y += 2.5f;		//右下に移動していく
				}
				if (t == pattern_2)
				{
					g_Enemy[i].pos.y += 0;			//停止信号
				}
				if (t > pattern_2 + wait)
				{
					g_Enemy[i].pos.x -= 1.0f;
					g_Enemy[i].pos.y += 2.5f;		//左下に移動していく
				}
			}

			else if (i < 18)  //敵の動作処理
			{
				int t = g_Enemy[i].cnt;
				int pattern_3 = 100;		//上からきて止まるまでのフレーム数
				int wait = 100;				//止まるフレーム数

				if (t < pattern_3)
				{
					g_Enemy[i].pos.y += 2.5f;		//下がる
				}
				if (t == pattern_3)
				{
					g_Enemy[i].pos.y += 0;			//停止信号
				}
				if (t > pattern_3 + wait)
				{
					g_Enemy[i].pos.y += 2.5f;		//下がる
				}
			}

			else if (i < 24)  //敵の動作処理
			{
				int t = g_Enemy[i].cnt;
				int pattern_4 = 200;		//上からきて止まるまでのフレーム数

				if (t < pattern_4)
				{
					g_Enemy[i].pos.y += 4.0f;		//下がる
				}

			}

			else if (i < 30)  //敵の動作処理
			{
				int t = g_Enemy[i].cnt;
				int pattern_5 = 100;		//上からきて止まるまでのフレーム数
				int wait = 100;				//止まるフレーム数

				if (t < pattern_5)
				{
					g_Enemy[i].pos.y += 2.5f;		//下がる
				}
				if (t == pattern_5)
				{
					g_Enemy[i].pos.y += 0;			//停止信号
				}
				if (t > pattern_5 + wait)
				{
					g_Enemy[i].pos.y -= 2.5f;		//上昇
				}

			}

			else if (i < 36)  //敵の動作処理
			{
				int t = g_Enemy[i].cnt;
				int pattern_6 = 100;		//上からきて止まるまでのフレーム数
				int wait = 100;				//止まるフレーム数

				if (t < pattern_6)
				{
					g_Enemy[i].pos.y += 2.5f;		//下がる
				}
				if (t == pattern_6)
				{
					g_Enemy[i].pos.y += 0;			//停止信号
				}
				if (t > pattern_6 + wait)
				{
					g_Enemy[i].pos.x -= 2.5f;		//右に移動
				}

			}

			else if (i < 42)  //敵の動作処理
			{
				int t = g_Enemy[i].cnt;
				int pattern_7 = 100;		//上からきて止まるまでのフレーム数
				int wait = 100;				//止まるフレーム数

				if (t < pattern_7)
				{
					g_Enemy[i].pos.y += 2.5f;		//下がる
				}

				if (t == pattern_7)
				{
					g_Enemy[i].pos.y += 0;			//停止信号
				}

				if (t > pattern_7 + wait)
				{
					g_Enemy[i].pos.x += 2.5f;		//左に移動
				}

			}

			else if (i < 48)  //敵の動作処理
			{
				int t = g_Enemy[i].cnt;
				int pattern_8 = 100;		//上からきて止まるまでのフレーム数
				int wait = 100;				//止まるフレーム数

				if (t < pattern_8)
				{
					g_Enemy[i].pos.x -= 1.0f;		//右に移動していく
					g_Enemy[i].pos.y += 2.5f;		//下に移動していく
				}

				if (t == pattern_8)
				{
					g_Enemy[i].pos.y += 0;			//停止信号
				}

				if (t > pattern_8 + wait)
				{
					g_Enemy[i].pos.y += 2.5f;		//下がる
				}

			}

			else if (i < 54)  //敵の動作処理
			{
				int t = g_Enemy[i].cnt;
				int pattern_9 = 100;		//上からきて止まるまでのフレーム数
				int wait = 100;				//止まるフレーム数

				if (t < pattern_9)
				{
					g_Enemy[i].pos.x += 1.0f;		//左に移動していく
					g_Enemy[i].pos.y += 2.5f;		//下に移動していく
				}

				if (t == pattern_9)
				{
					g_Enemy[i].pos.y += 0;			//停止信号
				}

				if (t > pattern_9 + wait)
				{
					g_Enemy[i].pos.y += 2.5f;		//下がる
				}
			}

			else if (i < 60)  //敵の動作処理
			{
				int t = g_Enemy[i].cnt;
				int pattern_10 = 250;		//上からきて止まるまでのフレーム数

				if (t < pattern_10)
				{
					g_Enemy[i].pos.x += 1.0f;		//左に移動していく
					g_Enemy[i].pos.y += 2.5f;		//下に移動していく
				}

			}

			else if (i < 66)  //敵の動作処理
			{
				int t = g_Enemy[i].cnt;
				int pattern_10 = 250;		//上からきて止まるまでのフレーム数

				if (t < pattern_10)
				{
					g_Enemy[i].pos.x -= 1.0f;		//右に移動していく
					g_Enemy[i].pos.y += 2.5f;		//下に移動していく
				}

			}

			// 移動が終わったらプレイヤーとの当たり判定
			{
				PLAYER* player = GetPlayer();

				// プレイヤーの数分当たり判定を行う
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// 生きてるプレイヤーと当たり判定をする
					if (player[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_Enemy[i].pos, g_Enemy[i].w, g_Enemy[i].h,
							player[j].pos, player[j].w, player[j].h);
						// 当たっている？
						if (ans == TRUE)
						{
							// 当たった時の処理
						//	player[j].use = FALSE;	// デバッグで一時的に無敵にする
						}
					}
				}
			}


			g_EnemyCnt++;		// 生きてた敵の数
		}
	}

	if (g_Count >= 3500)	//3600フレーム(約1分)たったらゲームリザルトに行く
	{
		SetFade(FADE_OUT, MODE_RESULT);

	}

	//デバック用(コメントインするとタイトルからリザルトに行ける)
	// エネミー全滅したらリザルト画面に行く
	/*if (g_EnemyCnt >= 0)
	{
		SetFade(FADE_OUT, MODE_RESULT);
	}*/

#ifdef _DEBUG	// デバッグ情報を表示する


#endif

}



//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
{
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	BG* bg = GetBG();

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_Enemy[i].use == TRUE)			// このエネミーが使われている？
		{									// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_Enemy[i].texNo]);

			//エネミーの位置やテクスチャー座標を反映
			float px = g_Enemy[i].pos.x - bg->pos.x;	// エネミーの表示位置X
			float py = g_Enemy[i].pos.y - bg->pos.y;	// エネミーの表示位置Y
			float pw = g_Enemy[i].w;		// エネミーの表示幅
			float ph = g_Enemy[i].h;		// エネミーの表示高さ

			// アニメーション用
			//float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
			//float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
			//float tx = (float)(g_Player[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// テクスチャの左上X座標
			//float ty = (float)(g_Player[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// テクスチャの左上Y座標

			float tw = 1.0f;	// テクスチャの幅
			float th = 1.0f;	// テクスチャの高さ
			float tx = 0.0f;	// テクスチャの左上X座標
			float ty = 0.0f;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer, px, py, pw, ph, tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_Enemy[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}



}




//=============================================================================
// Enemy構造体の先頭アドレスを取得
//=============================================================================
ENEMY* GetEnemy(void)
{
	return &g_Enemy[0];
}


void Straight(XMFLOAT3 startPos, XMFLOAT3 endPos, float speed)	//エネミーの動作管理
{



}


//=============================================================================
// カウントを加算する
// 引数:add :追加する点数。マイナスも可能
//=============================================================================
void AddCount(int add)
{
	g_Count += add;
	if (g_Count > GAME_COUNT)
	{
		g_Count = GAME_COUNT;
	}

}




