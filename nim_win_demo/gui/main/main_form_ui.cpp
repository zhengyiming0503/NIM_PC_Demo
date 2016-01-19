#include "resource.h"
#include "main_form.h"
#include "gui/about/about_form.h"
#include "gui/msglogmanage/msglog_manage_form.h"
#include "gui/invoke_chat_form/invoke_chat_form.h"
#include "util/user.h"
#include "callback/team/team_callback.h"


using namespace ui;

const LPCTSTR MainForm::kClassName	= L"MainForm";

MainForm::MainForm()
{
	is_trayicon_left_double_clicked_ = false;

	OnUserInfoChangeCallback cb1 = nbase::Bind(&MainForm::OnUserInfoChange, this, std::placeholders::_1);
	unregister_cb.Add(nim_ui::UserManager::GetInstance()->RegUserInfoChange(cb1));

	OnUserPhotoReadyCallback cb2 = nbase::Bind(&MainForm::OnUserPhotoReady, this, std::placeholders::_1, std::placeholders::_2);
	unregister_cb.Add(nim_ui::UserManager::GetInstance()->RegUserPhotoReady(cb2));
}

MainForm::~MainForm()
{

}

std::wstring MainForm::GetSkinFolder()
{
	return L"main";
}

std::wstring MainForm::GetSkinFile()
{
	return L"main.xml";
}

ui::UILIB_RESOURCETYPE MainForm::GetResourceType() const
{
	return ui::UILIB_FILE;
}

std::wstring MainForm::GetZIPFileName() const
{
	return L"main.zip";
}

std::wstring MainForm::GetWindowClassName() const 
{
	return kClassName;
}

std::wstring MainForm::GetWindowId() const 
{
	return kClassName;
}

UINT MainForm::GetClassStyle() const 
{
	return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
}

void MainForm::OnEsc( BOOL &bHandled )
{
	bHandled = TRUE;
}

void MainForm::OnFinalMessage(HWND hWnd)
{
	TrayIcon::GetInstance()->Destroy();
	nim_ui::SessionListManager::GetInstance()->AttachListBox(nullptr);
	nim_ui::ContactsListManager::GetInstance()->AttachFriendListBox(nullptr);
	nim_ui::ContactsListManager::GetInstance()->AttachGroupListBox(nullptr);

	__super::OnFinalMessage(hWnd);
}

LRESULT MainForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
	::ShowWindow(m_hWnd, SW_HIDE);
	return 0;
}

LRESULT MainForm::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void MainForm::InitWindow()
{
	SetIcon(IDI_ICON);
	SetTaskbarTitle(L"���� Demo");
	m_pRoot->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&MainForm::Notify, this, std::placeholders::_1));
	m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&MainForm::OnClicked, this, std::placeholders::_1));

	Button* closebtn = (Button*) FindControl(L"btn_wnd_close");
	closebtn->AttachClick(nbase::Bind(&MainForm::OnCloseBtnClicked, this, std::placeholders::_1));

	btn_header_ = (Button*) FindControl(L"btn_header");
	label_name_ = (Label*) FindControl(L"label_name");
	InitHeader();

	((OptionBox*) FindControl(L"btn_session_list"))->Selected(true, true);
	ui::ListBox* session_list = (ListBox*)FindControl(L"session_list");
	nim_ui::SessionListManager::GetInstance()->AttachListBox(session_list);
	ui::TreeView* friend_list = (TreeView*) FindControl(L"friend_list");
	nim_ui::ContactsListManager::GetInstance()->AttachFriendListBox(friend_list);
	ui::TreeView* group_list = (TreeView*) FindControl(L"group_list");
	nim_ui::ContactsListManager::GetInstance()->AttachGroupListBox(group_list);

	Button* main_menu_button = (Button*) FindControl(L"main_menu_button");
	main_menu_button->AttachClick(nbase::Bind(&MainForm::MainMenuButtonClick, this, std::placeholders::_1));

	search_edit_ = static_cast<RichEdit*>(FindControl(_T("search_edit")));
	search_edit_->AttachTextChange(nbase::Bind(&MainForm::SearchEditChange, this, std::placeholders::_1));
	search_edit_->SetLimitText(30);
	btn_clear_input_ = static_cast<ui::Button*>(FindControl(L"clear_input"));
	btn_clear_input_->AttachClick(nbase::Bind(&MainForm::OnClearInputBtnClicked, this, std::placeholders::_1));
	btn_clear_input_->SetNoFocus();

	search_result_list_ = static_cast<ui::ListBox*>(FindControl(_T("search_result_list")));

	TrayIcon::GetInstance()->Init(this);
	TrayIcon::GetInstance()->SetTrayIcon(::LoadIconW(nbase::win32::GetCurrentModuleHandle(), MAKEINTRESOURCE(IDI_ICON)), L"���� Demo");

	nim_ui::ContactsListManager::GetInstance()->InvokeGetAllUserInfo();
	nim_ui::SessionListManager::GetInstance()->InvokeLoadSessionList();
	nim_ui::SessionListManager::GetInstance()->QueryUnreadCount();
}

void MainForm::OnUserInfoChange(const std::list<nim::UserNameCard> &uinfos)
{
	for (auto iter = uinfos.cbegin(); iter != uinfos.cend(); iter++)
	{
		if (nim_ui::LoginManager::GetInstance()->IsEqual(iter->GetAccId()))
		{
			InitHeader();
			break;
		}
	}
}

void MainForm::OnUserPhotoReady(const std::string& account, const std::wstring& photo_path)
{
	if (nim_ui::LoginManager::GetInstance()->GetAccount() == account)
		btn_header_->SetBkImage(photo_path);
}

void MainForm::InitHeader()
{
	std::string my_id = nim_ui::LoginManager::GetInstance()->GetAccount();
	label_name_->SetText(nim_ui::UserManager::GetInstance()->GetUserName(my_id, false));
	btn_header_->SetBkImage(nim_ui::UserManager::GetInstance()->GetUserPhoto(my_id));
}

bool MainForm::Notify( ui::EventArgs* msg )
{
	return true;
}

bool MainForm::OnCloseBtnClicked(ui::EventArgs* msg)
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
	::ShowWindow(m_hWnd, SW_HIDE);
	return false;
}

bool MainForm::OnClicked( ui::EventArgs* msg )
{
	std::wstring name = msg->pSender->GetName();
	if(name == L"btn_search_team")
	{
		nim_ui::WindowsManager::GetInstance()->SingletonShow<nim_comp::TeamSearchForm>(nim_comp::TeamSearchForm::kClassName);
	}
	else if(name == L"btn_create_group")
	{
		nim_comp::InvokeChatForm *invite_user_form = (nim_comp::InvokeChatForm *)nim_comp::WindowsManager::GetInstance()->GetWindow\
			(nim_comp::InvokeChatForm::kClassName, L"CreateGroupWnd");
		std::wstring caption = ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRING_INVITEUSERFORM_CREATE_GROUP");
		if (!invite_user_form)
		{
			nim_comp::InvokeChatForm::SelectedCompletedCallback cb = ToWeakCallback([this](const std::list<std::string> &id_list)
			{
				if (id_list.empty())
				{
					ShowMsgBox(m_hWnd, L"����ʧ�ܣ����������", nullptr, L"��ʾ", L"ȷ��", L"");
					return;
				}

				UTF16String user_names;
				auto it = id_list.cbegin();
				for (int i = 0; it != id_list.cend() && i < 2; it++, i++)
					user_names += nim_ui::UserManager::GetInstance()->GetUserName(*it, false) + L";";
				user_names += nim_ui::UserManager::GetInstance()->GetUserName(it == id_list.end() ? nim_ui::LoginManager::GetInstance()->GetAccount() : *it, false);

				nim::TeamInfo tinfo;
				tinfo.SetType(nim::kNIMTeamTypeNormal);
				tinfo.SetName(nbase::UTF16ToUTF8(user_names));
				nim::Team::CreateTeamAsync(tinfo, id_list, "", nbase::Bind(&nim_comp::TeamCallback::OnTeamEventCallback, std::placeholders::_1));
			});
			invite_user_form = new nim_comp::InvokeChatForm("CreateGroupWnd", std::list<UTF8String>(), cb);
			invite_user_form->Create(NULL, L"", UI_WNDSTYLE_FRAME& ~WS_MAXIMIZEBOX, 0L);
			invite_user_form->CenterWindow();
		}
		else
		{
			invite_user_form->ActiveWindow();
		}
	}
	else if (name == L"btn_create_team")
	{
		std::wstring caption = ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRING_INVITEUSERFORM_INVITE_JOINCHAT");
		nim_comp::TeamInfoForm* team_info_form = (nim_comp::TeamInfoForm*)nim_ui::WindowsManager::GetInstance()->GetWindow\
			(nim_comp::TeamInfoForm::kClassName, nim_comp::TeamInfoForm::kTeamInfoWindowId);
		if (team_info_form == NULL)
		{
			team_info_form = new nim_comp::TeamInfoForm(true, nim::kNIMTeamTypeAdvanced, "", nim::TeamInfo());
			team_info_form->Create(NULL, caption.c_str(), WS_OVERLAPPEDWINDOW& ~WS_MAXIMIZEBOX, 0L);
			team_info_form->CenterWindow();
			team_info_form->ShowWindow(true);
		}
		else
		{
			team_info_form->ActiveWindow();
		}
	}
	else if (name == L"btn_header")
		nim_ui::WindowsManager::GetInstance()->ShowProfileForm(nim_ui::LoginManager::GetInstance()->GetAccount());

	return true;
}

void MainForm::LeftClick()
{
	this->ActiveWindow();
	if(is_trayicon_left_double_clicked_)
	{
		is_trayicon_left_double_clicked_ = false;
		return;
	}
	::SetForegroundWindow(m_hWnd);
	::BringWindowToTop(m_hWnd);
}

void MainForm::LeftDoubleClick()
{
	is_trayicon_left_double_clicked_ = true;
}

void MainForm::RightClick()
{
	POINT point;
	::GetCursorPos(&point);
	PopupMainTrayMenu(point);
}

void MainForm::PopupMainTrayMenu(POINT point)
{
	//�����˵�����
	CMenuWnd* pMenu = new CMenuWnd(NULL);
	STRINGorID xml(L"tray_menu.xml");
	pMenu->Init(xml, _T("xml"), point, CMenuWnd::RIGHT_TOP);
	//ע��ص�
	CMenuElementUI* display_session_list = (CMenuElementUI*)pMenu->FindControl(L"display_session_list");
	display_session_list->AttachSelect(nbase::Bind(&MainForm::SessionListMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* logoff = (CMenuElementUI*)pMenu->FindControl(L"logoff");
	logoff->AttachSelect(nbase::Bind(&MainForm::LogoffMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* quit = (CMenuElementUI*)pMenu->FindControl(L"quit");
	quit->AttachSelect(nbase::Bind(&MainForm::QuitMenuItemClick, this, std::placeholders::_1));
	//��ʾ
	pMenu->Show();
}

bool MainForm::MainMenuButtonClick(ui::EventArgs* param)
{
	RECT rect = param->pSender->GetPos();
	CPoint point;
	point.x = rect.left - 15;
	point.y = rect.bottom + 10;
	ClientToScreen(m_hWnd, &point);
	PopupMainMenu(point);
	return true;
}

void MainForm::PopupMainMenu(POINT point)
{
	//�����˵�����
	CMenuWnd* pMenu = new CMenuWnd(NULL);
	STRINGorID xml(L"main_menu.xml");
	pMenu->Init(xml, _T("xml"), point);
	//ע��ص�
	CMenuElementUI* look_log = (CMenuElementUI*)pMenu->FindControl(L"look_log");
	look_log->AttachSelect(nbase::Bind(&MainForm::LookLogMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* file_trans = (CMenuElementUI*)pMenu->FindControl(L"file_helper");
	file_trans->AttachSelect(nbase::Bind(&MainForm::FileTransMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* address = (CMenuElementUI*)pMenu->FindControl(L"address");
	address->AttachSelect(nbase::Bind(&MainForm::AddressMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* export_msglog = (CMenuElementUI*)pMenu->FindControl(L"export_msglog");
	export_msglog->AttachSelect(nbase::Bind(&MainForm::ExportMsglogMenuItemClick, this, std::placeholders::_1));
	CMenuElementUI* import_msglog = (CMenuElementUI*)pMenu->FindControl(L"import_msglog");
	import_msglog->AttachSelect(nbase::Bind(&MainForm::ImportMsglogMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* clear_chat_record = (CMenuElementUI*)pMenu->FindControl(L"clear_chat_record");
	clear_chat_record->AttachSelect(nbase::Bind(&MainForm::ClearChatRecordMenuItemClick, this, true, std::placeholders::_1));
	CMenuElementUI* clear_chat_record_ex = (CMenuElementUI*)pMenu->FindControl(L"clear_chat_record_ex");
	clear_chat_record_ex->AttachSelect(nbase::Bind(&MainForm::ClearChatRecordMenuItemClick, this, false, std::placeholders::_1));

	CMenuElementUI* clear_chat_record_p2p = (CMenuElementUI*)pMenu->FindControl(L"clear_chat_record_p2p");
	clear_chat_record_p2p->AttachSelect(nbase::Bind(&MainForm::ClearChatRecordBySessionTypeMenuItemClick, this, true, nim::kNIMSessionTypeP2P, std::placeholders::_1));
	CMenuElementUI* clear_chat_record_p2p_ex = (CMenuElementUI*)pMenu->FindControl(L"clear_chat_record_p2p_ex");
	clear_chat_record_p2p_ex->AttachSelect(nbase::Bind(&MainForm::ClearChatRecordBySessionTypeMenuItemClick, this, false, nim::kNIMSessionTypeP2P, std::placeholders::_1));
	CMenuElementUI* clear_chat_record_team = (CMenuElementUI*)pMenu->FindControl(L"clear_chat_record_team");
	clear_chat_record_team->AttachSelect(nbase::Bind(&MainForm::ClearChatRecordBySessionTypeMenuItemClick, this, true, nim::kNIMSessionTypeTeam, std::placeholders::_1));
	CMenuElementUI* clear_chat_record_team_ex = (CMenuElementUI*)pMenu->FindControl(L"clear_chat_record_team_ex");
	clear_chat_record_team_ex->AttachSelect(nbase::Bind(&MainForm::ClearChatRecordBySessionTypeMenuItemClick, this, false, nim::kNIMSessionTypeTeam, std::placeholders::_1));

	CMenuElementUI* vchat_setting = (CMenuElementUI*)pMenu->FindControl(L"vchat_setting");
	vchat_setting->AttachSelect(nbase::Bind(&MainForm::VChatSettingMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* rts_replay = (CMenuElementUI*)pMenu->FindControl(L"rts_replay");
	rts_replay->AttachSelect(nbase::Bind(&MainForm::RtsReplayMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* about = (CMenuElementUI*)pMenu->FindControl(L"about");
	about->AttachSelect(nbase::Bind(&MainForm::AboutMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* logoff = (CMenuElementUI*)pMenu->FindControl(L"logoff");
	logoff->AttachSelect(nbase::Bind(&MainForm::LogoffMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* quit = (CMenuElementUI*)pMenu->FindControl(L"quit");
	quit->AttachSelect(nbase::Bind(&MainForm::QuitMenuItemClick, this, std::placeholders::_1));
	//��ʾ
	pMenu->Show();
}

static void LookLogClick(HWND m_hWnd)
{
	//TODO����ʱ��ʾ�û��������ڵ�Ŀ¼�������ռ��û�������
	std::wstring dir = nim_ui::UserConfig::GetInstance()->GetUserDataPath();
	std::wstring tip = nbase::StringPrintf(L"��ǰ�û�����Ŀ¼��%s", dir.c_str());
	ShowMsgBox(m_hWnd, tip, MsgboxCallback(), L"��ʾ", L"֪����", L"");	
	HINSTANCE inst = ::ShellExecute(NULL, L"open", dir.c_str(), NULL, NULL, SW_SHOW);
	int ret = (int) inst;
	if(ret > 32)
	{
		QLOG_APP(L"open user data path: {0}") <<dir.c_str();
		return;
	}
	else
	{
		QLOG_ERR(L"failed to open user data path: {0}") <<dir.c_str();
		return;
	}
}

bool MainForm::LookLogMenuItemClick(ui::EventArgs* param)
{
	nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(&LookLogClick, GetHWND()));
	return false;
}
bool MainForm::FileTransMenuItemClick(ui::EventArgs* param)
{
	nim_ui::SessionManager::GetInstance()->OpenSessionForm(nim_ui::LoginManager::GetInstance()->GetAccount(), nim::kNIMSessionTypeP2P);
	return true;
}

bool MainForm::AddressMenuItemClick(ui::EventArgs* param)
{
	if (!nim_ui::LoginManager::GetInstance()->IsLinkActive())
		nim_ui::WindowsManager::GetInstance()->ShowLinkForm();
	else
		nim_ui::ContactsListManager::GetInstance()->InvokeGetAllUserInfo();
	return true;
}
bool MainForm::ExportMsglogMenuItemClick(ui::EventArgs* param)
{
	MsglogManageForm::ShowForm(true);
	return true;
}
bool MainForm::ImportMsglogMenuItemClick(ui::EventArgs* param)
{
	MsglogManageForm::ShowForm(false);
	return true;
}

bool MainForm::ClearChatRecordMenuItemClick(bool del_session, ui::EventArgs* param)
{
	nim::MsgLog::DeleteAllAsync(del_session, nim::MsgLog::DeleteAllCallback());
	return true;
}
bool MainForm::ClearChatRecordBySessionTypeMenuItemClick(bool del_session, nim::NIMSessionType type, ui::EventArgs* param)
{
	nim::MsgLog::DeleteBySessionTypeAsync(del_session, type, nim::MsgLog::DeleteBySessionTypeCallback());
	return true;
}
bool MainForm::VChatSettingMenuItemClick(ui::EventArgs* param)
{
	nim_ui::WindowsManager::GetInstance()->ShowVideoSettingForm();
	return true;
}
bool MainForm::RtsReplayMenuItemClick(ui::EventArgs* param)
{
	nim_ui::WindowsManager::SingletonShow<nim_comp::RtsReplay>(nim_comp::RtsReplay::kClassName);
	return true;
}

bool MainForm::SessionListMenuItemClick(ui::EventArgs* param)
{
	OptionBox* session = (OptionBox*) FindControl(L"btn_session_list");
	session->Selected(true, true);
	LeftClick();
	return true;
}

bool MainForm::AboutMenuItemClick(ui::EventArgs* param)
{
	nim_ui::WindowsManager::SingletonShow<AboutForm>(AboutForm::kClassName);
	return false;
}

bool MainForm::LogoffMenuItemClick(ui::EventArgs* param)
{
	QCommand::Set(kCmdRestart, L"true");
	std::wstring wacc = nbase::UTF8ToUTF16(nim_ui::LoginManager::GetInstance()->GetAccount());
	QCommand::Set(kCmdAccount, wacc);
	nim_ui::LoginManager::GetInstance()->DoLogout(false, nim::kNIMLogoutChangeAccout);
	return true;
}

bool MainForm::QuitMenuItemClick(ui::EventArgs* param)
{
	nim_ui::LoginManager::GetInstance()->DoLogout(false);
	return true;
}