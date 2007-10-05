#ifndef afx_filterextractdlg_h
#define afx_filterextractdlg_h

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
   CAboutDlg();
   
   // Dialog Data
   //{{AFX_DATA(CAboutDlg)
   enum { IDD = IDD_ABOUTBOX };
   //}}AFX_DATA
   
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CAboutDlg)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL
   
   // Implementation
protected:
   //{{AFX_MSG(CAboutDlg)
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CFilterExtractDlg dialog

class CFilterExtractDlg : public CDialog
{
// Construction
public:
   CFilterExtractDlg(CWnd* pParent = NULL);   // standard constructor

   libwin::CPSPlugIn*  m_pPSPlugIn;

// Dialog Data
   //{{AFX_DATA(CFilterExtractDlg)
	enum { IDD = IDD_DIALOG1 };
	CString	m_sPuncture;
	CString	m_sCodec;
	BOOL	m_bInterleave;
	BOOL	m_bPresetStrength;
	int		m_nEmbedRate;
	int		m_nEmbedSeed;
	double	m_dEmbedStrength;
	CString	m_sEmbedded;
	CString	m_sExtracted;
	int		m_nInterleaverSeed;
	double	m_dInterleaverDensity;
	CString	m_sSource;
	int		m_nSourceSeed;
	int		m_nSourceType;
	CString	m_sUniform;
	CString	m_sDecoded;
	CString	m_sResults;
	int		m_nFeedback;
	BOOL	m_bPrintBER;
	BOOL	m_bPrintSNR;
	BOOL	m_bPrintEstimate;
	BOOL	m_bPrintChiSquare;
	CString	m_sEmbeddedImage;
	CString	m_sExtractedImage;
	//}}AFX_DATA

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CFilterExtractDlg)
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   int m_nFileSize, m_nRawSize;
   int m_nCodecIn, m_nCodecOut;
   int m_nPunctureIn, m_nPunctureOut;

   void ComputeFileData();
   void ComputeCodecData();
   void ComputePunctureData();
	void UpdateDisplay();

   // Generated message map functions
   //{{AFX_MSG(CFilterExtractDlg)
   virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnLoadSource();
	afx_msg void OnLoadCodec();
	afx_msg void OnLoadPuncture();
	afx_msg void OnClearSource();
	afx_msg void OnClearCodec();
	afx_msg void OnClearPuncture();
	afx_msg void OnComputeStrength();
	afx_msg void OnInterleave();
	afx_msg void OnSelchangeSourceType();
	afx_msg void OnChangeInterleaverDensity();
	afx_msg void OnChangeEmbedRate();
	afx_msg void OnPresetStrength();
	afx_msg void OnSaveEmbedded();
	afx_msg void OnSaveExtracted();
	afx_msg void OnSaveUniform();
	afx_msg void OnClearEmbedded();
	afx_msg void OnClearExtracted();
	afx_msg void OnClearUniform();
	afx_msg void OnClearDecoded();
	afx_msg void OnSaveDecoded();
	afx_msg void OnClearResults();
	afx_msg void OnSaveResults();
	afx_msg void OnClearEmbeddedImage();
	afx_msg void OnSaveEmbeddedImage();
	afx_msg void OnClearExtractedImage();
	afx_msg void OnSaveExtractedImage();
	//}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif





