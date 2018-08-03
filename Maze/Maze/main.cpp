#include "MyDirectx9.h"

extern int CompTime;
extern int ReSet;
extern int Exit;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//��ʼ���һ�������Ĵ�����  
	WNDCLASSEX wndClass = { 0 };                //��WINDCLASSEX������һ�������࣬����wndClassʵ������WINDCLASSEX������֮�󴰿ڵĸ����ʼ��      
	wndClass.cbSize = sizeof(WNDCLASSEX);    //���ýṹ����ֽ�����С  
	wndClass.style = CS_HREDRAW | CS_VREDRAW;   //���ô��ڵ���ʽ  
	wndClass.lpfnWndProc = WndProc;             //����ָ�򴰿ڹ��̺�����ָ��  
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;             //ָ���������ڹ��̵ĳ����ʵ�������  
	wndClass.hIcon = (HICON)::LoadImage(NULL, _T("icon.ico"), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE); //��ȫ�ֵ�::LoadImage�����ӱ��ؼ����Զ���icoͼ��  
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);    //ָ��������Ĺ������
	wndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);  //ΪhbrBackground��Աָ��һ����ɫ��ˢ���  
	wndClass.lpszMenuName = NULL;                       //��һ���Կ���ֹ���ַ�����ָ���˵���Դ�����֡�  
	wndClass.lpszClassName = _T("ForTheDreamOfGameDevelop");        //��һ���Կ���ֹ���ַ�����ָ������������֡�  
	ShowCursor(false);											//����ȫ�����
	if (!RegisterClassEx(&wndClass))             //����괰�ں���Ҫ�Դ��������ע�ᣬ�������ܴ��������͵Ĵ���  
		return -1;

	HWND hwnd = CreateWindow(_T("ForTheDreamOfGameDevelop"), WINDOW_TITLE,          //ϲ���ּ��Ĵ������ں���CreateWindow  
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH,
		SCREEN_HEIGHT, NULL, NULL, hInstance, NULL);

	//Direct3D��Դ�ĳ�ʼ��������ʧ����messagebox������ʾ  
	if (!(S_OK == Direct3D_Init(hwnd)))
	{
		MessageBox(hwnd, _T("Direct3D��ʼ��ʧ��~��"), _T("Warning"), 0); //ʹ��MessageBox����������һ����Ϣ����   
	}

	MoveWindow(hwnd, (GetSystemMetrics(SM_CXSCREEN)-SCREEN_WIDTH)/2, (GetSystemMetrics(SM_CYSCREEN)-SCREEN_HEIGHT)/2, SCREEN_WIDTH, SCREEN_HEIGHT, true);   //����������ʾʱ��λ�ã��������Ͻ�λ����Ļ���꣨200��50����  
	ShowWindow(hwnd, nShowCmd);    //����Win32����ShowWindow����ʾ���� 
	//PlaySound(_T("KABANERIOFTHEIRONFORTRESS.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	while (ReSet)
	{
		Objects_Init();
		UpdateWindow(hwnd);  //�Դ��ڽ��и��£��������������·���Ҫװ��һ��  
		CompTime = timeGetTime();//������ģ�ͣ���ʼͳ����Ϸʱ��  
		MSG msg = { 0 };  //��ʼ��msg
		Exit = 0;
		while (!Exit)         //ʹ��whileѭ��  
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))   //�鿴Ӧ�ó�����Ϣ���У�����Ϣʱ�������е���Ϣ�ɷ���ȥ��  
			{
				TranslateMessage(&msg);       //���������Ϣת��Ϊ�ַ���Ϣ  
				DispatchMessage(&msg);        //�ú����ַ�һ����Ϣ�����ڳ���  
			}
			else
			{
				Direct3D_Render(hwnd);          //������Ⱦ���������л������Ⱦ
			}
		}
	}
	Direct3D_CleanUp();     //����Direct3D_CleanUp����������COM�ӿڶ���  
	UnregisterClass(_T("ForTheDreamOfGameDevelop"), wndClass.hInstance);
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)   //���ڹ��̺���WndProc  
{
	switch (message)               //switch��俪ʼ  
	{
	case WM_PAINT:                   // �ͻ����ػ���Ϣ  
		Direct3D_Render(hwnd);          //����Direct3D_Render���������л���Ļ���  
		ValidateRect(hwnd, NULL);   // ���¿ͻ�������ʾ  
		break;                                  //������switch���  

	case WM_KEYDOWN:                // ���̰�����Ϣ  
	{
		if (wParam == VK_ESCAPE)    // ESC��  
		{
			Exit = 1;
			ReSet = 0;
		}

		break;
	}
	default:                        //������case�����������ϣ���ִ�и�default���  
		return DefWindowProc(hwnd, message, wParam, lParam);      //����ȱʡ�Ĵ��ڹ�����ΪӦ�ó���û�д���Ĵ�����Ϣ�ṩȱʡ�Ĵ���  
	}

	return 0;                   //�����˳�  
}