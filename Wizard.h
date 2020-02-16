//---------------------------------------------------------------------------

#ifndef WizardH
#define WizardH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Grid.hpp>
#include <FMX.Memo.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.TabControl.hpp>
//---------------------------------------------------------------------------
class TWizardWn : public TForm
{
__published:	// IDE-managed Components
	TLabel *tLabel;
	TLine *Line1;
	TLabel *s8;
	TLabel *s1;
	TLabel *s2;
	TLabel *s4;
	TLabel *s6;
	TLabel *s7;
	TLabel *s5;
	TLabel *s3;
	TButton *BackBut;
	TButton *NextBut;
	TButton *FinishBut;
	TButton *CancBut;
	TLabel *stageLabel;
	TLine *Line2;
	TLabel *Label3;
	TLayout *Layout1;
	TLabel *Label4;
	TMemo *Memo1;
	TButton *Button1;
	TComboBox *aaComboBox1;
	TLabel *Label5;
	TButton *aaButton2;
	TButton *aaButton3;
	TGroupBox *aaGroupBox1;
	TLabel *AllLabel;
	TButton *eButton4;
	TPanel *Rectangle3;
	TPanel *Rectangle5;
	TToolBar *ToolBar1;
	TToolBar *Rectangle1;
	TImage *Image4;
	TButton *setSBut2;
	TButton *HelpBut;
	TImage *Image3;
	TButton *hpButton;
	void __fastcall WizButClick(TObject *Sender);
	void __fastcall FinishButClick(TObject *Sender);
	void __fastcall CancButClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall aaComboBox1Change(TObject *Sender);
	void __fastcall aaButton3Click(TObject *Sender);
	void __fastcall aaButton2Click(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift);
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift);
	void __fastcall eButton4Click(TObject *Sender);
	void __fastcall setSBut2Click(TObject *Sender);
	void __fastcall HelpButClick(TObject *Sender);
	void __fastcall hpButtonClick(TObject *Sender);

	private:	// User declarations
void UndoValidation();
void SetAmountDefaultPos();
void UpdateLabels();
bool GetTrialUnits(int tno);
void StartShow();
void EndShow();
void RemoveHItem(TListBoxItem *t);
void CheckEnableNext();
void AddHItem(TListBoxItem *t, int spacing);
String Find(String value, String col, bool test);
void UpdateOffLineTrialStatus();
void WriteWizardPos(int trialno, int currentpos);
void reloadTrial(int trialno);
public:		// User declarations
bool autoweigh;
void SetTState(int mode);
void SetStages();
TLabel *stepStr[10]; //only 9 steps allowed
int origcurrentpage; //to restore the wizards position if the user decides not to apply changes
int trialno;
void ShowSpec();
void SetTreeItemsVisible(int newindex);
int tstate;
int treeWidth;
int currentpage;
void Scale();
int origFormWidth;
bool somethingChanged;
TStringList* TraitHarvestList;
	__fastcall TWizardWn(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TWizardWn *WizardWn;
//---------------------------------------------------------------------------
#endif
