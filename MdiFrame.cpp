//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "MdiFrame.h"
#include "cpm_test.h"
#include <string.h>
#include "MkfsUnit.h"
#include <shlobj.h>
#include <stdlib.h>
#include <dir.h>
#include "AboutDlg.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TFrameForm *FrameForm;
extern char defpath[];
extern char cur_defpath[];
char tmp_dir[MAX_PATH];
AnsiString fmt;

//�h���b�O���h���b�v�i�C���[�W������o���j---------------------------------
// IDropSource�̎���
class TDropSource : public IDropSource
{
public:
  TDropSource() : FRefCount(0) {
  }
  HRESULT __stdcall QueryInterface(REFIID riid, void** ppv)
  {
    IUnknown *punk = NULL;
    if (riid == IID_IUnknown) {
      punk = static_cast<IUnknown*>(this);
    } else if (riid == IID_IDropSource) {
      punk = static_cast<IDropSource*>(this);
    }

    *ppv = punk;
    if (punk) {
      punk->AddRef();
      return S_OK;
    } else {
      return E_NOINTERFACE;
    }
  }
  ULONG __stdcall AddRef()
  {
    InterlockedIncrement(&FRefCount);
    return (ULONG)FRefCount;
  }
  ULONG __stdcall Release()
  {
    ULONG ret = (ULONG)InterlockedDecrement(&FRefCount);
    if (ret == 0) {
      delete this;
    }
    return ret;
  }

  /**
   * �h���b�O�A���h�h���b�v�������p�����邩�I�����邩��Ԃ��܂��B
   */
  HRESULT __stdcall QueryContinueDrag(BOOL fEsc, DWORD grfKeyState)
  {
    // ESC�������ꂽ���A�����̃{�^����������Ă���ꍇ�͒��~����
    if (fEsc ||
        (MK_LBUTTON | MK_RBUTTON) == (grfKeyState & (MK_LBUTTON | MK_RBUTTON))) {
      return DRAGDROP_S_CANCEL;
    }

    //�}�E�X�̍��{�^���������ꂽ�ꍇ�̓h���b�v������
    if ((grfKeyState & (MK_LBUTTON | MK_RBUTTON)) == 0) {
      return DRAGDROP_S_DROP;
    }

    //����ȊO��D&D�p��
    return S_OK;
  }

  /**
   * �}�E�X�J�[�\���̌`���ݒ肵�A���[�U�[�Ƀt�B�[�h�o�b�N��񋟂��܂��B
   */
  HRESULT __stdcall GiveFeedback(DWORD dwEffect)
  {
    //�f�t�H���g�̃J�[�\�����g��
    return DRAGDROP_S_USEDEFAULTCURSORS;
  }
private:
  LONG FRefCount;
};

//---------------------------------------------------------------------------
// Layout helpers -----------------------------------------------------------
// Extend a control's anchors (Left/Top are always kept) so it follows the
// right and/or bottom edge of its parent when the window is resized.
static void SetAnchors(TControl *ctrl, bool right, bool bottom)
{
    if (right)  ctrl->Anchors = ctrl->Anchors << akRight;
    if (bottom) ctrl->Anchors = ctrl->Anchors << akBottom;
}
//---------------------------------------------------------------------------
__fastcall TFrameForm::TFrameForm(TComponent* Owner)
	: TForm(Owner)
{
    FUpdatingLayout = false;

    strcpy(defpath, GetCurrentDir().c_str());
    strcpy(tmp_dir, defpath);
    strcat(defpath, "\\diskdefs");
    strcat(tmp_dir, "\\Cpmtools_tmp");

    //�h���b�O�Ŏ󂯎��\
    DragAcceptFiles(Edit1->Handle, true );
    DragAcceptFiles(ListView1->Handle, true );

    Application->OnMessage = AppMessage; //���b�Z�[�W����������֐����w��

    OleInitialize(NULL);

    InitResponsiveLayout();
}
//---------------------------------------------------------------------------
// Make the window resizable and let every control follow the window edges.
void __fastcall TFrameForm::InitResponsiveLayout()
{
    // Freely resizable window with a sane minimum size
    BorderStyle = bsSizeable;
    Constraints->MinWidth  = 720;
    Constraints->MinHeight = 480;
    DoubleBuffered = true;
    Position = poScreenCenter;

    // Controls inside Panel1 (image browser)
    SetAnchors(Edit1,     true,  false);   // stretch to the Select button
    SetAnchors(Button1,   true,  false);   // stick to the right edge
    SetAnchors(ListBox1,  false, true);    // grow downwards
    SetAnchors(ListView1, true,  true);    // fill the remaining area
    SetAnchors(ComboBox1, true,  true);    // stretch along the bottom

    // Controls inside Panel2 (directory browser)
    SetAnchors(DriveComboBox1,    true,  false);
    SetAnchors(DirectoryListBox1, true,  false);
    SetAnchors(FileListBox1,      true,  true);   // fill the remaining area
    SetAnchors(FilterComboBox1,   true,  true);   // stretch along the bottom

    // Flat, modern look
    Panel1->BevelOuter = bvNone;
    Panel2->BevelOuter = bvNone;
    Panel1->DoubleBuffered = true;
    Panel2->DoubleBuffered = true;

    // Start with a comfortable, larger window
    Width  = 1024;
    Height = 700;

    // Apply the responsive layout now and keep it up to date
    OnResize = FormResize;
    FormResize(NULL);
}
//---------------------------------------------------------------------------
// Position the two panels and the four action buttons to fill the client area.
void __fastcall TFrameForm::FormResize(TObject *Sender)
{
    if (FUpdatingLayout) return;
    FUpdatingLayout = true;

    const int BOTTOM_BAR   = 46;   // height reserved for the action buttons
    const int RIGHT_PANEL  = 200;  // width of the directory browser
    const int MIDDLE_STRIP = 56;   // width of the Get/Put button strip
    const int BTN_W        = 84;

    int W = ClientWidth;
    int H = ClientHeight;
    int mainH = H - BOTTOM_BAR;
    int leftW = W - RIGHT_PANEL - MIDDLE_STRIP;
    if (mainH < 160) mainH = 160;
    if (leftW < 120) leftW = 120;

    // Main panels
    Panel1->SetBounds(0, 0, leftW, mainH);
    Panel2->SetBounds(W - RIGHT_PANEL, 0, RIGHT_PANEL, mainH);

    // Get / Put buttons, centred in the gap between the two panels
    int bx = W - RIGHT_PANEL - MIDDLE_STRIP + 3;
    int by = mainH / 2 - 18;
    BitBtn1->SetBounds(bx, by,      MIDDLE_STRIP - 6, 26);
    BitBtn2->SetBounds(bx, by + 34, MIDDLE_STRIP - 6, 26);

    // New / Exit buttons along the bottom edge
    BitBtn4->SetBounds(8,          mainH + 10, BTN_W, 26);
    BitBtn3->SetBounds(W - 8 - BTN_W, mainH + 10, BTN_W, 26);

    FUpdatingLayout = false;
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::Exit1Click(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::Open1Click(TObject *Sender)
{
	if(OpenFileDialog->Execute())
    {
        Edit1->Text = OpenFileDialog->FileName;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::cpm_ls(char image[], char format[], char pat[])
{
    int dir_count=0;
    char **dir_name=NULL; //2023.05.08
    const char *err;

    err=cpm_ls_test(image, format, pat, &dir_count, &dir_name);
    if(err!=NULL){
		//�G���[
        MessageBox(Handle, err, "Message Box", MB_OK);
        ListView1->Items->Clear();
        if(dir_name != NULL){ //2023.05.08
         free(dir_name);
        }
        return;
    }

    int i;
    ListView1->Items->Clear();
    for(i=0; i<dir_count; i++){
        ListView1->Items->Add();
        ListView1->Items->Item[i]->Caption = String(dir_name[i]+2);
    }
    free(dir_name);
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::ComboBox1Change(TObject *Sender)
{
	cpm_ls(Edit1->Text.c_str(),fmt.c_str(), ComboBox1->Text.c_str());
}
//---------------------------------------------------------------------------

void __fastcall TFrameForm::Button1Click(TObject *Sender)
{
	if(OpenFileDialog->Execute())
    {
    	strcpy(cur_defpath, GetCurrentDir().c_str());
	    strcat(cur_defpath, "\\diskdefs");

        Edit1->Text = OpenFileDialog->FileName;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::Edit1Change(TObject *Sender)
{
    cpm_fmt();
    fmt = ListBox1->Items->Strings[ListBox1->ItemIndex];
	cpm_ls(Edit1->Text.c_str(),fmt.c_str(), ComboBox1->Text.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::cpm_fmt()
{
    int fmt_count=0;
    char **fmt_name;
    const char *err;

    err=cpm_fmt_test(&fmt_count, &fmt_name);
    if(err!=NULL){
    	//�G���[
        MessageBox(Handle, err, "Message Box", MB_OK);
        free(fmt_name);
        exit(1);
    }
    int i;
    ListBox1->Items->Clear();
    for(i=0; i<fmt_count; i++){
        ListBox1->Items->Add(String(fmt_name[i]));
    }
    ListBox1->ItemIndex = 0;
    free(fmt_name);
}
//---------------------------------------------------------------------------

void __fastcall TFrameForm::ListBox1Click(TObject *Sender)
{
	fmt = ListBox1->Items->Strings[ListBox1->ItemIndex];
    cpm_ls(Edit1->Text.c_str(),fmt.c_str(), ComboBox1->Text.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::BitBtn1Click(TObject *Sender)
{
	int i;
    AnsiString src, dest;
    const char *err;

    for(i=0; i< ListView1->Items->Count; i++){
    	if(ListView1->Items->Item[i]->Selected){
        	src = ListView1->Items->Item[i]->Caption;
            dest = DirectoryListBox1->Directory + String("\\") + src;
            if(FileExists(dest)){
            	if(MessageBox(Handle, "Overwrite?",
                                   src.c_str(), MB_YESNO)==IDNO) continue;
            }
            err=cpm_to_win_test(Edit1->Text.c_str(),fmt.c_str(),src.c_str() ,dest.c_str());
            if(err!=NULL){  //�G���[
            	MessageBox(Handle, err, src.c_str(), MB_OK);
            	//MessageBox(Handle, err, "Message Box", MB_OK);
                //return;
            }
        }
    }
    FileListBox1->Update();
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::BitBtn2Click(TObject *Sender)
{
	int i;
    AnsiString src, dest;
    const char *err;

    if(Edit1->Text.Length()==0) return;

    for(i=0; i< FileListBox1->Items->Count; i++){
    	if(FileListBox1->Selected[i]){
        	dest = FileListBox1->Items->Strings[i];
            src = DirectoryListBox1->Directory + String("\\") + dest;
            err=win_to_cpm_test(Edit1->Text.c_str(),fmt.c_str(),src.c_str() ,dest.c_str());
            if(err!=NULL){  //�G���[
            	MessageBox(Handle, err, dest.c_str(), MB_OK);
            	//MessageBox(Handle, err, "Message Box", MB_OK);
                //return;
            }

        }
    }
    cpm_ls(Edit1->Text.c_str(),fmt.c_str(), ComboBox1->Text.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::Delete1Click(TObject *Sender)
{
	int i;
    AnsiString src;
    const char *err;

	if(MessageBox(Handle, "Delete select file?",
                                   "check", MB_YESNO)==IDYES)
    {
    	for(i=0; i< ListView1->Items->Count; i++){
    		if(ListView1->Items->Item[i]->Selected){
        		src = ListView1->Items->Item[i]->Caption;
            	err=cpm_rm_test(Edit1->Text.c_str(),fmt.c_str(),src.c_str());
            	if(err!=NULL){  //�G���[
            		MessageBox(Handle, err, src.c_str(), MB_OK);
            		//MessageBox(Handle, err, "Message Box", MB_OK);
                	//break;
            	}
        	}
    	}
    	cpm_ls(Edit1->Text.c_str(),fmt.c_str(), ComboBox1->Text.c_str());
    }
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::PopupMenu1Popup(TObject *Sender)
{
    if(ListView1->SelCount) Delete1->Enabled=true;
    else Delete1->Enabled=false;
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::FormCreate(TObject *Sender)
{
	if(ParamCount()){
	    strcpy(cur_defpath, ParamStr(1).c_str());
        *strrchr(cur_defpath, '\\')='\0';
        strcat(cur_defpath, "\\diskdefs");

    	Edit1->Text =  ParamStr(1);
    }
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::BitBtn3Click(TObject *Sender)
{
	  Close();
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::Mkfs1Click(TObject *Sender)
{
	MkfsForm->ShowModal();
}
//---------------------------------------------------------------------------
//�h���b�O&�h���b�v(�O���̃t�@�C������R���|�[�l���g��)
void __fastcall TFrameForm::WMDropFiles( TWMDropFiles &Msg, HWND hwnd )
{
    // �ϐ��錾
    int FileCount;              //�󂯎�����t�@�C����
    char FileName[ MAX_PATH ];  //�t�@�C�����ꎞ�ۑ�
    Application->BringToFront();   //�A�v���P�[�V������O�ʂɈړ�

    // �t�@�C�����𓾂�
    FileCount = (int)DragQueryFile( (HDROP)Msg.Drop , 0xFFFFFFFF , NULL , MAX_PATH );

    if(hwnd == Edit1->Handle){  //�t�H�[���փC���[�W�t�@�C���̃h���b�O&�h���b�v
       	if(FileCount > 1) goto WMD_END;
    	DragQueryFile((HDROP)Msg.Drop, 0, FileName, MAX_PATH ); //�t�@�C���p�X�𓾂�
	   	strcpy(cur_defpath, FileName);
       	*strrchr(cur_defpath, '\\')='\0';
       	strcat(cur_defpath, "\\diskdefs");
       	Edit1->Text = FileName;
    }

    if(hwnd == ListView1->Handle){  //�C���[�W��CP/M�t�@�C���̃h���b�O&�h���b�v
	    for( int i=0 ; i < FileCount ; ++i ){
		    AnsiString src, dest;
		    const char *err;

    	    DragQueryFile((HDROP)Msg.Drop, i, FileName, MAX_PATH ); //�t�@�C���p�X�𓾂�
            if(ExtractFileDir(String(FileName)) == String(tmp_dir)){
                 remove(FileName);
            	 continue;
            }
            dest = strrchr(FileName, '\\')+1;
            src = FileName;
            err=win_to_cpm_test(Edit1->Text.c_str(),fmt.c_str(),src.c_str() ,dest.c_str());
            if(err!=NULL){  //�G���[
            	MessageBox(Handle, err, dest.c_str(), MB_OK);
            	//MessageBox(Handle, err, "Message Box", MB_OK);
                //goto WMD_END;
            }
         }
         cpm_ls(Edit1->Text.c_str(),fmt.c_str(), ComboBox1->Text.c_str());
    }
    WMD_END:
    DragFinish((HDROP)Msg.Drop);
    Msg.Result = true;//false�Ȃ�΁A���̃R���g���[���ł����������\�������邪false�ł��ǂ��B
}
//---------------------------------------------------------------------------
//�A�v���P�[�V�����̃��b�Z�[�W���[�v�𑀍삷��֐�������
void __fastcall TFrameForm::AppMessage(tagMSG &Msg, bool &Handled)
{
    if (Msg.message == WM_DROPFILES && (Msg.hwnd == Edit1->Handle ||
                                        Msg.hwnd == ListView1->Handle ))
    {
        //Msg��TWMDropFiles�\���̂̌`�ɍ��킹�ĕό`����
        TWMDropFiles MsgDrop;
        MsgDrop.Drop = Msg.wParam;
        WMDropFiles(MsgDrop, Msg.hwnd);
        Handled=MsgDrop.Result;
    }
    if (Msg.message == WM_DROPFILES && (Msg.hwnd == MkfsForm->Edit1->Handle ||
                                        Msg.hwnd == MkfsForm->Edit2->Handle ||
                                        Msg.hwnd == MkfsForm->Edit3->Handle ||
                                        Msg.hwnd == MkfsForm->Edit4->Handle ||
                                        Msg.hwnd == MkfsForm->Edit5->Handle ))
    {
        //Msg��TWMDropFiles�\���̂̌`�ɍ��킹�ĕό`����
        TWMDropFiles MsgDrop;
        MsgDrop.Drop = Msg.wParam;
        MkfsForm->WMDropFiles(MsgDrop, Msg.hwnd);
        Handled=MsgDrop.Result;
    }
}
//---------------------------------------------------------------------------

void __fastcall TFrameForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	OleUninitialize();
}
//---------------------------------------------------------------------------
/**
 * ��ƃt�H���_���t�@�C����IDataObject�C���^�[�t�F�C�X���擾
 * @param selcount �t�@�C����
 * @return ���s�����Ƃ���NULL
 */
IDataObject *GetFilePathDataObject(int selcount)
{

  OLECHAR  olePath[MAX_PATH];

  //�f�X�N�g�b�v��IShellFolder�C���^�[�t�F�C�X���擾
  IShellFolder* desktop = NULL;
  if (FAILED(SHGetDesktopFolder(&desktop))) { return NULL; }

  //�Ώۃt�@�C���̐e�t�H���_(��ƃt�H���_)�̍��ڎ��ʎq�𓾂�
  ITEMIDLIST* dirIDList;
  DWORD eaten = 0, attributes = 0;

  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, tmp_dir, -1, olePath, MAX_PATH);
  if (FAILED(desktop->ParseDisplayName(NULL, NULL, olePath, &eaten,
                                       &dirIDList, &attributes))) {
    desktop->Release();
    return NULL;
  }

  //�e�t�H���_��IShellFolder�C���^�[�t�F�C�X���擾
  IShellFolder* targetFolder;
  if (FAILED(desktop->BindToObject(dirIDList, NULL, IID_IShellFolder,
                                   (void**)&targetFolder))) {
    desktop->Release();
    return NULL;
  }
  //�������m��
  LPITEMIDLIST *ppidlAbsolute;
  ppidlAbsolute = (LPITEMIDLIST *)CoTaskMemAlloc(sizeof(LPITEMIDLIST) * selcount);

  int i,j=0;
  AnsiString file;

  for(i=0; i< FrameForm->ListView1->Items->Count; i++){
  	if(FrameForm->ListView1->Items->Item[i]->Selected){
    	file = FrameForm->ListView1->Items->Item[i]->Caption;
        //�Ώۃt�@�C���̍��ڎ��ʎq�𓾂�
  		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, file.c_str(), -1, olePath, MAX_PATH);
  		if (FAILED(targetFolder->ParseDisplayName(NULL, NULL, olePath, &eaten,
                                            &ppidlAbsolute[j], &attributes))) {
    		targetFolder->Release();
    		desktop->Release();
    		return NULL;
  		}
        j++;
    }
  }

  //�f�[�^�I�u�W�F�N�g�擾
  IDataObject* dataObject = NULL;
  targetFolder->GetUIObjectOf(NULL, selcount, (LPCITEMIDLIST*)ppidlAbsolute,
                              IID_IDataObject, NULL, (void**)&dataObject);

  //��n��
  for (i = 0; i < selcount; i++)
		CoTaskMemFree(ppidlAbsolute[i]);
  CoTaskMemFree(ppidlAbsolute);

  targetFolder->Release();
  desktop->Release();
  
  return dataObject;
}
//---------------------------------------------------------------------------

void __fastcall TFrameForm::ListView1MouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft) {
    FMouseDownPt.x = X;
    FMouseDownPt.y = Y;
    Perform(WM_LBUTTONUP, 0, 0);
  }
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::ListView1MouseMove(TObject *Sender,
	TShiftState Shift, int X, int Y)   //�C���[�W������o���h���b�O���h���b�v
{
  //���{�^���������ꂽ��ԂŁA�}�E�X�̈ʒu��臒l�𒴂�����h���b�O�J�n
  if ((Shift.Contains(ssLeft) == false) ||
          (abs(X - FMouseDownPt.x) < 3) ||
          (abs(Y - FMouseDownPt.y) < 3)  ) {
      return;
  }
  //��ƃt�H���_�쐬
  if(!DirectoryExists(String(tmp_dir))){
  	mkdir(tmp_dir);
  }
  //�C���[�W������o�����t�@�C������ƃt�H���_�ɍ쐬����
  int i,j=0;
  AnsiString src, dest;
  const char *err;

  for(i=0; i< ListView1->Items->Count; i++){
  	if(ListView1->Items->Item[i]->Selected){
    	src = ListView1->Items->Item[i]->Caption;
        dest = String(tmp_dir) + String("\\") + src;
        err=cpm_to_win_test(Edit1->Text.c_str(),fmt.c_str(),src.c_str() ,dest.c_str());
        if(err!=NULL){  //�G���[
        	MessageBox(Handle, err, src.c_str(), MB_OK);
        	//MessageBox(Handle, err, "Message Box", MB_OK);
            //return;
        }
        j++;
    }
  }
  // OLE�h���b�O���h���b�v�J�n
  IDataObject* dataObject = GetFilePathDataObject(j);
  if (dataObject == NULL) {
  	return;
  }
  DWORD dropEffect;
  TDropSource* dropSource = new TDropSource();
  dropSource->AddRef();
  DoDragDrop(dataObject, dropSource, DROPEFFECT_COPY, &dropEffect);
  dataObject->Release();
  dropSource->Release();

  //��ƃt�H���_�̃t�@�C�����폜����
  for(i=0; i< ListView1->Items->Count; i++){
  	if(ListView1->Items->Item[i]->Selected){
    	src = ListView1->Items->Item[i]->Caption;
        dest = String(tmp_dir) + String("\\") + src;
        remove(dest.c_str());
    }
  }
  rmdir(tmp_dir);
  
}
//---------------------------------------------------------------------------


void __fastcall TFrameForm::Edit2Click(TObject *Sender)
{
	//ListView1->SetFocus();

    if(ListView1->SelCount && ListView1->Focused()) Delete2->Enabled=true;
    else Delete2->Enabled=false;

    if(ListView1->SelCount) imgGet1->Enabled=true;
    else imgGet1->Enabled=false;
    
    if(FileListBox1->SelCount) imgPut1->Enabled=true;
    else imgPut1->Enabled=false;

}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::About1Click(TObject *Sender)
{
     About->ShowModal();	
}
//---------------------------------------------------------------------------
void __fastcall TFrameForm::Online1Click(TObject *Sender)
{
	//�E�F�u�u���E�U�Ŏw���URL���J��
	AnsiString url = "http://star.gmobb.jp/koji/cgi/wiki.cgi?page=CpmtoolsGUI";
	ShellExecute(0, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
