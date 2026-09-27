//---------------------------------------------------------------------------
#ifndef AboutDlgH
#define AboutDlgH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\Buttons.hpp>
//---------------------------------------------------------------------------
class TAbout : public TForm
{
__published:	// IDE 管理のコンポーネント
	TPanel *Panel1;
	TImage *Image1;
	TBitBtn *BitBtn1;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TAbout(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TAbout *About;
//---------------------------------------------------------------------------
#endif
