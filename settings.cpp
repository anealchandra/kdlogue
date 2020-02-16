//---------------------------------------------------------------------------
#include <fmx.h>
#pragma hdrstop
#include "main.h"
#include "settings.h"
#include "login.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TSettingsWn *SettingsWn;
//---------------------------------------------------------------------------
__fastcall TSettingsWn::TSettingsWn(TComponent* Owner)
	: TForm(Owner)
{
char pathDelim = (char)System::Ioutils::TPath::DirectorySeparatorChar;
path = GetHomePath() + pathDelim + "Documents" + pathDelim + "KDLog" + pathDelim;
if (FileExists(path + "DALParameters.txt")) ParamList->Lines->LoadFromFile(path + "DALParameters.txt");
else {
	LoginWn->WriteToLog("No settings file exists at " + path + " so you will need to create relevent parameters to run the DAL operations");
	ForceDirectories(path);
}
if (FileExists(path + "DALTabs.txt")) TabList->Lines->LoadFromFile(path + "DALTabs.txt");
else {
	LoginWn->WriteToLog("No tab list file exists at " + path + " so you will need to subdivide DAL operations");
	ForceDirectories(path);
}
CreateTabs();
}
//---------------------------------------------------------------------------
void TSettingsWn::CreateTabs()
{
for (int i = 0; i < TabList->Lines->Count; i++) {
	TTabItem* tab = new TTabItem(MainWn->TabControl1);
	tab->Parent = MainWn->TabControl1;
	tab->Text = TabList->Lines->Strings[i];
	TTreeView* tr = new TTreeView(tab);
	tr->Sorted = true;
	tr->OnClick = MainWn->TreeView1Change;
	tr->Parent = tab;
	tr->Width = 270;
	tr->Align = TAlignLayout::alLeft;
}
}
//---------------------------------------------------------------------------
void TSettingsWn::RemoveTabs()
{
while (MainWn->TabControl1->TabCount) {
	TTabItem* tab = (TTabItem*) MainWn->TabControl1->Tabs[0];
    //memory leak?
	tab->Parent = NULL;
	delete tab;
}
}
//---------------------------------------------------------------------------
void __fastcall TSettingsWn::SaveButClick(TObject *Sender)
{
//String path = ExtractFilePath(ParamStr(0));
ParamList->Lines->SaveToFile(path + "DALParameters.txt");
TabList->Lines->SaveToFile(path + "DALTabs.txt");
Close();
}
//---------------------------------------------------------------------------
void __fastcall TSettingsWn::Button1Click(TObject *Sender)
{
RemoveTabs();
CreateTabs();
String s = "list/operation";
bool success = LoginWn->DoQuery(s, true);
if (!success) {
	return;
}
LoginWn->FillTable(s);
}
//---------------------------------------------------------------------------
void __fastcall TSettingsWn::TabListKeyDown(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift)
{
Button1->Visible = true;
}
//---------------------------------------------------------------------------

