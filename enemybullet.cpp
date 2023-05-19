//=============================================================================
//
// バレット処理 [bullet.cpp]
// Author : 
//
//=============================================================================
#include "enemy.h"
#include "enemy_bullet.h"
#include "collision.h"
#include "score.h"
#include "bg.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(40/2)	// キャラサイズ
#define TEXTURE_HEIGHT				(40/2)	// 
#define TEXTURE_MAX					(1)		// テクスチャの数

#define TEXTURE_PATTERN_DIVIDE_X	(1)		// アニメパターンのテクスチャ内分割数（X)
#define TEXTURE_PATTERN_DIVIDE_Y	(1)		// アニメパターンのテクスチャ内分割数（Y)
#define ANIM_PATTERN_NUM			(TEXTURE_PATTERN_DIVIDE_X*TEXTURE_PATTERN_DIVIDE_Y)	// アニメーションパターン数
#define ANIM_WAIT					(4)		// アニメーションの切り替わるWait値


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;				// 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char* g_TexturName[] = {
	"data/TEXTURE/bullet00.png",
};

static BOOL		g_Load = FALSE;			// 初期化を行ったかのフラグ
static ENEMYBULLET	g_EnemyBullet[ENEMY_BULLET_MAX];	// バレット構造体


//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemyBullet(void)
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


	// バレット構造体の初期化
	for (int i = 0; i < ENEMY_BULLET_MAX; i++)
	{
		g_EnemyBullet[i].use = FALSE;			// 未使用（発射されていない弾）
		g_EnemyBullet[i].w = TEXTURE_WIDTH;
		g_EnemyBullet[i].h = TEXTURE_HEIGHT;
		g_EnemyBullet[i].pos = XMFLOAT3(300, 300.0f, 0.0f);
		g_EnemyBullet[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_EnemyBullet[i].texNo = 0;

		g_EnemyBullet[i].countAnim = 0;
		g_EnemyBullet[i].patternAnim = 0;

		g_EnemyBullet[i].move = XMFLOAT3(0.0f, +ENEMY_BULLET_SPEED, 0.0f);	// 移動量を初期化
	}

	g_Load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
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
// 更新処理
//=============================================================================
void UpdateEnemyBullet(void)
{
	int bulletCount = 0;				// 処理したバレットの数

	for (int i = 0; i < ENEMY_BULLET_MAX; i++)
	{
		if (g_EnemyBullet[i].use == TRUE)	// このバレットが使われている？
		{								// Yes
			// アニメーション  
			g_EnemyBullet[i].countAnim++;
			if ((g_EnemyBullet[i].countAnim % ANIM_WAIT) == 0)
			{
				// パターンの切り替え
				g_EnemyBullet[i].patternAnim = (g_EnemyBullet[i].patternAnim + 1) % ANIM_PATTERN_NUM;
			}

			// バレットの移動処理
			XMVECTOR pos = XMLoadFloat3(&g_EnemyBullet[i].pos);
			XMVECTOR move = XMLoadFloat3(&g_EnemyBullet[i].move);
			pos += move;
			XMStoreFloat3(&g_EnemyBullet[i].pos, pos);

			// 画面外まで進んだ？
			BG* bg = GetBG();
			if (g_EnemyBullet[i].pos.y < (-g_EnemyBullet[i].h / 2))		// 自分の大きさを考慮して画面外か判定している
			{
				g_EnemyBullet[i].use = false;
			}
			if (g_EnemyBullet[i].pos.y > (bg->h + g_EnemyBullet[i].h / 2))	// 自分の大きさを考慮して画面外か判定している
			{
				g_EnemyBullet[i].use = false;
			}

			// 当たり判定処理
			{
				ENEMY* enemy = GetEnemy();

				// エネミーの数分当たり判定を行う
				for (int j = 0; j < ENEMY_MAX; j++)
				{
					// 生きてるエネミーと当たり判定をする
					if (enemy[j].use == TRUE)
					{
						BOOL ans = CollisionBB(g_EnemyBullet[i].pos, g_EnemyBullet[i].w, g_EnemyBullet[i].h,
							enemy[j].pos, enemy[j].w, enemy[j].h);
						// 当たっている？
						if (ans == TRUE)
						{
							// 当たった時の処理

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
// 描画処理
//=============================================================================
void DrawEnemyBullet(void)
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

	for (int i = 0; i < ENEMY_BULLET_MAX; i++)
	{
		if (g_EnemyBullet[i].use == TRUE)		// このバレットが使われている？
		{									// Yes
			// テクスチャ設定
			GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_EnemyBullet[i].texNo]);

			//バレットの位置やテクスチャー座標を反映
			float px = g_EnemyBullet[i].pos.x - bg->pos.x;	// バレットの表示位置X
			float py = g_EnemyBullet[i].pos.y - bg->pos.y;	// バレットの表示位置Y
			float pw = g_EnemyBullet[i].w;		// バレットの表示幅
			float ph = g_EnemyBullet[i].h;		// バレットの表示高さ

			float tw = 1.0f / TEXTURE_PATTERN_DIVIDE_X;	// テクスチャの幅
			float th = 1.0f / TEXTURE_PATTERN_DIVIDE_Y;	// テクスチャの高さ
			float tx = (float)(g_EnemyBullet[i].patternAnim % TEXTURE_PATTERN_DIVIDE_X) * tw;	// テクスチャの左上X座標
			float ty = (float)(g_EnemyBullet[i].patternAnim / TEXTURE_PATTERN_DIVIDE_X) * th;	// テクスチャの左上Y座標

			// １枚のポリゴンの頂点とテクスチャ座標を設定
			SetSpriteColorRotation(g_VertexBuffer,
				px, py, pw, ph,
				tx, ty, tw, th,
				XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
				g_EnemyBullet[i].rot.z);

			// ポリゴン描画
			GetDeviceContext()->Draw(4, 0);
		}
	}

}


//=============================================================================
// バレット構造体の先頭アドレスを取得
//=============================================================================
ENEMYBULLET* GetEnemyBullet(void)
{
	return &g_EnemyBullet[0];
}


//=============================================================================
// バレットの発射設定
//=============================================================================
void SetEnemyBullet(XMFLOAT3 pos)
{
	// もし未使用の弾が無かったら発射しない( =これ以上撃てないって事 )
	for (int i = 0; i < ENEMY_BULLET_MAX; i++)
	{
		if (g_EnemyBullet[i].use == FALSE)		// 未使用状態のバレットを見つける
		{
			g_EnemyBullet[i].use = TRUE;			// 使用状態へ変更する
			g_EnemyBullet[i].pos = pos;			// 座標をセット
			return;							// 1発セットしたので終了する
		}
	}
}
