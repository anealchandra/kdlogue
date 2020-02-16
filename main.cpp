//---------------------------------------------------------------------------
#include <fmx.h>
#pragma hdrstop
#include "main.h"
#include "settings.h"
#include "login.h"
#include "unit1.h"
#include "unit4.h"
#include "unit5.h"
#include "wizard.h"
#include "ScanWn.h"
#include "wizard.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TMainWn *MainWn;
//authored by Aneal Chandra 30 July 2013
//An exploratory cross platform (OSX, Win32) DAL testing program that lists DAL functions in a tree
//Each DAL function can be called by clicking it on the tree
//Settings can be added from the Settings Window, e.g. by entering "_id=1"
//down the track we need to focus on operations relating to item, storage, specimen and genotype, ie which item is in the box!
//for issues on speed of controls  https://forums.embarcadero.com/thread.jspa?threadID=105855&tstart=15
//---------------------------------------------------------------------------
__fastcall TMainWn::TMainWn(TComponent* Owner)
	: TForm(Owner)
{
KDVersion = "2.2.4.2";    //+ .xxx
Application->Title = "DArT KDLogue v" + KDVersion;//first 3 digits belong to Puthick's kddart
LabelDaRT->Text = " (c) 2014, Diversity Arrays Technology DArT, Australia";
#ifdef _Windows
if (!TOSVersion::Check(6, 1)) {
	if (!TOSVersion::Check(6, 3)) {
		if (mrYes != MessageDlg("KDLogue has been designed for Windows 7 and Windows 8.1 only. You are attempting to run it on an untested Operating System, risking data corruption. Do you want to continue anyway?",
			TMsgDlgType::mtWarning, TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo, 0)) {
			exit(0);
		}
	}
}
#endif
}
//---------------------------------------------------------------------------
void TMainWn::addTreeNode(IXMLNode *xmlNode, TTreeViewItem *treeNode)
{
/*for (int i = 0; i < TabControl1->TabCount; i++) {
	TTabItem* tab = (TTabItem*) TabControl1->Tabs[i];
	((TTreeView*)tab->Children->Items[1]->Children->Items[0])->BeginUpdate();
} */
IXMLNode *xNode;
TTreeViewItem *tNode = treeNode;
IXMLNodeList *xNodeList;
if (xmlNode->HasChildNodes) { //The current node has children
	xNodeList = xmlNode->ChildNodes;
	for (int x = 0; x <= xNodeList->Count - 1; x++)   { //Loop through the child nodes
	xNode = xmlNode->ChildNodes->operator [](x) ;

	String operationstr = "";
	if (xNode->HasAttribute("REST")) {   //list/operation only!
		operationstr+= xNode->GetAttribute("REST");
		TTreeViewItem *tNode = new TTreeViewItem(this);
		tNode->Text = operationstr;
		//find the correct tab to add the function call to
		TTabItem *tab = NULL;
		bool found = false;
		for (int i = 0; i < TabControl1->TabCount; i++) {
			tab = (TTabItem*) TabControl1->Tabs[i];
			String smallstr = operationstr;
			//loop to find the appropriate tab anywhere in the operation text
			while (smallstr.Pos("/")!=0 && !found) {
				if (tab->Text == smallstr.SubString(1, smallstr.Pos("/") - 1)) {
					found = true;
					break;
				}
				//split operation string by '/'
				smallstr = smallstr.SubString(smallstr.Pos("/") + 1, smallstr.Length());
			}
			if (found) {
				break;
			}
		}
		//add the node to the relevent tab - using last tab if not found
		tNode->Parent = tab->Children->Items[1]->Children->Items[0];
	}
	else {
		IXMLNodeList *xn = xNode->GetAttributeNodes();    //get attribute name
        bool ignoreT = false;
		for (int xa = 0; xa <= xn->Count - 1; xa++)   { //Loop through the child nodes
			IXMLNode *aNode = xn->operator [](xa) ;
			String val = "";  //?
			try {
				if (aNode->GetText()!="") {
					val = aNode->GetNodeValue();
				}
			}
			catch (const Exception& E) { //eg list/specimengroup
				val = E.Message;
			}
			String title = aNode->GetNodeName();
			if (title == "Page" || title == "TagName") {  //we don't care for this tag
				ignoreT = true;
				break;
			}
			operationstr += title + "=" + val + "  ";
			TStringColumn *col = NULL;  //could store results in a map of vectors but for now just use stringgrid component
			bool found = false;
			for (int i = 0; i < StringGrid1->ColumnCount; i++) {
				col = (TStringColumn*)StringGrid1->ColumnByIndex(i);
				if (col->Header == title) {
					found = true;
					break;
				}
			}
			if (!found) {//first time through so lets add the column titles
				//note - order may change.
				col = new TStringColumn(StringGrid1);
				col->Parent = StringGrid1;
				col->Header = title;

			}
			StringGrid1->RowCount = rowcount + 1;
			StringGrid1->Cells[col->Index][rowcount] = val;

		}
		if (!(xNode->ChildNodes)->Count) {
			if (!ignoreT) {
				rowcount++;
			}
		}
		TTreeViewItem *tNode = new TTreeViewItem(this);
		tNode->Text = operationstr;
		tNode->Parent = treeNode;
	}
	//recursively add children to tnode
	addTreeNode(xNode, tNode);
	}
}
else ;//No children, so add the outer xml (trimming off whitespace)
//treeNode.Text = xmlNode.OuterXml.Trim();
/*for (int i = 0; i < TabControl1->TabCount; i++) {
	TTabItem* tab = (TTabItem*) TabControl1->Tabs[i];
	((TTreeView*)tab->Children->Items[1]->Children->Items[0])->EndUpdate();
} */
}
//---------------------------------------------------------------------------
void __fastcall TMainWn::LoginButClick(TObject *Sender)
{
LoginWn->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainWn::TreeView1Change(TObject *Sender)
{
TTreeViewItem *Node = ((TTreeView*)Sender)->Selected;
AniIndicator1->Visible = true;
Application->ProcessMessages();
if (Node->Level() == 1) {   //lets substitute the _setting for its appropriate value
	//ShowMessage(Application->Name);
	String Str = Node->Text;// + filterstr;
	TStringList *bits = new TStringList();
    bits->Delimiter =  '/';
    bits->DelimitedText = Str;
	for (int i = 0; i < bits->Count; i++) {  //iterate over all the string pieces
		if (bits->Strings[i].Pos("_") == 1) { //the string piece starts with a underscore, so lets find its setting value
			if (!SettingsWn->ParamList->Lines->Count) {
				TTreeViewItem *t = new TTreeViewItem(this);
				t->Text = bits->Strings[i] + " not declared in settings";
				t->Parent = Node;
				t->ParentItem()->Expand();
				AniIndicator1->Visible = false;
				return;
			}
			int j;
			for (j = 0; j < SettingsWn->ParamList->Lines->Count; j++) {
				if (SettingsWn->ParamList->Lines->Strings[j].SubString(0,bits->Strings[i].Length()) == bits->Strings[i]) {//it exists in the settings so lets extract the val
					String strVal = SettingsWn->ParamList->Lines->Strings[j].SubString(bits->Strings[i].Length() + 2, SettingsWn->ParamList->Lines->Strings[j].Length());
					if (strVal.IsEmpty()) {
						TTreeViewItem *t = new TTreeViewItem(this);
						t->Text = bits->Strings[i] + " not defined in settings";
						t->Parent = Node;
						t->ParentItem()->Expand();
						AniIndicator1->Visible = false;
						return;
					}
					else {
						bits->Strings[i] = strVal;
						break;
					}
				}
				if (j == SettingsWn->ParamList->Lines->Count - 1) {
					TTreeViewItem *t=new TTreeViewItem(this);
					t->Text = bits->Strings[i] + " not declared in settings";
					t->Parent = Node;
					t->ParentItem()->Expand();
					AniIndicator1->Visible = false;
					return;
				}
			}
		}
	}
	//if we get this far all is ok so lets proceed to DAL query
	String pstring = "";
	for (int i = 0; i < bits->Count; i++) {
		pstring += bits->Strings[i];
		if (i != bits->Count - 1) pstring += "/";
	}
	bits->Clear();
	delete bits;
	rowcount=0;

   //	filterstr = "";
	//eg
	//http://kddart.diversityarrays.com/dal/list/item/100/page/1?Filtering=StorageId=127
	//http://kddart.diversityarrays.com/dal/list/item/100/page/1?Filtering=ItemNote like 'Testing'%26StorageId=127

	bool success = LoginWn->DoQuery(pstring, true);
	if (!success) {
		return;
	}
	LoginWn->FillTable(pstring);
	/*if (Node->Text.Pos("/_nperpage/page/_num")) {//it can be filtered!
		TabControl2->Tabs[2]->Enabled = true;
		Fieldnamelist->Clear();
		for (int i = 0; i < StringGrid1->ColumnCount; i++) {
			TStringColumn* col = (TStringColumn*)StringGrid1->ColumnByIndex(i);
			Fieldnamelist->Items->Add(col->Header);
		}
	}
	else {
		TabControl2->Tabs[2]->Enabled = false;
	}*/

	Node->Expand();

}
 //the following is test code for demonstrating adding a grid to the tree
/*else if (Node->Level() == 3) {  //only possible if post has already finished
//parse returned data into a stringlist based on spaces
	TStringGrid *g = new TStringGrid(TableWn);
	g->RowCount = Node->Count + 1;
	for (int j = 0; j < Node->Count; j++) {
		//int j = 1;
		String Str = Node->Items[j]->Text;
		//parse each "xyz=323" into (key,value)
		int equal = Str.Pos("=");
		String piece = Str.SubString(equal,Str.Length());
		int equal2 = piece.Pos("=");
		TStringList *bits = new TStringList();
		bits->Delimiter =  '=';
		bits->DelimitedText = Str;



		//add to stringgrid
		for (int i = 0; i < bits->Count; i++) {  //iterate over all the string pieces
			 if (!j) {
				TStringColumn *Col = new TStringColumn(g);
				Col->Parent = g;

			 }
			 g->Cells[i][j] = bits->Strings[i];
		}
		delete bits;


	}
	 Visible = true;
	 g->Parent = TableWn;
	 g->Align = 9;//alClient;
	 g->Visible = true;
	 g->SetFocus();
} */
AniIndicator1->Visible = false;
//stuff to reset the scroll bar position
//StringGrid1->ShowScrollBars = false;
   //	StringGrid1->TopRow=0;
//	StringGrid1->ColumnIndex=0;
//	StringGrid1->Selected=0;
//StringGrid1->ShowScrollBars = true;

}
//---------------------------------------------------------------------------
void __fastcall TMainWn::FormActivate(TObject *Sender) {
if(!first_activated) {
	//for kdlog we are not logging in immediately
	//LoginWn->LoginButClick(0);

	KDLogButClick(Sender);
	Hide();

	if (!WizardWn->Visible) Form1->SetProgressBar();
	//Form1->Label1->Parent = NULL;
	//Form1->Label1->Parent = Form1->ProgressBar1;
	//Form1->CancelBut->Parent = NULL;
	//Form1->CancelBut->Parent = Form1->StatusBar1;
	//Form1->AniIndicator1->Parent = NULL;
	//Form1->AniIndicator1->Parent = Form1->StatusBar1;
	//for kdlog we want to hide the main window
	//doesn't work on APPLE (todo try setting window state to minimized, OnShowEvent, _DEBUG precompiler directives)
	//Form1->BringToFront();
}

}
//---------------------------------------------------------------------------
void __fastcall TMainWn::SettingsButClick(TObject *Sender) {
SettingsWn->Show();
}
//---------------------------------------------------------------------------
void __fastcall TMainWn::ScaleTrackChange(TObject *Sender)
{// change scale
  Layout1->Scale->X = ScaleTrack->Value;
  Layout1->Scale->Y = ScaleTrack->Value;
  TextScale->Text = IntToStr((int)(ScaleTrack->Value * 100)) + "%";
}
//---------------------------------------------------------------------------
void __fastcall TMainWn::FormResize(TObject *Sender) {
if(!first_activated) {
	char pathDelim = (char)System::Ioutils::TPath::DirectorySeparatorChar;
	String path = GetHomePath() + pathDelim + "Documents" + pathDelim + "KDLog" + pathDelim;
	if (FileExists(path + "settings3.txt")) {
		TStringList *settings3 = new TStringList(this);
		settings3->LoadFromFile(path + "settings3.txt");
		TSize sz = Screen->Size();

		int height = settings3->Strings[0].ToIntDef(sz.Height);
		if (height == 0) {
			height = sz.Height;
		}
		int width = settings3->Strings[1].ToIntDef(sz.Width);
		if (width == 0) {
			width = sz.Width;
		}
		if (settings3->Count > 6) {
			int left = settings3->Strings[5].ToIntDef(0);
			int top = settings3->Strings[6].ToIntDef(0);
			Left = (left) + (width - Width)/2;
			Top = (top) + (height - Height)/2;
		}

	}
}
TabControl2->Width = Width - 270;
TabControl2->Height = Height - StatusBar1->Height - 75;
StringGrid1->Width = TabControl2->Width - 20;
StringGrid1->Height = TabControl2->Height - 20;
}
//---------------------------------------------------------------------------
void __fastcall TMainWn::StringGrid1DblClick(TObject *Sender)
{
int col = StringGrid1->ColumnIndex;
int row = StringGrid1->Selected;
String searchtxt = StringGrid1->Cells[col][row];
DeleteBut->Enabled = false;
	if (searchtxt.Pos("delete/") != 0) {
	DeleteBut->Enabled = true;
}
/*else if (searchtxt.Pos("update/") != 0) {
	UpdateBut->Enabled = true;
}*/
}
//---------------------------------------------------------------------------
void __fastcall TMainWn::KDLogButClick(TObject *Sender) {
Form1 = new TForm1(this);
Form1->ProgressBar1->Align = TAlignLayout::alMostBottom;
Form1->ProgressBar1->Parent = Image1;

if (FileExists(SettingsWn->path + "settings3.txt")) {
	TStringList *settings3 = new TStringList(this);
	settings3->LoadFromFile(SettingsWn->path + "settings3.txt");
	Form1->Height = settings3->Strings[0].ToIntDef(Form1->Height);
	Form1->Width = settings3->Strings[1].ToIntDef(Form1->Width);
	Form1->TreeView1->Width = 1;//settings3->Strings[2].ToIntDef(Form1->TreeView1->Width);
	if (settings3->Count > 4) {
		Form1->Layout1->Scale->X = settings3->Strings[3].ToDouble();
		Form1->Layout1->Scale->Y = settings3->Strings[4].ToDouble();
		if (settings3->Count > 6) {
			Form1->Left = settings3->Strings[5].ToIntDef(0);
			Form1->Top = settings3->Strings[6].ToIntDef(0);
		}
		LoginWn->Scale();
		Form4->Scale();
		WizardWn->Scale();
		Scan->Scale();
	}
	delete settings3; settings3 = NULL;
}
WizardWn->treeWidth = Form1->TreeView1->Width;
bool login = false;
int tabIndex = -1;
int language = 0;
if (FileExists(SettingsWn->path + "settings4.txt")) {
	TStringList *settings4 = new TStringList(this);
	settings4->LoadFromFile(SettingsWn->path + "settings4.txt");
	login = settings4->Strings[0].ToIntDef(0);
	tabIndex = settings4->Strings[1].ToIntDef(0);
	if (settings4->Count > 2) {
		language = settings4->Strings[2].ToIntDef(0);
		if (settings4->Count > 3) LoginWn->DALBase->Text = settings4->Strings[3];
	}
	delete settings4; settings4 = NULL;
}

if (Form1->LoadTranslations()) {
	Form1->TranslationCombo->ItemIndex = language;
}
else {
	Form1->TranslationCombo->Enabled = false;
}
Form1->pstartpos = 30;
#ifndef _DEBUG
Form1->TabControl1->Visible = false;
Form1->TabControl2->Parent = Form1->Layout1;
Form1->ToolBar3->Parent = Form1->Layout1;
#endif

#ifdef _DEBUG
Form1->GroupBox4->Visible = true;
Form1->GroupBox2->Visible = true;
Form1->Update1->Visible = true;
Form1->UpdateAll->Visible = true;
Form1->pstartpos = 90;
#endif
Form1->Show();
//launch->Show();
BringToFront();
Form1->DoStart();
if (tabIndex == -1) {
	//Set the first enabled tab as active
	int z;//Set the first enabled tab as active
	for (z = 0; z < Form1->TabControl1->TabCount; z++) {
		if (Form1->TabControl1->Tabs[z]->Enabled) {
			break;
		}
	}
	Form1->TabControl1->TabIndex = z;
}
else {
	Form1->TabControl1->TabIndex = tabIndex;
}
//if (Form1->TabControl1->Tabs[Form1->TabControl1->TabIndex]->Text == "Trials") {
	Form1->TabControl1Change(0);
//}

if (login) {
	Form1->LogStatButClick(Sender);
}
Form1->LoadResButClick(0);
if (FileExists(LoginWn->logFileName)) {
	Form1->log->Items->LoadFromFile(LoginWn->logFileName);
	if (Form1->log->Count > 5000) { //only keep the last 5000 lines otherwise slow startup
		Form1->ClearLogButtonClick(0);
	}
}
LoginWn->WriteToLog(Application->Title + " Application Started");
Form1->TabControl2Change(0);
first_activated = true;
if (FileExists(Form1->updateFile)) {//should only happen if the software crashed during the wizard
	if (FileExists(SettingsWn->path + "tmppos.txt")) {
		TStringList *tmpos = new TStringList(this);
		tmpos->LoadFromFile(SettingsWn->path + "tmppos.txt");
		if (mrYes == MessageDlg("KDLogue has recovered trial " + tmpos->Strings[0] +
		". Do you want continue working with the trial data? (hint: to restore the trial click 'YES' and then when closing the wizard choose 'YES' to save changes)", TMsgDlgType::mtConfirmation,
			TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
			WizardWn->somethingChanged = true;
			WizardWn->trialno = tmpos->Strings[0].ToIntDef(0);
			WizardWn->currentpage = tmpos->Strings[1].ToIntDef(0);
			WizardWn->SetTState(tmpos->Strings[2].ToIntDef(-1));
			Form1->WizardButClick(0);
		}
		else {
			Form1->ClearShortButClick(0);
			DeleteFile(SettingsWn->path + "tmppos.txt");
		}
		delete tmpos; tmpos = NULL;
	}
	else { //currently can only happen if the program crashes during printing
		Form1->ClearShortButClick(0);
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TMainWn::DeleteButClick(TObject *Sender)
{ //working!
DeleteBut->Enabled = false;
//TButton *b = reinterpret_cast<TButton*>(Sender);
//String s = b->Text;//"delete/genus/582";
int col = StringGrid1->ColumnIndex;
int row = StringGrid1->Selected;
String s = StringGrid1->Cells[col][row];
LoginWn->data->Clear();
String randStr = String(Random(MaxInt));
LoginWn->data->Values["rand_num"] = randStr;
LoginWn->data->Values["url"] = LoginWn->DALBase->Text + s;

LoginWn->concatPString = "";;//order
LoginWn->concatVString = "";

LoginWn->data->Values["param_order"] = LoginWn->concatPString;
String sig = LoginWn->getSignature(LoginWn->DALBase->Text + s, randStr, LoginWn->concatVString, String(""));
LoginWn->data->Values["signature"] = sig;

LoginWn->DoQuery(s, false);
//refresh the table
TTabItem* tab = (TTabItem*) TabControl1->Tabs[TabControl1->TabIndex];
TreeView1Change((TTreeView*)tab->Children->Items[1]->Children->Items[0]);
}
//---------------------------------------------------------------------------

