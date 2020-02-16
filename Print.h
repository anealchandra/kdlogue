//---------------------------------------------------------------------------

#ifndef PrintH
#define PrintH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Dialogs.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.Colors.hpp>
#include <FMX.Edit.hpp>
#include <FMX.TabControl.hpp>
#include <FMX.Objects.hpp>
//---------------------------------------------------------------------------
class TPrintWn : public TForm
{
__published:	// IDE-managed Components
	TPrintDialog *PrintDialog1;
	TListBox *ListBox1;
	TListBox *ListBox2;
	TButton *rOneBut;
	TButton *rAllBut;
	TButton *aOneBut;
	TButton *aAllBut;
	TLabel *Label1;
	TLabel *Label2;
	TButton *priButton1;
	TLayout *Layout1;
	TGroupBox *GroupBox1;
	TComboBox *ComboBox1;
	TLabel *Label3;
	TLabel *Label5;
	TComboBox *labelBox;
	TScrollBox *ScrollBox1;
	TLabel *Label4;
	TNumberBox *Border;
	TLabel *Label7;
	TComboBox *ComboBox4;
	TCheckBox *CheckBox1;
	TTabItem *TabItem1;
	TTabControl *TabControl2;
	TTabItem *TabItem2;
	TTabItem *TabItem3;
	TTabControl *TabControl1;
	TToolBar *ToolBar1;
	TPanel *Panel1;
	TTabItem *TabItem4;
	TComboBox *ComboBox2;
	TLabel *Label6;
	void __fastcall priButton1Click(TObject *Sender);
	void __fastcall ChooseButClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall ScrollBox1MouseMove(TObject *Sender, TShiftState Shift, float X,
          float Y);
	void __fastcall ListBox1Change(TObject *Sender);
	void __fastcall ListBox2Change(TObject *Sender);
	void __fastcall CheckBox1Change(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall ShowPrintPreview(TObject *Sender);

private:	// User declarations
void PrintBarCode();
TImage *im[5000];
TLabel *la[5000];
TScrollBox *sb[300];
TTabItem *t[300];
String Type;
void CleanUpObjs();
void ListClear(TListBox *lb);
public:		// User declarations
void PrintSetup(String type);
void Scale();
void createsb(int pageno);
	__fastcall TPrintWn(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPrintWn *PrintWn;
//---------------------------------------------------------------------------
#endif
