// jpegDlg.cpp : implementation file
//

#include "stdafx.h"
#include "jpeg.h"
#include "jpegDlg.h"
#include "afxdialogex.h"

#include "CompareDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CJpegDlg dialog

CJpegDlg::CJpegDlg(CWnd* pParent /*=NULL*/)
    : CSimulationDialog(CJpegDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CJpegDlg::DoDataExchange(CDataExchange* pDX)
{
    CSimulationDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SRC, m_srcImg);
    DDX_Control(pDX, IDC_DST, m_dstImg);
    DDX_Control(pDX, IDC_EDIT1, m_srcSize);
    DDX_Control(pDX, IDC_EDIT2, m_dstSize);
    DDX_Control(pDX, IDC_EDIT3, m_ratio);
}

BEGIN_MESSAGE_MAP(CJpegDlg, CSimulationDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON4, &CJpegDlg::OnBnClickedButton4)
    ON_BN_CLICKED(IDC_BUTTON1, &CJpegDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &CJpegDlg::OnBnClickedButton2)
    ON_BN_CLICKED(IDC_BUTTON3, &CJpegDlg::OnBnClickedButton3)
    ON_BN_CLICKED(IDC_BUTTON5, &CJpegDlg::OnBnClickedButton5)
    ON_BN_CLICKED(IDC_BUTTON6, &CJpegDlg::OnBnClickedButton6)
END_MESSAGE_MAP()

// CJpegDlg message handlers

BOOL CJpegDlg::OnInitDialog()
{
    CSimulationDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);            // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // TODO: Add extra initialization here

    model::globs::init();

    m_srcImg.plot_layer.with(model::make_bmp_plot(m_data.src));
    m_dstImg.plot_layer.with(model::make_bmp_plot(m_data.dst));

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CJpegDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CSimulationDialog::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CJpegDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CJpegDlg::OnBnClickedButton4()
{
    m_data.src.bmp.jpeg_compress(m_data.rdst);
    m_data.dst.bmp.jpeg_decompress(m_data.rdst);
    m_data.dst.bmp.to_cbitmap(m_data.dst.cbmp);
    m_dstImg.RedrawWindow();
    CString fmt; fmt.Format(TEXT("%d"), m_data.rdst.size());
    m_dstSize.SetWindowText(fmt);
    fmt.Format(TEXT("%lf"), (double) m_data.rdst.size() / m_data.src.bmp.size() * 100);
    m_ratio.SetWindowText(fmt);
}


void CJpegDlg::OnBnClickedButton1()
{
    CFileDialog fd(TRUE, TEXT("bmp"));
    if (fd.DoModal() == IDOK)
    {
        CImage img; img.Load(fd.GetPathName());
        auto w = img.GetWidth(), h = img.GetHeight();
        m_data.src.cbmp.DeleteObject();
        m_data.src.cbmp.Attach(img.Detach());
        m_data.src.cbmp.SetBitmapDimension(w, h);
        m_data.src.bmp.from_cbitmap(m_data.src.cbmp);
        m_srcImg.RedrawWindow();
        CString fmt; fmt.Format(TEXT("%d"), m_data.src.bmp.size());
        m_srcSize.SetWindowText(fmt);
        
        OnBnClickedButton4();
    }
}


void CJpegDlg::OnBnClickedButton2()
{
    if (m_data.src.bmp.header.h == 0 || m_data.rdst.size() == 0) return;
    CFileDialog fd(TRUE);
    if (fd.DoModal() == IDOK)
    {
        auto p = fd.GetPathName();
        CFile f1(p + CString(".myjpeg"), CFile::modeWrite | CFile::modeCreate);
        CFile f2(p + CString(".mybmp"), CFile::modeWrite | CFile::modeCreate);

        for (size_t i = 0; i < m_data.src.bmp.header.h; ++i)
        for (size_t j = 0; j < m_data.src.bmp.header.w; ++j)
        {
            f1.Write(&m_data.src.bmp.pixels[i][j], 3);
        }

        for (size_t i = 0; i < m_data.rdst.data.size(); ++i)
        {
            f2.Write(&m_data.rdst.data[i], 2);
        }

        f1.Close();
        f2.Close();
    }
}


void CJpegDlg::OnBnClickedButton3()
{
    CCompareDialog cd(this, &m_data, 1);
    cd.DoModal();
}


void CJpegDlg::OnBnClickedButton5()
{
    CCompareDialog cd(this, &m_data, 2);
    cd.DoModal();
}


void CJpegDlg::OnBnClickedButton6()
{
    CCompareDialog cd(this, &m_data, 4);
    cd.DoModal();
}
