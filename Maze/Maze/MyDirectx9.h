#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <d3d.h>
#include <d3dx9.h>
#include <time.h>
#include <iostream>
#include <string>
#include <tchar.h>
#include <Mmsystem.h>
#include <cmath>
#include <vector>

#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

//宏定义
#define SCREEN_WIDTH    1280                     //为窗口宽度定义的宏，以方便在此处修改窗口宽度  
#define SCREEN_HEIGHT   720                         //为窗口高度定义的宏，以方便在此处修改窗口高度  
#define WINDOW_TITLE    _T("Maze") //为窗口标题定义的宏  
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }      //自定义一个SAFE_RELEASE()宏,便于资源的释放  
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define FVF_VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)  //顶点设定,表明它包含了位置，法线向量，纹理坐标的属性

using namespace std;

//函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HRESULT Direct3D_Init(HWND hwnd);
D3DMATERIAL9 InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p);
HRESULT Objects_Init();
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
void Direct3D_Render(HWND hwnd);							//渲染函数
void Message_Mouse_KeyBoard();								//管理键盘输入信息
void Direct3D_CleanUp();									//释放
void ReSetData();											//重置数据
void CreateMaze(int x, int y, int end_x, int end_y);		//创建迷宫数组
void CreateSky(int numv, int numi);							//创建天空盒
void Matrix_Set();											//投影矩阵
void CreateWall(float x,float y, float z,int block_num);    //生成墙壁数组
void Texture_init();										//整理贴图
void Update();												//处理根据时间变化的信息

const D3DXCOLOR      WHITE(D3DCOLOR_XRGB(255, 255, 255));
const D3DXCOLOR      BLACK(D3DCOLOR_XRGB(0, 0, 0));
const D3DXCOLOR        RED(D3DCOLOR_XRGB(255, 0, 0));
const D3DXCOLOR      GREEN(D3DCOLOR_XRGB(0, 255, 0));
const D3DXCOLOR       BLUE(D3DCOLOR_XRGB(0, 0, 255));
const D3DXCOLOR     YELLOW(D3DCOLOR_XRGB(255, 255, 0));
const D3DXCOLOR       CYAN(D3DCOLOR_XRGB(0, 255, 255));
const D3DXCOLOR    MAGENTA(D3DCOLOR_XRGB(255, 0, 255));
const D3DMATERIAL9 WHITE_MTRL = InitMtrl(WHITE, WHITE,BLACK, BLACK, 2.0f);
const D3DMATERIAL9 RED_MTRL = InitMtrl(RED, RED, RED, BLACK, 2.0f);
const D3DMATERIAL9 GREEN_MTRL = InitMtrl(GREEN, GREEN, GREEN, BLACK, 2.0f);
const D3DMATERIAL9 BLUE_MTRL = InitMtrl(BLUE, BLUE, BLUE, BLACK, 2.0f);
const D3DMATERIAL9 YELLOW_MTRL = InitMtrl(YELLOW, YELLOW, YELLOW, BLACK, 2.0f);


//结构体
struct Vertex
{
	Vertex() {}
	Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v)
	{
		_x = x;  _y = y;  _z = z;
		_nx = nx; _ny = ny; _nz = nz;
		_u = u;  _v = v;
	}
	float _x, _y, _z;
	float _nx, _ny, _nz;
	float _u, _v; // texture coordinates
};