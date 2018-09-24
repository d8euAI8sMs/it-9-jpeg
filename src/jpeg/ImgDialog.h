#pragma once

#include <util/common/gui/PlotControl.h>

// CImgDialog dialog

class CImgDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CImgDialog)

public:
	CImgDialog(CWnd* pParent, CBitmap * bmp);   // standard constructor
	virtual ~CImgDialog();

// Dialog Data
	enum { IDD = IDD_IMGDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    CBitmap * m_bmp;

	DECLARE_MESSAGE_MAP()
public:
    CPlotControl m_img;
    virtual BOOL OnInitDialog();
};
