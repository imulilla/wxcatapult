// $Id: ScreenShotDlg.cpp,v 1.3 2004/11/11 17:14:59 h_oudejans Exp $
// ScreenshotDlg.cpp: implementation of the FullScreenDlg class.
//
//////////////////////////////////////////////////////////////////////
#include "wx/wxprec.h"
#include "wx/xrc/xmlres.h"

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "ConfigurationData.h"
#include "ScreenShotDlg.h"

IMPLEMENT_CLASS(ScreenshotDlg, wxDialog)
BEGIN_EVENT_TABLE(ScreenshotDlg, wxDialog)
	EVT_BUTTON(XRCID(wxT("OkButton")),ScreenshotDlg::OnOk)	
END_EVENT_TABLE()


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ScreenshotDlg::ScreenshotDlg(wxWindow * parent)
{
	wxXmlResource::Get()->LoadDialog(this, parent, wxT("ScreenshotDialog"));
}

ScreenshotDlg::~ScreenshotDlg()
{
	
}

void ScreenshotDlg::OnOk(wxCommandEvent &event)
{
	ConfigurationData * config = ConfigurationData::instance();
	wxCheckBox * box = (wxCheckBox *)FindWindowByName(wxT("DoNotShow"));
	long notwarn = 0;
	if (box->IsChecked()) {
		notwarn = 1;
	}
	config->SetParameter(ConfigurationData::CD_SCREENSHOTINFO,notwarn);
	config->SaveData();
	EndModal (wxID_OK);
}
