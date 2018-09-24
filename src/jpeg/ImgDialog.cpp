// ImgDialog.cpp : implementation file
//

#include "stdafx.h"
#include "jpeg.h"
#include "ImgDialog.h"
#include "afxdialogex.h"


// CImgDialog dialog

IMPLEMENT_DYNAMIC(CImgDialog, CDialogEx)

CImgDialog::CImgDialog(CWnd* pParent, CBitmap * bmp)
	: CDialogEx(CImgDialog::IDD, pParent)
    , m_bmp(bmp)
{
}

CImgDialog::~CImgDialog()
{
}

void CImgDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_IMG, m_img);
}


BEGIN_MESSAGE_MAP(CImgDialog, CDialogEx)
END_MESSAGE_MAP()


// CImgDialog message handlers


BOOL CImgDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    if (!m_bmp) return TRUE;

    auto c = [this] (CDC & dc, const plot::viewport & vp)
    {
        if (m_bmp == nullptr) return;
        CDC memDC; memDC.CreateCompatibleDC(&dc);
        memDC.SelectObject(m_bmp);
        auto wh = m_bmp->GetBitmapDimension();
        dc.BitBlt(vp.screen.xmin, vp.screen.ymin, wh.cx, wh.cy,
                      &memDC, 0, 0, SRCCOPY);
    };

    m_img.plot_layer.with(plot::custom_drawable::create(c));

    auto wh = m_bmp->GetBitmapDimension();
    CRect sz;
    m_img.GetClientRect(&sz);

    int dx = wh.cx - sz.Width(), dy = wh.cy - sz.Height();
    m_img.SetWindowPos(nullptr, 0, 0, sz.Width() + dx, sz.Height() + dy, SWP_NOMOVE | SWP_NOZORDER);
    GetWindowRect(&sz);
    SetWindowPos(nullptr, sz.left, sz.top, sz.Width() + dx, sz.Height() + dy, SWP_NOMOVE | SWP_NOZORDER);

    m_img.RedrawWindow();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
