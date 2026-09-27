//---------------------------------------------------------------------------
#ifndef MdiFrameH
#define MdiFrameH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Menus.hpp>
#include <vcl\Dialogs.hpp>
#include <vcl\ComCtrls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\FileCtrl.hpp>
#include <vcl\ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TFrameForm : public TForm
{
__published:	// IDE �Ǘ��̃R���|�[�l���g
	TMainMenu *MainMenu1;
	TMenuItem *File1;
	TMenuItem *Open1;
	TMenuItem *N1;
	TMenuItem *Exit1;
	TOpenDialog *OpenFileDialog;
	TPanel *Panel1;
	TEdit *Edit1;
	TButton *Button1;
	TLabel *Label1;
	TListBox *ListBox1;
	TLabel *Label3;
	TPanel *Panel2;
	TDriveComboBox *DriveComboBox1;
	TDirectoryListBox *DirectoryListBox1;
	TFileListBox *FileListBox1;
	TFilterComboBox *FilterComboBox1;
	TImageList *ImageList1;
	TBitBtn *BitBtn1;
	TBitBtn *BitBtn2;
	TListView *ListView1;
	TComboBox *ComboBox1;
	TPopupMenu *PopupMenu1;
	TMenuItem *Delete1;
	TBitBtn *BitBtn3;
	TMenuItem *Mkfs1;
	TBitBtn *BitBtn4;
	TMenuItem *Edit2;
	TMenuItem *imgGet1;
	TMenuItem *imgPut1;
	TMenuItem *Delete2;
	TMenuItem *Help1;
	TMenuItem *About1;
	TMenuItem *Online1;
	void __fastcall Exit1Click(TObject *Sender);
	
	
	
	
	void __fastcall Open1Click(TObject *Sender);
	void __fastcall ComboBox1Change(TObject *Sender);
	
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Edit1Change(TObject *Sender);
	
	void __fastcall ListBox1Click(TObject *Sender);
	
	
	void __fastcall BitBtn1Click(TObject *Sender);
	void __fastcall BitBtn2Click(TObject *Sender);
	
	
	
	void __fastcall Delete1Click(TObject *Sender);
	void __fastcall PopupMenu1Popup(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall BitBtn3Click(TObject *Sender);
	void __fastcall Mkfs1Click(TObject *Sender);
	
	
	
	
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	void __fastcall ListView1MouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall ListView1MouseMove(TObject *Sender, TShiftState Shift, int X,
	int Y);
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	void __fastcall Edit2Click(TObject *Sender);
	
	void __fastcall About1Click(TObject *Sender);
	void __fastcall Online1Click(TObject *Sender);
	
private:	// ���[�U�[�錾
    void __fastcall cpm_ls(char image[], char format[], char pat[]);
    void __fastcall cpm_fmt();

    void __fastcall WMDropFiles( TWMDropFiles &Msg, HWND hwnd );//���̐錾��ǉ�

    // --- responsive / modernised layout ---
    void __fastcall InitResponsiveLayout();
    void __fastcall FormResize(TObject *Sender);
    bool FUpdatingLayout;

    TPoint FMouseDownPt;

public:		// ���[�U�[�錾
	__fastcall TFrameForm(TComponent* Owner);

    void __fastcall AppMessage(tagMSG &Msg, bool &Handled);//���̐錾��ǉ�

};
//---------------------------------------------------------------------------
extern TFrameForm *FrameForm;
//---------------------------------------------------------------------------
#endif
