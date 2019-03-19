/***********************************************
 * [input.cpp]
 * インプット処理
 ***********************************************/
#include "input.h"
#include "d3d.h"

#define GP_DEADZONE				2500			// 各軸の25%を無効ゾーンとする
#define GP_RANGE_MAX			1000			// 有効範囲の最大値
#define GP_RANGE_MIN			-1000			// 有効範囲の最小値

LPDIRECTINPUT8			DINPUT::pDInput = NULL;
LPDIRECTINPUTDEVICE8	DINPUT::pDIDevKeyboard = NULL;
BYTE				   	DINPUT::keyStates[NUM_KEY_MAX] = { NULL };		 	// キーボードの押下状態を保持するワーク
BYTE				   	DINPUT::keyStatesTrigger[NUM_KEY_MAX] = { NULL };	// キーボードのトリガー状態を保持するワーク
BYTE				   	DINPUT::keyStatesRelease[NUM_KEY_MAX] = { NULL };	// キーボードのリリース状態を保持するワーク
BYTE				   	DINPUT::keyStatesRepeat[NUM_KEY_MAX] = { NULL }; 	// キーボードのリピート状態を保持するワーク
int				   		DINPUT::keyStatesRepeatCnt[NUM_KEY_MAX] = { NULL };	// キーボードのリピートカウンタ


LPDIRECTINPUTDEVICE8	DINPUT::pMouse = NULL;			// mouse

DIMOUSESTATE2			DINPUT::mouseState = {};		// マウスのダイレクトな状態
DIMOUSESTATE2			DINPUT::mouseTrigger = {};	// 押された瞬間だけON
long					DINPUT::mouseMovement[] = { 0, 0, 0 };

LPDIRECTINPUTDEVICE8	DINPUT::pGamePad[NUM_PAD_MAX] = { NULL };// パッドデバイス
DWORD					DINPUT::padState[NUM_PAD_MAX] = { NULL };	// パッド情報（複数対応）
DWORD					DINPUT::padTrigger[NUM_PAD_MAX] = { NULL };
UINT					DINPUT::padCount = 0;			// 検出したパッドの数


BOOL DINPUT::KeyPressed(BYTE key)
{
	return !!(keyStates[key] & 0x80);
}

BOOL DINPUT::KeyTriggered(BYTE key)
{
	return !!(keyStatesTrigger[key] & 0x80);
}

BOOL DINPUT::KeyRepeated(BYTE key)
{
	return !!(keyStatesRepeat[key] & 0x80);
}

BOOL DINPUT::KeyReleased(BYTE key)
{
	return !!(keyStatesRelease[key] & 0x80);;
}

BOOL DINPUT::ButtonPressed(int padNo, DWORD button)
{
	return (button & padState[padNo]);
}

BOOL DINPUT::ButtonPressed(DWORD button)
{
	for (UINT i = 0; i < padCount; i++) {
		if (ButtonPressed(i, button))
			return TRUE;
	}
	return FALSE;
}

BOOL DINPUT::ButtonTriggered(int padNo, DWORD button)
{
	return (button & padTrigger[padNo]);
}

BOOL DINPUT::ButtonTriggered(DWORD button)
{
	for (UINT i = 0; i < padCount; i++) {
		if (ButtonTriggered(i, button))
			return TRUE;
	}
	return FALSE;
}
BOOL DINPUT::MouseLeftPressed(void)
{
	return (BOOL)(mouseState.rgbButtons[0] & 0x80);	// 押されたときに立つビットを検査
}
BOOL DINPUT::MouseLeftTriggered(void)
{
	return (BOOL)(mouseTrigger.rgbButtons[0] & 0x80);
}
BOOL DINPUT::MouseRightPressed(void)
{
	return (BOOL)(mouseState.rgbButtons[1] & 0x80);
}
BOOL DINPUT::MouseRightTriggered(void)
{
	return (BOOL)(mouseTrigger.rgbButtons[1] & 0x80);
}
BOOL DINPUT::MouseCenterPressed(void)
{
	return (BOOL)(mouseState.rgbButtons[2] & 0x80);
}
BOOL DINPUT::MouseCenterTriggered(void)
{
	return (BOOL)(mouseTrigger.rgbButtons[2] & 0x80);
}
//------------------
long DINPUT::MouseX(void)
{
	return mouseState.lX;
}
long DINPUT::MouseY(void)
{
	return mouseState.lY;
}
long DINPUT::MouseZ(void)
{
	return mouseState.lZ;
}

HRESULT DINPUT::Init(HINSTANCE hInst, HWND hWnd)
{
	InitKeyboard(hInst, hWnd);
	InitGamePad(hInst, hWnd);
	InitMouse(hInst, hWnd);

	return S_OK;
}

HRESULT DINPUT::InitKeyboard(HINSTANCE hInst, HWND hWnd)
{
	HRESULT hr;

	if (!pDInput)
	{
		// DirectInputオブジェクトの作成
		hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
			IID_IDirectInput8, (void**)&pDInput, NULL);
	}

	// デバイスオブジェクトを作成
	hr = pDInput->CreateDevice(GUID_SysKeyboard, &pDIDevKeyboard, NULL);
	if (FAILED(hr) || pDIDevKeyboard == NULL)
	{
		MessageBox(hWnd, _T("キーボードがねぇ！"), _T("警告！"), MB_ICONWARNING);
		return hr;
	}

	// データフォーマットを設定
	hr = pDIDevKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr))
	{
		MessageBox(hWnd, _T("キーボードのデータフォーマットを設定できませんでした。"), _T("警告！"), MB_ICONWARNING);
		return hr;
	}

	// 協調モードを設定（フォアグラウンド＆非排他モード）
	hr = pDIDevKeyboard->SetCooperativeLevel(hWnd, (DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
	if (FAILED(hr))
	{
		MessageBox(hWnd, _T("キーボードの協調モードを設定できませんでした。"), _T("警告！"), MB_ICONWARNING);
		return hr;
	}

	// キーボードへのアクセス権を獲得(入力制御開始)
	hr = pDIDevKeyboard->Acquire();

	return hr;
}

HRESULT DINPUT::InitGamePad(HINSTANCE hInst, HWND hWnd)
{
	HRESULT		hr;
	padCount = 0;

	// ジョイパッドを探す
	if (!pDInput) {
		hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
			IID_IDirectInput8, (void**)&pDInput, NULL);

		if (FAILED(hr)) {
			return hr;
		}
	}

	pDInput->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK)SearchGamePadCallback, NULL, DIEDFL_ATTACHEDONLY);
	// セットしたコールバック関数が、パッドを発見した数だけ呼ばれる。

	for (UINT i = 0; i < padCount; i++) {
		// ジョイスティック用のデータ・フォーマットを設定
		hr = pGamePad[i]->SetDataFormat(&c_dfDIJoystick);
		if (FAILED(hr)) {
			return hr;
		}

		// モードを設定（フォアグラウンド＆非排他モード）
//		hr = pGamePad[i]->SetCooperativeLevel(hWindow, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
//		if ( FAILED(hr) )
//			return false; // モードの設定に失敗

		// 軸の値の範囲を設定
		// X軸、Y軸のそれぞれについて、オブジェクトが報告可能な値の範囲をセットする。
		// (max-min)は、最大10,000(?)。(max-min)/2が中央値になる。
		// 差を大きくすれば、アナログ値の細かな動きを捕らえられる。(パッドの性能による)
		DIPROPRANGE				diprg;
		ZeroMemory(&diprg, sizeof(diprg));
		diprg.diph.dwSize = sizeof(diprg);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);
		diprg.diph.dwHow = DIPH_BYOFFSET;
		diprg.lMin = GP_RANGE_MIN;
		diprg.lMax = GP_RANGE_MAX;
		// X軸の範囲を設定
		diprg.diph.dwObj = DIJOFS_X;
		pGamePad[i]->SetProperty(DIPROP_RANGE, &diprg.diph);
		// Y軸の範囲を設定
		diprg.diph.dwObj = DIJOFS_Y;
		pGamePad[i]->SetProperty(DIPROP_RANGE, &diprg.diph);

		// Z軸の範囲を設定
		diprg.diph.dwObj = DIJOFS_Z;
		pGamePad[i]->SetProperty(DIPROP_RANGE, &diprg.diph);
		// Z回転の範囲を設定
		diprg.diph.dwObj = DIJOFS_RZ;
		pGamePad[i]->SetProperty(DIPROP_RANGE, &diprg.diph);

		// 各軸ごとに、無効のゾーン値を設定する。
		// 無効ゾーンとは、中央からの微少なジョイスティックの動きを無視する範囲のこと。
		// 指定する値は、10000に対する相対値(2000なら20パーセント)。
		DIPROPDWORD				dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
		dipdw.diph.dwHow = DIPH_BYOFFSET;
		dipdw.dwData = GP_DEADZONE;
		// X軸の無効ゾーンを設定
		dipdw.diph.dwObj = DIJOFS_X;
		pGamePad[i]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
		// Y軸の無効ゾーンを設定
		dipdw.diph.dwObj = DIJOFS_Y;
		pGamePad[i]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
		// Z軸の無効ゾーンを設定
		dipdw.diph.dwObj = DIJOFS_Z;
		pGamePad[i]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
		// Z回転の無効ゾーンを設定
		dipdw.diph.dwObj = DIJOFS_RZ;
		pGamePad[i]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

		//ジョイスティック入力制御開始
		pGamePad[i]->Acquire();
	}

	return S_OK;
}

HRESULT DINPUT::InitMouse(HINSTANCE hInst, HWND hWnd)
{
	HRESULT hr;
	// デバイス作成
	hr = pDInput->CreateDevice(GUID_SysMouse, &pMouse, NULL);
	if (FAILED(hr) || pMouse == NULL)
	{
		MessageBox(hWnd, _T("No mouse"), _T("Warning"), MB_OK | MB_ICONWARNING);
		return hr;
	}
	// データフォーマット設定
	hr = pMouse->SetDataFormat(&c_dfDIMouse2);
	if (FAILED(hr))
	{
		MessageBox(hWnd, _T("Can't setup mouse"), _T("Warning"), MB_OK | MB_ICONWARNING);
		return hr;
	}
	// 他のアプリと協調モードに設定
	hr = pMouse->SetCooperativeLevel(hWnd, (DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
	if (FAILED(hr))
	{
		MessageBox(hWnd, _T("Mouse mode error"), _T("Warning"), MB_OK | MB_ICONWARNING);
		return hr;
	}

	// デバイスの設定
	DIPROPDWORD prop;

	prop.diph.dwSize = sizeof(prop);
	prop.diph.dwHeaderSize = sizeof(prop.diph);
	prop.diph.dwObj = 0;
	prop.diph.dwHow = DIPH_DEVICE;
	prop.dwData = DIPROPAXISMODE_REL;		// マウスの移動値　相対値

	hr = pMouse->SetProperty(DIPROP_AXISMODE, &prop.diph);
	if (FAILED(hr))
	{
		MessageBox(hWnd, _T("Mouse property error"), _T("Warning"), MB_OK | MB_ICONWARNING);
		return hr;
	}

	// アクセス権を得る
	pMouse->Acquire();
	return hr;
}

void DINPUT::Destroy(void)
{
#define SAFE_UNAQUIRE(p)	{ if(p) {(p)->Unacquire(); (p)->Release(); } }
	for (int i = 0; i < NUM_PAD_MAX; i++) {
		SAFE_UNAQUIRE(pGamePad[i]);
	}
	SAFE_RELEASE(DINPUT::pMouse);
	SAFE_UNAQUIRE(DINPUT::pDIDevKeyboard);
	SAFE_RELEASE(DINPUT::pDInput);
#undef SAFE_UNAQUIRE
}

HRESULT DINPUT::Update(void)
{
	UpdateKeyboard();
	UpdateGamePad();
	UpdateMouse();

	return S_OK;
}

HRESULT DINPUT::UpdateKeyboard(void)
{
	HRESULT hr;
	BYTE keyStatesOld[NUM_KEY_MAX];

	memcpy(keyStatesOld, keyStates, NUM_KEY_MAX);

	// デバイスからデータを取得
	hr = pDIDevKeyboard->GetDeviceState(sizeof(keyStates), keyStates);

	if (SUCCEEDED(hr))
	{
		for (int nCntKey = 0; nCntKey < NUM_KEY_MAX; nCntKey++)
		{
			keyStatesTrigger[nCntKey] = (keyStatesOld[nCntKey] ^ keyStates[nCntKey]) & keyStates[nCntKey];
			keyStatesRelease[nCntKey] = (keyStatesOld[nCntKey] ^ keyStates[nCntKey]) & ~keyStates[nCntKey];
			keyStatesRepeat[nCntKey] = keyStatesTrigger[nCntKey];

			if (keyStates[nCntKey])
			{
				keyStatesRepeatCnt[nCntKey]++;
				if (keyStatesRepeatCnt[nCntKey] >= 20)
				{
					keyStatesRepeat[nCntKey] = keyStates[nCntKey];
				}
				else
				{
					keyStatesRepeat[nCntKey] = 0;
				}
			}
			else
			{
				keyStatesRepeatCnt[nCntKey] = 0;
				keyStatesRepeat[nCntKey] = 0;
			}

			keyStates[nCntKey] = keyStates[nCntKey];
		}
	}
	else
	{
		// キーボードへのアクセス権を取得
		pDIDevKeyboard->Acquire();
	}

	return S_OK;
}

HRESULT DINPUT::UpdateGamePad(void)
{
	HRESULT			result = S_OK;
	DIJOYSTATE2		dijs;

	for (UINT i = 0; i < padCount; i++)
	{
		DWORD lastPadState;
		lastPadState = padState[i];
		padState[i] = 0x0UL;	// 初期化

		result = pGamePad[i]->Poll();	// ジョイスティックにポールをかける
		if (FAILED(result)) {
			result = pGamePad[i]->Acquire();
			while (result == DIERR_INPUTLOST) {
				result = pGamePad[i]->Acquire();
			}
		}

		result = pGamePad[i]->GetDeviceState(sizeof(DIJOYSTATE), &dijs);	// デバイス状態を読み取る
		if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
			result = pGamePad[i]->Acquire();
			while (result == DIERR_INPUTLOST) {
				result = pGamePad[i]->Acquire();
			}
		}

		// ３２の各ビットに意味を持たせ、ボタン押下に応じてビットをオンにする
		//* y-axis (forward)
		if (dijs.lY < 0)					padState[i] |= BUTTON_MOVE_UP;
		//* y-axis (backward)
		if (dijs.lY > 0)					padState[i] |= BUTTON_MOVE_DOWN;
		//* x-axis (left)
		if (dijs.lX < 0)					padState[i] |= BUTTON_MOVE_LEFT;
		//* x-axis (right)
		if (dijs.lX > 0)					padState[i] |= BUTTON_MOVE_RIGHT;
		
		//* z-axis (rotate conterclockwise)
		if (dijs.lRz < 0)					padState[i] |= BUTTON_ROTATE_UP;
		//* z-axis (rotate clockwise)
		if (dijs.lRz > 0)					padState[i] |= BUTTON_ROTATE_DOWN;
		//* z-axis (left)
		if (dijs.lZ < 0)					padState[i] |= BUTTON_ROTATE_LEFT;
		//* z-axis (right)
		if (dijs.lZ > 0)					padState[i] |= BUTTON_ROTATE_RIGHT;

		//	//* y-axis (forward)
		//	if (dijs.lRy < 0)					padState[i] |= BUTTON_ROTATE_UP;
		//	//* y-axis (backward)
		//	if (dijs.lRy > 0)					padState[i] |= BUTTON_ROTATE_DOWN;
		//	//* x-axis (left)
		//	if (dijs.lRx < 0)					padState[i] |= BUTTON_ROTATE_LEFT;
		//	//* x-axis (right)
		//	if (dijs.lRx > 0)					padState[i] |= BUTTON_ROTATE_RIGHT;

		//* Ａボタン
		if (dijs.rgbButtons[0] & 0x80)		padState[i] |= BUTTON_X;
		//* Ｂボタン
		if (dijs.rgbButtons[1] & 0x80)		padState[i] |= BUTTON_A;
		//* Ｃボタン
		if (dijs.rgbButtons[2] & 0x80)		padState[i] |= BUTTON_B;
		//* Ｘボタン
		if (dijs.rgbButtons[3] & 0x80)		padState[i] |= BUTTON_Y;
		//* Ｙボタン
		if (dijs.rgbButtons[4] & 0x80)		padState[i] |= BUTTON_LB; //LB
		//* Ｚボタン
		if (dijs.rgbButtons[5] & 0x80)		padState[i] |= BUTTON_RB; //RB
		//* Ｌボタン
		if (dijs.rgbButtons[6] & 0x80)		padState[i] |= BUTTON_LT; //LT
		//* Ｒボタン
		if (dijs.rgbButtons[7] & 0x80)		padState[i] |= BUTTON_RT; //RT
		//* ＳＴＡＲＴボタン
		if (dijs.rgbButtons[8] & 0x80)		padState[i] |= BUTTON_BACK; //BACK
		//* Ｍボタン
		if (dijs.rgbButtons[9] & 0x80)		padState[i] |= BUTTON_START; //START

		// Trigger設定
		padTrigger[i] = ((lastPadState ^ padState[i])	// 前回と違っていて
			& padState[i]);					// しかも今ONのやつ

	}
	return result;
}

HRESULT DINPUT::UpdateMouse(void)
{
	HRESULT hr;
	// 前回の値保存
	DIMOUSESTATE2 lastMouseState = mouseState;
	// データ取得
	hr = pMouse->GetDeviceState(sizeof(mouseState), &mouseState);
	if (SUCCEEDED(hr))
	{
		mouseMovement[0] = mouseState.lX - mouseTrigger.lX;
		mouseTrigger.lX = mouseState.lX;
		mouseMovement[1] = mouseState.lY - mouseTrigger.lY;
		mouseTrigger.lY = mouseState.lY;
		mouseMovement[2] = mouseState.lZ - mouseTrigger.lZ;
		mouseTrigger.lZ = mouseState.lZ;
		// マウスのボタン状態
		for (int i = 0; i < 8; i++)
		{
			mouseTrigger.rgbButtons[i] = ((lastMouseState.rgbButtons[i] ^
				mouseState.rgbButtons[i]) & mouseState.rgbButtons[i]);
		}
	}
	else// 取得失敗
	{
		// アクセス権を得てみる
		hr = pMouse->Acquire();
	}
	return hr;
}

BOOL DINPUT::SearchGamePadCallback(LPDIDEVICEINSTANCE lpddi, LPVOID)
{
	HRESULT result;
	result = pDInput->CreateDevice(lpddi->guidInstance, &pGamePad[padCount++], NULL);

	return DIENUM_CONTINUE;	// 次のデバイスを列挙
}
