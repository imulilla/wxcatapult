#include "SessionPage.h"
#include "wxCatapultFrm.h"
#include "SessionPage.h"
#include "StatusPage.h"
#include "MiscControlPage.h"
#include "VideoControlPage.h"
#include "openMSXController.h"
#include "RomTypeDlg.h"
#include "IPSSelectionDlg.h"
#include "utils.h"
#include <algorithm>
#include <wx/wxprec.h>
#include <wx/xrc/xmlres.h>
#include <wx/button.h>
#include <wx/bmpbuttn.h>
#include <wx/clntdata.h>
#include <wx/combobox.h>
#include <wx/dcmemory.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/listbox.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/dnd.h>
#include <wx/tooltip.h>

#ifdef __WXMSW__
#ifndef __VISUALC__
#define _WIN32_IE 0x400	// to be able to use shell objects
#endif
#include <shlobj.h>
#endif

enum {
	// menu items
	Disk_Insert_New = 10,
	Disk_Browse_File,
	Disk_Browse_Dir,
	Disk_Browse_Ips,
	Disk_Eject,
	Cart_Browse_File,
	Cart_Eject,
	Cart_Select_Mapper,
	Cart_Browse_Ips,
	Cas_Browse_File,
	Cas_Eject,
	Cas_Rewind,
	Cas_MotorControl,
	Cas_AutoCreateFile
};

IMPLEMENT_CLASS(SessionPage, wxPanel)
BEGIN_EVENT_TABLE(SessionPage, wxPanel)
	EVT_COMBOBOX(XRCID("DiskAContents"), SessionPage::OnClickDiskACombo)
	EVT_COMBOBOX(XRCID("DiskBContents"), SessionPage::OnClickDiskBCombo)
	EVT_COMBOBOX(XRCID("CartAContents"), SessionPage::OnClickCartACombo)
	EVT_COMBOBOX(XRCID("CartBContents"), SessionPage::OnClickCartBCombo)
	EVT_COMBOBOX(XRCID("CassetteContents"), SessionPage::OnClickCassetteCombo)
	EVT_TOGGLEBUTTON(XRCID("PlayButton"), SessionPage::OnModePlay)
	EVT_TOGGLEBUTTON(XRCID("RecordButton"), SessionPage::OnModeRecord)
	EVT_BUTTON(XRCID("DiskA_Button"), SessionPage::OnClickMediaMenu)
	EVT_BUTTON(XRCID("DiskB_Button"), SessionPage::OnClickMediaMenu)
	EVT_BUTTON(XRCID("CartA_Button"), SessionPage::OnClickMediaMenu)
	EVT_BUTTON(XRCID("CartB_Button"), SessionPage::OnClickMediaMenu)
	EVT_BUTTON(XRCID("CassetteButton"), SessionPage::OnClickMediaMenu)
	EVT_BUTTON(XRCID("BrowseDiskA"), SessionPage::OnBrowseDiskA)
	EVT_BUTTON(XRCID("BrowseDiskB"), SessionPage::OnBrowseDiskB)
	EVT_BUTTON(XRCID("BrowseCartA"), SessionPage::OnBrowseCartA)
	EVT_BUTTON(XRCID("BrowseCartB"), SessionPage::OnBrowseCartB)
	EVT_BUTTON(XRCID("BrowseCassette"), SessionPage::OnBrowseCassette)
	EVT_BUTTON(XRCID("ClearDiskA"), SessionPage::OnEjectDiskA)
	EVT_BUTTON(XRCID("ClearDiskB"), SessionPage::OnEjectDiskB)
	EVT_BUTTON(XRCID("ClearCartA"), SessionPage::OnEjectCartA)
	EVT_BUTTON(XRCID("ClearCartB"), SessionPage::OnEjectCartB)
	EVT_BUTTON(XRCID("ClearCassette"), SessionPage::OnClearCassette)
	EVT_BUTTON(XRCID("RewindButton"), SessionPage::OnRewind)
	EVT_TEXT(XRCID("DiskAContents"), SessionPage::OnChangeDiskAContents)
	EVT_TEXT(XRCID("DiskBContents"), SessionPage::OnChangeDiskBContents)
	EVT_TEXT(XRCID("CartAContents"), SessionPage::OnChangeCartAContents)
	EVT_TEXT(XRCID("CartBContents"), SessionPage::OnChangeCartBContents)
	EVT_TEXT(XRCID("CassetteContents"), SessionPage::OnChangeCassetteContents)
	EVT_MENU(Disk_Insert_New, SessionPage::OnInsertEmptyDiskByMenu)
	EVT_MENU(Disk_Browse_File, SessionPage::OnBrowseDiskByMenu)
	EVT_MENU(Disk_Browse_Dir, SessionPage::OnBrowseDiskDirByMenu)
	EVT_MENU(Disk_Browse_Ips, SessionPage::OnBrowseDiskIps)
	EVT_MENU(Disk_Eject, SessionPage::OnEjectByMenu)
	EVT_MENU(Cart_Browse_File, SessionPage::OnBrowseCartByMenu)
	EVT_MENU(Cart_Eject, SessionPage::OnEjectByMenu)
	EVT_MENU(Cart_Select_Mapper, SessionPage::OnSelectMapper)
	EVT_MENU(Cart_Browse_Ips, SessionPage::OnSelectIPS)
	EVT_MENU(Cas_Browse_File, SessionPage::OnBrowseCassette)
	EVT_MENU(Cas_Eject, SessionPage::OnClearCassette)
	EVT_MENU(Cas_Rewind, SessionPage::OnRewind)
	EVT_MENU(Cas_MotorControl, SessionPage::OnMotorControl)
	EVT_MENU(Cas_AutoCreateFile, SessionPage::OnAutoCassettefile)
END_EVENT_TABLE()

SessionPage::SessionPage(wxWindow* parent, openMSXController& controller)
	: m_controller(controller)
{
	wxXmlResource::Get()->LoadPanel(this, parent, wxT("SessionPage"));

	for (int j = 0; j < 2; ++j) {
		m_diskMenu[j] = new wxMenu(wxT(""), 0);
//		m_diskMenu[j]->Append(Disk_Insert_New, wxT("Insert empty disk"), wxT(""), wxITEM_NORMAL);
		m_diskMenu[j]->Append(Disk_Browse_File, wxT("Browse for disk image"), wxT(""), wxITEM_NORMAL);
		m_diskMenu[j]->Append(Disk_Browse_Dir, wxT("Browse for disk folder (DirAsDisk)"), wxT(""), wxITEM_NORMAL);
		m_diskMenu[j]->Append(Disk_Eject, wxT("Eject disk"), wxT(""), wxITEM_NORMAL);
		m_diskMenu[j]->Append(Disk_Browse_Ips, wxT("Select IPS Patches (None selected)"), wxT(""), wxITEM_NORMAL);

		m_cartMenu[j] = new wxMenu(wxT(""), 0);
		m_cartMenu[j]->Append(Cart_Browse_File, wxT("Browse ROM image"), wxT(""), wxITEM_NORMAL);
		m_cartMenu[j]->Append(Cart_Eject, wxT("Eject ROM"), wxT(""), wxITEM_NORMAL);
		m_cartMenu[j]->Append(Cart_Select_Mapper, wxT("Select ROM type (AUTO)"), wxT(""), wxITEM_NORMAL);
		m_cartMenu[j]->Append(Cart_Browse_Ips, wxT("Select IPS Patches (None selected)"), wxT(""), wxITEM_NORMAL);
	}
	m_casMenu = new wxMenu(wxT(""), 0);
	m_casMenu->Append(Cas_Browse_File, wxT("Browse cassette image"), wxT(""), wxITEM_NORMAL);
	m_casMenu->Append(Cas_Eject, wxT("Eject cassette"), wxT(""), wxITEM_NORMAL);
	m_casMenu->Append(Cas_Rewind, wxT("Rewind cassette"), wxT(""), wxITEM_NORMAL);
	m_casMenu->Append(Cas_MotorControl, wxT("Motor control"), wxT(""), wxITEM_CHECK);
	m_casMenu->Append(Cas_AutoCreateFile, wxT("Auto create Cassette file for recording"), wxT(""), wxITEM_CHECK);

	m_extensionList  = (wxListBox*)FindWindowByName(wxT("ExtensionList"));
	m_machineList    = (wxComboBox*)FindWindowByName(wxT("MachineList"));
	m_browseCassette = (wxBitmapButton*)FindWindowByName(wxT("BrowseCassette"));
	m_clearCassette  = (wxBitmapButton*)FindWindowByName(wxT("ClearCassette"));
	m_playButton     = (wxToggleButton*)FindWindowByName(wxT("PlayButton"));
	m_recordButton   = (wxToggleButton*)FindWindowByName(wxT("RecordButton"));
	m_rewindButton   = (wxButton*)FindWindowByName(wxT("RewindButton"));
	m_diskAButton    = (wxButton*)FindWindowByName(wxT("DiskA_Button"));
	m_diskBButton    = (wxButton*)FindWindowByName(wxT("DiskB_Button"));
	m_cartAButton    = (wxButton*)FindWindowByName(wxT("CartA_Button"));
	m_cartBButton    = (wxButton*)FindWindowByName(wxT("CartB_Button"));
	m_cassetteButton = (wxButton*)FindWindowByName(wxT("CassetteButton"));
	m_machineListLabel   = (wxStaticText*)FindWindowByName(wxT("MachineListLabel"));
	m_extensionListLabel = (wxStaticText*)FindWindowByName(wxT("ExtensionLabel"));
	//SetupHardware(true, false); // No need to do this, it's done in wxCatapultFrm's constructor

	m_hasCassettePort = true; // avoid UMR
	m_cassetteControl = true; // see comments in OnMotorControl()
	m_romTypeDialog.reset(new RomTypeDlg(wxGetTopLevelParent(this)));
	GetRomTypes();
	m_ipsDialog.reset(new IPSSelectionDlg(wxGetTopLevelParent(this)));
	media[DISKA].reset(new MediaInfo(
		*m_diskMenu[0], wxT("diska"), wxT("DiskAContents"),
		ConfigurationData::MB_DISKA, ConfigurationData::CD_HISTDISKA,
		m_diskAButton, DISKETTE, Disk_Browse_Ips, 0));
	media[DISKB].reset(new MediaInfo(
		*m_diskMenu[1], wxT("diskb"), wxT("DiskBContents"),
		ConfigurationData::MB_DISKB, ConfigurationData::CD_HISTDISKB,
		m_diskBButton, DISKETTE, Disk_Browse_Ips, 0));
	media[CARTA].reset(new MediaInfo(
		*m_cartMenu[0], wxT("carta"), wxT("CartAContents"),
		ConfigurationData::MB_CARTA, ConfigurationData::CD_HISTCARTA,
		m_cartAButton, CARTRIDGE, Cart_Browse_Ips, Cart_Select_Mapper));
	media[CARTB].reset(new MediaInfo(
		*m_cartMenu[1], wxT("cartb"), wxT("CartBContents"),
		ConfigurationData::MB_CARTB, ConfigurationData::CD_HISTCARTB,
		m_cartBButton, CARTRIDGE, Cart_Browse_Ips, Cart_Select_Mapper));
	media[CAS].reset(new MediaInfo(
		*m_casMenu, wxT("cassetteplayer"), wxT("CassetteContents"),
		ConfigurationData::MB_CASSETTE, ConfigurationData::CD_HISTCASSETTE,
		m_cassetteButton, CASSETTE, 0, 0));
	for (auto& m : media) {
		m->control.SetDropTarget(new SessionDropTarget(&m->control));
	}

	int autorecord;
	ConfigurationData::instance().GetParameter(ConfigurationData::CD_AUTORECORD, &autorecord);
	m_cassetteAutoCreate = autorecord == 1;
	SetCassetteControl();
}

void SessionPage::FixLayout()
{
	// Needs to be called AFTER setuphardware!
	// Adjust the minimum size of the extension and listbox
	wxMemoryDC tempDC;
	tempDC.SetFont(m_machineList->GetFont());
	int dx, dy;
	m_machineList->GetSize(&dx, &dy); //default size
	int wMax = dx;
	int items = m_machineList->GetCount();
	for (int i = 0; i < items; ++i) {
		int w, h;
		tempDC.GetTextExtent(m_machineList->GetString(i), &w, &h);
		wMax = std::max(wMax, w);
	}
	m_machineList->SetSizeHints(wMax, -1);

	tempDC.SetFont(m_extensionList->GetFont());
	m_extensionList->GetSize(&dx, &dy); //default size
	wMax = dx;
	items = m_extensionList->GetCount();
	for (int i = 0; i < items; ++i) {
		int w, h;
		tempDC.GetTextExtent(m_extensionList->GetString(i) + wxT("W"), &w, &h); // no idea why we need the extra W...
		wMax = std::max(wMax, w);
	}
	wMax = std::min(wMax, 300); // just to have some limit
	m_extensionList->SetSizeHints(wMax + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X), -1);
}

void SessionPage::OnEjectDiskA(wxCommandEvent& event)
{
	EjectMedia(*media[DISKA]);
}
void SessionPage::OnEjectDiskB(wxCommandEvent& event)
{
	EjectMedia(*media[DISKB]);
}
void SessionPage::OnEjectCartA(wxCommandEvent& event)
{
	EjectMedia(*media[CARTA]);
}
void SessionPage::OnEjectCartB(wxCommandEvent& event)
{
	EjectMedia(*media[CARTB]);
}
void SessionPage::OnClearCassette(wxCommandEvent& event)
{
	EjectMedia(*media[CAS]);
}
void SessionPage::OnEjectByMenu(wxCommandEvent& event)
{
	if (auto* target = GetLastMenuTarget()) {
		EjectMedia(*target);
	}
}

void SessionPage::EjectMedia(MediaInfo& m)
{
	m.control.SetValue(wxT(""));
	m.control.SetSelection(wxNOT_FOUND);
	if (m.mediaType == CARTRIDGE) SetMapperType(m, wxT(""));
	insertMedia(m);
}

void SessionPage::OnRewind(wxCommandEvent& event)
{
	m_controller.WriteCommand(wxT("cassetteplayer rewind"));
}

void SessionPage::OnModePlay(wxCommandEvent& event)
{
	m_controller.WriteCommand(wxT("cassetteplayer play"));
}

void SessionPage::OnModeRecord(wxCommandEvent& event)
{
	wxString cmd = wxT("cassetteplayer new");
	if (!m_cassetteAutoCreate) {
		wxString path = wxT("Cassette files (*.wav)|*.wav|All files|*.*||");
		wxString defaultpath = ::wxPathOnly(media[CAS]->control.GetValue());
		wxFileDialog filedlg(this, wxT("Select Cassettefile to save to"),
		                     defaultpath, wxT(""), path,
		                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (filedlg.ShowModal() != wxID_OK) return;
		cmd += wxT(" ") + utils::ConvertPath(filedlg.GetPath());
	}
	m_controller.WriteCommand(cmd);
}

void SessionPage::OnMotorControl(wxCommandEvent& event)
{
	// TODO m_cassetteControl should be queried from openmsx
	//    -> before this patch it was not even initialized
	//    -> if this is changed in openMSX itself catapult gets out-of-sync
	// Would it be a good idea to just drop this function from catapult? Is
	// this really something that's useful for typical catapult users?
	m_cassetteControl = !m_cassetteControl;
	if (m_cassetteControl) {
		m_controller.WriteCommand(wxT("cassetteplayer motorcontrol on"));
	} else {
		m_controller.WriteCommand(wxT("cassetteplayer motorcontrol off"));
	}
}

void SessionPage::OnAutoCassettefile(wxCommandEvent& event)
{
	m_cassetteAutoCreate = !m_cassetteAutoCreate;
	auto& config = ConfigurationData::instance();
	config.SetParameter(ConfigurationData::CD_AUTORECORD, (long)(m_cassetteAutoCreate ? 1 : 0));
	config.SaveData();
}

void SessionPage::OnBrowseDiskA(wxCommandEvent& event)
{
	BrowseDisk(*media[DISKA]);
}
void SessionPage::OnBrowseDiskB(wxCommandEvent& event)
{
	BrowseDisk(*media[DISKB]);
}
void SessionPage::OnBrowseCartA(wxCommandEvent& event)
{
	BrowseCart(*media[CARTA]);
}
void SessionPage::OnBrowseCartB(wxCommandEvent& event)
{
	BrowseCart(*media[CARTB]);
}

void SessionPage::BrowseDisk(MediaInfo& m)
{
	BrowseMedia(m,
		wxT("All known disk files|*.dsk;*.DSK;*.xsa;*.XSA;*.dmk;*.DMK;*.zip;*.ZIP;*.gz;*.GZ;*.di1;*.DI1;*.di2;*.DI2|Uncompressed disk files|*.dsk;*.DSK;*.xsa;*.XSA;*.di1;*.DI1;*.di2;*.DI2|Compressed files (*.zip;*.gz)|*.gz;*.GZ;*.zip;*.ZIP|All files|*.*||"),
		wxT("Select disk image"));
}

void SessionPage::BrowseCart(MediaInfo& m)
{
	BrowseMedia(m,
		wxT("All known cartridge files|*.rom;*.ROM;*.ri;*.RI;*.zip;*.ZIP;*.gz;*.GZ|Uncompressed cartridge files|*.rom;*.ROM;*.ri;*.RI|Compressed files (*.zip;*.gz)|*.gz;*.GZ;*.zip;*.ZIP|All files|*.*||"),
		wxT("Select ROM image"));
}

void SessionPage::OnBrowseCassette(wxCommandEvent& event)
{
	BrowseMedia(*media[CAS],
		wxT("All known cassette files|*.zip;*.ZIP;*.gz;*.GZ;*.wav;*.WAV;*.cas;*.CAS|Uncompressed cassette files|*.wav;*.WAV;*.cas;*.CAS|Compressed files (*.zip;*.gz)|*.gz;*.GZ;*.zip;*.ZIP|All files|*.*||"),
		wxT("Select cassette image"));
}

void SessionPage::BrowseMedia(MediaInfo& m, const wxString& path, const wxString title)
{
	wxString defaultpath = ::wxPathOnly(m.control.GetValue());
	wxFileDialog filedlg(this, title, defaultpath, wxT(""), path, wxFD_OPEN);
	if (filedlg.ShowModal() == wxID_OK) {
		m.control.SetValue(filedlg.GetPath());
		if (m.mediaType == CARTRIDGE) SetMapperType(m, wxT(""));
		insertMedia(m);
	}
}

void SessionPage::OnClickDiskACombo(wxCommandEvent& event)
{
	ClickMediaCombo(event, *media[DISKA]);
}
void SessionPage::OnClickDiskBCombo(wxCommandEvent& event)
{
	ClickMediaCombo(event, *media[DISKB]);
}
void SessionPage::OnClickCartACombo(wxCommandEvent& event)
{
	ClickMediaCombo(event, *media[CARTA]);
}
void SessionPage::OnClickCartBCombo(wxCommandEvent& event)
{
	ClickMediaCombo(event, *media[CARTB]);
}
void SessionPage::OnClickCassetteCombo(wxCommandEvent& event)
{
	ClickMediaCombo(event, *media[CAS]);
}
void SessionPage::ClickMediaCombo(wxCommandEvent& event, MediaInfo& m)
{
	OnClickCombo(event);
	if (m.mediaType == CARTRIDGE) {
		auto* c = m.control.GetClientObject(event.GetInt());
		SetMapperType(m, static_cast<wxStringClientData*>(c)->GetData());
	}
	insertMedia(m);
}

void SessionPage::OnChangeDiskAContents(wxCommandEvent& event)
{
	ChangeMediaContents(*media[DISKA]);
}
void SessionPage::OnChangeDiskBContents(wxCommandEvent& event)
{
	ChangeMediaContents(*media[DISKB]);
}
void SessionPage::OnChangeCartAContents(wxCommandEvent& event)
{
	ChangeMediaContents(*media[CARTA]);
}
void SessionPage::OnChangeCartBContents(wxCommandEvent& event)
{
	ChangeMediaContents(*media[CARTB]);
}
void SessionPage::OnChangeCassetteContents(wxCommandEvent& event)
{
	ChangeMediaContents(*media[CAS]);
}
void SessionPage::ChangeMediaContents(MediaInfo& m)
{
	if (m.ipsLabel) {
		m.ips.Clear();
		m.menu.SetLabel(Disk_Browse_Ips, wxT("Select IPS Patches (None selected)"));
	}
	if (m.mediaType == CARTRIDGE) SetMapperType(m, wxT(""));
	if (m.mediaType == CASSETTE) SetCassetteMode(wxT("play"));
}

void SessionPage::SetMapperType(MediaInfo& m, const wxString& type)
{
	m.type = type;
	m.menu.SetLabel(m.typeLabel,
		(!type.IsEmpty() && (type.Upper() != wxT("AUTO")))
		? wxT("Select cartridge type (") + type + wxT(")")
		: wxT("Select cartridge type (AUTO)"));
}

static int CompareCaseInsensitive(const wxString& first, const wxString& second)
{
	int result = first.CmpNoCase(second);
	if (result != 0) return result;
	return first.Cmp(second);
}

void SessionPage::SetupHardware(bool initial, bool reset)
{
	wxString currentMachine;
	wxArrayString currentExtensions;
	if (!initial) {
		wxArrayInt sel;
		currentMachine = m_machineList->GetValue();
		currentExtensions.Clear();
		if (m_extensionList->GetSelections(sel) > 0) {
			for (unsigned i = 0; i < sel.GetCount(); ++i) {
				currentExtensions.Add(m_extensionList->GetString(sel[i]));
			}
		}
	}
	auto& config = ConfigurationData::instance();
	m_machineArray.Clear();
	m_extensionArray.Clear();
	m_machineList->Clear();
	m_machineList->Append(wxT(" <default> "));
	m_extensionList->Clear();
	wxString checkedMachines;
	wxString checkedExtensions;
	config.GetParameter(ConfigurationData::CD_MACHINES, checkedMachines);
	config.GetParameter(ConfigurationData::CD_EXTENSIONS, checkedExtensions);
	if (!checkedMachines.IsEmpty() && !checkedExtensions.IsEmpty() && !reset) {
		while (true) {
			int pos = checkedMachines.Find(wxT("::"));
			if (pos == wxNOT_FOUND) break;
			m_machineArray.Add(checkedMachines.Left(pos));
			checkedMachines = checkedMachines.Mid(pos + 2);
		}
		while (true) {
			int pos = checkedExtensions.Find(wxT("::"));
			if (pos == wxNOT_FOUND) break;
			m_extensionArray.Add(checkedExtensions.Left(pos));
			checkedExtensions = checkedExtensions.Mid(pos + 2);
		}
	} else {
		wxString sharepath;
		config.GetParameter(ConfigurationData::CD_SHAREPATH, sharepath);
		prepareExtensions(sharepath, m_extensionArray);
		prepareMachines(sharepath, m_machineArray);
		wxString personalShare;
#ifdef __UNIX__
		personalShare = ::wxGetHomeDir() + wxT("/.openMSX/share");
#else
		TCHAR temp[MAX_PATH + 1];
		SHGetSpecialFolderPath(0, temp, CSIDL_PERSONAL, 1);
		personalShare = wxString((const wxChar*)temp) + wxT("/openMSX/share");
#endif
		prepareExtensions(personalShare, m_extensionArray, true);
		prepareMachines(personalShare, m_machineArray, true);
	}
	m_extensionArray.Sort(CompareCaseInsensitive);
	m_machineArray.Sort(CompareCaseInsensitive);
	fillExtensions(m_extensionArray);
	fillMachines(m_machineArray);
	if (!initial) {
		int sel = m_machineList->FindString(currentMachine);
		if (sel != wxNOT_FOUND) {
			m_machineList->SetSelection(sel);
		} else {
			m_machineList->SetSelection(0);
		}
		for (unsigned i = 0; i < currentExtensions.GetCount(); ++i) {
			sel = m_extensionList->FindString(currentExtensions[i]);
			if (sel != wxNOT_FOUND) {
				m_extensionList->SetSelection(sel);
			}
		}
	}
}

void SessionPage::prepareExtensions(const wxString& sharepath, wxArrayString& extensionArray, bool optional)
{
	wxString extPath = sharepath + wxT("/extensions");
	if (!::wxDirExists(extPath)) {
		if (!optional) {
			wxMessageBox(wxT("Directory: ") + extPath +
			             wxT(" does not exist"));
		}
		return;
	}
	wxDir sharedir(extPath);
	wxString extension;
	bool succes = sharedir.GetFirst(&extension);
	while (succes) {
		wxString fullPath = extPath + wxT("/") + extension;
		wxString base;
		if (extension.EndsWith(wxT(".xml"), &base)) {
			if (::wxFileExists(fullPath)) {
				if (extensionArray.Index(base, true) == wxNOT_FOUND) {
					extensionArray.Add(base);
				}
			}
		} else if (::wxFileExists(fullPath + wxT("/hardwareconfig.xml"))) {
			if (extensionArray.Index(extension, true) == wxNOT_FOUND) {
				extensionArray.Add(extension);
			}
		}
		succes = sharedir.GetNext(&extension);
	}
}

void SessionPage::fillExtensions(const wxArrayString& extensionArray)
{
	for (auto tmp : extensionArray) {
		tmp.Replace(wxT("_"), wxT(" "));
		m_extensionList->Append(tmp);
	}
}

void SessionPage::prepareMachines(
	const wxString& sharepath, wxArrayString& machineArray, bool optional)
{
	wxString cmd;
	ConfigurationData::instance().GetParameter(ConfigurationData::CD_EXECPATH, cmd);
	wxString machPath = sharepath + wxT("/machines");
	if (!::wxDirExists(machPath)) {
		if (!optional) {
			wxMessageBox(wxT("Directory: ") + machPath +
			             wxT(" does not exist"));
		}
		return;
	}
	wxDir sharedir(machPath);
	wxString machine;
	bool succes = sharedir.GetFirst(&machine);
	while (succes) {
		wxString fullPath = machPath + wxT("/") + machine;
		wxString base;
		if (machine.EndsWith(wxT(".xml"), &base)) {
			if (::wxFileExists(fullPath)) {
				if (machineArray.Index(base, true) == wxNOT_FOUND) {
					machineArray.Add(base);
				}
			}
		} else if (::wxFileExists(fullPath + wxT("/hardwareconfig.xml"))) {
			if (machineArray.Index(machine, true) == wxNOT_FOUND) {
				machineArray.Add(machine);
			}
		}
		succes = sharedir.GetNext(&machine);
	}
	m_machineList->SetSelection(0);
}

void SessionPage::fillMachines(const wxArrayString& machineArray)
{
	for (auto tmp : machineArray) {
		tmp.Replace(wxT("_"), wxT(" "));
		m_machineList->Append(tmp);
	}
}

void SessionPage::HandleFocusChange(wxWindow* oldFocus, wxWindow* newFocus)
{
	checkLooseFocus(oldFocus, *media[DISKA]);
	checkLooseFocus(oldFocus, *media[DISKB]);
	checkLooseFocus(oldFocus, *media[CARTA]);
	checkLooseFocus(oldFocus, *media[CARTB]);
	checkLooseFocus(oldFocus, *media[CAS]);
}
void SessionPage::checkLooseFocus(wxWindow* oldFocus, MediaInfo& m)
{
	if (oldFocus != &m.control) return;
	if (m.control.GetValue() == m.lastContents) return;
	if (m.mediaType == CARTRIDGE) SetMapperType(m, wxT(""));
	insertMedia(m);
}

void SessionPage::insertMedia(MediaInfo& m)
{
	if (m.ipsLabel) {
		m.ips.Clear();
		m.menu.SetLabel(m.ipsLabel, wxT("Select IPS Patches (None selected)"));
	}
	wxString contents = m.control.GetValue();
	m.lastContents = contents;
	wxString cmd = m.deviceName + wxT(" ");
	if (!contents.IsEmpty()) {
		cmd += utils::ConvertPath(contents);
		if (!m.type.IsEmpty()) {
			assert(m.mediaType == CARTRIDGE);
			cmd += wxT(" -romtype ") + m.type;
		}
		AddHistory(m);
	} else {
		cmd += wxT("eject");
	}
	m_controller.WriteCommand(cmd);
}

void SessionPage::SetControlsOnLaunch()
{
	m_machineList->Enable(false);
	m_extensionList->Enable(false);
	m_extensionListLabel->Enable(false);
	m_machineListLabel->Enable(false);
}

void SessionPage::SetControlsOnEnd()
{
	m_machineList->Enable(true);
	m_extensionList->Enable(true);
	m_extensionListLabel->Enable(true);
	m_machineListLabel->Enable(true);
	m_playButton->SetValue(false);
	m_recordButton->SetValue(false);
	if (auto* temp = FindWindowByLabel(wxT("Cartridge Slots"))) {
		temp->Enable(true);
	}
	SetCassetteControl();
}

// This method should be called whenever one of these conditions changes:
//  - m_controller.IsOpenMSXRunning()
//  - m_hasCassettePort
//  - media[CAS]->contents.IsEmpty()
//  - play/record button changes state
void SessionPage::SetCassetteControl()
{
	bool enable1 = m_controller.IsOpenMSXRunning() && m_hasCassettePort;
	m_recordButton->Enable(enable1 && !m_recordButton->GetValue());

	bool enable2 = enable1 && !media[CAS]->control.GetValue().IsEmpty();
	m_playButton  ->Enable(enable2 && !m_playButton->GetValue());
	m_rewindButton->Enable(enable2);

	bool enable3 = !m_controller.IsOpenMSXRunning() || m_hasCassettePort;
	m_cassetteButton  ->Enable(enable3);
	m_clearCassette   ->Enable(enable3);
	m_browseCassette  ->Enable(enable3);
	media[CAS]->control.Enable(enable3);
	if (auto* temp = FindWindowByLabel(wxT("Cassette Player"))) {
		temp->Enable(enable3);
	}
}

wxArrayString SessionPage::getMedia() const
{
	wxArrayString result;
	for (auto& m : media) {
		result.Add(m->control.GetValue());
	}
	return result;
}

wxArrayString SessionPage::getTypes() const
{
	wxArrayString result;
	for (auto& m : media) {
		result.Add(m->type);
	}
	return result;
}

void SessionPage::getPatches(wxArrayString* parameters) const
{
	unsigned i = 0;
	for (auto& m : media) {
		parameters[i++] = m->ips;
	}
}

wxArrayString SessionPage::getHardware() const
{
	wxArrayString result;
	int pos = m_machineList->GetSelection();
	if (pos == 0) {
		result.Add(wxT(" <default> "));
	} else {
		result.Add(utils::ConvertPathNoSlash(m_machineArray[pos - 1]));
	}

	wxArrayInt sel;
	if (m_extensionList->GetSelections(sel) > 0) {
		for (unsigned i = 0; i < sel.GetCount(); ++i) {
			result.Add(m_extensionArray[sel[i]]);
		}
	}
	return result;
}

void SessionPage::UpdateSessionData()
{
	m_InsertedMedia = 0;
	for (auto& m : media) {
		m->lastContents = m->control.GetValue();
		if (!m->lastContents.IsEmpty()) {
			AddHistory(*m);
			m_InsertedMedia |= m->mediaBits;
		}
	}

	wxArrayString hardware = getHardware();
	m_usedMachine = hardware[0];
	m_usedExtensions.Clear();
	for (size_t i = 1; i < hardware.GetCount(); ++i) {
		m_usedExtensions << hardware[i] << wxT("::");
	}
}

void SessionPage::AddHistory(MediaInfo& m)
{
	wxString currentItem = m.control.GetValue();
#ifdef __WXMSW__
	currentItem.Replace(wxT("/"), wxT("\\"));
#else
	currentItem.Replace(wxT("\\"), wxT("/"));
#endif

	int pos = m.control.FindString(currentItem);
	if (pos != wxNOT_FOUND) {
		m.control.Delete(pos);
	}
	auto* c = (m.mediaType == CARTRIDGE) ? new wxStringClientData(m.type) : 0;
	m.control.Insert(currentItem, 0, c);
	while (m.control.GetCount() > HISTORY_SIZE) {
		m.control.Delete(HISTORY_SIZE);
	}
	m.control.SetSelection(0);

	SaveHistory();
}

void SessionPage::RestoreHistory()
{
	ConfigurationData::ID typeID[] = {
		ConfigurationData::CD_TYPEHISTCARTA,
		ConfigurationData::CD_TYPEHISTCARTB
	};
	auto& config = ConfigurationData::instance();
	config.GetParameter(ConfigurationData::CD_MEDIAINSERTED, &m_InsertedMedia);
	int hist = -1;
	for (auto& m : media) {
		m->control.Clear();
		wxString value;
		config.GetParameter(m->confId, value);
		while (true) {
			int pos = value.Find(wxT("::"));
			if (pos == wxNOT_FOUND) break;
			m->control.Append(value.Left(pos));
			value = value.Mid(pos + 2);
		}
		if (m->mediaType == CARTRIDGE) {
			++hist;
			wxString types;
			config.GetParameter(typeID[hist], types);
			for (unsigned i = 0; i < m->control.GetCount(); ++i) {
				int pos = types.Find(wxT("::"));
				wxString s;
				if (pos == wxNOT_FOUND) {
					s = wxT("auto");
				} else {
					s = types.Left(pos);
					types = types.Mid(pos + 2);
				}
				m->control.SetClientObject(i, new wxStringClientData(s));
			}
		}
		if ((m_InsertedMedia & m->mediaBits) && !m->control.IsEmpty()) {
			m->control.SetSelection(0);
			if (m->mediaType == CARTRIDGE) {
				auto* c = m->control.GetClientObject(0);
				SetMapperType(*m, static_cast<wxStringClientData*>(c)->GetData());
			}
		} else {
			m->control.SetValue(wxT(""));
		}
	}
	config.GetParameter(ConfigurationData::CD_USEDMACHINE, m_usedMachine);
	// printf("Last used machine: %s....", (const char*)(wxConvUTF8.cWX2MB(m_usedMachine)));
	if (!m_usedMachine.IsEmpty()) {
		// printf ("OK, it's not empty\n");
		wxString temp = m_usedMachine;
		temp.Replace(wxT("_"), wxT(" "));
		temp.Replace(wxT("\""), wxT(""));
		int pos = m_machineList->FindString(temp);
		if (pos != wxNOT_FOUND) {
			m_machineList->SetSelection(pos);
		} else {
			m_machineList->SetSelection(0);
			// printf(" Can't find the machine\n");
		}
	}
	wxString value;
	config.GetParameter(ConfigurationData::CD_USEDEXTENSIONS, value);
	m_usedExtensions = value;
	while (true) {
		int pos = value.Find(wxT("::"));
		if (pos == wxNOT_FOUND) break;
		wxString temp = value.Left(pos);
		temp.Replace(wxT("_"), wxT(" "));
		if (m_extensionList->FindString(temp) != wxNOT_FOUND) {
			m_extensionList->SetStringSelection(temp);
		}
		value = value.Mid(pos + 2);
	}
}

void SessionPage::SaveHistory()
{
	ConfigurationData::ID typeID[] = {
		ConfigurationData::CD_TYPEHISTCARTA,
		ConfigurationData::CD_TYPEHISTCARTB
	};
	auto& config = ConfigurationData::instance();
	int hist = -1;
	for (auto& m : media) {
		wxString temp, temp2;
		for (unsigned j = 0; j < m->control.GetCount(); ++j) {
			temp << m->control.GetString(j) << wxT("::");
			if (m->mediaType == CARTRIDGE) {
				auto* c = m->control.GetClientObject(j);
				auto s = static_cast<wxStringClientData*>(c)->GetData();
				temp2 << (s.IsEmpty() ? wxT("auto") : s) << wxT("::");
			}
		}
		config.SetParameter(m->confId, temp);
		if (m->mediaType == CARTRIDGE) {
			++hist;
			config.SetParameter(typeID[hist], temp2);
		}
	}
	config.SetParameter(ConfigurationData::CD_MEDIAINSERTED, (long)m_InsertedMedia);
	config.SetParameter(ConfigurationData::CD_USEDMACHINE, m_usedMachine);
	config.SetParameter(ConfigurationData::CD_USEDEXTENSIONS, m_usedExtensions);
	if (!config.SaveData()) {
		wxMessageBox(wxT("Error saving configuration data"));
	}
}

void SessionPage::EnableCassettePort(wxString data)
{
	// result of   catch {cassetteplayer}
	//   0 -> cassetteplayer command exists,         enable  cassette stuff
	//   1 -> cassetteplayer command does not exist, disbale cassette stuff
	m_hasCassettePort = data == wxT("0");
	SetCassetteControl();
}

void SessionPage::SetCassetteMode(wxString data)
{
	m_playButton->SetValue(!media[CAS]->control.GetValue().IsEmpty() &&
	                       (data == wxT("play")));
	m_recordButton->SetValue(data == wxT("record"));
	SetCassetteControl();
}

SessionPage::MediaInfo* SessionPage::GetLastMenuTarget() const
{
	for (auto& m : media) {
		if (m_lastUsedPopup == m->button) return m.get();
	}
	return nullptr;
}

void SessionPage::OnClickMediaMenu(wxCommandEvent& event)
{
	m_lastUsedPopup = (wxButton*)event.GetEventObject();
	auto* m = GetLastMenuTarget(); assert(m);
	bool enable = !m->control.GetValue().IsEmpty();
	if (m->ipsLabel)  m->menu.Enable(m->ipsLabel,  enable);
	if (m->typeLabel) m->menu.Enable(m->typeLabel, enable);
	if (m->mediaType == CASSETTE) {
		m->menu.Enable(Cas_Rewind, m_rewindButton->IsEnabled());
		bool enable = m_controller.IsOpenMSXRunning() && m_hasCassettePort;
		m->menu.Enable(Cas_MotorControl, enable);
		m->menu.Check(Cas_MotorControl, m_cassetteControl);
		m->menu.Check(Cas_AutoCreateFile, m_cassetteAutoCreate);
	}
	wxRect myRect = m_lastUsedPopup->GetRect();
	PopupMenu(&m->menu, myRect.GetLeft(), myRect.GetBottom());
}

void SessionPage::OnInsertEmptyDiskByMenu(wxCommandEvent& event)
{
	if (auto* target = GetLastMenuTarget()) {
		wxString defaultpath = ::wxPathOnly(target->control.GetValue());
		wxFileDialog filedlg(this, wxT("Create disk image"),
		                     defaultpath, wxT(""), wxT("*.*"),
		                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (filedlg.ShowModal() == wxID_OK) {
			target->control.SetValue(filedlg.GetPath());
			insertMedia(*target);
		}
	}
}

void SessionPage::OnBrowseCartByMenu(wxCommandEvent& event)
{
	if (m_lastUsedPopup == m_cartAButton) {
		OnBrowseCartA(event);
	} else if (m_lastUsedPopup == m_cartBButton) {
		OnBrowseCartB(event);
	}
}

void SessionPage::OnBrowseDiskByMenu(wxCommandEvent& event)
{
	if (m_lastUsedPopup == m_diskAButton) {
		OnBrowseDiskA(event);
	} else if (m_lastUsedPopup == m_diskBButton) {
		OnBrowseDiskB(event);
	}
}

void SessionPage::OnBrowseDiskDirByMenu(wxCommandEvent& event)
{
	if (auto* target = GetLastMenuTarget()) {
		wxString defaultpath = ::wxPathOnly(target->control.GetValue());
		wxDirDialog dirdlg(this, wxT("Select Directory to use as disk"), defaultpath);
		if (dirdlg.ShowModal() == wxID_OK) {
			target->control.SetValue(dirdlg.GetPath());
		}
	}
}

void SessionPage::OnSelectMapper(wxCommandEvent& event)
{
	if (auto* target = GetLastMenuTarget()) {
		m_romTypeDialog->CenterOnParent();
		if (m_romTypeDialog->ShowModal(target->type) == wxID_OK) {
			SetMapperType(*target, m_romTypeDialog->GetSelectedType());
			insertMedia(*target); // TODO this shouldn't clear IPS list
		}
	}
}

void SessionPage::OnSelectIPS(wxCommandEvent& event)
{
	if (auto* target = GetLastMenuTarget()) {
		m_ipsDialog->CenterOnParent();
		if (m_ipsDialog->ShowModal(target->ips, target->ipsdir) == wxID_OK) {
			target->ips = m_ipsDialog->GetIPSList();
			int count = target->ips.GetCount();
			wxString item = (count > 0)
				? wxString::Format(wxT("Select IPS Patches (%d selected)"), count)
				: wxString(wxT("Select IPS Patches (None selected)"));

			target->menu.SetLabel(Cart_Browse_Ips, item);
			target->ipsdir = m_ipsDialog->GetLastBrowseLocation();
		}
	}
}

void SessionPage::OnBrowseDiskIps(wxCommandEvent& event)
{
	if (auto* target = GetLastMenuTarget()) {
		m_ipsDialog->CenterOnParent();
		if (m_ipsDialog->ShowModal(target->ips, target->ipsdir) == wxID_OK) {
			target->ips = m_ipsDialog->GetIPSList();
			int count = target->ips.GetCount();
			wxString item = (count > 0)
				? wxString::Format(wxT("Select IPS Patches (%d selected)"), count)
				: wxString(wxT("Select IPS Patches (None selected)"));
			target->menu.SetLabel(Disk_Browse_Ips, item);
			wxString devicename;
			if (m_lastUsedPopup == m_diskAButton) {
				devicename = wxT("diska ");
			} else if (m_lastUsedPopup == m_diskBButton){
				devicename = wxT("diskb ");
			}
			wxString command = devicename + wxT(" ") + utils::ConvertPath(target->control.GetValue());
			for (unsigned i = 0; i < target->ips.GetCount(); ++i) {
				command += wxT(" ") + utils::ConvertPath(target->ips[i]);
			}
			m_controller.WriteCommand(command);
			target->ipsdir = m_ipsDialog->GetLastBrowseLocation();
		}
	}
}

void SessionPage::GetRomTypes()
{
	// TODO get it from openMSX
	// NOTE: this list is incomplete! REALLY GET IT FROM OPENMSX!
	SetupRomType(wxT(""), wxT("Autodetect type"));
	SetupRomType(wxT("AUTO"), wxT("Autodetect type"));
	SetupRomType(wxT("Page0"), wxT("Plain 16 kB Page 0"));
	SetupRomType(wxT("Page1"), wxT("Plain 16 kB Page 1"));
	SetupRomType(wxT("Page2"), wxT("Plain 16 kB Page 2 (Basic)"));
	SetupRomType(wxT("Normal8000"), wxT("Plain 16 kB Page 2 (Basic)"));
	SetupRomType(wxT("Page3"), wxT("Plain 16 kB Page 3"));
	SetupRomType(wxT("Page01"), wxT("Plain 32 kB Page 0-1"));
	SetupRomType(wxT("Page12"), wxT("Plain 32 kB Page 1-2"));
	SetupRomType(wxT("Page23"), wxT("Plain 32 kB Page 2-3"));
	SetupRomType(wxT("Page012"), wxT("Plain 48 kB Page 0-2"));
	SetupRomType(wxT("Page123"), wxT("Plain 48 kB Page 1-3"));
	SetupRomType(wxT("Page0123"), wxT("Plain 64 kB"));
	SetupRomType(wxT("Mirrored"), wxT("Plain (Any size)"));
	SetupRomType(wxT("8kB"), wxT("Generic 8kB"));
	SetupRomType(wxT("MSXDOS2"), wxT("MSX-DOS 2"));
	SetupRomType(wxT("Konami"), wxT("Konami without SCC"));
	SetupRomType(wxT("KonamiSCC"), wxT("Konami with SCC"));
	SetupRomType(wxT("ASCII8"), wxT("Ascii 8kB"));
	SetupRomType(wxT("ASCII16"), wxT("Ascii 16kB"));
	SetupRomType(wxT("Padial8"), wxT("Padial 8kB"));
	SetupRomType(wxT("Padial16"), wxT("Padial 16kB"));
	SetupRomType(wxT("R-Type"), wxT("R-Type"));
	SetupRomType(wxT("CrossBlaim"), wxT("Cross Blaim"));
	SetupRomType(wxT("MSX-AUDIO"), wxT("MSX-Audio"));
	SetupRomType(wxT("HarryFox"), wxT("Harry Fox"));
	SetupRomType(wxT("Halnote"), wxT("Halnote"));
	SetupRomType(wxT("Playball"), wxT("Playball"));
	SetupRomType(wxT("Zemina80in1"), wxT("Zemina Multigame (80 in 1)"));
	SetupRomType(wxT("Zemina90in1"), wxT("Zemina Multigame (90 in 1)"));
	SetupRomType(wxT("Zemina126in1"), wxT("Zemina Multigame (126 in 1)"));
	SetupRomType(wxT("HolyQuran"), wxT("Holy Qu'ran"));
	SetupRomType(wxT("ASCII16SRAM2"), wxT("Ascii 16kB with SRAM"));
	SetupRomType(wxT("ASCII8SRAM8"), wxT("Ascii 8kB with SRAM"));
	SetupRomType(wxT("KoeiSRAM8"), wxT("Koei with 8kB SRAM"));
	SetupRomType(wxT("KoeiSRAM32"), wxT("Koei with 32kB SRAM"));
	SetupRomType(wxT("Wizardry"), wxT("Wizardry"));
	SetupRomType(wxT("GameMaster2"), wxT("Konami Game Master 2"));
	SetupRomType(wxT("Synthesizer"), wxT("Konami Synthesizer"));
	SetupRomType(wxT("Majutsushi"), wxT("Hai no Majutsushi"));
	SetupRomType(wxT("KeyboardMaster"), wxT("Konami Keyboard Master"));
}

void SessionPage::SetupRomType(wxString type, wxString fullname)
{
	m_romTypeDialog->AddRomType(type);
	m_romTypeDialog->SetRomTypeFullName(type, fullname);
}

const wxArrayString& SessionPage::GetDetectedMachines() const
{
	return m_machineArray;
}
const wxArrayString& SessionPage::GetDetectedExtensions() const
{
	return m_extensionArray;
}

SessionDropTarget::SessionDropTarget(wxComboBox* target)
{
	m_target = target;
}

bool SessionDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
	if (!filenames.IsEmpty()) {
		m_target->Append(filenames[0]); // just the first for starters
	}
	return true;
}
