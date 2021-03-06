#ifndef WXCATAPULTFRAME_H
#define WXCATAPULTFRAME_H

#include <wx/tglbtn.h>
#include <wx/timer.h>
#include <wx/socket.h>
#include <wx/frame.h>
#include <wx/notebook.h> // forward declaration doesn't work on wx 3.0 for some reason
#include <memory>

class CatapultXMLParser;
class SessionPage;
class StatusPage;
class InputPage;
class VideoControlPage;
class AudioControlPage;
class MiscControlPage;
class openMSXController;
class wxButton;
class wxNotebookEvent;
class wxStaticBitmap;

// Define a new frame type: this is going to be our main frame
class wxCatapultFrame : public wxFrame
{
public:
	wxCatapultFrame(wxWindow* parent = nullptr);

	void OpenMSXStarted();
	void OpenMSXStopped();
	void EnableMainWindow();
	void UpdateLed(const wxString& ledname, const wxString& ledstate);
	void UpdateState(const wxString& statename, const wxString& state);
	void CheckConfigs();

	SessionPage* m_sessionPage;
	StatusPage* m_statusPage;
	VideoControlPage* m_videoControlPage;
	MiscControlPage* m_miscControlPage;
	AudioControlPage* m_audioControlPage;
	InputPage* m_inputPage;

	wxNotebook* m_tabControl;

private:
	void StartTimers();
	void StopTimers();
	void SetControlsOnLaunch();
	void SetControlsOnEnd();
	void EnableSaveSettings(bool enabled);

	// event handlers (these functions should _not_ be virtual)
	void OnControllerEvent(wxCommandEvent& event);
	void OnMenuQuit(wxCommandEvent& event);
	void OnMenuCheckConfigs(wxCommandEvent& event);
	void OnMenuAbout(wxCommandEvent& event);
	void OnMenuEditConfig(wxCommandEvent& event);
	void OnMenuLoadSettings(wxCommandEvent& event);
	void OnMenuSaveSettings(wxCommandEvent& event);
	void OnMenuSaveSettingsAs(wxCommandEvent& event);
	void OnMenuSaveOnExit(wxCommandEvent& event);
	void OnMenuDisplayBroken (wxCommandEvent& event);
	void OnMenuClose (wxMenuEvent& event);
	void OnMenuOpen (wxMenuEvent& event);
	void OnMenuHighlight(wxMenuEvent& event);
	void OnLaunch(wxCommandEvent& event);
	void OnUpdateFPS(wxTimerEvent& event);
	void OnEnableMainWindow(wxTimerEvent& event);
	void OnCheckFocus(wxTimerEvent& event);
	void OnChangePage(wxNotebookEvent& event);
	void OnDeselectCatapult(wxActivateEvent& event);
	bool EditConfig(bool fatalIfFailed = false);

	std::unique_ptr<openMSXController> m_controller;

	wxButton* m_launch_AbortButton;

	wxStaticBitmap* m_powerLed;
	wxStaticBitmap* m_capsLed;
	wxStaticBitmap* m_kanaLed;
	wxStaticBitmap* m_pauseLed;
	wxStaticBitmap* m_turboLed;
	wxStaticBitmap* m_fddLed;
	wxMenu* settingsMenu;
	wxMenu* viewMenu;

	wxTimer m_fpsTimer;
	wxTimer m_focusTimer;
	wxTimer m_safetyTimer;
	wxWindow* m_currentFocus;
	wxString m_tempStatus;
	wxString m_settingsfile;

	DECLARE_CLASS(wxCatapultFrame)
	DECLARE_EVENT_TABLE()
};

#endif
