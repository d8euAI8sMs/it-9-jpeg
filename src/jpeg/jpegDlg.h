// jpegDlg.h : header file
//

#include <util/common/gui/SimulationDialog.h>
#include <util/common/gui/PlotControl.h>

#include "model.h"

#pragma once

// CJpegDlg dialog
class CJpegDlg : public CSimulationDialog
{
// Construction
public:
    CJpegDlg(CWnd* pParent = NULL);    // standard constructor

// Dialog Data
    enum { IDD = IDD_JPEG_DIALOG };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    CPlotControl m_srcImg;
    CPlotControl m_dstImg;
    CEdit m_srcSize[4];
    CEdit m_dstSize[4];
    CEdit m_ratio[4];
    CEdit m_msqDistance;
    CSliderCtrl m_quality;
    model::model_data m_data;
    afx_msg void OnBnClickedButton4();
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();
    afx_msg void OnBnClickedButton3();
    afx_msg void OnBnClickedButton5();
    afx_msg void OnBnClickedButton6();
    afx_msg void OnBnClickedButton7();
    void AnalyzeAndCompare();
    void CompressImg(double q);
    void SaveImg(model::img_type t);
    std::unique_ptr < CBitmap > LoadImg();
    afx_msg void OnBnClickedButton8();
    afx_msg void OnBnClickedButton9();
    afx_msg void OnBnClickedButton10();
    afx_msg void OnBnClickedButton11();
    afx_msg void OnBnClickedButton12();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
