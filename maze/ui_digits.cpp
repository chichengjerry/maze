/***********************************************
 * [ui_digits.cpp]
 * 数値UI処理
 ***********************************************/
#include "ui_digits.h"

#define	IMAGE_DIGIT		_T("data/TEXTURE/ui/digits.png")

DWORD				DIGITIMAGE::count = 0;
LPDIRECT3DTEXTURE9	DIGITIMAGE::pTex = NULL;

DIGITIMAGE::DIGITIMAGE(INT digit, D3DRECT* rect)
{
	if (!count++) {
		D3D::LoadTexture(&pTex, IMAGE_DIGIT);
	}
	this->digit = -1;
	image = new IMAGE(pTex, rect);
	SetDigit(digit);
}

DIGITIMAGE::~DIGITIMAGE()
{
	SAFE_DELETE(image);
	if (!--count) {
		SAFE_RELEASE(pTex);
	}
}

void DIGITIMAGE::SetDigit(INT digit)
{
	if (this->digit != digit) {
		this->digit = digit;
		image->nFrameIndex = digit;
		image->nFrameTotal = 10;
		image->SetTexture();
	}
}

HRESULT DIGITIMAGE::Draw()
{
	return image->Draw();
}

void DIGITIMAGE::Update()
{
}
