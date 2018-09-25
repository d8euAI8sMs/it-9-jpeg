#pragma once

#include <util/common/gui/PlotControl.h>

#include "model.h"

// CAnalyzingDialog dialog

class CAnalyzingDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CAnalyzingDialog)

public:
	CAnalyzingDialog(CWnd* pParent, model::bitmap * src);   // standard constructor
	virtual ~CAnalyzingDialog();

// Dialog Data
	enum { IDD = IDD_ANALYZINGDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    model::bitmap * m_src;
    model::plot_config m_qualityCfg;
    model::plot_config m_compressionCfg;
    model::plot_data m_qualityData;
    model::plot_data m_jpegData;

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    CPlotControl m_quality;
    CPlotControl m_compression;
};
