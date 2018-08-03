#include "MyDirectx9.h"

extern int CompTime;
extern int ReSet;
extern int Exit;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//开始设计一个完整的窗口类  
	WNDCLASSEX wndClass = { 0 };                //用WINDCLASSEX定义了一个窗口类，即用wndClass实例化了WINDCLASSEX，用于之后窗口的各项初始化      
	wndClass.cbSize = sizeof(WNDCLASSEX);    //设置结构体的字节数大小  
	wndClass.style = CS_HREDRAW | CS_VREDRAW;   //设置窗口的样式  
	wndClass.lpfnWndProc = WndProc;             //设置指向窗口过程函数的指针  
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;             //指定包含窗口过程的程序的实例句柄。  
	wndClass.hIcon = (HICON)::LoadImage(NULL, _T("icon.ico"), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE); //从全局的::LoadImage函数从本地加载自定义ico图标  
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);    //指定窗口类的光标句柄。
	wndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);  //为hbrBackground成员指定一个灰色画刷句柄  
	wndClass.lpszMenuName = NULL;                       //用一个以空终止的字符串，指定菜单资源的名字。  
	wndClass.lpszClassName = _T("ForTheDreamOfGameDevelop");        //用一个以空终止的字符串，指定窗口类的名字。  
	ShowCursor(false);											//隐藏全局鼠标
	if (!RegisterClassEx(&wndClass))             //设计完窗口后，需要对窗口类进行注册，这样才能创建该类型的窗口  
		return -1;

	HWND hwnd = CreateWindow(_T("ForTheDreamOfGameDevelop"), WINDOW_TITLE,          //喜闻乐见的创建窗口函数CreateWindow  
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH,
		SCREEN_HEIGHT, NULL, NULL, hInstance, NULL);

	//Direct3D资源的初始化，调用失败用messagebox予以显示  
	if (!(S_OK == Direct3D_Init(hwnd)))
	{
		MessageBox(hwnd, _T("Direct3D初始化失败~！"), _T("Warning"), 0); //使用MessageBox函数，创建一个消息窗口   
	}

	MoveWindow(hwnd, (GetSystemMetrics(SM_CXSCREEN)-SCREEN_WIDTH)/2, (GetSystemMetrics(SM_CYSCREEN)-SCREEN_HEIGHT)/2, SCREEN_WIDTH, SCREEN_HEIGHT, true);   //调整窗口显示时的位置，窗口左上角位于屏幕坐标（200，50）处  
	ShowWindow(hwnd, nShowCmd);    //调用Win32函数ShowWindow来显示窗口 
	//PlaySound(_T("KABANERIOFTHEIRONFORTRESS.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	while (ReSet)
	{
		Objects_Init();
		UpdateWindow(hwnd);  //对窗口进行更新，就像我们买了新房子要装修一样  
		CompTime = timeGetTime();//加载完模型，开始统计游戏时间  
		MSG msg = { 0 };  //初始化msg
		Exit = 0;
		while (!Exit)         //使用while循环  
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))   //查看应用程序消息队列，有消息时将队列中的消息派发出去。  
			{
				TranslateMessage(&msg);       //将虚拟键消息转换为字符消息  
				DispatchMessage(&msg);        //该函数分发一个消息给窗口程序。  
			}
			else
			{
				Direct3D_Render(hwnd);          //调用渲染函数，进行画面的渲染
			}
		}
	}
	Direct3D_CleanUp();     //调用Direct3D_CleanUp函数，清理COM接口对象  
	UnregisterClass(_T("ForTheDreamOfGameDevelop"), wndClass.hInstance);
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)   //窗口过程函数WndProc  
{
	switch (message)               //switch语句开始  
	{
	case WM_PAINT:                   // 客户区重绘消息  
		Direct3D_Render(hwnd);          //调用Direct3D_Render函数，进行画面的绘制  
		ValidateRect(hwnd, NULL);   // 更新客户区的显示  
		break;                                  //跳出该switch语句  

	case WM_KEYDOWN:                // 键盘按下消息  
	{
		if (wParam == VK_ESCAPE)    // ESC键  
		{
			Exit = 1;
			ReSet = 0;
		}

		break;
	}
	default:                        //若上述case条件都不符合，则执行该default语句  
		return DefWindowProc(hwnd, message, wParam, lParam);      //调用缺省的窗口过程来为应用程序没有处理的窗口消息提供缺省的处理。  
	}

	return 0;                   //正常退出  
}