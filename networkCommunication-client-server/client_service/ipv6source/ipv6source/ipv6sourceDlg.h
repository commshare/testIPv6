
// ipv6sourceDlg.h : 头文件
//

#pragma once
#include "CvvImage.h"
#define WM_SHOW_IMAGE WM_USER+1
// Cipv6sourceDlg 对话框
class Cipv6sourceDlg : public CDialogEx
{
// 构造
public:
	Cipv6sourceDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_IPV6SOURCE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	void DrawPicToHDC(IplImage *img,unsigned int id);
	LRESULT DrawPic(WPARAM wParam,LPARAM lParam);
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
