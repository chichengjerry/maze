/***********************************************
 * [main.h]
 * メイン処理
 ***********************************************/
#ifndef _MAIN_H_
#define _MAIN_H_

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。

#define DIRECTINPUT_VERSION	(0x0800)	/* 警告対策 */

#include <windows.h>

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>

#include <vector>

#include "d3dx9.h"
#include "dinput.h"
#include "mmsystem.h"

#include "xaudio2.h"

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "winmm.lib")

#endif // ! _MAIN_H_
