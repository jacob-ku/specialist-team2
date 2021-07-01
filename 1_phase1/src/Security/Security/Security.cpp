
// Security.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "Security.h"
#include "SecurityDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSecurityApp

BEGIN_MESSAGE_MAP(CSecurityApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSecurityApp 생성

CSecurityApp::CSecurityApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CSecurityApp 개체입니다.

CSecurityApp theApp;


// CSecurityApp 초기화

BOOL CSecurityApp::InitInstance()
{
	CWinApp::InitInstance();

	CSecurityDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();
	
	return FALSE;
}

