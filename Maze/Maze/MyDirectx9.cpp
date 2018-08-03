#include "MyDirectx9.h"


LPDIRECT3DDEVICE9                     d3ddev = NULL; //Direct3D设备对象  
ID3DXFont*                           g_pFont = NULL;//字体COM接口  
LPDIRECT3DVERTEXBUFFER9     g_pVertexBuffer = NULL;//顶点缓存对象  
LPDIRECT3DINDEXBUFFER9      g_pIndexBuffer = NULL;// 索引缓存对象 
LPD3DXMESH                               g_pMesh;//网格对象
D3DMATERIAL9*		   	   g_pMaterials = NULL; // 网格的材质信息
LPDIRECT3DTEXTURE9*	   	   g_pTextures = NULL; // 网格的纹理信息
DWORD				         g_dwNumMtrls = 0;// 材质的数目

//公共变量 
int Exit = 0,ReSet = 1;
char Fltime[128];//包含时间的字符数组
int LastTime = timeGetTime();		//记录时间经过
int FunctionKeyTime = timeGetTime();	//控制功能键的间隔
int Cloudtime = timeGetTime();		//天空盒旋转时间
POINT Mouse_Ptr;					//鼠标位置指针
int Mid_W = SCREEN_WIDTH/2;
int Mid_H = SCREEN_HEIGHT/2;
int maze[21][21]{};					//迷宫数组
int CompTime;						//计算通过迷宫的时间
int num_block = 0;					//计算墙壁的砖块数
int Main_View = 1;					//判断是否为主视角
int Jumping = 0;					//判断是否在跳跃
float Sky_Angle = 0;

//主摄像机参数
D3DXVECTOR3 main_target(2.0f,1.6f,3.0f);			//目标位置
D3DXVECTOR3 main_pos(2.0f, 1.6f, 2.0f);			    //摄像机位置
D3DXVECTOR3 _look =  main_target - main_pos;		//看的方向
D3DXVECTOR3 _right(1.0f, 0.0f, 0.0f);				//右向量
D3DXVECTOR3 main_up(0.0f, 1.0f, 0.0f);				//上向量
D3DXVECTOR3 _forward(0.0f, 0.0, 1.0f);				//前向量
//副摄像机参数
D3DXVECTOR3 sate_target(19.0f,0.0f,19.0f);			//俯瞰点
D3DXVECTOR3 sate_pos(19.0f,60.0f,19.0f);			//卫星位置
D3DXVECTOR3 sate_up(0.0f,0.0f,-1.0f);
//摄像机参数
D3DXVECTOR3 *_target = &main_target;
D3DXVECTOR3 *_pos = &main_pos;
D3DXVECTOR3 *_up = &main_up;

//模型参数
float model_angle = D3DX_PI;
D3DXVECTOR3 model_pos(2.0f, 0.6f, 2.0f);

//物体对象
LPDIRECT3DTEXTURE9 block = nullptr, ground = nullptr, sky[6] = {};//墙壁，地面,天空贴图
Vertex *v=nullptr;//顶点
WORD* i=nullptr;//顶点索引
D3DXMATRIX World;//世界矩阵


HRESULT Direct3D_Init(HWND hwnd)
{
	SetCursorPos(Mid_W, Mid_H); // 移动鼠标到屏幕中央
	LPDIRECT3D9  pD3D = NULL; //Direct3D接口对象的创建  
	if (NULL == (pD3D = Direct3DCreate9(D3D_SDK_VERSION))) //初始化Direct3D接口对象，并进行DirectX版本协商  
		return E_FAIL;

	D3DCAPS9 caps; int vp = 0;
	if (FAILED(pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps)))
	{
		return E_FAIL;
	}
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;   //支持硬件顶点运算，我们就采用硬件顶点运算，妥妥的  
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING; //不支持硬件顶点运算，无奈只好采用软件顶点运算  				
	D3DPRESENT_PARAMETERS d3dpp;

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = SCREEN_WIDTH;
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.Windowed = true;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = 0;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,hwnd, vp, &d3dpp, &d3ddev)))
		return E_FAIL;


	d3ddev->GetTransform(D3DTS_WORLD, &World);
	d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	SAFE_RELEASE(pD3D) //LPDIRECT3D9接口对象的使命完成，我们将其释放掉  */
	return S_OK;
}

void Matrix_Set()
{
	D3DXMATRIX matView; //定义一个矩阵
	D3DXMatrixLookAtLH(&matView, _pos, _target, _up); //计算出取景变换矩阵
	d3ddev->SetTransform(D3DTS_VIEW, &matView); //应用取景变换矩阵

	D3DXMATRIX matProj; //定义一个矩阵  
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4.0f, 16.0f / 9.0f, 0.4f, 30000.0f); //计算投影变换矩阵  
	d3ddev->SetTransform(D3DTS_PROJECTION, &matProj);  //设置投影变换矩阵  

	D3DVIEWPORT9 vp; //实例化一个D3DVIEWPORT9结构体，然后做填空题给各个参数赋值就可以了  
	vp.X = 0;      //表示视口相对于窗口的X坐标  
	vp.Y = 0;      //视口相对对窗口的Y坐标  
	vp.Width = SCREEN_WIDTH;   //视口的宽度  
	vp.Height = SCREEN_HEIGHT; //视口的高度  
	vp.MinZ = 0.0f; //视口在深度缓存中的最小深度值  
	vp.MaxZ = 1.0f;   //视口在深度缓存中的最大深度值  
	d3ddev->SetViewport(&vp); //视口的设置  
}


void Message_Mouse_KeyBoard()         //键盘、鼠标输入信息
{
	if (timeGetTime() - LastTime < 30)
		return;
	//先鼠标
	_look = main_target - main_pos;
	GetCursorPos(&Mouse_Ptr);		//获取鼠标位置
	D3DXVec3Cross(&_right, &main_up, &_look);
	D3DXVec3Normalize(&_right, &_right);
	float angle;   //角度
	D3DXMATRIX T;  //临时矩阵

	//左右移动鼠标
	angle = atan((float)(Mouse_Ptr.x - Mid_W) / Mid_W);
	D3DXMatrixRotationAxis(&T, &main_up, angle);
	model_angle += angle;
	if (model_angle > 2 * D3DX_PI)
	{
		model_angle -= 2 * D3DX_PI;
	}
	D3DXVec3TransformCoord(&_right, &_right, &T);
	D3DXVec3TransformCoord(&_look, &_look, &T);
	D3DXVec3Normalize(&_look, &_look);
	main_target = main_pos + _look * D3DXVec3Length(&(main_target - main_pos));

	//上下移动鼠标
	D3DXVECTOR3 temp;
	angle = atan((float)(Mouse_Ptr.y - Mid_H)/Mid_H);
	D3DXVec3Normalize(&_right, &_right);
	D3DXMatrixRotationAxis(&T, &_right, angle);
	D3DXVec3TransformCoord(&temp, &_look, &T);
	D3DXVec3Normalize(&temp, &temp);
	if (temp.y < 0.9f&&temp.y>-0.9f)		//防止摄像机反转
	{
		_look = temp;
	}
	main_target = main_pos + _look * D3DXVec3Length(&(main_target - main_pos));

	_forward = _look; 
	_forward.y = 0.0f;
	D3DXVec3Normalize(&_forward, &_forward);
	D3DXVec3Normalize(&_right, &_right);
	if (KEY_DOWN('W'))
	{
		main_pos += _forward;
		if (maze[(int)(main_pos.z+1.0f)/2][(int)(main_pos.x+1.0f)/2] == 1)
		{
			main_pos -= _forward;
		}
		else
		{
			main_target += _forward*0.2f;
			main_pos -= _forward*0.8f;
			model_pos += _forward*0.2f;
			D3DXVECTOR3 check_beside = main_pos + _right*0.5f;
			while (maze[(int)(check_beside.z + 1.0f) / 2][(int)(check_beside.x + 1.0f) / 2] == 1)
			{
				main_pos -= 0.05f*_right;
				model_pos -= 0.05f*_right;
				check_beside = main_pos + _right*0.5f;
			}
			check_beside = main_pos - _right*0.5f;
			while (maze[(int)(check_beside.z + 1.0f) / 2][(int)(check_beside.x + 1.0f) / 2] == 1)
			{
				main_pos += 0.05f*_right;
				model_pos += 0.05f*_right;
				check_beside = main_pos + _right*0.5f;
			}
		}
	}

	if (KEY_DOWN('S'))
	{
		main_pos -= _forward;
		if (maze[(int)(main_pos.z+1.0f)/2][(int)(main_pos.x+1.0f)/2] == 1)
		{
			main_pos += _forward;
		}
		else
		{
			main_target -= _forward*0.2f;
			main_pos += _forward*0.8f;
			model_pos -= _forward*0.2f;
			D3DXVECTOR3 check_beside = main_pos + _right*0.5f;
			while (maze[(int)(check_beside.z + 1.0f) / 2][(int)(check_beside.x + 1.0f) / 2] == 1)
			{
				main_pos -= 0.05f*_right;
				model_pos -= 0.05f*_right;
				check_beside = main_pos + _right*0.5f;
			}
			check_beside = main_pos - _right*0.5f;
			while (maze[(int)(check_beside.z + 1.0f) / 2][(int)(check_beside.x + 1.0f) / 2] == 1)
			{
				main_pos += 0.05f*_right;
				model_pos += 0.05f*_right;
				check_beside = main_pos + _right*0.5f;
			}

		}
	}

	if (KEY_DOWN('A'))
	{
		main_pos -= _right;
		if (maze[(int)(main_pos.z+1.0f)/2][(int)(main_pos.x+1.0f)/2] == 1)
		{
			main_pos += _right;
		}
		else
		{
			main_target -= _right*0.2f;
			main_pos += _right*0.8f;
			model_pos -= _right*0.2f;
			D3DXVECTOR3 check_beside = main_pos + _forward*0.4f;
			while (maze[(int)(check_beside.z + 1.0f) / 2][(int)(check_beside.x + 1.0f) / 2] == 1)
			{
				main_pos -= 0.05f*_forward;
				model_pos -= 0.05f*_forward;
				check_beside = main_pos + _forward*0.4f;
			}
			check_beside = main_pos - _forward*0.4f;
			while (maze[(int)(check_beside.z + 1.0f) / 2][(int)(check_beside.x + 1.0f) / 2] == 1)
			{
				main_pos += 0.05f*_forward;
				model_pos += 0.05f*_forward;
				check_beside = main_pos + _forward*0.4f;
			}
		}
	}

	if (KEY_DOWN('D'))
	{
		main_pos += _right;
		if (maze[(int)(main_pos.z+1.0f)/2][(int)(main_pos.x+1.0f)/2] == 1)
		{
			main_pos -= _right;
		}
		else
		{
			main_target += _right*0.2f;
			main_pos -= _right*0.8f;
			model_pos += _right*0.2f;
			D3DXVECTOR3 check_beside = main_pos + _forward*0.4f;
			while (maze[(int)(check_beside.z + 1.0f) / 2][(int)(check_beside.x + 1.0f) / 2] == 1)
			{
				main_pos -= 0.05f*_forward;
				model_pos -= 0.05f*_forward;
				check_beside = main_pos + _forward*0.4f;
			}
			check_beside = main_pos - _forward*0.4f;
			while (maze[(int)(check_beside.z + 1.0f) / 2][(int)(check_beside.x + 1.0f) / 2] == 1)
			{
				main_pos += 0.05f*_forward;
				model_pos += 0.05f*_forward;
				check_beside = main_pos + _forward*0.4f;
			}
		}
	}

	if (KEY_DOWN(' '))
	{
		if (Jumping == 0)
		{
			Jumping = 1;
		}
	}

	main_target = main_pos + _look * D3DXVec3Length(&(main_target - main_pos));

	SetCursorPos(Mid_W, Mid_H);

	if (KEY_DOWN('M') && timeGetTime() - FunctionKeyTime>200)		//打开地图
	{
		if (Main_View == 1)
		{
			_target = &main_target;
			_pos = &main_pos;
			_up = &main_up;
			Main_View = 0;
		}
		else
		{
			_target = &sate_target;
			_pos = &sate_pos;
			_up = &sate_up;
			Main_View = 1;
		}
		FunctionKeyTime = timeGetTime();
	}

	if ((main_pos.z + 1.0f) / 2 >= 21)
	{
		char tostr[100];
		sprintf(tostr, "耗时：%f秒。\n是否再来一局？",(timeGetTime() - CompTime) *0.001f);
		if (MessageBox(NULL, tostr, "通关成功！", MB_YESNO) == IDYES)
		{
			Exit = 1;
		}
		else
		{
			Exit = 1;
			ReSet = 0;
		}
	}
	LastTime = timeGetTime();
}

void Direct3D_Render(HWND hwnd)							//渲染函数
{
	//定义一个矩形，用于获取主窗口矩形  
	RECT formatRect;
	GetClientRect(hwnd, &formatRect);

	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
	
	// new position/orientation.

	Update();
	Message_Mouse_KeyBoard();
	Matrix_Set();


	d3ddev->BeginScene();                     // 开始绘制  

	D3DXMATRIX model_scale,model_world,model_move,model_rota;
	D3DXMatrixRotationY(&model_rota, model_angle);
	D3DXMatrixTranslation(&model_move, model_pos.x,model_pos.y,model_pos.z);
	D3DXMatrixScaling(&model_scale, 0.015f, 0.015f, 0.015f);
	model_world = model_scale*model_rota*model_move*World;
	d3ddev->SetTransform(D3DTS_WORLD, &model_world);//设置模型的世界矩阵，为绘制做准备

	for (DWORD i = 0; i < g_dwNumMtrls; i++)
	{
		d3ddev->SetMaterial(&g_pMaterials[i]);
		d3ddev->SetTexture(0, g_pTextures[i]);
		g_pMesh->DrawSubset(i);
	}


	d3ddev->SetTransform(D3DTS_WORLD, &World);

	int charCount = sprintf_s(Fltime, 32, _T("通关时间:%fs"), (float)(timeGetTime() - CompTime)*0.001f);
	g_pFont->DrawTextA(NULL, Fltime, charCount, &formatRect, DT_TOP | DT_RIGHT, D3DCOLOR_RGBA(0, 239, 136, 255));


	d3ddev->SetStreamSource(0, g_pVertexBuffer, 0, sizeof(Vertex));
	d3ddev->SetIndices(g_pIndexBuffer);
	d3ddev->SetFVF(FVF_VERTEX);

	//画迷宫
	d3ddev->SetMaterial(&WHITE_MTRL);
	d3ddev->SetTexture(0, block);
	d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24 * num_block, 0, 12 * num_block);
	
	//画地板
	d3ddev->SetMaterial(&WHITE_MTRL);
	d3ddev->SetTexture(0, ground);
	d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, num_block * 24, 4, num_block * 36, 2);	
	//画天空盒
	int time = timeGetTime() - Cloudtime;
	D3DXMATRIX Sky_World = World;		//天空盒矩阵
	if ( time > 20)
	{
		Sky_Angle += time*D3DX_PI*0.00001f;
		if (Sky_Angle > D3DX_PI * 2)	//防溢出
		{
			Sky_Angle -= D3DX_PI * 2;
		}
		Cloudtime = timeGetTime();
	}
	D3DXMatrixRotationY(&Sky_World, Sky_Angle);
	d3ddev->SetTransform(D3DTS_WORLD, &Sky_World);
	for (int a = 0; a < 6; a++)
	{
		d3ddev->SetMaterial(&WHITE_MTRL);
		d3ddev->SetTexture(0, sky[a]);
		d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, num_block * 24 + 4 + a * 6, 4, num_block * 36 + 6 + a * 6, 2);
	}


	d3ddev->EndScene();                       // 结束绘制
	d3ddev->Present(NULL, NULL, NULL, NULL);  // 翻转与显示  
}

HRESULT Objects_Init()
{
	ReSetData();
	D3DXCreateFont(d3ddev, 30, 0, 0, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, _T("宋体"), &g_pFont);
	// 设置渲染状态  
	d3ddev->SetRenderState(D3DRS_LIGHTING, true);   //关闭光照  
	d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);   //开启背面消隐  
	d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);  //设置线框填充模式  
															//光照
	D3DLIGHT9 light;
	::ZeroMemory(&light, sizeof(light));

	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Ambient = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	light.Specular = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
	light.Direction = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	d3ddev->SetLight(0, &light);
	d3ddev->LightEnable(0, true);
	
	d3ddev->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	d3ddev->SetRenderState(D3DRS_SPECULARENABLE, true);

	d3ddev->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	d3ddev->SetRenderState(D3DRS_SPECULARENABLE, true);

	//贴图
	Texture_init();
	//迷宫建造
	for (auto &a : maze)//重置迷宫
	{
		for (int &b : a)
		{
			b = 0;
		}
	}
	for (int a = 0; a < 21; a++)//外围墙壁
	{
		maze[0][a] = 1;
		maze[a][0] = 1;
		maze[20][a] = 1;
		maze[a][20] = 1;
	}
	maze[20][19] = 0;//迷宫出口
	srand(time(0));
	CreateMaze(1, 1, 19, 19);

	for (auto &a : maze)//计算要做多少个砖头
	{
		for (int &b : a)
		{
			if (b == 1)
			{
				num_block++;
			}
		}
	}
	num_block *= 2;//乘以层数 

	d3ddev->CreateVertexBuffer((num_block * 24 + 4 + 24) * sizeof(Vertex), D3DUSAGE_WRITEONLY, FVF_VERTEX, D3DPOOL_MANAGED, &g_pVertexBuffer, 0);
	d3ddev->CreateIndexBuffer((num_block * 36 + 6 + 36) * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g_pIndexBuffer, 0);

	g_pVertexBuffer->Lock(0, 0, (void**)&v, 0);
	g_pIndexBuffer->Lock(0, 0, (void**)&i, 0);
	num_block = 0;//计算做到第几个墙壁
	for (int floor = 0; floor < 2; floor++)
	{
		for (int a = 0; a < 21; a++)
		{
			for (int b = 0; b<21; b++)
			{
				if (maze[a][b] == 1)
				{
					CreateWall((float)2 * b, (float)2 * floor, (float)2 * a, num_block);
					num_block++;
				}
			}
		}
	}
	//加载地板
	int numv = num_block * 24, numi = num_block * 36;
	v[numv + 0] = Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[numv + 1] = Vertex(-1.0f, -1.0f, 41.0f, 0.0f, 1.0f, 0.0f, 0.0f, 21.0f);
	v[numv + 2] = Vertex(41.0f, -1.0f, 41.0f, 0.0f, 1.0f, 0.0f, 21.0f, 21.0f);
	v[numv + 3] = Vertex(41.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 21.0f, 0.0f);
	i[numi + 0] = numv + 0; i[numi + 1] = numv + 1; i[numi + 2] = numv + 2;
	i[numi + 3] = numv + 0; i[numi + 4] = numv + 2; i[numi + 5] = numv + 3;
	//加载天空盒
	CreateSky(numv + 4, numi + 6);

	g_pVertexBuffer->Unlock();
	g_pIndexBuffer->Unlock();

	// 从X文件中加载网格数据
	LPD3DXBUFFER pAdjBuffer = NULL;
	LPD3DXBUFFER pMtrlBuffer = NULL;

	D3DXLoadMeshFromX(_T("miki.X"), D3DXMESH_MANAGED, d3ddev,
		&pAdjBuffer, &pMtrlBuffer, NULL, &g_dwNumMtrls, &g_pMesh);

	// 读取材质和纹理数据
	D3DXMATERIAL *pMtrls = (D3DXMATERIAL*)pMtrlBuffer->GetBufferPointer(); //创建一个D3DXMATERIAL结构体用于读取材质和纹理信息
	g_pMaterials = new D3DMATERIAL9[g_dwNumMtrls];
	g_pTextures = new LPDIRECT3DTEXTURE9[g_dwNumMtrls];

	for (DWORD i = 0; i<g_dwNumMtrls; i++)
	{
		//获取材质，并设置一下环境光的颜色值
		g_pMaterials[i] = pMtrls[i].MatD3D;
		g_pMaterials[i].Ambient = g_pMaterials[i].Diffuse;

		//创建一下纹理对象
		g_pTextures[i] = nullptr;
		D3DXCreateTextureFromFileA(d3ddev, pMtrls[i].pTextureFilename, &g_pTextures[i]);
	}

	SAFE_RELEASE(pAdjBuffer)
	SAFE_RELEASE(pMtrlBuffer)
	FunctionKeyTime = CompTime = timeGetTime();//加载完模型，开始统计游戏时间
	return S_OK;
}

D3DMATERIAL9 InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p)
{
	D3DMATERIAL9 mtrl;
	mtrl.Ambient = a;		//指定表面反射的环境光。
	mtrl.Diffuse = d;		//指定表面反射的漫反射光。
	mtrl.Specular = s;		//指定表面反射的镜面光。
	mtrl.Emissive = e;		//表面本身自发光。
	mtrl.Power = p;			//镜面高光，它的值是高光的锐利值，该值越大表示高光强度与周围亮度相差越大。
	return mtrl;
}


void CreateWall(float x, float y,float z,int block_num)
{
	int numv = block_num * 24, numi = block_num * 36;

	// 前顶点
	v[numv + 0] = Vertex(-1.0f+x, -1.0f+y, -1.0f+z, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[numv + 1] = Vertex(-1.0f+x, 1.0f+y, -1.0f+z, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[numv + 2] = Vertex(1.0f+x, 1.0f+y, -1.0f+z, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
	v[numv + 3] = Vertex(1.0f+x, -1.0f+y, -1.0f+z, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	//后顶点
	v[numv + 4] = Vertex(-1.0f+x, -1.0f+y, 1.0f+z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[numv + 5] = Vertex(1.0f+x, -1.0f+y, 1.0f+z, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[numv + 6] = Vertex(1.0f+x, 1.0f+y, 1.0f+z, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	v[numv + 7] = Vertex(-1.0f+x, 1.0f+y, 1.0f+z, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);

	//上顶点
	v[numv + 8] = Vertex(-1.0f+x, 1.0f+y, -1.0f+z, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[numv + 9] = Vertex(-1.0f+x, 1.0f+y, 1.0f+z, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[numv + 10] = Vertex(1.0f+x, 1.0f+y, 1.0f+z, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	v[numv + 11] = Vertex(1.0f+x, 1.0f+y, -1.0f+z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);

	//下顶点
	v[numv + 12] = Vertex(-1.0f+x, -1.0f+y, -1.0f+z, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	v[numv + 13] = Vertex(1.0f+x, -1.0f+y, -1.0f+z, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[numv + 14] = Vertex(1.0f+x, -1.0f+y, 1.0f+z, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	v[numv + 15] = Vertex(-1.0f+x, -1.0f+y, 1.0f+z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
	//左顶点
	v[numv + 16] = Vertex(-1.0f+x, -1.0f+y, 1.0f+z, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[numv + 17] = Vertex(-1.0f + x, 1.0f +y, 1.0f + z, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[numv + 18] = Vertex(-1.0f+x, 1.0f+y, -1.0f+z, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[numv + 19] = Vertex(-1.0f+x, -1.0f+y, -1.0f+z, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	//右顶点
	v[numv + 20] = Vertex(1.0f+x, -1.0f+y, -1.0f+z, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[numv + 21] = Vertex(1.0f+x, 1.0f+y, -1.0f+z, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[numv + 22] = Vertex(1.0f+x, 1.0f+y, 1.0f+z, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[numv + 23] = Vertex(1.0f+x, -1.0f+y, 1.0f+z, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	//前顶点索引
	i[numi+0] = numv + 0; i[numi + 1] = numv + 1; i[numi + 2] = numv + 2;
	i[numi + 3] = numv + 0; i[numi + 4] = numv + 2; i[numi + 5] = numv + 3;

	//后顶点索引
	i[numi + 6] = numv + 4; i[numi + 7] = numv + 5; i[numi + 8] = numv + 6;
	i[numi + 9] = numv + 4; i[numi + 10] = numv + 6; i[numi + 11] = numv + 7;

	//上顶点索引
	i[numi + 12] = numv + 8; i[numi + 13] = numv + 9; i[numi + 14] = numv + 10;
	i[numi + 15] = numv + 8; i[numi + 16] = numv + 10; i[numi + 17] = numv + 11;

	//下顶点索引
	i[numi + 18] = numv + 12; i[numi + 19] = numv + 13; i[numi + 20] = numv + 14;
	i[numi + 21] = numv + 12; i[numi + 22] = numv + 14; i[numi + 23] = numv + 15;

	//左顶点索引
	i[numi + 24] = numv + 16; i[numi + 25] = numv + 17; i[numi + 26] = numv + 18;
	i[numi + 27] = numv + 16; i[numi + 28] = numv + 18; i[numi + 29] = numv + 19;

	//右顶点索引
	i[numi + 30] = numv + 20; i[numi + 31] = numv + 21; i[numi + 32] = numv + 22;
	i[numi + 33] = numv + 20; i[numi + 34] = numv + 22; i[numi + 35] = numv + 23;

}

void CreateSky(int numv, int numi)
{

	// 前顶点
	v[numv + 0] = Vertex(-10001.0f, -10001.0f, 10001.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
	v[numv + 1] = Vertex(-10001.0f, 10001.0f, 10001.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[numv + 2] = Vertex(10001.0f, 10001.0f, 10001.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[numv + 3] = Vertex(10001.0f, -10001.0f, 10001.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);

	//后顶点
	v[numv + 4] = Vertex(-10001.0f, -10001.0f, -10001.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[numv + 5] = Vertex(10001.0f, -10001.0f, -10001.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	v[numv + 6] = Vertex(10001.0f, 10001.0f, -10001.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[numv + 7] = Vertex(-10001.0f, 10001.0f, -10001.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);

	//下顶点
	v[numv + 8] = Vertex(-10001.0f, -10001.0f, -10001.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[numv + 9] = Vertex(-10001.0f, -10001.0f, 10001.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[numv + 10] = Vertex(10001.0f, -10001.0f, 10001.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	v[numv + 11] = Vertex(10001.0f, -10001.0f, -10001.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);

	//上顶点
	v[numv + 12] = Vertex(-10001.0f, 10001.0f, -10001.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
	v[numv + 13] = Vertex(10001.0f, 10001.0f, -10001.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	v[numv + 14] = Vertex(10001.0f, 10001.0f, 10001.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	v[numv + 15] = Vertex(-10001.0f, 10001.0f, 10001.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	//左顶点
	v[numv + 16] = Vertex(10001.0f, -10001.0f, 10001.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[numv + 17] = Vertex(10001.0f, 10001.0f, 10001.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[numv + 18] = Vertex(10001.0f, 10001.0f, -10001.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[numv + 19] = Vertex(10001.0f, -10001.0f, -10001.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	//右顶点
	v[numv + 20] = Vertex(-10001.0f, -10001.0f, -10001.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[numv + 21] = Vertex(-10001.0f, 10001.0f, -10001.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[numv + 22] = Vertex(-10001.0f, 10001.0f, 10001.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[numv + 23] = Vertex(-10001.0f, -10001.0f, 10001.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	//前顶点索引
	i[numi + 0] = numv + 0; i[numi + 1] = numv + 1; i[numi + 2] = numv + 2;
	i[numi + 3] = numv + 0; i[numi + 4] = numv + 2; i[numi + 5] = numv + 3;

	//后顶点索引
	i[numi + 6] = numv + 4; i[numi + 7] = numv + 5; i[numi + 8] = numv + 6;
	i[numi + 9] = numv + 4; i[numi + 10] = numv + 6; i[numi + 11] = numv + 7;

	//下顶点索引
	i[numi + 12] = numv + 8; i[numi + 13] = numv + 9; i[numi + 14] = numv + 10;
	i[numi + 15] = numv + 8; i[numi + 16] = numv + 10; i[numi + 17] = numv + 11;

	//上顶点索引
	i[numi + 18] = numv + 12; i[numi + 19] = numv + 13; i[numi + 20] = numv + 14;
	i[numi + 21] = numv + 12; i[numi + 22] = numv + 14; i[numi + 23] = numv + 15;

	//右顶点索引
	i[numi + 24] = numv + 16; i[numi + 25] = numv + 17; i[numi + 26] = numv + 18;
	i[numi + 27] = numv + 16; i[numi + 28] = numv + 18; i[numi + 29] = numv + 19;

	//左顶点索引
	i[numi + 30] = numv + 20; i[numi + 31] = numv + 21; i[numi + 32] = numv + 22;
	i[numi + 33] = numv + 20; i[numi + 34] = numv + 22; i[numi + 35] = numv + 23;

}

void CreateMaze(int x, int y, int end_x, int end_y)
{
	if (end_x <= x || end_y <= y)//长或宽等于1时不能分
	{
		return;
	}
	int mid_x, mid_y;
	mid_x = mid_y = 3;
	while (mid_x % 2 == 1)//随机生成新墙壁
	{
		mid_x = rand() % (end_x - x + 1) + x;
	}
	while (mid_y % 2 == 1)
	{
		mid_y = rand() % (end_y - y + 1) + y;
	}
	for (int a = x; a <= end_x; a++)//填充墙壁
	{
		maze[mid_y][a] = 1;
	}
	for (int a = y; a <= end_y; a++)
	{
		maze[a][mid_x] = 1;
	}
	int door, nodoor = rand() % 4;//选择一个墙不开门
	if (nodoor != 0)//开门
	{
		door = 2;
		while (door % 2 == 0)
		{
			door = rand() % (mid_x - x + 1) + x;
		}
		maze[mid_y][door] = 0;
	}
	if (nodoor != 1)
	{
		door = 2;
		while (door % 2 == 0)
		{
			door = rand() % (end_x - mid_x + 1) + mid_x;
		}
		maze[mid_y][door] = 0;
	}
	if (nodoor != 2)
	{
		door = 2;
		while (door % 2 == 0)
		{
			door = rand() % (mid_y - y + 1) + y;
		}
		maze[door][mid_x] = 0;
	}
	if (nodoor != 3)
	{
		door = 2;
		while (door % 2 == 0)
		{
			door = rand() % (end_y - mid_y + 1) + mid_y;
		}
		maze[door][mid_x] = 0;
	}
	CreateMaze(x, y, mid_x - 1, mid_y - 1);
	CreateMaze(mid_x + 1, y, end_x, mid_y - 1);
	CreateMaze(x, mid_y + 1, mid_x - 1, end_y);
	CreateMaze(mid_x + 1, mid_y + 1, end_x, end_y);
}

void ReSetData()
{
	num_block = 0;
	Main_View = 1;
	main_target = D3DXVECTOR3(2.0f, 1.6f, 3.0f);			//目标位置
	main_pos = D3DXVECTOR3(2.0f, 1.6f, 2.0f);			    //摄像机位置
	_look = main_target - main_pos;		//看的方向
	_right = D3DXVECTOR3(1.0f, 0.0f, 0.0f);				//右向量
	main_up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);				//上向量
	_forward = D3DXVECTOR3(0.0f, 0.0, 1.0f);				//前向量
														//副摄像机参数
	sate_target = D3DXVECTOR3(19.0f, 0.0f, 19.0f);			//俯瞰点
	sate_pos = D3DXVECTOR3(19.0f, 60.0f, 19.0f);			//卫星位置
	sate_up = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	//摄像机参数
	_target = &main_target;
	_pos = &main_pos;
	_up = &main_up;
	//模型参数
	model_angle = D3DX_PI;
	model_pos = D3DXVECTOR3(2.0f, 0.6f, 2.0f);
}

void Update()
{
	if (timeGetTime() - LastTime < 30)
		return;
	if (Jumping!=0)					//处理跳跃
	{
		float height_t = 0.06f*Jumping;
		main_target.y += height_t;	//目标位置
		main_pos.y += height_t;	    //摄像机位置
		model_pos.y += height_t;
		if (main_pos.y >= 3.6f)
		{
			Jumping = -1;
		}
		else if (main_pos.y <= 1.6)
		{
			main_pos.y = 1.6f;
			model_pos.y = 0.6f;
			Jumping = 0;
		}
	}
}

void Texture_init()
{
	D3DXCreateTextureFromFileA(d3ddev, "block.jpg", &block);
	D3DXCreateTextureFromFileA(d3ddev, "ground.jpg", &ground);
	D3DXCreateTextureFromFileA(d3ddev, "up.png", &sky[3]);
	D3DXCreateTextureFromFileA(d3ddev, "front.png", &sky[0]);
	D3DXCreateTextureFromFileA(d3ddev, "back.png", &sky[1]);
	D3DXCreateTextureFromFileA(d3ddev, "left.png", &sky[5]);
	D3DXCreateTextureFromFileA(d3ddev, "right.png", &sky[4]);
	D3DXCreateTextureFromFileA(d3ddev, "down.png", &sky[2]);
}

void Direct3D_CleanUp()			//释放资源
{
	SAFE_RELEASE(g_pFont)
	SAFE_RELEASE(g_pVertexBuffer)
	SAFE_RELEASE(g_pIndexBuffer)
	SAFE_RELEASE(d3ddev)
	SAFE_RELEASE(ground)
	SAFE_RELEASE(block)
	for (auto &a : sky)
		SAFE_RELEASE(a)
}