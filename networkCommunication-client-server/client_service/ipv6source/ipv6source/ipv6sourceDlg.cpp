
// ipv6sourceDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ipv6source.h"
#include "ipv6sourceDlg.h"
#include "afxdialogex.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "highgui.h"
#include "cv.h"
using namespace std;
// 所需库连接
#pragma comment (lib,"Ws2_32.lib")
void showpic(void*);

HWND m_FatherHwnd;//父窗口的句柄
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Cipv6sourceDlg 对话框




Cipv6sourceDlg::Cipv6sourceDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cipv6sourceDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cipv6sourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Cipv6sourceDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_MESSAGE(WM_SHOW_IMAGE,&Cipv6sourceDlg::DrawPic)
END_MESSAGE_MAP()


// Cipv6sourceDlg 消息处理程序

BOOL Cipv6sourceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_FatherHwnd=this->m_hWnd;
	_beginthread(showpic,0,(void*)0);
	//SetTimer(1,1,NULL);
	//SetTimer(2,1,NULL);
		
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void showpic(void*)
{
	WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET client_sock;  //客户端本地套接字
    struct addrinfo server_addr, *addrinfo=NULL; //服务器端地址 
    memset(&server_addr,0,sizeof(server_addr));//在getaddrinfo()之前必须清0
    server_addr.ai_family = AF_INET6;  //指定是ipv6协议
    server_addr.ai_socktype = SOCK_STREAM; //流式
    server_addr.ai_protocol = IPPROTO_TCP;  //TCP协议,也可以是0，让系统自己选
    int result = getaddrinfo("2001::1","2000",&server_addr,&addrinfo);  //服务器端ipv6地址, 服务器监听的端口
	IplImage* img=cvCreateImage(cvSize(640,480),8,3);
	//IplImage* img;
	while(1)
    {
	   SOCKET service_sock = socket(addrinfo->ai_family,addrinfo->ai_socktype,addrinfo->ai_protocol);
	   int iframe=connect(service_sock,addrinfo->ai_addr,addrinfo->ai_addrlen);
	   if(iframe<0)continue;
	   char recvBuf[100];
	   memset(recvBuf,0,sizeof(recvBuf));
	   FILE* tof=fopen("output.jpg","wb+");
	   while (recv(service_sock,recvBuf,40,0)>0)
	   {
		   fwrite(recvBuf,sizeof(char),40,tof);
	   }
	   fclose(tof);
	   closesocket(service_sock);
	   //Sleep(100);
	   IplImage*  showimg=cvLoadImage("output.jpg");   
	   //cvZero(img);
	   //img=cvCopy(showimg);
	   cvCopy(showimg,img,NULL);
	   ::PostMessage(m_FatherHwnd,WM_SHOW_IMAGE,(WPARAM)img,NULL);
	   cvReleaseImage(&showimg);
	   
    }
	cvReleaseImage(&img);
}
void Cipv6sourceDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Cipv6sourceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void Cipv6sourceDlg::DrawPicToHDC(IplImage *img,unsigned int id)
{
	CDC* pDC = GetDlgItem(id)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(id)->GetClientRect(&rect);
	CvvImage cimg;
	cimg.CopyOf(img,img->nChannels); 
	cimg.DrawToHDC(hDC,&rect);
	ReleaseDC(pDC);
}
LRESULT Cipv6sourceDlg::DrawPic(WPARAM wParam,LPARAM lParam)
{
	IplImage *img = (IplImage*)wParam;
	DrawPicToHDC(img,IDC_VIDEOCOLOR);

	return 0;
}

void Cipv6sourceDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnTimer(nIDEvent);
}
