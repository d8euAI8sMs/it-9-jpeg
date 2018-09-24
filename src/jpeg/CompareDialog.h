#pragma once

#include <util/common/gui/PlotControl.h>

#include "model.h"

// CCompareDialog dialog

class CCompareDialog : public CDialogEx
{
    DECLARE_DYNAMIC(CCompareDialog)

public:
    CCompareDialog(CWnd* pParent, model::model_data * d, size_t mag);   // standard constructor
    virtual ~CCompareDialog();

// Dialog Data
    enum { IDD = IDD_COMPAREDIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    model::model_data * m_data;
    size_t m_mag;

    DECLARE_MESSAGE_MAP()
public:
    CPlotControl m_img;
    virtual BOOL OnInitDialog();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};
