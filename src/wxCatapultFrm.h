// $Id$
#ifndef wxCatapultFrame_H
#define wxCatapultFrame_H

#include <wx/tglbtn.h>
#include <wx/slider.h>
#include <wx/notebook.h>

class CatapultXMLParser;
class SessionPage;
class StatusPage;
class VideoControlPage;
class MiscControlPage;
class openMSXController;

// Define a new frame type: this is going to be our main frame
class wxCatapultFrame : public wxFrame
{
	public:
		void OnControllerEvent (wxCommandEvent & event);
		void OnApplyChanges (wxCommandEvent & event);

		// ctor(s)
		wxCatapultFrame(wxWindow * parent=(wxWindow *)NULL);
		virtual ~wxCatapultFrame();

		// event handlers (these functions should _not_ be virtual)
		void OnMenuQuit(wxCommandEvent& event);
		void OnMenuAbout(wxCommandEvent& event);
		void OnMenuEditConfig(wxCommandEvent& event);
		void OnLaunch(wxCommandEvent& event);
		void DisableControls();

		SessionPage * m_sessionPage;
		StatusPage * m_statusPage;
		VideoControlPage * m_videoControlPage;
		MiscControlPage * m_miscControlPage;

		wxString m_lastDiskA;
		wxString m_lastDiskB;
		wxString m_lastTape1;
		wxString m_lastTape2;
		wxButton * m_applyButton;
		openMSXController * m_controller;
		wxNotebook * m_tabControl;
		wxButton * m_launch_AbortButton;
	private:
		enum QueryState {QIDLE, QSOUND, QRENDERER, QSCALER};

		void SetLaunchDefaults();
		wxString ConvertPath (wxString path, bool ConvertSlash=false);

		void EnableControls();
		CatapultXMLParser * m_parser;

		int m_InsertedMedia;
		wxString m_usedMachine;
		wxString m_usedExtensions;
		void SaveHistory();
		void RestoreHistory();
		void AddHistory (wxComboBox * media);

		bool WriteMessage (wxString msg);

		wxPanel * m_audioPanel;
		QueryState m_lastQuery;
		wxArrayString m_audioChannels;

		void WriteQuery (wxString query, QueryState type);

		DECLARE_CLASS(wxCatapultFrame)
			// any class wishing to process wxWindows events must use this macro
			DECLARE_EVENT_TABLE()
};

#endif // wxCatapultFrame_H
