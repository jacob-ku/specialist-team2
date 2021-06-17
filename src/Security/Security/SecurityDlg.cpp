
// SecurityDlg.cpp
//

#include "pch.h"
#include "framework.h"
#include "Security.h"
#include "SecurityDlg.h"
#include "afxdialogex.h"
#include <atlimage.h>
#include <openssl/rand.h>

#include "NetworkTCP.h"
#include "NetworkTLS.h"

#include "RecvImageTCP.h"
#include "TcpSendRecvJpeg.h"
#include "CommandInterface.h"
#include "CommandEncoder.h"
#include "CommandDecoder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT ThreadForDisplay(LPVOID param)   // Non-secure Thread
{
	CSecurityDlg* pMain = (CSecurityDlg*)param;
	ssize_t readsize = 0;
	CommandDecoder decoder;

	while (pMain->m_isWorkingThread)
	{
		S_MSG_CMN_HDR stRcvMsgHdr;
		CString str = _T("");
		int msgHdlResult = 0;

		memset(&stRcvMsgHdr, 0x00, sizeof(S_MSG_CMN_HDR));

		if (pMain->m_isCheckSecureMode == 0)  // Non-Secure connection
		{
			readsize = ReadDataTcp(pMain->m_tcpConnectedPort, (unsigned char*)&stRcvMsgHdr, sizeof(stRcvMsgHdr));
		}
		else
		{
			readsize = ReadDataTls(pMain->ssl, (unsigned char*)&stRcvMsgHdr, sizeof(stRcvMsgHdr));
		}

		if (readsize == -1) //socket close
		{
			pMain->DisconnectServer(false);
			continue;
		}

		if (readsize != sizeof(stRcvMsgHdr))
			msgHdlResult = -1;

		decoder.DecodeCommonHeader(&stRcvMsgHdr);

		switch (stRcvMsgHdr.msgType)
		{
			case MSG_TYPE_RESPONSE:
				msgHdlResult = pMain->HandleResponseData(stRcvMsgHdr);
				break;

			case MSG_TYPE_STREAM:
				msgHdlResult = pMain->HandleStreamData(stRcvMsgHdr.dataLen);
				break;

			case MSG_TYPE_REQUEST: // FALL THROUGH
			default:
				msgHdlResult = -1;
				str.Format(_T("Invalid Message Type: %d"), stRcvMsgHdr.msgType);
				pMain->WriteLog(str);
				break;
		}

		if (msgHdlResult == -1)
		{
			str.Format(_T("Cannot handle message (corrupted)"));
			pMain->WriteLog(str);

			pMain->DisconnectServer(true);
		}
	}

	return 0;
}

CSecurityDlg::CSecurityDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SECURITY_DIALOG, pParent)
	, m_isCheckSecureMode(0), m_isConnecting(0), m_seqNum(0), m_isWorkingThread(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pBitmapInfo = NULL;
	m_pThread = NULL;
	m_tcpConnectedPort = NULL;
	m_clientSock = NULL;
	ssl = NULL;
	ctx = NULL;
	m_isCaptured = false;
}

void CSecurityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SECURE_MODE, m_isCheckSecureMode);
	DDX_Control(pDX, IDC_LOG_EDIT, m_LogView);
}


BEGIN_MESSAGE_MAP(CSecurityDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_BTN_CONNECT, &CSecurityDlg::OnBnClickedBtnConnect)
	ON_BN_CLICKED(IDC_BTN_CAPTURE, &CSecurityDlg::OnBnClickedBtnCapture)
	ON_BN_CLICKED(IDC_BTN_CONFIRM, &CSecurityDlg::OnBnClickedBtnConfirm)
	ON_BN_CLICKED(IDC_CHECK_SECURE_MODE, &CSecurityDlg::OnBnClickedCheckSecureMode)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_ID_EDIT, &CSecurityDlg::OnEnChangeIdEdit)
END_MESSAGE_MAP()


BOOL CSecurityDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);	
	SetIcon(m_hIcon, FALSE);

	SetWindowText(_T("[Team2] Project")); // Set Title

	GetDlgItem(IDC_NAME_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CAPTURE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CONFIRM)->EnableWindow(FALSE);

	GetDlgItem(IDC_RADIO_LEARNING)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_RUN)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIO_TESTRUN)->EnableWindow(FALSE);

	m_isConnecting = false;

	CButton* pCheck = (CButton*)GetDlgItem(IDC_RADIO_RUN);
	pCheck->SetCheck(1);

	// Set default sequence number
	SetRandSeqNum();

	WriteLog("---------------------\n");
	WriteLog("Start!\n");
	WriteLog("---------------------\n");

	return TRUE; 
}

void CSecurityDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CSecurityDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSecurityDlg::SetRandSeqNum()
{
	unsigned char* pBuff = new unsigned char[sizeof(unsigned int)];
	
	if(RAND_bytes(pBuff, sizeof(unsigned int) == 1))
		memcpy_s(&m_seqNum, sizeof(unsigned int), pBuff, sizeof(unsigned int));

	delete[] pBuff;
}

unsigned short CSecurityDlg::GetModeValue()
{
	unsigned short modeInMsg = 0;
	unsigned int modeCtrlValue = GetCheckedRadioButton(IDC_RADIO_LEARNING, IDC_RADIO_TESTRUN);

	CString str = _T("");

	switch (modeCtrlValue)
	{
	case IDC_RADIO_LEARNING:
		modeInMsg = MODE_LEARNING;
		break;

	case IDC_RADIO_RUN:
		modeInMsg = MODE_RUN;
		break;

	case IDC_RADIO_TESTRUN:
		modeInMsg = MODE_TESTRUN;
		break;

	default:
		str.Format(_T("Invalid Value (%d) in modeCtrlValue"), modeCtrlValue);
		WriteLog(str);
		break;
	}

	return modeInMsg;
}

bool CSecurityDlg::IsValidIpAddress()
{
	char* ipAddress = LPSTR(LPCTSTR(m_ipStr));

	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));

	return (result == 1);
}

bool CSecurityDlg::IsValidPortNum()
{
	int nPortNum = _ttoi(m_portStr);
	
	if ((nPortNum < 0) || (USHRT_MAX < nPortNum))
		return false;

	return true;
}

bool CSecurityDlg::IsValidString(CString& str)
{
	char* pStr = LPSTR(LPCTSTR(str));
	int nStrLen = str.GetLength();
	
	// Check length
	if ((nStrLen < MIN_STRING_LEN) || (nStrLen > MAX_STRING_LEN))
		return false;

	// Check string by whitelist (0~9, a~Z)
	for (int i = 0; i < nStrLen; i++)
	{
		if (!((pStr[i] >= 'a' && pStr[i] <= 'z') ||
			(pStr[i] >= 'A' && pStr[i] <= 'Z') ||
			(pStr[i] >= '0' && pStr[i] <= '9')))
		{
			return false;
		}
	}

	return true;
}

bool CSecurityDlg::IsValidLoginParam()
{
	// Check IP Addr Validity
	if (IsValidIpAddress() == false)
	{
		MessageBox(_T("Check IP Address"), _T("Invalid"), MB_ICONWARNING);
		return false;
	}

	// Check Port No Validity
	if (IsValidPortNum() == false)
	{
		MessageBox(_T("Check Port Number (0~65535)"), _T("Invalid"), MB_ICONWARNING);
		return false;
	}

	// Check ID Validity
	if (IsValidString(m_idStr) == false)
	{
		MessageBox(_T("Check ID (Length: 1~16, Only Alphabet and Number)"), _T("Invalid"), MB_ICONWARNING);
		return false;
	}

	// Check PW Validity
	if (IsValidString(m_pwdStr) == false)
	{
		MessageBox(_T("Check Password (Length: 1~16, Only Alphabet and Number)"), _T("Invalid"), MB_ICONWARNING);
		return false;
	}

	return true;
}

int CSecurityDlg::ConnectServer(unsigned short modeInMsg)
{
	GetDlgItemText(IDC_IP_EDIT, m_ipStr);
	GetDlgItemText(IDC_PORT_EDIT, m_portStr);
	GetDlgItemText(IDC_ID_EDIT, m_idStr);
	GetDlgItemText(IDC_PW_EDIT, m_pwdStr);

	char* pSendMsg = NULL;
	unsigned int sentLen = 0;

	if (IsValidLoginParam() == false)
		return -1;

	if (m_isCheckSecureMode == 0)  // Non-Secure connection
	{
		WriteLog("Non-secure mode\n");
		WriteLog(CStringA(m_ipStr).GetBuffer());
		WriteLog(CStringA(m_portStr).GetBuffer());


		m_tcpConnectedPort = OpenTcpConnection(CStringA(m_ipStr).GetBuffer(), CStringA(m_portStr).GetBuffer());
		if (m_tcpConnectedPort == NULL)  // Open TCP Network port
		{
			WriteLog("Open Tcp Connection Fail.\n");
			return -1;
		}

		m_clientSock = m_tcpConnectedPort->ConnectedFd;
		WriteLog("Open Tcp Connection Success\n");
	}
	else
	{
		WriteLog("Secure mode\n");

		ctx = InitCTX();
		if (ctx == NULL) {
			WriteLog("InitCTX Fail.\n");
			return -1;
		}

		WriteLog(CStringA(m_ipStr).GetBuffer());
		WriteLog(CStringA(m_portStr).GetBuffer());

		m_clientSock = OpenTLSConnection(CStringA(m_ipStr).GetBuffer(), CStringA(m_portStr).GetBuffer());
		if (m_clientSock <= 0) {
			WriteLog("Open TLS Connection Fail.\n");
			return -1;
		}

		SetCertForClient(ctx);

		ssl = SSL_new(ctx);							/* create new SSL connection state */
		if (ssl == NULL) {
			WriteLog("SSL_new() Fail.\n");
			return -1;
		}

		SSL_set_fd(ssl, (int)m_clientSock);					/* attach the socket descriptor */
		if (SSL_connect(ssl) == FAIL) 				/* perform the connection */
		{
			WriteLog("SSL_connect() Fail.\n");
			return -1;
		}
		
		CString str;
		X509* cert = NULL;
		char* line = NULL;

		cert = SSL_get_peer_certificate(ssl);    /* get the server's certificate */

		if (cert == NULL)
		{
			CloseTLSConnection(ssl);
			return -1;
		}
		
		long result = SSL_get_verify_result(ssl);

		if (result != X509_V_OK)
		{
			str.Format(_T("client verification with SSL_get_verify_result(%d) failed."), result);
			WriteLog(str);
			CloseTLSConnection(ssl);
			return -1;
		}

		WriteLog("client verification with SSL_get_verify_result() succeeded.\n");

		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		str.Format(_T("Subject: %s"), line);
		WriteLog(str);
		ErrorHandling(line);
		free(line);                            /* free the malloc'ed string */

		WriteLog("Open TLS Connection Success.\n");
	}

	m_isConnecting = true;
	m_isWorkingThread = true;

	sentLen = m_cmdEncoder.SendConnectReq(m_isCheckSecureMode, m_clientSock, ssl, &pSendMsg, m_seqNum, modeInMsg,
		m_idStr.GetLength(), CStringA(m_idStr).GetBuffer(), m_pwdStr.GetLength(), CStringA(m_pwdStr).GetBuffer());

	m_pThread = AfxBeginThread(ThreadForDisplay, this);

	// UI Update
	DisableControlBox();

	delete[] pSendMsg;

	return 0;
}

void CSecurityDlg::DisconnectServer(bool bClientClose)
{
	m_isWorkingThread = false;
	m_isConnecting = false;

	// Close TCP or TLS Connection
	if (bClientClose == true)
	{
		if (m_isCheckSecureMode == 0)  // Non-Secure connection
		{
			CloseTcpConnectedPort(&m_tcpConnectedPort);
		}
		else
		{
			CloseTLSConnection(ssl);
		}
	}
	else
	{
		WriteLog("Close Connection.\n");
	}

	// UI Update
	EnableControlBox();
}

void CSecurityDlg::OnBnClickedBtnConnect()
{
	if (m_isConnecting == true)
	{
		DisconnectServer(true);
	}
	else 
	{
		unsigned short modeInMsg = 0;

		if ((modeInMsg = GetModeValue()) == 0)
			return;

		ConnectServer(modeInMsg);
	}
}

int CSecurityDlg::CreateBitmapInfo(int w, int h, int bpp)
{
	if (m_pBitmapInfo != NULL)
	{
		delete m_pBitmapInfo;
		m_pBitmapInfo = NULL;
	}

	if (bpp == 8)
		m_pBitmapInfo = (BITMAPINFO*) new BYTE[sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD)];
	else // 24 or 32bit
		m_pBitmapInfo = (BITMAPINFO*) new BYTE[sizeof(BITMAPINFO)];

	if (m_pBitmapInfo == NULL)
		return 1;

	m_pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBitmapInfo->bmiHeader.biPlanes = 1;
	m_pBitmapInfo->bmiHeader.biBitCount = bpp;
	m_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	m_pBitmapInfo->bmiHeader.biSizeImage = 0;
	m_pBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
	m_pBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
	m_pBitmapInfo->bmiHeader.biClrUsed = 0;
	m_pBitmapInfo->bmiHeader.biClrImportant = 0;

	if (bpp == 8)
	{
		for (int i = 0; i < 256; i++)
		{
			m_pBitmapInfo->bmiColors[i].rgbBlue = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbGreen = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbRed = (BYTE)i;
			m_pBitmapInfo->bmiColors[i].rgbReserved = 0;
		}
	}

	m_pBitmapInfo->bmiHeader.biWidth = w;
	m_pBitmapInfo->bmiHeader.biHeight = -h;

	return 0;
}

void CSecurityDlg::DrawImage()
{
	CClientDC dc(GetDlgItem(IDC_PIC_VIEW));

	CRect rect;
	GetDlgItem(IDC_PIC_VIEW)->GetClientRect(&rect);

	SetStretchBltMode(dc.GetSafeHdc(), COLORONCOLOR);
	StretchDIBits(dc.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, 
		m_matImage.cols, m_matImage.rows, m_matImage.data, m_pBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

void CSecurityDlg::EnableControlBox()
{
	SetDlgItemText(ID_BTN_CONNECT, _T("CONNECT"));
	SetDlgItemText(IDC_BTN_CAPTURE, _T("CAPTURE"));

	GetDlgItem(IDC_IP_EDIT)->EnableWindow(TRUE);
	GetDlgItem(IDC_PORT_EDIT)->EnableWindow(TRUE);
	GetDlgItem(IDC_ID_EDIT)->EnableWindow(TRUE);
	GetDlgItem(IDC_PW_EDIT)->EnableWindow(TRUE);

	GetDlgItem(IDC_CHECK_SECURE_MODE)->EnableWindow(TRUE);

	GetDlgItem(IDC_RADIO_LEARNING)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIO_RUN)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIO_TESTRUN)->EnableWindow(TRUE);

	GetDlgItem(IDC_NAME_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CAPTURE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CONFIRM)->EnableWindow(FALSE);
}

void CSecurityDlg::DisableControlBox()
{
	SetDlgItemText(ID_BTN_CONNECT, _T("DISCONNECT"));

	GetDlgItem(IDC_IP_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_PORT_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ID_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_PW_EDIT)->EnableWindow(FALSE);

	GetDlgItem(IDC_CHECK_SECURE_MODE)->EnableWindow(FALSE);

	GetDlgItem(IDC_RADIO_LEARNING)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_RUN)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_TESTRUN)->EnableWindow(FALSE);

	if (GetCheckedRadioButton(IDC_RADIO_LEARNING, IDC_RADIO_TESTRUN) == IDC_RADIO_LEARNING)
	{
		m_isCaptured = false;
		GetDlgItem(IDC_NAME_EDIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_CAPTURE)->EnableWindow(TRUE);
		//GetDlgItem(IDC_BTN_CONFIRM)->EnableWindow(TRUE);
	}
}

void CSecurityDlg::OnBnClickedBtnCapture()
{
	char* pSendMsg = NULL; 
	unsigned int sentLen = 0;

	if (m_isCaptured == false)
	{
		sentLen = m_cmdEncoder.SendLearnCaptureReq(m_isCheckSecureMode, m_clientSock, ssl, &pSendMsg, m_seqNum);
		m_isCaptured = true;
		SetDlgItemText(IDC_BTN_CAPTURE, _T("CANCEL"));
		GetDlgItem(IDC_BTN_CONFIRM)->EnableWindow(TRUE);
	}
	else
	{
		sentLen = m_cmdEncoder.SendLearnCancelReq(m_isCheckSecureMode, m_clientSock, ssl, &pSendMsg, m_seqNum);
		m_isCaptured = false;
		SetDlgItemText(IDC_BTN_CAPTURE, _T("CAPTURE"));
		GetDlgItem(IDC_BTN_CONFIRM)->EnableWindow(FALSE);
	}

	delete[] pSendMsg;
}

void CSecurityDlg::OnBnClickedBtnConfirm()
{
	char* pSendMsg = NULL;

	GetDlgItemText(IDC_NAME_EDIT, m_nameStr);

	// Check Name Validity
	if (IsValidString(m_nameStr) == false)
	{
		MessageBox(_T("Check Name (Length: 1~16, Only Alphabet and Number)"), _T("Invalid"), MB_ICONWARNING);
		return;
	}

	unsigned int sentLen = m_cmdEncoder.SendLearnConfirmReq(m_isCheckSecureMode, m_clientSock, ssl, &pSendMsg, m_seqNum,
		m_nameStr.GetLength(), CStringA(m_nameStr).GetBuffer());

	delete[] pSendMsg;

	m_isCaptured = false;
}

void CSecurityDlg::WriteLog(CString msg)
{
	msg += _T("\n");
	int len = m_LogView.GetWindowTextLength(); 
	m_LogView.SetSel(len, len); 

	m_LogView.ReplaceSel(msg);

	TRACE(msg);

	unsigned int size = (unsigned int)strnlen_s(msg, MAX_LENGTH_LOG);
	Log(Normal, size, msg);				// Write log files
}

void CSecurityDlg::OnBnClickedCheckSecureMode()
{
	UpdateData(TRUE);
}

int CSecurityDlg::HandleResponseData(S_MSG_CMN_HDR &stRcvMsgHdr)
{
	CString str = _T("");

	if (stRcvMsgHdr.mode == MODE_RESULT_OK)
	{
		str.Format(_T("Response is OK (MsgID : %u) \r\n"), stRcvMsgHdr.msgId);
		WriteLog(str);

		return 0;
	}
	else
	{
		str.Format(_T("Response is NOK (MsgID : %u) \r\n"), stRcvMsgHdr.msgId);
		WriteLog(str);

		DisconnectServer(true);

		return -1;
	}
}

int CSecurityDlg::HandleStreamData(unsigned int dataLen)
{
	unsigned int imagesize = dataLen;
	ssize_t readsize = 0;
	unsigned char* buff = NULL;	/* receive buffer */
	CString str = _T("");

	buff = new unsigned char[imagesize];
	if (buff == NULL)
	{
		str.Format(_T("Cannot allocate memory (Len: %d)\n"), imagesize);
		WriteLog(str);
		return -1;
	}

	// Read Stream Data
	if (m_isCheckSecureMode == 0)  // Non-Secure connection
	{
		readsize = ReadDataTcp(m_tcpConnectedPort, buff, imagesize);
	}
	else
	{
		readsize = ReadDataTls(ssl, buff, imagesize);
	}

	// Check readsize and imagesize
	if (readsize != imagesize)
	{
		str.Format(_T("readsize: %lld, imagesize: %d\n"), readsize, imagesize);
		WriteLog(str); 
		
		delete[] buff;
		return -1;
	}
	
	// decode image
	cv::imdecode(cv::Mat(imagesize, 1, CV_8UC1, buff), cv::IMREAD_COLOR, &(m_matImage));
	delete[] buff;

	if (m_matImage.empty())
	{
		str.Format(_T("matImage is empty\n"));
		WriteLog(str); 
		return -1;
	}

	// Draw Image
	if(CreateBitmapInfo(m_matImage.cols, m_matImage.rows, m_matImage.channels() * 8) == 0)
		DrawImage();

	return 0;	
}

CCriticalSection g_criticalLog;
CCriticalSection g_criticalExe;


void Log(int nType, unsigned int Size, const char* fmt, ...)
{
	g_criticalLog.Lock();

	char* szLog = NULL;
	FILE* fp = NULL;
	CTime tm = CTime::GetCurrentTime();

	CString strLog = _T("");
	CString strPath = _T("");

	va_list args;
	SYSTEMTIME cur_time;

	if (Size > (MAX_LENGTH_LOG - 64)) return;

	if (fmt == NULL)
	{
		g_criticalLog.Unlock();
		return;
	}

	szLog = (char*)malloc(MAX_LENGTH_LOG);

	if (szLog != NULL) {
		ZeroMemory(szLog, MAX_LENGTH_LOG);
	}
	else {
		return;
	}

	va_start(args, fmt);
	wvsprintf(szLog, fmt, args);
	va_end(args);

	char* path_0 = (char*)malloc(MAX_PATH);
	if (path_0 != NULL) {
		GetCurrentDirectory(MAX_PATH, path_0);
		CreateDirectory("Logs", NULL);
		CString path_1;
		path_1.Format(_T("%s"), path_0);
		CString path_2 = tm.Format(_T("\\Logs\\%Y_%m_%d_%H_Log.txt"));
		strPath = path_1 + path_2;
		free(path_0);
	}
	else {
		free(szLog);
		return;
	}

	fopen_s(&fp, (LPSTR)(LPCSTR)strPath, "a+");
	GetLocalTime(&cur_time);

	strLog.Format("%04d-%02d-%02d %02d:%02d:%02d:%03ld : ",
	cur_time.wYear,
		cur_time.wMonth,
		cur_time.wDay,
		cur_time.wHour,
		cur_time.wMinute,
		cur_time.wSecond,
		cur_time.wMilliseconds);

	if (fp != NULL) {
		switch (nType)
		{
		case Normal: 
			strLog += _T("[NORMAL]:"); break;
		case Debug: 
			strLog += _T("[DEBUG]:"); break;
		case End: 
			strLog += _T("[END]:"); break;
		case Err: 
			strLog += _T("[ERROR]:"); break;
		default:
			strLog += _T("[DEBUG]:"); break;
		}

		strLog += szLog;

		fprintf(fp, "%s", (LPSTR)(LPCSTR)strLog );
		fflush(fp);
		fclose(fp);
	}
	else {
		CString strErrorMsg;
		DWORD dwErrNo = GetLastError();
		strErrorMsg.Format("LOG FILE Open Fail: Code=[ %d ],", dwErrNo);
	}
	free(szLog);
	g_criticalLog.Unlock();
}

void CSecurityDlg::OnClose()
{
	WriteLog("Close.");
	m_isConnecting = false;
	m_isWorkingThread = false;

	if (m_pBitmapInfo != NULL)
	{
		delete m_pBitmapInfo;
		m_pBitmapInfo = NULL;
	}

	if (ssl != NULL) {
		SSL_free(ssl);                                /* release connection state */
		closesocket(m_clientSock);                    /* close socket */
		SSL_CTX_free(ctx);                            /* release context */
	}

	CDialogEx::OnClose();
}


void CSecurityDlg::OnEnChangeIdEdit()
{
	CString str;

	GetDlgItemText(IDC_ID_EDIT, str);

	if (str.Compare("admin") == 0)
	{
		GetDlgItem(IDC_RADIO_LEARNING)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_RUN)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_TESTRUN)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_RADIO_LEARNING)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_RUN)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_TESTRUN)->EnableWindow(FALSE);
	}
}
