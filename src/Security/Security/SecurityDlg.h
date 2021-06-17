
// SecurityDlg.h: 헤더 파일
//

#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <winsock.h>
#include "NetworkTCP.h"
#include "RecvImageTCP.h"
#include "TcpSendRecvJpeg.h"
#include "CommandInterface.h"
#include "CommandEncoder.h"

using namespace std;
using namespace cv;

void Log(int nType, unsigned int Size, const char* fmt, ...);

constexpr unsigned int MIN_STRING_LEN = (1);
constexpr unsigned int MAX_STRING_LEN = (16);

enum LogType
{
	Debug = 0,
	Normal,
	End,
	Err
};

// CSecurityDlg 대화 상자
class CSecurityDlg : public CDialogEx
{
// 생성입니다.
private: 
	Mat m_matImage;
	BITMAPINFO* m_pBitmapInfo;

	bool IsValidIpAddress();
	bool IsValidPortNum();
	bool IsValidString(CString &str);
	bool IsValidLoginParam();
	int ConnectServer(unsigned short modeInMsg);
	unsigned short GetModeValue();
	//unsigned int RandomUnsignedInt(unsigned int limit);
	int CreateBitmapInfo(int w, int h, int bpp);
	void DrawImage();
	void SetRandSeqNum();

public:
	CSecurityDlg(CWnd* pParent = nullptr);
	
	void EnableControlBox();
	void DisableControlBox();
	void WriteLog(CString msg);
	//void WriteLogHex(int msgLen, char* inMsg);
	void DisconnectServer(bool bClientClose);

	int HandleResponseData(S_MSG_CMN_HDR &stRcvMsgHdr);
	int HandleStreamData(unsigned int dataLen);

	// State
	bool m_isConnecting;
	TTcpConnectedPort* m_tcpConnectedPort;
	SOCKET m_clientSock;

	// TLS connection
	SSL* ssl;
	SSL_CTX* ctx;

	// Thread
	CWinThread* m_pThread;
	bool m_isWorkingThread;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SECURITY_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnConnect();
	afx_msg void OnBnClickedBtnCapture();
	afx_msg void OnBnClickedBtnConfirm();
	int m_isCheckSecureMode;
	bool m_isCaptured;

	CEdit m_LogView;

	// Connect Procedure
	CString m_ipStr;
	CString m_portStr;
	CString m_idStr;
	CString m_pwdStr;
	CString m_nameStr;

	unsigned int m_seqNum;
	afx_msg void OnBnClickedCheckSecureMode();

private:
	CommandEncoder m_cmdEncoder;
public:
	afx_msg void OnClose();
	afx_msg void OnEnChangeIdEdit();
};
