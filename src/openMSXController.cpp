// $Id: openMSXController.cpp,v 1.10 2004/03/20 19:53:32 manuelbi Exp $
// openMSXController.cpp: implementation of the openMSXController class.
//
//////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxCatapultFrm.h"
#include "ConfigurationData.h"
#include "CatapultXMLParser.h"
#include "openMSXController.h"
#include "StatusPage.h"
#include "VideoControlPage.h"
#include "MiscControlPage.h"
#include "AudioControlPage.h"
#include "wxCatapultApp.h"
#include <cassert>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

openMSXController::openMSXController(wxWindow * target)
{
	m_openMSXID = 0;
	m_appWindow = (wxCatapultFrame *)target;
	m_openMsxRunning = false;
	m_launchMode = LAUNCH_NONE;
}

openMSXController::~openMSXController()
{
}

bool openMSXController::HandleMessage(wxCommandEvent &event)
{
	int id = event.GetId();
	switch (id)
	{
		case MSGID_STDOUT:
			HandleStdOut(event);
			break;
		case MSGID_STDERR:
			HandleStdErr(event);
			break;
		case MSGID_PARSED:
			HandleParsedOutput(event);
			break;
		case MSGID_ENDPROCESS:
			HandleEndProcess(event);
			break;
		default:
			return false; // invalid ID
	}
	return true;
}

bool openMSXController::PreLaunch()
{
	m_parser = new CatapultXMLParser(m_appWindow);
	return true;
}

bool openMSXController::PostLaunch()
{
	WriteMessage (_("<openmsx-control>\n"));
	switch (m_launchMode){
	case LAUNCH_HIDDEN:
		WriteCommand (_("openmsx_info"));
		break;
	case LAUNCH_NORMAL:
		WriteCommand (GetInfoCommand(_("renderer")));
		break;
	default:
		assert (false);
		break;
	}

	m_appWindow->StartTimer();
	m_appWindow->SetStatusText("Running emulator");
	return true;
}

void openMSXController::HandleEndProcess(wxCommandEvent &event)
{
	if (!m_openMsxRunning) 
		return;

	m_appWindow->StopTimer();
	m_appWindow->SetStatusText("Ready");
	delete m_parser;
	m_commands.clear();
	m_appWindow->m_audioControlPage->DestroyAudioMixer();
	m_openMsxRunning = false;
	m_appWindow->m_launch_AbortButton->Enable(true);
	m_appWindow->DisableControls();
	m_appWindow->m_launch_AbortButton->SetLabel(_("Launch"));
}

void openMSXController::HandleStdOut(wxCommandEvent &event)
{
	wxString * data = (wxString *)event.GetClientData();
	m_parser->ParseXmlInput(*data,m_openMSXID);
	delete data;
}

void openMSXController::HandleStdErr(wxCommandEvent &event)
{
	wxString * data = (wxString *)event.GetClientData();
	if (*data == "\n")
		return;
	m_appWindow->m_tabControl->SetSelection(1);	
	m_appWindow->m_statusPage->m_outputtext->SetDefaultStyle (wxTextAttr(wxColour(255,23,23),wxNullColour,wxFont(10,wxMODERN,wxNORMAL,wxNORMAL)));

	m_appWindow->m_statusPage->m_outputtext->AppendText(*data);
#ifdef __UNIX__
	m_appWindow->m_statusPage->m_outputtext->AppendText(_("\n"));
#endif
}

void openMSXController::HandleParsedOutput(wxCommandEvent &event)
{
	CatapultXMLParser::ParseResult * data = (CatapultXMLParser::ParseResult *)event.GetClientData();
	if (data->openMSXID != m_openMSXID)
		return; // another instance is allready started, ignore this message
	switch (data->parseState)
	{
		case CatapultXMLParser::TAG_UPDATE:
			if (m_launchMode != LAUNCH_HIDDEN){
				if (data->updateType == CatapultXMLParser::UPDATE_LED)
					m_appWindow->m_statusPage->UpdateLed (data->name, data->contents);
			}
			break;
		case CatapultXMLParser::TAG_REPLY:
			switch (m_launchMode){
				case LAUNCH_HIDDEN:
					HandleHiddenLaunchReply(event);
					break;
				default:
					switch (data->replyState) {			
						case CatapultXMLParser::REPLY_OK:
							if (m_launchMode == LAUNCH_NORMAL){
								HandleNormalLaunchReply(event);
							}
							else{
								wxString command = GetPendingCommand();
								if (command == GetInfoCommand("fps")) m_appWindow->SetFPSdisplay(data->contents);
							}
							break;
						case CatapultXMLParser::REPLY_NOK:
							m_appWindow->m_statusPage->m_outputtext->SetDefaultStyle(wxTextAttr(wxColour(174,0,0),wxNullColour,wxFont(10,wxMODERN,wxNORMAL,wxBOLD)));
							m_appWindow->m_statusPage->m_outputtext->AppendText(_("Warning: NOK received on command: "));
							m_appWindow->m_statusPage->m_outputtext->AppendText(GetPendingCommand());
							m_appWindow->m_statusPage->m_outputtext->AppendText(_("\n"));
							if (!data->contents.IsEmpty()){
								m_appWindow->m_statusPage->m_outputtext->AppendText(_("contents = "));
								m_appWindow->m_statusPage->m_outputtext->AppendText(data->contents);
								m_appWindow->m_statusPage->m_outputtext->AppendText(_("\n"));
							}
							break;
						case CatapultXMLParser::REPLY_UNKNOWN:
							m_appWindow->m_statusPage->m_outputtext->SetDefaultStyle(wxTextAttr(wxColour(174,0,0),wxNullColour,wxFont(10,wxMODERN,wxNORMAL,wxBOLD)));
							m_appWindow->m_statusPage->m_outputtext->AppendText(_("Warning: Unknown reply received!\n"));
							break;
					}
					break;				
			}
			
			break; 
		case CatapultXMLParser::TAG_LOG:
			if (m_launchMode != LAUNCH_HIDDEN){
				switch (data->logLevel) {
				case CatapultXMLParser::LOG_WARNING:
					m_appWindow->m_statusPage->m_outputtext->SetDefaultStyle (wxTextAttr(wxColour(174,0,0),wxNullColour,wxFont(10,wxMODERN,wxNORMAL,wxNORMAL)));
					break;
				case CatapultXMLParser::LOG_INFO:
				case CatapultXMLParser::LOG_UNKNOWN:
					m_appWindow->m_statusPage->m_outputtext->SetDefaultStyle (wxTextAttr(wxColour(0,0,0),wxNullColour,wxFont(10,wxMODERN,wxNORMAL,wxNORMAL)));
					break;
				}	
				m_appWindow->m_statusPage->m_outputtext->AppendText(data->contents);		
				m_appWindow->m_statusPage->m_outputtext->AppendText(_("\n"));
			}
			break;
		default:
			break;
	}
	delete data;
}

bool openMSXController::WriteCommand(wxString msg)
{
	if (!m_openMsxRunning) 
		return false;
	m_commands.push_back(msg);
	if (!WriteMessage (wxString(_("<command>") + msg + _("</command>\n"))))
		return false;
	return true;
}

wxString openMSXController::GetPendingCommand()
{
	assert (!m_commands.empty()); 
	wxString pending = m_commands.front();
	m_commands.pop_front();
	return wxString(pending);
}

void openMSXController::StartOpenMSX(wxString cmd, bool hidden)
{
	m_openMSXID++;
	if (hidden){
		m_launchMode = LAUNCH_HIDDEN;
	}
	else {
		m_launchMode = LAUNCH_NORMAL;
	}
	Launch(cmd);			
	
}

void openMSXController::HandleHiddenLaunchReply(wxCommandEvent &event)
{
	CatapultXMLParser::ParseResult * data = (CatapultXMLParser::ParseResult *)event.GetClientData();
	if (data->replyState == CatapultXMLParser::REPLY_UNKNOWN){
		wxMessageBox (_("Unable to determen openMSX capacities"));
		m_launchMode = LAUNCH_NONE;
		return;
	}
	wxString command = GetPendingCommand();
	if (command == _("openmsx_info")){
		switch (data->replyState){
		case CatapultXMLParser::REPLY_OK:
			m_infoCommand = _("openmsx_info");
			if (data->contents.Find("\n") == -1){
				m_infoCommand = _("openmsx_info_tcl");	
			}
			WriteCommand(_("quit"));
			break;
		case CatapultXMLParser::REPLY_NOK:
			WriteCommand("info");
			return;
		default:
			break;
		}
	} 
	else if (command == _("info")) {
		switch (data->replyState) {
		case CatapultXMLParser::REPLY_OK:
			m_infoCommand = _("info");
			WriteCommand(_("quit"));
			break;
		case CatapultXMLParser::REPLY_NOK:
			wxMessageBox (_("Please update openMSX to 0.3.3 or higher"));
			m_launchMode = LAUNCH_NONE;
			return;
		default:
			break;
		}
	}
	else if (command == _("quit")){
		m_launchMode = LAUNCH_NONE;
		m_appWindow->m_launch_AbortButton->Enable(true);
	}

}

void openMSXController::HandleNormalLaunchReply(wxCommandEvent &event)
{
	int i;
	CatapultXMLParser::ParseResult * data = (CatapultXMLParser::ParseResult *)event.GetClientData();
	wxString command = GetPendingCommand();
	if (command == GetInfoCommand(_("renderer"))){
		WriteCommand (GetInfoCommand(_("scaler")));
		m_appWindow->m_videoControlPage->FillRenderers(data->contents);		
	}
	else if (command == GetInfoCommand(_("scaler"))){
		WriteCommand (_("set power on"));
		m_appWindow->m_videoControlPage->FillScalers(data->contents);		
	}
	else if (command == _("set power on")){
		WriteCommand (_("restoredefault renderer"));		
	}
	else if (command == _("restoredefault renderer")){
		WriteCommand (_("set renderer"));
	}
	else if (command == _("set renderer")){
		m_appWindow->m_videoControlPage->SetRenderer(FilterCurrentValue(data->contents));
		WriteCommand (_("set scaler"));
	}
	else if (command == _("set scaler")){
		m_appWindow->m_videoControlPage->SetScaler(FilterCurrentValue(data->contents));
		WriteCommand (_("set accuracy"));
	}
	else if (command == _("set accuracy")){
		m_appWindow->m_videoControlPage->SetAccuracy(FilterCurrentValue(data->contents));
		WriteCommand (_("set deinterlace"));
	}
	else if (command == _("set deinterlace")){
		m_appWindow->m_videoControlPage->SetDeinterlace(FilterCurrentValue(data->contents));
		WriteCommand (_("set limitsprites"));
	}
	else if (command == _("set limitsprites")){
		m_appWindow->m_videoControlPage->SetLimitSprites(FilterCurrentValue(data->contents));
		WriteCommand (_("set blur"));
	}
	else if (command == _("set blur")){
		m_appWindow->m_videoControlPage->SetBlur(FilterCurrentValue(data->contents));
		WriteCommand (_("set glow"));
		}
	else if (command == _("set glow")){
		m_appWindow->m_videoControlPage->SetGlow(FilterCurrentValue(data->contents));
		WriteCommand (_("set gamma"));
	}
	else if (command == _("set gamma")){
		m_appWindow->m_videoControlPage->SetGamma(FilterCurrentValue(data->contents));
		WriteCommand (_("set scanline"));
	}
	else if (command == _("set scanline")){
		m_appWindow->m_videoControlPage->SetScanline(FilterCurrentValue(data->contents));
		WriteCommand (_("set speed"));
	}
	else if (command == _("set speed")){
		m_appWindow->m_miscControlPage->SetSpeed(FilterCurrentValue(data->contents));
		WriteCommand (_("set frameskip"));
	}
	else if (command == _("set frameskip")){
		m_appWindow->m_miscControlPage->SetFrameskip(FilterCurrentValue(data->contents));
		WriteCommand (_("set throttle"));
	}
	else if (command == _("set throttle")){
		m_appWindow->m_miscControlPage->SetThrottle(FilterCurrentValue(data->contents));
		WriteCommand (_("set cmdtiming"));
	}
	else if (command == _("set cmdtiming")){
		m_appWindow->m_miscControlPage->SetCmdTiming(FilterCurrentValue(data->contents));
		WriteCommand (GetInfoCommand(_("sounddevice")));
	}
	else if (command == GetInfoCommand(_("sounddevice"))){
		m_appWindow->m_audioControlPage->InitAudioChannels(data->contents);
		WriteCommand (wxString(GetInfoCommand(_("sounddevice ")+m_appWindow->m_audioControlPage->GetAudioChannelName(1))));
	}
	else if (m_appWindow->m_audioControlPage->GetNumberOfAudioChannels() >0){
			for (i=0;i<m_appWindow->m_audioControlPage->GetNumberOfAudioChannels()-1;i++){
				if (command == wxString(_("set ") + m_appWindow->m_audioControlPage->GetAudioChannelName(i)+_("_volume"))){
					m_appWindow->m_audioControlPage->SetChannelVolume(i,FilterCurrentValue(data->contents));
					WriteCommand (wxString(_("set ") + m_appWindow->m_audioControlPage->GetAudioChannelName(i+1)+("_volume")));
				}
				if (command == wxString(_("set ") + m_appWindow->m_audioControlPage->GetAudioChannelName(i)+_("_mode"))){
					m_appWindow->m_audioControlPage->SetChannelMode(i,FilterCurrentValue(data->contents));
					WriteCommand (wxString(_("set ") + m_appWindow->m_audioControlPage->GetAudioChannelName(i+1)+("_mode")));
				}
				if (command == wxString(GetInfoCommand(_("sounddevice ")+m_appWindow->m_audioControlPage->GetAudioChannelName(i)))){
					m_appWindow->m_audioControlPage->AddChannelType(i,FilterCurrentValue(data->contents));
					WriteCommand (wxString(GetInfoCommand(_("sounddevice ")+m_appWindow->m_audioControlPage->GetAudioChannelName(i+1))));
				}
			}
			if (command == wxString(GetInfoCommand(_("sounddevice ")+m_appWindow->m_audioControlPage->GetAudioChannelName(m_appWindow->m_audioControlPage->GetNumberOfAudioChannels()-1)))){
				m_appWindow->m_audioControlPage->AddChannelType(m_appWindow->m_audioControlPage->GetNumberOfAudioChannels()-1,FilterCurrentValue(data->contents));
				m_appWindow->m_audioControlPage->SetupAudioMixer();		
				WriteCommand (wxString(_("set ") + m_appWindow->m_audioControlPage->GetAudioChannelName(0)+_("_volume")));
			}
			if (command == wxString(_("set ") + m_appWindow->m_audioControlPage->GetAudioChannelName(m_appWindow->m_audioControlPage->GetNumberOfAudioChannels()-1)+_("_volume"))){
				m_appWindow->m_audioControlPage->SetChannelVolume(m_appWindow->m_audioControlPage->GetNumberOfAudioChannels()-1,FilterCurrentValue(data->contents));
				WriteCommand (wxString(_("set ") + m_appWindow->m_audioControlPage->GetAudioChannelName(1)+_("_mode")));
			}		
			if (command == wxString(_("set ") + m_appWindow->m_audioControlPage->GetAudioChannelName(m_appWindow->m_audioControlPage->GetNumberOfAudioChannels()-1)+_("_mode"))){
				m_appWindow->m_audioControlPage->SetChannelMode(m_appWindow->m_audioControlPage->GetNumberOfAudioChannels()-1,FilterCurrentValue(data->contents));
				wxSize tempsize = m_appWindow->GetSize();
				tempsize.SetHeight(tempsize.GetHeight()+1);
				tempsize.SetWidth(tempsize.GetWidth()+1);
				m_appWindow->SetSize(tempsize);
				m_appWindow->EnableControls();
				m_launchMode = LAUNCH_NONE; // interactive mode
			}
	}
}

// if openmsx version < 0.3.5 filter the current value from the explanation

wxString openMSXController::FilterCurrentValue(wxString value)
{
	wxString temp = value;
	int pos = temp.Find(_("current value   : "));
	if (pos != -1){
		wxString temp2;
		temp = temp.Mid(pos+18);
		pos = temp.Find(0x0A);
		if (pos != -1){
			return (wxString(temp.Left(pos)));
		}
	}
	return value; // can't find it, just return the input
}

wxString openMSXController::GetInfoCommand (wxString parameter)
{
	wxString infoCommand;
	wxString tempCmd = m_infoCommand;
	wxString endCmd = "";
	if (m_infoCommand == _("openmsx_info_tcl")){
		tempCmd = _("join [openmsx_info");
		endCmd = _("] \\n");
	}
	infoCommand = tempCmd + _(" ") + parameter + endCmd;
	return wxString (infoCommand);
}
