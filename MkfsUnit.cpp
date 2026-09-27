//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include <string.h>
#include "cpm_test.h"
#include "MkfsUnit.h"
#include "MdiFrame.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TMkfsForm *MkfsForm;
extern char cur_defpath[];
AnsiString fmt2;
extern char FullSizeFlg;
extern char BootSkewFlg;
//---------------------------------------------------------------------------
// Extend a control's anchors (Left/Top are always kept) so it follows the
// right and/or bottom edge of its parent when the window is resized.
static void SetAnchors(TControl *ctrl, bool right, bool bottom)
{
    if (right)  ctrl->Anchors = ctrl->Anchors << akRight;
    if (bottom) ctrl->Anchors = ctrl->Anchors << akBottom;
}
//---------------------------------------------------------------------------
__fastcall TMkfsForm::TMkfsForm(TComponent* Owner)
	: TForm(Owner)
{
    //�h���b�O�Ŏ󂯎��\
    DragAcceptFiles(Edit1->Handle, true );
    DragAcceptFiles(Edit2->Handle, true );
    DragAcceptFiles(Edit3->Handle, true );
    DragAcceptFiles(Edit4->Handle, true );
    DragAcceptFiles(Edit5->Handle, true );

    Application->OnMessage = FrameForm->AppMessage; //���b�Z�[�W����������֐����w��

    // Responsive layout: controls follow the resizable dialog edges
    DoubleBuffered = true;
    Constraints->MinWidth  = 480;
    Constraints->MinHeight = 330;

    SetAnchors(ListBox1,  false, true);   // grow downwards
    SetAnchors(Label3,    true,  false);
    SetAnchors(Edit2,     true,  false);
    SetAnchors(Edit3,     true,  false);
    SetAnchors(Edit4,     true,  false);
    SetAnchors(Edit5,     true,  false);
    SetAnchors(Button2,   true,  false);
    SetAnchors(Button3,   true,  false);
    SetAnchors(Button4,   true,  false);
    SetAnchors(Button5,   true,  false);
    SetAnchors(CheckBox1, true,  false);
    SetAnchors(CheckBox2, true,  false);
    SetAnchors(BitBtn1,   false, true);   // Make  - bottom left of the column
    SetAnchors(BitBtn2,   true,  true);   // Close - bottom right corner
}
//---------------------------------------------------------------------------
void __fastcall TMkfsForm::Button1Click(TObject *Sender)
{
	if(OpenDialog1->Execute())
    {
	    strcpy(cur_defpath, GetCurrentDir().c_str());
        strcat(cur_defpath, "\\diskdefs");

        Edit1->Text = OpenDialog1->FileName;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMkfsForm::cpm_fmt()
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
//----------------------------------------------------------------------
void __fastcall TMkfsForm::Edit1Change(TObject *Sender)
{
    cpm_fmt();
    fmt2 = ListBox1->Items->Strings[ListBox1->ItemIndex];
}
//---------------------------------------------------------------------------
void __fastcall TMkfsForm::ListBox1Click(TObject *Sender)
{
	fmt2 = ListBox1->Items->Strings[ListBox1->ItemIndex];
}
//---------------------------------------------------------------------------
void __fastcall TMkfsForm::Button2Click(TObject *Sender)
{
	if(OpenDialog1->Execute())
    {
        Edit2->Text = OpenDialog1->FileName;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMkfsForm::Button3Click(TObject *Sender)
{
	if(OpenDialog1->Execute())
    {
        Edit3->Text = OpenDialog1->FileName;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMkfsForm::Button4Click(TObject *Sender)
{
	if(OpenDialog1->Execute())
    {
        Edit4->Text = OpenDialog1->FileName;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMkfsForm::Button5Click(TObject *Sender)
{
	if(OpenDialog1->Execute())
    {
        Edit5->Text = OpenDialog1->FileName;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMkfsForm::BitBtn1Click(TObject *Sender)
{
	const char *err;
    char boot[4][255];
    char *bootptr[4];
    int i;

    if(Edit1->Text.IsEmpty()) return;
    
    strcpy(boot[0],Edit2->Text.c_str());
    strcpy(boot[1],Edit3->Text.c_str());
    strcpy(boot[2],Edit4->Text.c_str());
    strcpy(boot[3],Edit5->Text.c_str());

    for(i=0; i<4; i++){
    	if(strcmp(boot[i],""))bootptr[i]=boot[i]; else bootptr[i]=0;
    }

	if(MessageBox(Handle, "Make file with this setting. Ok?",
                                   "check", MB_YESNO)==IDYES)
    {
     	err=creat_cpm_test(Edit1->Text.c_str(), fmt2.c_str(), bootptr);
        if(err!=NULL){
        	//�G���[
        	MessageBox(Handle, err, "Message Box", MB_OK);
            return;
        }
        MessageBox(Handle, "Completed!", "Message Box", MB_OK);
        FrameForm->Edit1->Text = Edit1->Text;
    }
}
//---------------------------------------------------------------------------
//�h���b�O&�h���b�v���������邽�߂Ɉȉ��̃R�[�h��ǉ��B
void __fastcall TMkfsForm::WMDropFiles( TWMDropFiles &Msg, HWND hwnd )
{
    // �ϐ��錾
    int FileCount;              //�󂯎�����t�@�C����
    char FileName[ MAX_PATH ];  //�t�@�C�����ꎞ�ۑ�
    Application->BringToFront();   //�A�v���P�[�V������O�ʂɈړ�

    // �t�@�C�����𓾂�
    FileCount = (int)DragQueryFile( (HDROP)Msg.Drop , 0xFFFFFFFF , NULL , MAX_PATH );

    if(FileCount > 1) goto WMD_END;
    DragQueryFile((HDROP)Msg.Drop, 0, FileName, MAX_PATH ); //�t�@�C���p�X�𓾂�

    if(hwnd == Edit1->Handle){
	   	strcpy(cur_defpath, FileName);
       	*strrchr(cur_defpath, '\\')='\0';
       	strcat(cur_defpath, "\\diskdefs");
       	Edit1->Text = FileName;
    }else if(hwnd == Edit2->Handle){
       	Edit2->Text = FileName;
    }else if(hwnd == Edit3->Handle){
       	Edit3->Text = FileName;
    }else if(hwnd == Edit4->Handle){
       	Edit4->Text = FileName;
    }else if(hwnd == Edit5->Handle){
       	Edit5->Text = FileName;
    }
    WMD_END:
    DragFinish((HDROP)Msg.Drop);
    Msg.Result = true;//false�Ȃ�΁A���̃R���g���[���ł����������\�������邪false�ł��ǂ��B
}
//---------------------------------------------------------------------------

void __fastcall TMkfsForm::CheckBox1Click(TObject *Sender)
{
	if(CheckBox1->Checked == true) FullSizeFlg = 1; else FullSizeFlg = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMkfsForm::CheckBox2Click(TObject *Sender)
{
	if(CheckBox2->Checked == true) BootSkewFlg = 1; else BootSkewFlg = 0;
}
//---------------------------------------------------------------------------
