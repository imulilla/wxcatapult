// $Id: MiscControlPage.cpp,v 1.9 2004/04/08 18:57:23 h_oudejans Exp $
// MiscControlPage.cpp: implementation of the MiscControlPage class.
//
//////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"
#include "wx/xrc/xmlres.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "SessionPage.h"
#include "MiscControlPage.h"
#include "wxCatapultFrm.h"
#include "openMSXController.h"

	IMPLEMENT_CLASS(MiscControlPage, wxPanel)
BEGIN_EVENT_TABLE(MiscControlPage, wxPanel)
	EVT_BUTTON(XRCID("ResetButton"),MiscControlPage::OnReset)
	EVT_TOGGLEBUTTON (XRCID("PauseButton"),MiscControlPage::OnPause)
	EVT_TOGGLEBUTTON (XRCID("PowerButton"),MiscControlPage::OnPower)
	EVT_TOGGLEBUTTON (XRCID("FirmwareButton"),MiscControlPage::OnFirmware)
	EVT_COMMAND_SCROLL (XRCID("SpeedSlider"),MiscControlPage::OnSpeedChange)
	EVT_BUTTON(XRCID("NormalSpeedButton"),MiscControlPage::OnSetNormalSpeed)
	EVT_BUTTON(XRCID("MaxSpeedButton"),MiscControlPage::OnSetMaxSpeed)
	EVT_COMMAND_SCROLL (XRCID("FrameSkipSlider"),MiscControlPage::OnFrameSkipChange)
	EVT_BUTTON(XRCID("ZeroFrameSkipButton"),MiscControlPage::OnSetZeroFrameSkip)
	EVT_TOGGLEBUTTON(XRCID("AutoFrameSkipButton"),MiscControlPage::OnSetAutoFrameSkip)
	EVT_TOGGLEBUTTON(XRCID("ThrottleButton"),MiscControlPage::OnSetThrottle)
	EVT_TOGGLEBUTTON(XRCID("CmdTimingButton"),MiscControlPage::OnSetCmdTiming)
	EVT_TEXT(XRCID("FrameSkipIndicator"),MiscControlPage::OnInputFrameskip)
	EVT_TEXT(XRCID("SpeedIndicator"),MiscControlPage::OnInputSpeed)
	EVT_TEXT(XRCID("Joyport1Selector"),MiscControlPage::OnChangeJoystick)
	EVT_TEXT(XRCID("Joyport2Selector"),MiscControlPage::OnChangeJoystick)
END_EVENT_TABLE()

	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////

MiscControlPage::MiscControlPage(wxWindow * parent, openMSXController * controller)
:CatapultPage(parent)
{
	wxXmlResource::Get()->LoadPanel(this, parent, _("MiscControlPage"));
	m_controller = controller;
	m_powerButton = (wxToggleButton *)FindWindow(_("PowerButton"));
	m_pauseButton = (wxToggleButton *)FindWindow(_("PauseButton"));	
	m_resetButton = (wxButton *)FindWindow(_("ResetButton"));
	m_firmwareButton = (wxToggleButton *)FindWindow(_("FirmwareButton"));
	m_speedIndicator = (wxTextCtrl *)FindWindow(_("SpeedIndicator"));
	m_speedSlider = (wxSlider *)FindWindow(_("SpeedSlider"));

	m_speedNormalButton = (wxButton *)FindWindow (_("NormalSpeedButton"));
	m_speedMaxButton = (wxButton *)FindWindow (_("MaxSpeedButton"));
	m_frameSkipIndicator = (wxTextCtrl *)FindWindow(_("FrameSkipIndicator"));
	m_frameSkipSlider = (wxSlider *)FindWindow(_("FrameSkipSlider"));

	m_frameSkipZeroButton = (wxButton *)FindWindow (_("ZeroFrameSkipButton"));
	m_frameSkipAutoButton = (wxToggleButton *)FindWindow (_("AutoFrameSkipButton"));
	m_throttleButton = (wxToggleButton *)FindWindow (_("ThrottleButton"));
	m_cmdTimingButton = (wxToggleButton *)FindWindow (_("CmdTimingButton"));

	m_speedSlider->SetTickFreq (25,1);
	m_frameSkipSlider->SetTickFreq (5,1);
	m_autoFrameSkipEnabled = false;
	m_frameSkipSetting = "maxframeskip";
}

MiscControlPage::~MiscControlPage()
{
}

void MiscControlPage::OnReset(wxCommandEvent &event)
{
	m_controller->WriteCommand ("reset");
}

void MiscControlPage::OnPause(wxCommandEvent &event)
{
	m_controller->WriteCommand ("toggle pause");
}

void MiscControlPage::OnPower(wxCommandEvent &event)
{
	m_controller->WriteCommand ("toggle power");
	if (m_powerButton->GetValue())
	{
		m_controller->WriteCommand ("set pause off");
		m_pauseButton->SetValue(false);
	}
}

void MiscControlPage::OnFirmware(wxCommandEvent &event)
{
	m_controller->WriteCommand ("toggle frontswitch");
}

void MiscControlPage::OnSpeedChange(wxScrollEvent &event)
{
	wxString speedText;
	speedText.sprintf("%ld", event.GetInt());
	m_speedIndicator->SetValue(speedText);
	m_controller->WriteCommand (wxString("set speed ") + speedText);
	m_throttleButton->SetValue(true);
	m_throttleButton->SetLabel(_("On"));
	m_controller->WriteCommand ("set throttle on");
}

void MiscControlPage::OnSetNormalSpeed(wxCommandEvent &event)
{
	m_controller->WriteCommand ("set speed 100");
	m_speedIndicator->SetValue(_("100"));
	m_speedSlider->SetValue (100);
	m_throttleButton->SetValue(true);
	m_throttleButton->SetLabel(_("On"));
	m_controller->WriteCommand ("set throttle on");
}

void MiscControlPage::OnSetMaxSpeed(wxCommandEvent &event)
{
	m_controller->WriteCommand ("set speed 500");
	m_speedIndicator->SetValue(_("500"));
	m_speedSlider->SetValue (500);
	m_throttleButton->SetValue(false);
	m_throttleButton->SetLabel(_("Off"));
	m_controller->WriteCommand ("set throttle off");
}

void MiscControlPage::OnFrameSkipChange(wxScrollEvent &event)
{
	wxString skipText;
	skipText.sprintf("%ld", event.GetInt());
	m_frameSkipIndicator->SetValue(skipText);
	m_controller->WriteCommand (wxString("set " + m_frameSkipSetting + " ") + skipText);
}

void MiscControlPage::OnSetZeroFrameSkip(wxCommandEvent &event)
{
	m_controller->WriteCommand ("set " + m_frameSkipSetting + " 0");
	m_frameSkipIndicator->Enable();
	m_frameSkipIndicator->SetValue(_("0"));
	m_frameSkipSlider->Enable();
	m_frameSkipSlider->SetValue (0);
	m_frameSkipAutoButton->SetValue(false);
}

void MiscControlPage::OnSetAutoFrameSkip(wxCommandEvent &event)
{
	wxToggleButton * button = (wxToggleButton *)event.m_eventObject;
	if (button->GetValue()){
		m_frameSkipIndicator->SetValue(_("auto"));
	}
	else{
		int val = m_frameSkipSlider->GetValue();
		wxString strval;
		strval.sprintf ("%d",val);
		m_frameSkipIndicator->SetValue(strval);
	}
}

void MiscControlPage::OnSetThrottle(wxCommandEvent &event)
{
	wxToggleButton * button = (wxToggleButton *)event.m_eventObject;
	if (button->GetValue())
	{
		m_controller->WriteCommand ("set throttle on");
		button->SetLabel("On");
	}
	else
	{
		m_controller->WriteCommand ("set throttle off");
		button->SetLabel("Off");
	}	
}

void MiscControlPage::OnSetCmdTiming(wxCommandEvent &event)
{
	wxToggleButton * button = (wxToggleButton *)event.m_eventObject;
	if (button->GetValue())
	{
		m_controller->WriteCommand ("set cmdtiming broken");
		button->SetLabel("Broken");
	}
	else
	{
		m_controller->WriteCommand ("set cmdtiming real");
		button->SetLabel("Real");
	}
}

void MiscControlPage::SetControlsOnLaunch()
{
	m_speedSlider->Enable(true);
	m_speedIndicator->Enable(true);
	m_speedNormalButton->Enable(true);
	m_speedMaxButton->Enable(true);

	m_frameSkipSlider->Enable(true);
	m_frameSkipIndicator->Enable(true);
	m_frameSkipZeroButton->Enable(true);

	m_powerButton->Enable(true);
	m_resetButton->Enable(true);
	m_pauseButton->Enable(true);

	m_throttleButton->Enable(true);
	m_cmdTimingButton->Enable(true);
	m_pauseButton->SetValue(false);
	m_firmwareButton->SetValue(false);
}

void MiscControlPage::EnableFirmware()
{
	m_firmwareButton->Enable(true);
}

void MiscControlPage::SetControlsOnEnd()
{
	m_speedSlider->Enable(false);
	m_speedIndicator->Enable(false);
	m_speedNormalButton->Enable(false);
	m_speedMaxButton->Enable(false);

	m_frameSkipSlider->Enable(false);
	m_frameSkipIndicator->Enable(false);
	m_frameSkipAutoButton->Enable(false);
	m_frameSkipZeroButton->Enable(false);

	m_powerButton->Enable(false);
	m_resetButton->Enable(false);
	m_firmwareButton->Enable(false);
	m_pauseButton->Enable(false);

	m_throttleButton->Enable(false);
	m_cmdTimingButton->Enable(false);
	wxComboBox * box;
	box = (wxComboBox *)FindWindow (_("Joyport1Selector"));
	box->Clear();
	box->Enable(false);
	box = (wxComboBox *)FindWindow (_("Joyport2Selector"));
	box->Clear();
	box->Enable(false);
}

void MiscControlPage::OnInputSpeed(wxCommandEvent &event)
{
	wxString text = m_speedIndicator->GetValue ();
	if (text.IsNumber())
	{
		unsigned long num;
		text.ToULong(&num);
		if (num >500) 
		{
			num = 500;
			text = _("500");
			m_speedIndicator->SetValue(text);
		}
		if (num >= 1)
		{
			m_controller->WriteCommand (wxString("set speed ") + text);
			m_speedSlider->SetValue(num);
		}
	}
}

void MiscControlPage::OnInputFrameskip(wxCommandEvent &event)
{
	wxString text = m_frameSkipIndicator->GetValue ();
	if (text.IsNumber())
	{
		unsigned long num;
		text.ToULong(&num);
		if (num > 100)
		{
			num = 100;
			text = _("100");
			m_frameSkipIndicator->SetValue(text);
		}
		if (num >= 0)
		{
			m_controller->WriteCommand (wxString("set " + m_frameSkipSetting + " ") + text);
			m_frameSkipSlider->SetValue(num);
			m_frameSkipAutoButton->SetValue(false);
		}
	}	
	if (!text.CmpNoCase(_("auto")) && m_autoFrameSkipEnabled)
	{
		m_controller->WriteCommand ("set frameskip auto");
		m_frameSkipAutoButton->SetValue(true);
	}
}

void MiscControlPage::SetSpeed (wxString value)
{
	m_speedIndicator->SetValue(value);
}

void MiscControlPage::SetFrameskip (wxString value)
{
	m_frameSkipIndicator->SetValue(value);
}

void MiscControlPage::SetThrottle (wxString value)
{
	if (value == _("on")){
		m_throttleButton->SetValue(true);
		m_throttleButton->SetLabel(_("On"));
	}
	else {
		m_throttleButton->SetValue(false);
		m_throttleButton->SetLabel(_("Off"));
	}
}

void MiscControlPage::SetCmdTiming (wxString value)
{
	if (value == _("broken")){
		m_cmdTimingButton->SetValue(true);
		m_cmdTimingButton->SetLabel(_("Broken"));
	}
	else {
		m_cmdTimingButton->SetValue(false);
		m_cmdTimingButton->SetLabel(_("Real"));
	}
}

void MiscControlPage::EnableAutoFrameSkip()
{
	m_frameSkipAutoButton->Enable(true);
	m_frameSkipAutoButton->SetValue(false);
	m_autoFrameSkipEnabled = true;
	m_frameSkipSetting = "frameskip";

}

void MiscControlPage::InitConnectorPanel ()
{
	wxArrayString connectors;
	wxString currentClass;
	m_controller->GetConnectors(connectors);
	if (connectors.GetCount() > 0){
		for (unsigned i=0;i<connectors.GetCount();i++){
			currentClass = m_controller->GetConnectorClass(connectors[i]);
			if (connectors[i] == _("joyporta")){
				InitJoystickPort (connectors[i], "Joyport1Selector", currentClass);
			} else if (connectors[i] == _("joyportb")){
				InitJoystickPort (connectors[i], "Joyport2Selector", currentClass);
			}
		}
	}
}

void MiscControlPage::InitJoystickPort (wxString connector, wxString control, wxString connectorClass)
{
	wxArrayString pluggables;
	wxArrayString classes;
	m_controller->GetPluggableClasses(classes);
	if (classes.GetCount() ==0) return;
	wxComboBox * box = (wxComboBox *)FindWindow (control);
	box->Enable(true);
	box->Clear();
	box->Append(_("--empty--"));
	wxString currentClass;
	m_controller->GetPluggables(pluggables);
	if (pluggables.GetCount() > 0){
		for (unsigned i=0;i<pluggables.GetCount();i++){
			if (classes[i] == connectorClass){
				box->Append(pluggables[i]);
			}
		}
	}
	wxString plugged = m_controller->GetConnectorContents(connector);
	box->SetSelection(box->FindString(plugged.Mid(connector.Len()+2,plugged.Len()-connector.Len()-3)));
}

void MiscControlPage::OnChangeJoystick(wxCommandEvent & event)
{
	wxComboBox * box = (wxComboBox *)event.GetEventObject();
	wxString cmd = "unplug ";
	wxString connector;
	wxString value = "";
	if (box->GetName() == "Joyport1Selector"){
		connector = "joyporta ";
	} 
	else if (box->GetName() == "Joyport2Selector"){
		connector = "joyportb ";
	}
	
	if (box->GetValue() != _("--empty--")){
		value = box->GetValue();
		cmd = "plug ";
	}

	m_controller->WriteCommand(wxString(cmd + connector + value));
}
