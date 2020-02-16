//---------------------------------------------------------------------------

#ifndef SampleH
#define SampleH
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
#include <FMX.Memo.hpp>
#include <FMX.Objects.hpp>
//---------------------------------------------------------------------------
class TSampleWn : public TForm
{
__published:	// IDE-managed Components
	TLayout *Layout1;
	TGroupBox *GroupBox1;
	TLabel *Label3;
	TLabel *Label5;
	TListBox *ListBox1;
	TEdit *SelBox;
	TEdit *AmountBox;
	TLabel *Label2;
	TLabel *unitlabel;
	TLabel *ounitlabel;
	TCheckBox *AutoWeigh;
	TStatusBar *StatusBar1;
	TProgressBar *ProgressBar1;
	TComboBox *ComboBox1;
	TLabel *Label6;
	TNumberBox *NumberBox1;
	TButton *HelpBut;
	TImage *Image3;
	void __fastcall AddButClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall ListBox1Change(TObject *Sender);
	void __fastcall SelBoxKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift);
	void __fastcall CheckBox2Change(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift);
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift);
	void __fastcall AutoWeighChange(TObject *Sender);
	void __fastcall ComboBox1Change(TObject *Sender);
	void __fastcall AutoWeighKeyDown(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift);
private:	// User declarations
public:		// User declarations
int CalcMeasured();
void Scale();
void FindInListbox(String searchStr);
String getselText();
bool update;
	__fastcall TSampleWn(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSampleWn *SampleWn;
//---------------------------------------------------------------------------
#endif
