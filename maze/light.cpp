/***********************************************
 * [light.cpp]
 * ライト処理
 ***********************************************/
#include "light.h"
#include "d3d.h"

#define ROTATE_SPEED		D3DXToRadian(2.0f)
#define SUN_COLOR_DAY		D3DXCOLOR(D3DCOLOR_ARGB(255, 255, 255, 255))
#define SUN_COLOR_NIGHT		D3DXCOLOR(D3DCOLOR_ARGB(255,  86,  90, 114))
// #define SUN_COLOR_NIGHT		D3DXCOLOR(D3DCOLOR_ARGB(255, 128, 128, 128))
#define SUN_COLOR_DAWN		D3DXCOLOR(D3DCOLOR_ARGB(255, 240, 187, 117))
#define SKY_COLOR_DAY		SUN_COLOR_DAY
#define SKY_COLOR_NIGHT		D3DXCOLOR(D3DCOLOR_ARGB(255, 128, 128, 128))
#define SKY_COLOR_DAWN		SUN_COLOR_DAY
//#define SKY_COLOR_NIGHT		D3DXCOLOR(D3DCOLOR_ARGB(255,  43,  45,  57))
//#define SKY_COLOR_DAWN		D3DXCOLOR(D3DCOLOR_ARGB(255, 235, 155,  71))
#define FOG_COLOR			D3DXCOLOR(D3DCOLOR_ARGB(255, 122, 145, 188))
#define COLOR_AMBIENT		D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f)
#define SUN_DIRECTION		D3DXVECTOR3(0.5f, 1.0f, 0.0f)
SUNLIGHT::SUNLIGHT()
{
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();
	D3DXVECTOR3 vecDir;

	ZeroMemory(&lightSun, sizeof(D3DLIGHT9));
	lightSun.Type = D3DLIGHT_DIRECTIONAL;
	lightSun.Diffuse = SUN_COLOR_DAY;
	lightSun.Ambient = COLOR_AMBIENT;
	vecDir = SUN_DIRECTION;
	D3DXVec3Normalize((D3DXVECTOR3*)&lightSun.Direction, &vecDir);
	pDevice->SetLight(0, &lightSun);
	pDevice->LightEnable(0, TRUE);

	ZeroMemory(&lightMoon, sizeof(D3DLIGHT9));
	lightMoon.Type = D3DLIGHT_DIRECTIONAL;
	lightMoon.Diffuse = SUN_COLOR_NIGHT;
	lightMoon.Ambient = COLOR_AMBIENT;
	vecDir = D3DXVECTOR3(0.5f, -1.0f, 0.0f);
	D3DXVec3Normalize((D3DXVECTOR3*)&lightMoon.Direction, &vecDir);
	pDevice->SetLight(1, &lightMoon);
	pDevice->LightEnable(1, TRUE);

	pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
}

SUNLIGHT::~SUNLIGHT()
{
}

void SUNLIGHT::Update()
{
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	D3DXVECTOR3 src;
	D3DXVECTOR4 temp = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
	D3DXMATRIX rot;
	//D3DXMatrixRotationYawPitchRoll(&rot, D3DXToRadian(1.0f) * D3D::fAnimationRate, ROTATE_SPEED * D3D::fAnimationRate, ROTATE_SPEED * D3D::fAnimationRate);
	D3DXMatrixIdentity(&rot);
	D3DXVec3Transform(&temp, (D3DXVECTOR3*)&lightSun.Direction, &rot);
	src = D3DXVECTOR3(temp.x, temp.y, temp.z);

	D3DXVec3Normalize(&src, &src);
	lightSun.Direction = *(D3DVECTOR*)&src;

	isDayLight = src.y < 0.0f;
	lightSun.Diffuse = GetSunColor();
	lightMoon.Diffuse = GetSunColor();

	// if (isDayLight) {
	// 	pDevice->LightEnable(0, TRUE);
	// 	pDevice->LightEnable(1, FALSE);
	// }
	// else {
	// 	pDevice->LightEnable(0, FALSE);
	// 	pDevice->LightEnable(1, TRUE);
	// }
	pDevice->SetLight(0, &lightSun);
	pDevice->SetLight(1, &lightMoon);
}

D3DXCOLOR SUNLIGHT::GetSkyColor()
{
	D3DXCOLOR col;

	FLOAT t = min(fabsf(lightSun.Direction.y), 0.5f) * 2;

	if (isDayLight) {
		D3DXColorLerp(&col, &SKY_COLOR_DAWN, &SKY_COLOR_DAY, -t * (t - 2));
	}
	else {
		D3DXColorLerp(&col, &SKY_COLOR_DAWN, &SKY_COLOR_NIGHT, -t * (t - 2));
	}
	return col;
}

D3DXCOLOR SUNLIGHT::GetFogColor()
{
	D3DXCOLOR col;

	FLOAT t = min(fabsf(lightSun.Direction.y), 0.5f) * 2;

	if (isDayLight) {
		D3DXColorLerp(&col, &SKY_COLOR_DAWN, &FOG_COLOR, -t * (t - 2));
	}
	else {
		D3DXColorLerp(&col, &SKY_COLOR_DAWN, &SKY_COLOR_NIGHT, -t * (t - 2));
	}
	return col;
}
D3DXCOLOR SUNLIGHT::GetSunColor()
{
	D3DXCOLOR col;

	FLOAT t = min(fabsf(lightSun.Direction.y), 0.5f) * 2;

	if (isDayLight) {
		D3DXColorLerp(&col, &SUN_COLOR_DAWN, &SUN_COLOR_DAY, -t * (t - 2));
	}
	else {
		D3DXColorLerp(&col, &SUN_COLOR_DAWN, &SUN_COLOR_NIGHT, -t * (t - 2));
		//col = SUN_COLOR_NIGHT;
	}
	return col;
}
