// CompareDialog.cpp : implementation file
//

#include "stdafx.h"
#include "jpeg.h"
#include "CompareDialog.h"
#include "afxdialogex.h"


// CCompareDialog dialog

IMPLEMENT_DYNAMIC(CCompareDialog, CDialogEx)

CCompareDialog::CCompareDialog(CWnd* pParent, model::model_data * d, size_t mag)
    : CDialogEx(CCompareDialog::IDD, pParent)
    , m_data(d)
    , m_mag(mag)
{
}

CCompareDialog::~CCompareDialog()
{
}

void CCompareDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PICT, m_img);
}


BEGIN_MESSAGE_MAP(CCompareDialog, CDialogEx)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CCompareDialog message handlers


BOOL CCompareDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    auto c1 = [this] (CDC & dc, const plot::viewport & vp)
    {
        if (m_data == nullptr || m_data->src.bmp.header.h == 0) return;
        CDC memDC; memDC.CreateCompatibleDC(&dc);
        memDC.SelectObject(&m_data->src.cbmp);
        auto wh = m_data->src.cbmp.GetBitmapDimension();
        dc.StretchBlt(vp.screen.xmin, vp.screen.ymin, wh.cx * m_mag, wh.cy * m_mag,
                      &memDC, 0, 0, wh.cx, wh.cy, SRCCOPY);
    };
    auto c2 = [this] (CDC & dc, const plot::viewport & vp)
    {
        if (m_data == nullptr || m_data->dst.bmp.header.h == 0) return;
        CDC memDC; memDC.CreateCompatibleDC(&dc);
        memDC.SelectObject(&m_data->dst.cbmp);
        auto wh = m_data->dst.cbmp.GetBitmapDimension();
        dc.StretchBlt(vp.screen.xmin, vp.screen.ymin, wh.cx * m_mag, wh.cy * m_mag,
                      &memDC, 0, 0, wh.cx, wh.cy, SRCCOPY);
    };

    m_img.plot_layer.with(plot::custom_drawable::create(c1));
    m_img.plot_layer.with(plot::custom_drawable::create(c2));
    m_img.plot_layer.layers[1]->visible = false;

    auto wh = m_data->src.bmp.header;
    wh.w *= m_mag;
    wh.h *= m_mag;
    CRect sz;
    m_img.GetClientRect(&sz);

    int dx = wh.w - sz.Width(), dy = wh.h - sz.Height();
    m_img.SetWindowPos(nullptr, 0, 0, sz.Width() + dx, sz.Height() + dy, SWP_NOMOVE | SWP_NOZORDER);
    GetWindowRect(&sz);
    SetWindowPos(nullptr, sz.left, sz.top, sz.Width() + dx, sz.Height() + dy, SWP_NOMOVE | SWP_NOZORDER);

    m_img.RedrawWindow();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CCompareDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_img.plot_layer.layers[0]->visible = false;
    m_img.plot_layer.layers[1]->visible = true;

    m_img.RedrawWindow();

    CDialogEx::OnLButtonDown(nFlags, point);
}


void CCompareDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_img.plot_layer.layers[0]->visible = true;
    m_img.plot_layer.layers[1]->visible = false;

    m_img.RedrawWindow();

    CDialogEx::OnLButtonUp(nFlags, point);
}
