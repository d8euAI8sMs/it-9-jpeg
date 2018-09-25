// AnalyzingDialog.cpp : implementation file
//

#include "stdafx.h"
#include "jpeg.h"
#include "AnalyzingDialog.h"
#include "afxdialogex.h"


// CAnalyzingDialog dialog

IMPLEMENT_DYNAMIC(CAnalyzingDialog, CDialogEx)

CAnalyzingDialog::CAnalyzingDialog(CWnd* pParent, model::bitmap * src)
	: CDialogEx(CAnalyzingDialog::IDD, pParent)
    , m_src(src)
{
}

CAnalyzingDialog::~CAnalyzingDialog()
{
}

void CAnalyzingDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_Q, m_quality);
    DDX_Control(pDX, IDC_C, m_compression);
}


BEGIN_MESSAGE_MAP(CAnalyzingDialog, CDialogEx)
END_MESSAGE_MAP()


// CAnalyzingDialog message handlers


BOOL CAnalyzingDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    model::globs::init();

    m_qualityCfg = model::make_plot_config();
    m_compressionCfg = model::make_plot_config();
    m_jpegData = model::make_plot_data(plot::palette::pen(RGB(155, 0, 0), 2));
    m_qualityData = model::make_plot_data(plot::palette::pen(RGB(155, 0, 0), 2));
    
    m_quality.plot_layer.with(model::make_root_drawable(m_qualityCfg, {{ m_qualityData.plot }}));
    m_compression.plot_layer.with(model::make_root_drawable(m_compressionCfg, {{ m_jpegData.plot }}));
    
    model::bitmap bmp;
    model::jpeg_data rle;
    for (size_t c = 0; c <= 50; ++c)
    {
        m_src->jpeg_compress(rle, c / 100.);
        bmp.jpeg_decompress(rle);
        
        double dist = 0;
        for (size_t i = 0; i < bmp.header.h; ++i)
        for (size_t j = 0; j < bmp.header.w; ++j)
        {
            auto & p1 = m_src->pixels[i][j];
            auto & p2 = bmp.pixels[i][j];
            double s1 = (double)p1.r - (double)p2.r;
            double s2 = (double)p1.g - (double)p2.g;
            double s3 = (double)p1.b - (double)p2.b;
            dist += (s1 * s1 + s2 * s2 + s3 * s3) / 255 / 255;
        }
        dist /= bmp.header.h * bmp.header.w;
        
        m_qualityData.data->push_back({ c / 100., dist });
        m_jpegData.data->push_back({ c / 100., (double) rle.size() / m_src->size() });
    }
    m_qualityCfg.autoworld->setup(*m_qualityData.data);
    m_compressionCfg.autoworld->setup(*m_jpegData.data);

    UpdateData(FALSE);
    
    m_quality.RedrawWindow();
    m_compression.RedrawWindow();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
