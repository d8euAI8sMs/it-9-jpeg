// jpegDlg.cpp : implementation file
//

#include "stdafx.h"
#include "jpeg.h"
#include "jpegDlg.h"
#include "afxdialogex.h"

#include "CompareDialog.h"
#include "ImgDialog.h"

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
    DDX_Control(pDX, IDC_EDIT1, m_srcSize[0]);
    DDX_Control(pDX, IDC_EDIT2, m_dstSize[0]);
    DDX_Control(pDX, IDC_EDIT3, m_ratio[0]);
    DDX_Control(pDX, IDC_EDIT4, m_srcSize[1]);
    DDX_Control(pDX, IDC_EDIT9, m_dstSize[1]);
    DDX_Control(pDX, IDC_EDIT5, m_ratio[1]);
    DDX_Control(pDX, IDC_EDIT7, m_srcSize[2]);
    DDX_Control(pDX, IDC_EDIT6, m_dstSize[2]);
    DDX_Control(pDX, IDC_EDIT8, m_ratio[2]);
    DDX_Control(pDX, IDC_EDIT10, m_srcSize[3]);
    DDX_Control(pDX, IDC_EDIT12, m_dstSize[3]);
    DDX_Control(pDX, IDC_EDIT11, m_ratio[3]);
    DDX_Control(pDX, IDC_SLIDER1, m_quality);
    DDX_Control(pDX, IDC_EDIT13, m_msqDistance);
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
    ON_BN_CLICKED(IDC_BUTTON7, &CJpegDlg::OnBnClickedButton7)
    ON_BN_CLICKED(IDC_BUTTON8, &CJpegDlg::OnBnClickedButton8)
    ON_BN_CLICKED(IDC_BUTTON9, &CJpegDlg::OnBnClickedButton9)
    ON_BN_CLICKED(IDC_BUTTON10, &CJpegDlg::OnBnClickedButton10)
    ON_BN_CLICKED(IDC_BUTTON11, &CJpegDlg::OnBnClickedButton11)
    ON_BN_CLICKED(IDC_BUTTON12, &CJpegDlg::OnBnClickedButton12)
    ON_WM_HSCROLL()
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
    
    m_quality.SetRange(0, 50, TRUE);
    m_quality.SetPos(50);

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
    CompressImg(m_quality.GetPos() / 100.);
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
        m_srcSize[0].SetWindowText(fmt);
        m_srcSize[1].SetWindowText(fmt);
        m_srcSize[2].SetWindowText(fmt);
        m_srcSize[3].SetWindowText(fmt);
        
        OnBnClickedButton4();
    }
}


void CJpegDlg::OnBnClickedButton2()
{
    SaveImg(model::img_type::bmp);
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


void CJpegDlg::OnBnClickedButton7()
{
    if (m_data.src.bmp.header.h == 0) return;
    if (m_data.dst.bmp.header.h == 0) return;

    m_data.src.bmp.rle_compress(m_data.rsrc);

    model::make_huffman_table(m_data.hsrc.table, model::get_rle_stat(m_data.rsrc));
    model::make_huffman_table(m_data.hdst.table, model::get_rle_stat(m_data.rdst));

    model::huffman_compress(m_data.hsrc, m_data.rsrc);
    model::huffman_compress(m_data.hdst, m_data.rdst);

    CString fmt; fmt.Format(TEXT("%d"), m_data.rsrc.size());
    m_dstSize[1].SetWindowText(fmt);
    fmt.Format(TEXT("%lf"), (double) m_data.rsrc.size() / m_data.src.bmp.size() * 100);
    m_ratio[1].SetWindowText(fmt);

    fmt.Format(TEXT("%d"), (m_data.hdst.size_bits + 7) / 8);
    m_dstSize[2].SetWindowText(fmt);
    fmt.Format(TEXT("%lf"), (double) (m_data.hdst.size_bits + 7) / 8 / m_data.src.bmp.size() * 100);
    m_ratio[2].SetWindowText(fmt);

    fmt.Format(TEXT("%d"), (m_data.hsrc.size_bits + 7) / 8);
    m_dstSize[3].SetWindowText(fmt);
    fmt.Format(TEXT("%lf"), (double) (m_data.hsrc.size_bits + 7) / 8 / m_data.src.bmp.size() * 100);
    m_ratio[3].SetWindowText(fmt);
}

void CJpegDlg::SaveImg(model::img_type t)
{
    if (m_data.src.bmp.header.h == 0 || m_data.rdst.size() == 0 || m_data.hdst.size_bits == 0) return;

    CString ext; model::byte_t flags; model::img_header header; double jpeg_q;
    switch (t)
    {
    case model::img_type::bmp:       ext = "mybmp";  flags = 0; header = m_data.src.bmp.header; break;
    case model::img_type::rle:       ext = "mybmp";  flags = 1; header = m_data.src.bmp.header; break;
    case model::img_type::huff:      ext = "mybmp";  flags = 2; header = m_data.src.bmp.header; break;
    case model::img_type::jpeg:      ext = "myjpeg"; flags = 0; header = m_data.dst.bmp.header; jpeg_q = m_data.rdst.quality; break;
    case model::img_type::jpeg_huff: ext = "myjpeg"; flags = 2; header = m_data.dst.bmp.header; jpeg_q = m_data.rdst.quality; break;
    default: return;
    }

    CFileDialog fd(FALSE, ext, TEXT("Untitled"));
    if (fd.DoModal() == IDOK)
    {
        auto p = fd.GetPathName();

        CFile f(p, CFile::modeWrite | CFile::modeCreate);

        f.Write(&flags, 1);
        f.Write(&header, sizeof(model::img_header));
        
        model::rle_data * rle = nullptr;
        model::huffman_data * huff = nullptr;

        switch (t)
        {
        case model::img_type::rle:  rle  = &m_data.rsrc;    break;
        case model::img_type::huff: huff = &m_data.hsrc;    break;
        case model::img_type::jpeg: rle  = &m_data.rdst;    break;
        case model::img_type::jpeg_huff: huff = &m_data.hdst; break;
        default: break;
        }

        size_t sz;

        switch (t)
        {
        case model::img_type::bmp:
            for (size_t i = 0; i < m_data.src.bmp.header.h; ++i)
            for (size_t j = 0; j < m_data.src.bmp.header.w; ++j)
            {
                f.Write(&m_data.src.bmp.pixels[i][j], 3);
            }
            break;
        case model::img_type::rle:
        case model::img_type::jpeg:
            if (t == model::img_type::jpeg) f.Write(&jpeg_q, sizeof(double));
            sz = rle->data.size();
            f.Write(&sz, sizeof(size_t));
            f.Write(rle->data.data(), sizeof(model::rle) * sz);
            break;
        case model::img_type::huff:
        case model::img_type::jpeg_huff:
            if (t == model::img_type::jpeg_huff) f.Write(&jpeg_q, sizeof(double));
            sz = huff->table.index.size();
            f.Write(&sz, sizeof(size_t));
            for each (auto & p in huff->table.index)
            {
                f.Write(&p.first, 1);
                f.Write(&p.second, sizeof(model::huffman_code));
            }
            sz = huff->size_bits;
            f.Write(&sz, sizeof(size_t));
            f.Write(huff->data.data(), (sz + 7) / 8);
            break;
        default: break;
        }

        f.Close();
    }
}

std::unique_ptr < CBitmap > CJpegDlg::LoadImg()
{
    CString ext; model::img_type t; model::byte_t flags; model::img_header header;

    CFileDialog fd(TRUE, ext, TEXT("Untitled"));
    if (fd.DoModal() == IDOK)
    {
        auto p = fd.GetPathName();
        ext = fd.GetFileExt();

        CFile f(p, CFile::modeRead);

        f.Read(&flags, 1);
        f.Read(&header, sizeof(model::img_header));
        
        if (ext == CString("mybmp"))
        {
            if (flags == 0) t = model::img_type::bmp;
            else if (flags == 1) t = model::img_type::rle;
            else if (flags == 2) t = model::img_type::huff;
            else return {};
        }
        else if (ext == CString("myjpeg"))
        {
            if (flags == 0) t = model::img_type::jpeg;
            else if (flags == 2) t = model::img_type::jpeg_huff;
            else return {};
        }
        else return {};

        switch (t)
        {
        case model::img_type::bmp:
        {
            model::bitmap bmp;
            bmp.header = header;
            bmp.pixels.resize(header.h);
            for (size_t i = 0; i < header.h; ++i)
            {
                bmp.pixels[i].resize(header.w);
                for (size_t j = 0; j < header.w; ++j)
                {
                    f.Read(&bmp.pixels[i][j], 3);
                }
            }
            auto r = std::make_unique < CBitmap > ();
            bmp.to_cbitmap(*r);

            f.Close();
            return r;
        }
        case model::img_type::rle:
        case model::img_type::jpeg:
        {
            model::jpeg_data rle;
            rle.header = header;
            if (t == model::img_type::jpeg) f.Read(&rle.quality, sizeof(double));
            size_t sz;
            f.Read(&sz, sizeof(size_t));
            rle.data.resize(sz);
            f.Read(rle.data.data(), sz * sizeof(model::rle));
            auto r = std::make_unique < CBitmap > ();
            model::bitmap bmp;
            if (t == model::img_type::rle)
                bmp.rle_decompress(rle);
            else bmp.jpeg_decompress(rle);
            bmp.to_cbitmap(*r);

            f.Close();
            return r;
        }
        case model::img_type::huff:
        case model::img_type::jpeg_huff:
        {
            model::huffman_data huff;
            model::jpeg_data rle;
            huff.header = header;
            size_t sz;
            if (t == model::img_type::jpeg_huff) f.Read(&rle.quality, sizeof(double));
            f.Read(&sz, sizeof(size_t));
            huff.table.min_bits = 0xff;
            for (size_t i = 0; i < sz; ++i)
            {
                model::byte_t c; model::huffman_code hc;
                f.Read(&c, 1); f.Read(&hc, sizeof(model::huffman_code));
                huff.table.min_bits = min(huff.table.min_bits, hc.bits);
                huff.table.index.emplace(c, hc);
                huff.table.rev_index.emplace(hc, c);
            }
            f.Read(&sz, sizeof(size_t));
            huff.size_bits = sz;
            huff.data.resize((sz + 7) / 8);
            f.Read(huff.data.data(), (sz + 7) / 8);
            model::huffman_decompress(huff, rle);
            auto r = std::make_unique < CBitmap > ();
            model::bitmap bmp;
            if (t == model::img_type::huff)
                bmp.rle_decompress(rle);
            else bmp.jpeg_decompress(rle);
            bmp.to_cbitmap(*r);

            f.Close();
            return r;
        }
        default:
            break;
        }
    }
    return {};
}


void CJpegDlg::OnBnClickedButton8()
{
    SaveImg(model::img_type::jpeg);
}


void CJpegDlg::OnBnClickedButton9()
{
    SaveImg(model::img_type::rle);
}


void CJpegDlg::OnBnClickedButton10()
{
    SaveImg(model::img_type::jpeg_huff);
}


void CJpegDlg::OnBnClickedButton11()
{
    SaveImg(model::img_type::huff);
}


void CJpegDlg::OnBnClickedButton12()
{
    auto r = LoadImg();
    CImgDialog d(this, r.get());
    d.DoModal();
}


void CJpegDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CompressImg(m_quality.GetPos() / 100.);

    CSimulationDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CJpegDlg::CompressImg(double q)
{
    m_data.src.bmp.jpeg_compress(m_data.rdst, q);
    m_data.dst.bmp.jpeg_decompress(m_data.rdst);
    m_data.dst.bmp.to_cbitmap(m_data.dst.cbmp);
    m_dstImg.RedrawWindow();
    CString fmt; fmt.Format(TEXT("%d"), m_data.rdst.size());
    m_dstSize[0].SetWindowText(fmt);
    fmt.Format(TEXT("%lf"), (double) m_data.rdst.size() / m_data.src.bmp.size() * 100);
    m_ratio[0].SetWindowText(fmt);

    double dist = 0;
    for (size_t i = 0; i < m_data.src.bmp.header.h; ++i)
    for (size_t j = 0; j < m_data.src.bmp.header.w; ++j)
    {
        auto & p1 = m_data.src.bmp.pixels[i][j];
        auto & p2 = m_data.dst.bmp.pixels[i][j];
        double s1 = (double)p1.r - (double)p2.r;
        double s2 = (double)p1.g - (double)p2.g;
        double s3 = (double)p1.b - (double)p2.b;
        dist += (s1 * s1 + s2 * s2 + s3 * s3) / 255 / 255;
    }
    dist /= m_data.src.bmp.header.h * m_data.src.bmp.header.w;

    fmt.Format(TEXT("%lf"), std::sqrt(dist));
    m_msqDistance.SetWindowText(fmt);
}