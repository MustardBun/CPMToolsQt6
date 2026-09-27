//---------------------------------------------------------------------------
#ifndef MkfsUnitH
#define MkfsUnitH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\Dialogs.hpp>
//---------------------------------------------------------------------------
class TMkfsForm : public TForm
{
__published:	// IDE 管理のコンポーネント
	TPanel *Panel1;
	TEdit *Edit1;
	TButton *Button1;
	TListBox *ListBox1;
	TEdit *Edit2;
	TButton *Button2;
	TEdit *Edit3;
	TButton *Button3;
	TEdit *Edit4;
	TButton *Button4;
	TEdit *Edit5;
	TButton *Button5;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TBitBtn *BitBtn1;
	TBitBtn *BitBtn2;
	TOpenDialog *OpenDialog1;
	TCheckBox *CheckBox1;
	TCheckBox *CheckBox2;
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Edit1Change(TObject *Sender);
	void __fastcall ListBox1Click(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button3Click(TObject *Sender);
	void __fastcall Button4Click(TObject *Sender);
	void __fastcall Button5Click(TObject *Sender);
	void __fastcall BitBtn1Click(TObject *Sender);
	
	
	void __fastcall CheckBox1Click(TObject *Sender);
	void __fastcall CheckBox2Click(TObject *Sender);
private:	// ユーザー宣言
    void __fastcall TMkfsForm::cpm_fmt();

public:		// ユーザー宣言
	__fastcall TMkfsForm(TComponent* Owner);

    void __fastcall WMDropFiles( TWMDropFiles &Msg, HWND hwnd );//この宣言を追加

};
//---------------------------------------------------------------------------
extern TMkfsForm *MkfsForm;
//---------------------------------------------------------------------------
#endif
