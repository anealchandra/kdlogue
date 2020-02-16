//---------------------------------------------------------------------------

#ifndef settingsH
#define settingsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.IOUtils.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Memo.hpp>
#include <FMX.Types.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.TabControl.hpp>
//---------------------------------------------------------------------------
class TSettingsWn : public TForm
{
__published:	// IDE-managed Components
	TMemo *ParamList;
	TToolBar *ToolBar1;
	TButton *SaveBut;
	TTabControl *TabControl1;
	TTabItem *TabItem1;
	TTabItem *TabItem2;
	TMemo *TabList;
	TButton *Button1;
	void __fastcall SaveButClick(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall TabListKeyDown(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift);
private:	// User declarations
void RemoveTabs();
void CreateTabs();

public:		// User declarations
String path;
	__fastcall TSettingsWn(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSettingsWn *SettingsWn;
//---------------------------------------------------------------------------
#endif
