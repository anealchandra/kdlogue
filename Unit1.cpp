//---------------------------------------------------------------------------
#include <fmx.h>
#pragma hdrstop

#include "Unit1.h"
#include "main.h"
#include "settings.h"
#include "login.h"
#include "unit3.h"
#include "unit4.h"
#include "unit5.h"
#include "unit7.h"
#include "print.h"
#include "sample.h"
#include "prepare.h"
#include "ScanWn.h"
#include "wizard.h"
#include "unit8.h"
#include <map>
//#include "thread.h"
#include <process.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "Winsoft.FireMonkey.Barcode"
#pragma resource "*.fmx"
TForm1 *Form1;
//---------------------------------------------------------------------------
int __fastcall mySort(TStringList *List, int Index1, int Index2) { //todo sort depending on column type
int Result;
if (List->CaseSensitive) { //hack for TDatetime columns
	//so lets convert the strings to datetime object
	TFormatSettings fmt;
	fmt.ShortDateFormat= "yyyy-MM-dd";
	fmt.DateSeparator  = '-';
	fmt.LongTimeFormat = "HH:mm:ss";
	fmt.TimeSeparator  = ':';
	try {//workaround for Redmine #1183
		if (List->Strings[Index1] == "0000-00-00 00:00:00" ) {
			Result = 0;
		}
		else if (List->Strings[Index2] == "0000-00-00 00:00:00") {
			Result = 1;
		}
		else {
			TDateTime d1 = StrToDateTime(List->Strings[Index1], fmt);
			TDateTime d2 = StrToDateTime(List->Strings[Index2], fmt);
			Result = d1 < d2;
		}
	}
	catch(EConvertError &e) {
		Result = 0;
	}
}
else {
	char *p1, *p2;
	double d1 = std::strtod(AnsiString(List->Strings[Index1]).c_str(), &p1 );
	double d2 = strtod(AnsiString(List->Strings[Index2]).c_str(), &p2 );
	if (*p1 == 0 && *p2 == 0) {
		Result = d1 < d2;
	}
	else {
		Result = CompareStr(List->Strings[Index1], List->Strings[Index2]);
	}
}
if (!List->WriteBOM) {
	Result = -Result;
}
return Result;
}
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner): TForm(Owner)   {
Caption = Application->Title;
selButton = NULL;
itemsTBU = 0;
UseCase = NULL;
ignore = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)  {
//check that a treefile doesn't aleady exist and if it does load it and enable the browse.
updateFile = SettingsWn->path + "KDLoguetemp.txt";
if (FileExists(SettingsWn->path + "styles.txt")) {
	TStringList *asettings = new TStringList(this);
	asettings->LoadFromFile(SettingsWn->path + "styles.txt");
	cbStyles->Items->Assign(asettings);
	delete asettings; asettings = NULL;
}
if (FileExists(SettingsWn->path + "settings5.txt")) {
	TStringList *settings5 = new TStringList(this);
	settings5->LoadFromFile(SettingsWn->path + "settings5.txt");
	int styleIndex = settings5->Strings[0].ToIntDef(-1);
	cbStyles->ItemIndex = styleIndex;
}
//seedprep = true; //load this from the shortlist as it tells us the shortlist type (could change the all text shortlist to entrylist)
}
//---------------------------------------------------------------------------
bool TForm1::LoadTranslations() {
try {
	String fname = SettingsWn->path + "translations6.csv";
	TMemo* m = new TMemo(this);
	if (FileExists(fname)) {//todo check for locking eg in MS-Excel
		m->Lines->LoadFromFile(fname);
		String dataS = m->Lines->Strings[0];
		int p = dataS.Pos(",");
		dataS = dataS.SubString(p + 1, dataS.Length() - p);
		TranslationCombo->Clear();
		p = dataS.Pos(",");
		while (p) {
			TranslationCombo->Items->Add(dataS.SubString(0, p - 1));
			dataS = dataS.SubString(p + 1, dataS.Length() - p);
			p = dataS.Pos(",");
		}
		TranslationCombo->Items->Add(dataS);
		//init translations

		for (int i = 0; i < TranslationCombo->Count; i++) {
			translationList[i] = new TStringList();
		}
		for (int j = 1; j < m->Lines->Count; j++) {
			String dataS = m->Lines->Strings[j];
			int p = dataS.Pos(",");
			String name = dataS.SubString(0, p - 1);

			dataS = dataS.SubString(p + 1, dataS.Length() - p);
			p = dataS.Pos(",");
			int count = 0;
			while (p) {
				translationList[count]->Values[name] = dataS.SubString(0, p - 1);
				dataS = dataS.SubString(p + 1, dataS.Length() - p);
				p = dataS.Pos(",");
				count++;
			}
			translationList[count]->Values[name] = dataS;
		}
		delete m; m = NULL;
		return true;
	}
	else {
		TTextControl* lab = NULL;
		String controlValue, controlName;
		m->Lines->Add("Name, English, Spanish, Italian");
		for (int i = 0; i < ComponentCount; i++) {
			lab = dynamic_cast<TTextControl*>(Components[i]);
			TComboBox *cb =dynamic_cast<TComboBox*>(Components[i]);


			if (lab && !cb) {
				if (!lab->Name.IsEmpty()) {
				controlName  = lab->Name;
				controlValue = lab->Text;
				m->Lines->Add(controlName + "," + controlValue);
				}
			}
		}
		for (int i = 0; i < WizardWn->ComponentCount; i++) {
			lab = dynamic_cast<TTextControl*>(WizardWn->Components[i]);
			if (lab) {
				if (!lab->Name.IsEmpty()) {
					controlName  = lab->Name;
					controlValue = lab->Text;
					m->Lines->Add(controlName + "," + controlValue);
				}
			}
		}
		m->Lines->SaveToFile(fname /*+ ".debug"*/);
		delete m; m = NULL;
		return false;
	}
}
catch (...) {
	return false;
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TranslationComboChange(TObject *Sender) {
//populate the ControlName and ValueString into TStringList;
TTextControl *lab = NULL;
String controlValue, controlName;
String language = TranslationCombo->Items->Strings[TranslationCombo->ItemIndex];
Label1->Text = "Please wait...converting to" + language;
//bool NoTranslate = (TranslationCombo->Items->Strings[0] == "English only");
Application->ProcessMessages();
for (int i = 0; i < ComponentCount; i++) { //loop for each control on the form
	lab = dynamic_cast<TTextControl*>(Components[i]);
	if (lab) {
		/*if (NoTranslate) {
			TComboBox *combo = reinterpret_cast<TComboBox*>(lab->ParentControl);
			if (!combo) { //temp fix for 'English only' appearing in the comboboxes - todo find out why?
				controlName  = lab->Name;
				controlValue = translationList[TranslationCombo->ItemIndex]->Values[controlName];
				if (!controlValue.IsEmpty()) lab->Text = controlValue;
			}
		}
		else */ {
			controlName  = lab->Name;
			controlValue = translationList[TranslationCombo->ItemIndex]->Values[controlName];
			if (!controlValue.IsEmpty()) lab->Text = controlValue;
		}
	}
}
for (int i = 0; i < WizardWn->ComponentCount; i++) { //loop for each control on the form
	lab = dynamic_cast<TTextControl*>(WizardWn->Components[i]);
	if (lab) {
		/*if (NoTranslate) {
			TComboBox *combo = reinterpret_cast<TComboBox*>(lab->ParentControl);
			if (!combo) { //temp fix for 'English only' appearing in the comboboxes - todo find out why?
				controlName  = lab->Name;
				controlValue = translationList[TranslationCombo->ItemIndex]->Values[controlName];
				if (!controlValue.IsEmpty()) lab->Text = controlValue;
			}
		}
		else */{
			controlName  = lab->Name;
			controlValue = translationList[TranslationCombo->ItemIndex]->Values[controlName];
			if (!controlValue.IsEmpty()) lab->Text = controlValue;
		}
	}
}
Label1->Text = "Language set to" + language;
SaveLoggedStatus();
}
//---------------------------------------------------------------------------
void TForm1::DoStart() {
AddCheckBoxes();
sNameBox->Clear();
for (int i = 0; i < iNameList->Lines->Count; i++) {
	String sid = iNameList->Lines->Strings[i];
	sNameBox->Items->Add(sid);
}
StSet->Items = sNameList->Lines;
ItSet->Items = iNameList->Lines;
ItSort->Items = iNameList->Lines;
ShSort->Items = iNameList->Lines;
ItSort->ItemIndex = 0;

if (FileExists(SettingsWn->path + "settings2.txt")) {
	TStringList *settings2 = new TStringList(this);
	settings2->LoadFromFile(SettingsWn->path + "settings2.txt");
	if (settings2->Strings[0] == "-1") settings2->Strings[0] = 1;
	StSet->ItemIndex = settings2->Strings[0].ToIntDef(1);
    if (settings2->Strings[1] == "-1") settings2->Strings[1] = 4;
	ItSet->ItemIndex = settings2->Strings[1].ToIntDef(4);
	int count = 2;
	for (int i = 0; i < GroupBox4->ChildrenCount; i++) {
		TCheckBox *c = dynamic_cast<TCheckBox*>(GroupBox4->Children->Items[i]);
		if (c) {
			c->IsChecked = settings2->Strings[count].ToIntDef(1);
			count++;
		}
	}
	delete settings2; settings2 = NULL;
}
else {
	StSet->ItemIndex = 1; 
	ItSet->ItemIndex = 4;
}

//extract Name and populate radiobuttons
SeSettings->Height = (iNameList->Lines->Count + 1) * 18 + 5;

AddDALOperators(sOpBox);
FormResize(0);
LoadTables();
if (FileExists(updateFile) && FileExists(SettingsWn->path + "tmppos.txt")) {//should only happen if the software crashed during the wizard
	Label1->Text = "Recovering shortlist...";
	Application->ProcessMessages();
	LoadNow(0);
	ClearShortBut->Enabled = true;
	SaveShortBut->Enabled = true;
}
else {
	Label1->Text = "Initialising KDLogue...";
	Application->ProcessMessages();
	ShortlistTab->Enabled = false;
	ClearShortBut->Enabled = false;
	SaveShortBut->Enabled = false;
}
SearchTab->Enabled = false;  //we are not logging in at startup

for (TSymbology symbology = syCode11; symbology <= syGridMatrix; symbology = symbology + 1) {
	ComboBoxSymbology->Items->Add(Barcode1->SymbologyNames[symbology]);
}
//ComboBoxSymbology->Sorted = true;
Form1->ComboBoxSymbology->ItemIndex = Form1->ComboBoxSymbology->Items->IndexOf("QR Code");
//set the first items to be ON
sName[0] = sNameBox;
sOperator[0] = sOpBox;
sField[0] = ComboBox1;
sValue[0] = valEdit;

//sNameBox->ItemIndex = 0;
//sOpBox->ItemIndex = 0;

}
//---------------------------------------------------------------------------
void __fastcall TForm1::Update1Click(TObject *Sender) {
if (Sender) {
	if (!Login()) return;
}
String s;
if (selButton) {
	if (selButton->Color == TAlphaColorRec::Green || selButton->Color == TAlphaColorRec::Yellow) {  //only update those requiring an update!
		return;
	}
	else if (selButton->Color == TAlphaColorRec::White || selButton->Color == TAlphaColorRec::Black) {
		NodeData *d = (NodeData*)(TreeView1->Selected->Data);
		TStringList *l = d->final;
		if (!l->Values["ItemGroupId"].IsEmpty()) {
			Label1->Text = "Adding "+TreeView1->Selected->Text.TrimLeft() + " cancelled - envelope for planting";
			return; 	//temp prevent the user from updating a subsampled item
		}
	}
}
else {//only continue if a new storage location needs to be created
	if (TreeView1->Selected->Text != "new") {
		return;
	}
}

TStringList *upitemlist = new TStringList();
TStringList *l;

NodeData *d = (NodeData*)(TreeView1->Selected->Data);
l = d->final;
String iid = getTreeStrFromObject(TreeView1->Selected, "ItemId");
if (selButton->Color == TAlphaColorRec::Black) {//delete
	s = "delete/item/" + iid;
}

else {//must check for any new fields and add them if found
	//iterate over all the field combos to find the item's field with the same value
	TStringList *functionCalls = new TStringList();
	functionCalls->Values["ContainerTypeId"] = "add/type/container";
	functionCalls->Values["ItemTypeId"] = "add/type/item";
	functionCalls->Values["StorageId"] = "add/storage";
	functionCalls->Values["ItemStateId"] = "add/type/state";
	functionCalls->Values["SpecimenId"] = "add/specimen";
	functionCalls->Values["ItemSourceId"] = "add/contact";
	functionCalls->Values["ItemUnitId"] = "add/itemunit";
	functionCalls->Values["ScaleId"] = "add/deviceregistration";
	//functionCalls->Values["TrialUnitSpecimenId"] = "?None available";

	TStringList *uplist = new TStringList();
	for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
		TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
		if (combo) {
			if (l->Values[combo->Name].IsEmpty()) {//item field has not been set
				if (combo->Tag) {  //and it is required
					ShowMessage("Required item field <" + combo->Name + "> has not been set. Update has been cancelled");
					SaveNow(0);
					combo->SetFocus();
					return;
				}
			}
			else {
				upitemlist->Add(combo->Name); //note - this is used later!
			}
			String search = l->Values[combo->Name];
			//find the field in the relevent combo
			if (search.ToIntDef(0) < 0) {// the item's field is a newly added so we need to add it to DAL first
				int index = -999;

				for (int q = 0; q < combo->Count; q++) {
					TStringList *s = (TStringList*)combo->Items->Objects[q];
					if (s->Values[getMapping(combo)] == search) {
						index = q;
						//lets determine whether the field in the combo
						break;
					}
				}
				if (index == -999) { //Note this is a bit redundant as the fields should always exist!
					ShowMessage("Error: The value " + search + " can't be found in " + combo->Name);
					return;
				}
				TButton *b = LoginWn->getButton(combo);
				TStringGrid *sg = (TStringGrid*)b->TagObject;
				TStringList *sl = (TStringList*)combo->Items->Objects[index];
				for (int i = 0; i < sg->RowCount ; i++) {
					String title = sg->Cells[GetCol("Name", sg)][i]; //note bug rowcount needs to be rowcount + 1;
					if (sl->Values[title].IsEmpty()) {
						if (sg->Cells[GetCol("Required", sg)][i] == "1") {
							ShowMessage("Required " + combo->Name + " field <" + title + "> has missing value! Update has been cancelled");
							SaveNow(0);
							combo->SetFocus();
							Label1->Text = "Select a value for " + combo->Name;
							return;
						}
					}
					else  {
						uplist->Add(title);
					}
				}
				s = functionCalls->Values[combo->Name];
				if (!LoginWn->DoUpdate(uplist, sl, s)) return;
				//if success change the appropriate identifier for the field from "-1"
				String newVal = LoginWn->ExtractValue("Value", s);
				sl->Values[getMapping(combo)] = newVal;
				l->Values[combo->Name] = newVal;
				//kludge to update combo text
				int pp = combo->ItemIndex;
				combo->ItemIndex = -1;
				combo->Items->Strings[pp] = sl->Values[combo->ListBoxResource];
				combo->ItemIndex = pp;
				Application->ProcessMessages();
				LoginWn->SaveCombo(combo);
				uplist->Clear();
				//iterate over all the items in the tree, and change any of the item details to the new identifier
				for (int r = 0; r < TreeView1->Count; r++) {
					NodeData *d = (NodeData*)TreeView1->Items->Objects[r];
					if (d->final->Values[combo->Name] == search ) {
						d->final->Values[combo->Name] = newVal;
					}
				}
			}
		}
	}
	delete functionCalls; functionCalls = NULL;
	delete uplist; uplist = NULL;
	if (!l->Values["Amount"].IsEmpty()) {
		upitemlist->Add("Amount");
	}
	if (!l->Values["ItemNote"].IsEmpty()) {
		upitemlist->Add("ItemNote");
	}
	if (!l->Values["ItemBarcode"].IsEmpty()) {
		upitemlist->Add("ItemBarcode");
	}
	else {
		ShowMessage("Required item field <ItemBarcode> has not been set. Update has been cancelled");
		SaveNow(0);
		ItemBarcode->SetFocus();
		return;
	}
	if (selButton->Color == TAlphaColorRec::Red || selButton->Color == TAlphaColorRec::White) {
		if (selButton->Color == TAlphaColorRec::White) {
			s = "add/item";
			upitemlist->Add("DateAdded");
			upitemlist->Add("AddedByUserId");
			//have to do this here as shortlisted items can be created before logging in!
			l->Values["AddedByUserId"] = LoginWn->UserID->Text;
		}
		else {
			s = "update/item/" + iid;

			if (d->final->Values["LastMeasuredDate"] != d->orig->Values["LastMeasuredDate"]) {
				upitemlist->Add("LastMeasuredDate");
				upitemlist->Add("LastMeasuredUserId");
				l->Values["LastMeasuredUserId"] = LoginWn->UserID->Text; //only record a new user if the measurement has been changed
			}
            //attempt some validation based on whether someone else has updated the item
			String validationDate = d->orig->Values["LastMeasuredDate"];
			if (!LoginWn->DoValidation(validationDate, iid)) return;
		}
	}
}
bool success = LoginWn->DoUpdate(upitemlist, l, s);
delete upitemlist; upitemlist = NULL;
if (!success) return;

if (selButton->Color == TAlphaColorRec::Black) {//delete
	Label1->Text = "Item "+ TreeView1->Selected->Text.TrimLeft() + " removed from database";  //todo memory leak?
	TreeView1->Selected->Parent = NULL;   //todo fix for updateall function (iterate backwards)
	//no node is selected so lets hide things
	ItemDetailBox->Visible = false;
	Delete->Visible =false;
}
else {
	if (selButton->Color == TAlphaColorRec::White) {// must update the id from the returned string
		String P = LoginWn->ExtractValue("Value", s);

		Label1->Text = "Item " + TreeView1->Selected->Text.TrimLeft() + " added to database";
		d->final->Values["LastMeasuredUserId"] = "-1"; //todo check whether this is correct. If amount has been set then the UserId and MeasuredDate probably needs to be set!
		d->final->Values["ItemId"] = P;
		TreeView1->Selected->Text = "        " + d->final->Values[GetItemLabelType()];
	}

	else {
		Label1->Text = "Item " + TreeView1->Selected->Text.Trim() + " updated in database";
	}
	/*if (d->orig->Values["StorageId"] != d->final->Values["StorageId"]) {//we need to move the new item in the tree
		TListBoxItem *v = TreeView1->Selected;
		v->Parent = NULL;
		//delete v; v = NULL;
		TListBoxItem *tNode = NULL;
		bool found = false;
		for (int i = 0; i < TreeView1->Count; i++) {
			tNode = TreeView1->Items[i];
			if (getStorageID(tNode) == d->final->Values["StorageId"]) {
				found = true;
				break;
			}
		}
		if (!found) {
			tNode = new TListBoxItem(this);
			TStringList *stl = new TStringList();
			stl->Assign(GetSearchComboObject(d->final->Values["StorageId"]));
			tNode->TagObject = stl;
			tNode->Text = stl->Values[GetStorageLabelType()];
			tNode->Parent = TreeView1;
		}
		v->Parent = tNode; //assumes that if the item has been moved we want to see where it has gone!
		tNode->Expand();
	} */
	selButton->Color = TAlphaColorRec::Green;
	for (int i = 0; i < d->orig->Count; i++) {//reset
		d->orig->Strings[i] = d->final->Strings[i];
	}
}

itemsTBU--;
Update1->Enabled = false;
UndoBut->Visible = false;
if (Sender != UpdateAll) {
	UpdateItemsTBU();
}
}
//---------------------------------------------------------------------------
void TForm1::UpdateItemsTBU() {
if (itemsTBU > 0) {
	UpdateAll->Enabled = true;
	Caption = Application->Title + " (Items modified = " + String(itemsTBU) + ")";
}
else {
	UpdateAll->Enabled = false;
	Caption = Application->Title;
}
SaveNow(NULL);
}
//---------------------------------------------------------------------------
bool TForm1::Login() {
if (!LogOut->Visible) { //must login first
	if (mrYes == MessageDlg("You are not logged in. Log in now?", TMsgDlgType::mtInformation,
			TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
		LogStatButClick(0);
	}
	else return false;
}
return true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateAllClick(TObject *Sender)  {
if (!Login()) return;
//UpdateAll->Enabled = false;
updateAll = true;
ProgressBar1->Max = itemsTBU;
TDateTime startTime = Now();
CancelBut->Visible = true;
for (int i = 0; i < TreeView1->Count && CancelBut->Visible; i++) {
	TListBoxItem* t = TreeView1->ItemByIndex(i);
	TColorButton* b =GetButton(t);
	if (b->Color != TAlphaColorRec::Green && b->Color != TAlphaColorRec::Yellow) {//do the upload
		ProgressBar1->Value++;
		t->IsSelected = true;  //select the node
		TreeView1Click(TreeView1); //ensure its selected
		Application->ProcessMessages();
		Update1Click(0);      //call the update function
	}
}
if (itemsTBU > 0) {
	ShowMessage("Some updates were not uploaded. Please try again later");
}
updateAll = false;
ProgressBar1->Value = 0;
CancelBut->Visible = false;
TDateTime elapsedTime = Now() - startTime;

unsigned short hour, min, sec, msec;
elapsedTime.DecodeTime(&hour, &min, &sec, &msec);
UpdateItemsTBU();
Label1->Text = String(ProgressBar1->Max - itemsTBU) + " items updated in " + String(min) + "m " + String(sec) + "s";
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AddClick(TObject *Sender) { //todo remove preset values below
//lets add a temp node to the tree
ItemDetailBox->Visible = true;
NodeData *d = new NodeData();
TStringList *l;
//collect the datetime right now
TDateTime dateNow = Now();
for (int i=0; i < 2; i++) {
	if (!i) l = d->orig;
	else l = d->final;
	if (Sender) {
		l->Values["StorageId"] = getTreeStrFromObject(TreeView1->Selected, "StorageId");
	}
	else {//temp for trial setup
		l->Values["Amount"] = "0";//String(Random(1000000)/100.0); //~!1 reenable for trial source item recreation
		//l->Values["ItemUnitId"] = "7";//~!1 reenable for trial source item recreation
		l->Values["ItemTypeId"] = "3";
		//l->Values["ContainerTypeId"] = "7";//~!1 reenable for trial source item recreation
		//l->Values["StorageId"] = "4"; //~!1 reenable for trial source item recreation
		//l->Values["ScaleId"] = "1"; //~!1 reenable for trial source item recreation
		//assign storageId to random location by counting the size of the storageId combo and setting it to one of these
    }
	String ds = dateNow.FormatString("yyyy-MM-dd HH:mm:ss");
	l->Values["DateAdded"] = ds;
	l->Values["ItemId"] = "new";
	l->Values["ItemBarcode"] = "|*I" + String((double)dateNow * 86400) + "*|";


	//	+ FormatFloat("000", Random(999));   //just a fake number for now
	//l->Values["ItemOperation"] = "subsample"; //todo remove this for DAL v2.1.2 +
}
TListBoxItem* v = AddChildNode(0, d);
itemsTBU++;
TColorButton *but = GetButton(v);
if (!but) {
	ShowMessage("button error");
	return;
}
but->Color =  TAlphaColorRec::White;
Application->ProcessMessages();
v->IsSelected = true;
if (!DontSaveYet) {
	Update1->Enabled = true;
	TreeView1Click(TreeView1);
	UpdateItemsTBU();
}
}
//---------------------------------------------------------------------------
TColorButton* TForm1::GetButton(TListBoxItem *v) {
TColorButton *but = NULL;
for (int i = 0 ; i <  v->ChildrenCount; i++ ) {
	but = dynamic_cast<TColorButton*>(v->Children->Items[i]); //dynamic casting always returns NULL if the types don't match
	if (but) {
		return but;
	}
}
return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TreeView1Click(TObject *Sender) {
if (!updateAll) CancelBut->Visible = false; //stop any dal querys
TListBoxItem *Node = ((TListBox*)Sender)->Selected;
if (selButton) {
	if (Node) {  //a previous item was selected and a new node has been clicked
		if (GetButton(Node) != selButton) {
			if (selButton->Color == TAlphaColorRec::Yellow) {
				 selButton->Color = TAlphaColorRec::Green;
			}
			TListBoxItem *it = (TListBoxItem*)selButton->Parent;
			it->IsSelected = false;
		}
	}
}
if (!Node) {
	for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
		TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
		if (combo) { //found a combobox
			combo->ItemIndex = -1;
			UpdateButtonGraphic(combo);
		}
	}
	//clear item results fields
	ItemBarcode->Text = "";
	Amount->Text = "";
	ItemNote->Lines->Clear();
	if (selButton) {
		if (selButton->Color == TAlphaColorRec::Yellow) {
			selButton->Color = TAlphaColorRec::Green;
		}
		selButton = NULL;
	}
	return;
}
Add->Visible = true;
ItemDetailBox->Visible = true;
NodeData *d = (NodeData*)Node->Data;
TStringList *l = d->final;
selButton = GetButton(Node);
UndoBut->Visible = true;
ItemDetailBox->Enabled = true;
Update1->Enabled = true;
String text = getTreeStrFromObject(Node, "ItemNote");

if (selButton->Color == TAlphaColorRec::Green) {  //is not modifed
	#ifdef _DEBUG
	selButton->Color = TAlphaColorRec::Yellow;
	#endif
	Update1->Enabled = false;
	Delete->Visible = true;
	UndoBut->Visible = false;
}
else if (selButton->Color == TAlphaColorRec::Black) {//locally deleted
	ItemDetailBox->Enabled = false;
	Delete->Visible = false;
}
else if (selButton->Color == TAlphaColorRec::White) {//locally added
	Delete->Visible = false;
}
else if (selButton->Color == TAlphaColorRec::Red) {//locally modified
	Delete->Visible = true;
}
/*if (l->Values["delete"].IsEmpty()) { //todo to enable delete functionality will need to add this field back in
	Delete->Enabled = false;
}
else*/ {
	Delete->Enabled = true;
}
ItemBarcode->Text = l->Values["ItemBarcode"];
Barcode1->InputText = ItemBarcode->Text;
ShowBarcode(ItemBarcode);
Amount->Text = l->Values["Amount"];
ItemNote->Text = l->Values["ItemNote"];

for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
	TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
	if (combo) { //found a combobox
		String index = combo->Name;
		ignore = true;
		combo->ItemIndex = getIndex(combo, l->Values[index]);
		ignore = false;
		combo->Repaint();
		UpdateButtonGraphic(combo);
	}
}
if (Form1->TreeView1->Parent == WizardWn->Rectangle5) {
	WizardWn->hpButton->Enabled = true;
	if (WizardWn->currentpage == 4) {
		if (selButton->Color == TAlphaColorRec::White) {
			WizardWn->hpButton->Enabled = false;
		}
	}
	/*if (WizardWn->currentpage == 5) {
		WizardWn->ShowSpec();
	} */
}

UpdateAll->Enabled = (itemsTBU > 0);
}
//---------------------------------------------------------------------------
int TForm1::getIdOfTitle(TComboBox *combo, String searchtext) {
for (int i = 0; i < combo->Count ; i++) {
	String item = combo->Items->Strings[i];

	if (combo != ItemOperation) {
		if (item.Trim() == searchtext.Trim()) {
			TStringList *l = (TStringList*)combo->Items->Objects[i];
			return (l->Values[getMapping(combo)].ToIntDef(-1));
        }
	}
}
return -1;
}
//---------------------------------------------------------------------------
int TForm1::getIndex(TComboBox *combo, String searchId) {
String id = getMapping(combo);
for (int i = 0; i < combo->Count ; i++) {
	String item = NULL;
	if (combo != ItemOperation) {
		TStringList *l = (TStringList*)combo->Items->Objects[i];
		item = l->Values[id];
	}
	else {
		item = combo->Items->Strings[i];
	}
	if (item.Trim() == searchId.Trim()) {
		return i;
	}
}
return -1;
}
//---------------------------------------------------------------------------
TListBoxItem* TForm1::AddChildNode(int pos, NodeData *data) {
if (!pos) TreeView1->BeginUpdate();
int z;
if (!pos)
z = TreeView1->Items->AddObject("        " + data->final->Values[GetItemLabelType()], (TObject*)data);
else {
	TreeView1->Items->InsertObject( pos, "        " + data->final->Values[GetItemLabelType()], (TObject*)data);
	z = pos;
}
TListBoxItem *y =  TreeView1->ItemByIndex(z);

TColorButton *c = new TColorButton(this);
c->Parent = TreeView1->ItemByIndex(z);
c->Width = 25;
c->Height = 20;
c->Position->X = 0;
c->Opacity = 0.5;
c->HitTest = false;
//t->Position->X = 15;
c->Color = TAlphaColorRec::Green;
//selButton = t; //possible bug

/*if (Node->IsExpanded) {//kludge to get tree to repaint
	TreeView1->EndUpdate();
	Node->Collapse();
	Node->Expand();
	TreeView1->BeginUpdate();
} */

if (!pos) TreeView1->EndUpdate();
return TreeView1->ItemByIndex(z);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DeleteClick(TObject *Sender) {
if (selButton->Color == TAlphaColorRec::Red) {//ignore items to be updated since update+delete = delete

}
else {
	itemsTBU++;
	UpdateItemsTBU();
}
selButton->Color = TAlphaColorRec::Black;
TreeView1Click(TreeView1);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ComboBox2Change(TObject *Sender) {
ToolBar2->Visible = (ComboBox2->ItemIndex != 0);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CancelButClick(TObject *Sender) {
CancelBut->Visible = false;
}
//---------------------------------------------------------------------------
void TForm1::InitSearch() {
SearchBut->Enabled = false;
AniIndicator1->Visible = true;
Label1->Text = "Searching for items...";
Application->ProcessMessages();
if (sg) {
	delete sg;
	sg = NULL;
}
ToolBar2->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SearchButClick(TObject *Sender) {
if (BarcodeI) {
	BarcodeI = false;
	BarcodeSearch = "";
}
if (Sender) SaveResButClick(0);
InitSearch();

SearchPage = 1;
operation = "";
for (int r = 0; r < GroupBox3->Tag; r++) {
	String searchOp = "";
	String searchName = "";
	String searchVal = "";
	if (sOperator[r]->ItemIndex != -1 && sName[r]->ItemIndex != -1 && sValue[r]->Text != "") {
		searchName = sName[r]->Items->Strings[sName[r]->ItemIndex];
		String add = "";
		if (sField[r]->ItemIndex != -1) {   //do a lookup search
			//take controlc[r]->Text and use it locally on the relevent table to lookup the id(s) for the correct search
			String tableName = sName[r]->Items->Strings[sName[r]->ItemIndex]; //eg Specimen
			if (tableName == "AddedByUserId" || tableName == "LastMeasuredUserId") {
				tableName = "ItemSourceId";
			}
			String fieldName = sField[r]->Items->Strings[sField[r]->ItemIndex]; //eg IsActive
			searchOp = "IN ";
			searchVal = "(";
			String findTxt = sValue[r]->Text.UpperCase();
			for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
				TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
				if (combo) {
					if (combo->Name == tableName) {
						for (int i = 0; i < combo->Count; i++) {
							bool found = false;
							TStringList* sl = (TStringList*)combo->Items->Objects[i];
							if (sOperator[r]->ItemIndex == 0) {
								if (sValue[r]->Text == sl->Values[fieldName]) {//add id to a searchidlist
									found = true;
								}
							}
							else if (sOperator[r]->ItemIndex == 1) {
								String search = sl->Values[fieldName].UpperCase();
								if (search.Pos(findTxt)) {//add id to a searchidlist
									found = true;
								}
							}
							if (found) {
								String id = sl->Values[getMapping(combo)];
								if (id.ToIntDef(0) < 0) {
								}
								else {
									if (searchVal != "(") {
										searchVal += ",";
									}
									searchVal = searchVal + id;
								}
							}
						}
						break;
					}
				}
			}
			if (searchVal == "(") {
				String op = sOperator[r]->Items->Strings[sOperator[r]->ItemIndex];
				//ShowMessage("Nothing to search for since " + tableName + " has no " + fieldName + " records equal to " + controlc[r]->Text);

				Label1->Text = tableName + " has no " + fieldName + " records " + op + "'" + sValue[r]->Text + "'";
				sValue[r]->SetFocus();
				SearchBut->Enabled = true;
				AniIndicator1->Visible = false;
				return;
			}
			searchVal += ")";
		}
		else {  //do a non-lookup search
			if (searchName == "ItemBarcode" || searchName == "ItemOperation" || searchName == "ItemNote" || searchName == "DateAdded" || searchName == "LastMeasuredDate") {   //-EQ
				add = "'";
			}
			searchOp = sOperator[r]->Items->Strings[sOperator[r]->ItemIndex];
			searchVal = sValue[r]->Text.Trim();
		}

		String searchtxt = searchName + " " + searchOp;
		if (operation != "") {
			operation += "%26";
		}
		if (add == "'") {
			searchVal = StringReplace(searchVal," ","%20", TReplaceFlags() << rfReplaceAll);
			//searchVal = LoginWn->url_encode(AnsiString(searchVal).c_str()).c_str();
		}
		operation += LoginWn->url_encode(AnsiString(searchtxt).c_str()).c_str() + add + searchVal + add;
	}
}
NavButClick(firstBut);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::NavButClick(TObject *Sender) {
TButton *b = reinterpret_cast<TButton*>(Sender);
if (b == firstBut) {
	SearchPage = 1;
}
else if (b == prevBut) {
	SearchPage--;
}
else if (b == next2But) {
	SearchPage++;
}
else if (b == lastBut) {
	SearchPage = maxpages;
}
InitSearch();
DoSearch();
}
//---------------------------------------------------------------------------
void TForm1::DoSearch() {
Application->ProcessMessages();
String s = sFnBox->Items->Strings[sFnBox->ItemIndex];
if (ComboBox2->ItemIndex != 0) {
	s = s + "/" + ComboBox2->Items->Strings[ComboBox2->ItemIndex] + "/page/" + String(SearchPage);
}
else s = s + "/999999/page/1"; //todo make this a background thread as it could very slow!
String sortText = "";
if (SortChoice->ItemIndex != 0) {
	sortText = "Sorting=" + ItSort->Items->Strings[ItSort->ItemIndex] + "+";
	if (SortChoice->ItemIndex == 1) {
		sortText += "ASC";
	}
	else sortText += "DESC";
}
if (GroupBox3->Enabled && !operation.IsEmpty()) { //lets add a filter
	if (!sortText.IsEmpty()) {
		s += "?Filtering=" + String(operation) + "&" + sortText;
	}
	else {
		s += "?Filtering=" + String(operation);
    }
}
else if (!sortText.IsEmpty()) {
	s += "?" + sortText;
}
Label1->Text = "Please wait...searching database";
if (ComboBox2->ItemIndex != 0) Label1->Text = Label1->Text + " for page " + String(SearchPage);
Application->ProcessMessages();
bool success = LoginWn->DoQuery(s, true);  //read ahead
if (!success) {
	Label1->Text = s;
	SearchBut->Enabled = true;
	AniIndicator1->Visible = false;
	return;
}
bool speeddebug = false;
if (speeddebug) {//just for debugging
	LoginWn->FillTable(s);
}
int p1 = s.Pos("NumOfPages");
int p2 = s.Pos("NumOfRecords");
int total_rec;
String sp1;
p2 = p2 + String("NumofRecords").Length() + 2;
p1 = p1 - 2;
//calculate total no of additional requests
//eg 200 records, split into 20 requests of 10 each (nperpage = 10)
sp1 = s.SubString(p2, p1 - p2);
total_rec = sp1.ToIntDef(0);
Label23->Text = String(total_rec) + " items";
maxpages = 1;
if (ComboBox2->ItemIndex != 0) {
	maxpages += total_rec / ComboBox2->Items->Strings[ComboBox2->ItemIndex].ToIntDef(1);
}
next2But->Enabled = true;
lastBut->Enabled = true;
prevBut->Enabled = true;
firstBut->Enabled = true;
if (maxpages == SearchPage) {
	next2But->Enabled = false;
	lastBut->Enabled = false;
}
if (SearchPage == 1) {
	prevBut->Enabled = false;
	firstBut->Enabled = false;
}
Label24->Text = "Page " + String(SearchPage) + " of " + String(maxpages);
int totalcount = 0;
LoginWn->FillTable(s);

if (total_rec == 0) {
	Label1->Text = "No items found";
	SearchBut->Enabled = true;
	AniIndicator1->Visible = false;
	return;
}
ToolBar2->Enabled = true;
CreateSearchCols();

typedef std::map<String, std::multimap<String, int> > outerMap;
typedef std::multimap<String, int> innerMap;
typedef pair< String, std::multimap<String, int> > my_pair;
outerMap outer_map;

bool metadataMissing = false;
if (speeddebug) { //just for debugging (todo replace slow stringgrid component with vector of vectors)
}
else { //process the first page

	sg->RowCount += MainWn->StringGrid1->RowCount;
	int count = 0;

	for (int j = 1; j < sg->ColumnCount; j++) {    //iterate over the current search results headers
		innerMap inner_map;
		String s1 = ((TStringColumn*)sg->ColumnByIndex(j))->Header;
		String tname = s1 + "Id";
		if (tname == "AddedByUserId" || tname == "LastMeasuredUserId") {
			tname = "ItemSourceId";
		}
		int id = GetCol(s1, MainWn->StringGrid1);    //get the corresponding main column
		Application->ProcessMessages();
		int idcol = GetCol(ItSet->Items->Strings[ItSet->ItemIndex], MainWn->StringGrid1);
		count = 0;
		for (int i = 0; i < MainWn->StringGrid1->RowCount; i++) {//iterate over the main table rows
			String itemInfo = MainWn->StringGrid1->Cells[idcol][i];

			if (j == 1) {
				itemR[totalcount + count] = new TCheckBox(sg);
				itemR[totalcount + count]->Parent = sg;
				itemR[totalcount + count]->IsChecked = false;
				itemR[totalcount + count]->Position->Y = 1 + (totalcount + count) * sg->RowHeight;
				itemR[totalcount + count]->Position->X = 4;
				//itemR[totalcount + count]->Text = "";//itemId;  String(totalcount + count);//dont need it
				itemR[totalcount + count]->Width = 25;
				itemR[totalcount + count]->OnChange = CheckBoxChange;
				itemR[totalcount + count]->Tag = totalcount + count;
			}
			if (id == -1) {  //we need to get metadata from internal - see redmine #1192
				int id2 = GetCol(tname, MainWn->StringGrid1);
				String sid2 = MainWn->StringGrid1->Cells[id2][i];
				for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
					TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
					if (combo) {
						if (combo->Name == tname) { //found lookup
							bool found = false;
							String index = getMapping(combo);
							for (int m = 0; m < combo->Count; m++) {
								TStringList* sx = (TStringList*)combo->Items->Objects[m];
								if (sid2 == sx->Values[index]) {//add id to a searchidlist
									found = true;
									sg->Cells[j][totalcount + count] = sx->Values[combo->ListBoxResource];
									break;
								}
							}
							if (!found) {
								if (!sid2.IsEmpty() && sid2 !="0") {//retrieve new metadata snapshot
									//use datastructure for column j containing the positions and id
									//for a new item search where column j IN (id1, id2, etc) where id1 and id2 are unique
									inner_map.insert(pair<String, int>(sid2, totalcount + count)); //eg specimenid, rowpos
									metadataMissing = true;
									sg->Cells[j][totalcount + count] = "error Id=" + sid2 + " missing";
								}
							}
							break;
						}
					}
				}
			}
			else {
				String sid = MainWn->StringGrid1->Cells[id][i]; //get the main value
				if (sid != "(empty)") {   //workaround for overlaying checkbox
					sg->Cells[j][totalcount + count] = sid;
				}
			}
			count++;
		}
		if (!inner_map.empty()) {
			outer_map.insert(my_pair(s1, inner_map));
		}
	}
	totalcount += count;
	sg->RowCount = totalcount;
	//if it has already been shortlisted lets check it
	for (int i = 0; i < sg->RowCount; i++) {//iterate over the main table rows
		//iterate over treeview finding the location
		for (int r = 0; r < TreeView1->Count; r++) {
			TListBoxItem *t = TreeView1->ItemByIndex(r);
			String searchItem = sg->Cells[GetCol("ItemId", sg)][i];//itemR[i]->Text;
			if (getTreeStrFromObject(t, "ItemId") == searchItem) {
				ignore = true;
				itemR[i]->IsChecked = true;
				ignore = false;
				break;
			}
		}
	}
}
if (metadataMissing) {
	sg->Enabled = false;
	ProgressBar1->Value = 0;
	ProgressBar1->Max = outer_map.size();
	for (outerMap::iterator i = outer_map.begin(), iend = outer_map.end(); i != iend; ++i) {
		String notFound = "";
		String tname = i->first + "Id";
		if (tname == "AddedByUserId" || tname == "LastMeasuredUserId") {
			tname = "ItemSourceId";
		}
		Label1->Text = "Searching database for matching " + tname + " metadata...";
		Application->ProcessMessages();
		//identify all missing metadata
		innerMap &inner_map = i->second;

		TStringList *notFoundList = new TStringList();
		notFoundList->Sorted = true;
		notFoundList->Duplicates = TDuplicates::dupIgnore;    //only show unique items
		for (innerMap::iterator j = inner_map.begin(), jend = inner_map.end(); j != jend; ++j) {
			notFoundList->Add(j->first);
		}
		String searchVal = "";
		for (int wi = 0; wi < notFoundList->Count; wi++) {
			if (searchVal.IsEmpty()) searchVal = "(" + notFoundList->Strings[wi];
			else searchVal = searchVal + "," + notFoundList->Strings[wi];
		}
		if (!searchVal.IsEmpty()) {
			ProgressBar1->Value++;
			Application->ProcessMessages();
			searchVal = searchVal + ")";
			//cutdown for SpecimenId, TrialUnitSpecimenId & ItemSourceId so it doesn't grab the big tables
			LoginWn->GetCombosFromDAL(tname, searchVal); //add offline metadata for missing metadata only
			//substitute missing data back into the table (only required if we keep the database search tab)
			for (innerMap::iterator j = inner_map.begin(), jend = inner_map.end(); j != jend; ++j) {
				String jid = j->first; 				//get the missing id
				int jcol = GetCol(i->first, sg);    //find the table column
				int jrow = j->second;				//get the table row no
				//find the missing metadata
				for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
					TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
					if (combo) {
						if (combo->Name == tname) { //found lookup
							bool found = false;
							String index = getMapping(combo);
							for (int m = 0; m < combo->Count; m++) {
								TStringList* sl = (TStringList*)combo->Items->Objects[m];
								if (jid == sl->Values[index]) {//add id to a searchidlist
									found = true;
									sg->Cells[jcol][jrow] = sl->Values[combo->ListBoxResource];
									break;
								}
							}
						}
					}
				}
			}
		}
		delete notFoundList; notFoundList = NULL;
	}
	//StartShortList();
	ProgressBar1->Value = 0;
	sg->Enabled = true;
}

if (ComboBox2->ItemIndex != 0) Label1->Text = "Page " + String(SearchPage) + " retrieved";
else Label1->Text = "All " + String(total_rec) + " items retrieved";

SearchBut->Enabled = true;
AniIndicator1->Visible = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SettingsChange(TObject *Sender)  {
TCheckBox *c = reinterpret_cast<TCheckBox*>(Sender);
int colno = c->Tag;
if (sg) {
	for (int j = 1; j < sg->ColumnCount; j++) {    //iterate over the current search results headers
		String title = ((TStringColumn*)sg->ColumnByIndex(j))->Header;
		if (title == c->Text) {
			int idcol = title.Pos("Id");
			if (idcol && title != "ItemId") {
				((TStringColumn*)sg->ColumnByIndex(j))->Visible = false;
				((TStringColumn*)sg->ColumnByIndex(j + 1))->Visible = c->IsChecked;
			}
			else ((TStringColumn*)sg->ColumnByIndex(j))->Visible = c->IsChecked;
			break;
		}
	}
	ResizeGrid(sg);
}
//is this required after remove tree?
/*if (show3) {
	TreeView1->BeginUpdate();
	for (int i = 0; i < TreeView1->Count; i++) {
		if (TreeView1->Items[i]->IsExpanded) {//kludge to get tree to repaint
			TreeView1->EndUpdate();
			TreeView1->Items[i]->Collapse();
			TreeView1->Items[i]->Expand();
			TreeView1->BeginUpdate();
		}
	}
	TreeView1->EndUpdate();
}*/

TStringList *settings = new TStringList(this);
for (int i = 0; i < iNameList->Lines->Count; i++) {
	settings->Add(iNameList->Lines->Strings[i] + "," + String((int)visF[i]->IsChecked));
}
settings->SaveToFile(SettingsWn->path + "settings.txt");
delete settings; settings = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::selRow(TObject *Sender) {
int row = sg->Selected;
if (itemR[row]->IsChecked) {//checkbox is checked so lets select the item in the tree
	int i = itemR[row]->Tag;
	String searchItem = sg->Cells[GetCol("ItemId", sg)][i];
	//iterate over treeview finding the item
	bool itemFound = false;

	TListBoxItem *t = NULL;
	for (int j = 0; j < TreeView1->Count; j++) {
		t = TreeView1->ItemByIndex(j);
		if (getTreeStrFromObject(t, "ItemId") == searchItem) {
			itemFound = true;
			break;
		}
	}
	if (!itemFound) {
		//currently could occur if right-click on tree and remove item
		ShowMessage("Error - mismatch between shortlist and search results");
		//todo decide whether remove item from shortlist functionality is required from search or from tree
		return;
	}
	t->IsSelected = true;
	TreeView1Click(TreeView1); //ensure its selected
}
}
//---------------------------------------------------------------------------
void TForm1::CreateSearchCols() {
//create the 'listview' search component with hidden columns and itemId at always at start
sg = new TStringGrid(this);
sg->OnClick = selRow;
sg->Parent = SearchTab;
sg->ReadOnly = true;
sg->Align = TAlignLayout::alClient;
sg->ShowSelectedCell = false;
sg->ShowScrollBars = false;
String sid;
TStringColumn *col = new TStringColumn(sg);
col->Parent = sg;
col->Header = "Use";
for (int i = 0; i < iNameList->Lines->Count; i++) {
	TStringColumn *col = new TStringColumn(sg);
	col->Parent = sg;
	String title = iNameList->Lines->Strings[i];
	col->Header = title;
	int idcol = iNameList->Lines->Strings[i].Pos("Id");
	if (idcol && title != "ItemId") {
		col->Visible = false;
		TStringColumn *col2 = new TStringColumn(sg);
		col2->Visible = visF[i]->IsChecked;
		col2->Parent = sg;
		col2->Header = title.SubString(0, title.Length() - 2);
	}
	else col->Visible = visF[i]->IsChecked;
}
ResizeGrid(sg);
sg->RowCount = 0;
SetResource(sg);
sg->Selected = NULL;
}
//---------------------------------------------------------------------------
bool TForm1::GetResultsVisible(String name) {//enables some results columns to always be hidden
for (int i = 0; i <iNameList->Lines->Count; i++) {
	if (visF[i]->Text == name) {
		return visF[i]->IsChecked;
	}
}
return false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::OnButClick(TObject *Sender)  {
for (int i = 1; i <iNameList->Lines->Count; i++) {
	visF[i]->IsChecked = true;
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::OffButClick(TObject *Sender)  {
for (int i = 1; i <iNameList->Lines->Count; i++) {
	visF[i]->IsChecked = false;
}
}
//---------------------------------------------------------------------------
TListBoxItem* TForm1::AddLocBlankShortList()  {
TListBoxItem *tNode = new TListBoxItem(this);
TStringList *stl = new TStringList();
//if (StorageId->ItemIndex != -1) stl->Assign((TStringList*)StorageId->Items->Objects[StorageId->ItemIndex]);
tNode->Data = stl;
tNode->Text = stl->Values[GetStorageLabelType()];
tNode->Parent = TreeView1;
SaveNow(0);
return tNode;
}
//---------------------------------------------------------------------------
TStringList* TForm1::GetSearchComboObject(String searchStorage) {
for (int i = 0; i < StorageId->Count ; i++) {
	TStringList *l = (TStringList*)StorageId->Items->Objects[i];
	if (l->Values["StorageId"] == searchStorage) {
		return l;
	}
}
return NULL;
}
//---------------------------------------------------------------------------
String TForm1::GetSearchComboText(String searchStorage) {
String storLabelType = GetStorageLabelType();
for (int i = 0; i < StorageId->Count ; i++) {
	TStringList *l = (TStringList*)StorageId->Items->Objects[i];
	if (l->Values["StorageId"] == searchStorage) {
		searchStorage = l->Values[storLabelType];
		return searchStorage;
	}
}
return "";
}
//---------------------------------------------------------------------------
String TForm1::getTreeStrFromObject(TListBoxItem *t, String Id) {
NodeData *d = (NodeData*)t->Data;
return d->final->Values[Id];
}
//---------------------------------------------------------------------------
String TForm1::getListStrFromObject(TListBoxItem *lb, String Id) {
NodeData *d = (NodeData*)lb->Data;
return d->final->Values[Id];
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CheckBoxChange(TObject *Sender) {
if (ignore) {
	return;
}
TCheckBox *c = reinterpret_cast<TCheckBox*>(Sender);
if (PrepareWn) {
	if (sg->Parent == PrepareWn->Panel1) { //user clicking on "search results" inside the wizard!
		if (!singleSelect) {//if the item is unchecked, uncheck everything and check the selected item
			singleSelect = true;
			for (int i = 0; i < sg->RowCount; i++) {
				if (itemR[i] != c) {
					if (itemR[i]->IsChecked) {
						itemR[i]->IsChecked = false;
					}
				}
			}
			singleSelect = false;
		}
	}
}
StartShortList();

int i = c->Tag;
String searchItem = sg->Cells[GetCol("ItemId", sg)][i];
bool itemFound = false;
TListBoxItem *t = NULL;
for (int j = 0; j < TreeView1->Count; j++) {
	t = TreeView1->ItemByIndex(j);
	if (getTreeStrFromObject(t, "ItemId") == searchItem) {
		itemFound = true;
		break;
	}
}
if (c->IsChecked) {
	if (!itemFound) {//add to tree
		NodeData *d = new NodeData();
		for (int s = sg->ColumnCount - 1; s >=0  ; s--) {
			d->orig->Values[((TStringColumn*) sg->ColumnByIndex(s))->Header] = sg->Cells[s][i];
			d->final->Values[((TStringColumn*) sg->ColumnByIndex(s))->Header] = sg->Cells[s][i];
		}
		TListBoxItem* z = AddChildNode(0, d);
		if (!DontSaveYet) {
			z->IsSelected = true;
			TreeView1Click(TreeView1);
			Label1->Text = "Item "+ z->Text.TrimLeft() + " added to shortlist";
		}
	}
}
else if (!c->IsChecked) {
	if (itemFound) {
		TColorButton *but = GetButton(t);
		if (but->Color == TAlphaColorRec::Green || but->Color == TAlphaColorRec::Yellow) {
			//remove the item
			Label1->Text = "  Item "+ t->Text.TrimLeft() + " removed from shortlist";
			RemoveTreeItem(t);

			selButton = NULL;
			t->IsSelected = false;
			TreeView1Click(TreeView1);
		}
		else {
			ShowMessage("You have locally modified the item and therefore it can't be removed from the shortlist"); //change this later
			c->IsChecked = true;
		}
	}
}

if (!DontSaveYet) {
	SaveNow(Sender);
}

}
//---------------------------------------------------------------------------
void __fastcall TForm1::ShortButClick(TObject *Sender) {
TStringColumn *b = reinterpret_cast<TStringColumn*>(Sender);
int count = 0;
hintText = "";
String txt = "adding items to shortlist";
if (b->Tag) txt = "removing items from shortlist";
Label1->Text = "Please wait..." + txt;
Application->ProcessMessages();
//TreeView1->BeginUpdate();
for (int i = 0; i <sg->RowCount; i++) {
	if (i % 20 == 0) {
		//ProgressBar1->Value = j;
		TreeView1->EndUpdate();
		Application->ProcessMessages();
		TreeView1->BeginUpdate();
	}
	if (!itemR[i]) { //possible bug? since never being set to null
		 break;
	}
	else {
		 DontSaveYet = true;
		 if (itemR[i]->IsChecked == (b->Tag)) {
			itemR[i]->IsChecked = (!b->Tag); //calls CheckBoxChange
			count++;
		 }
	}
}
TreeView1->EndUpdate();
DontSaveYet = false;
SaveNow(Sender);
if (b->Tag) {
	b->Tag = 0;
	Label1->Text = String(count) + " items removed from shortlist";
}
else {
	b->Tag = 1;
	Label1->Text = String(count) + " items added to shortlist";
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::EdChange(TObject *Sender){
if(ignore) return;
TComboBox *combo = dynamic_cast<TComboBox*>(Sender);
if (combo) UpdateButtonGraphic(combo);
if (!selButton) {
	if (Sender == StorageId) {
		if (StorageId->ItemIndex != -1 && !TreeView1->Count) AddLocBlankShortList();
	}
}
else if (selButton->Color != TAlphaColorRec::Black){ //modify item - prevent this for locally deleted items
	//first update the object in the tree
	NodeData *data = (NodeData*)(TreeView1->Selected->Data);
	TStringList *l = data->final;
	String name;
	String value;
	if (combo) { //found a combobox
		if (combo != ItemOperation) {
			if (combo->ItemIndex != -1) { //only update our shortlist with the selected
				TStringList *s = (TStringList*)combo->Items->Objects[combo->ItemIndex];
				String index = getMapping(combo);
				name = combo->Name;
				value = s->Values[index];
				if (s->Values[index].IsEmpty()) {
					ShowMessage("Error mapping fields. Can't find " + name);
					return;
				}
			}
		}
		else {
			name = combo->Name;
			value = ItemOperation->Items->Strings[combo->ItemIndex];
		}
	}
	else {
		TEdit *edit = dynamic_cast<TEdit*>(Sender);
		if (edit) {
			name = edit->Name;
			value = edit->Text;
		}
		else {
			TMemo *memo = dynamic_cast<TMemo*>(Sender);
			name = memo->Name;
			value = memo->Text;
		}
	}
	l->Values[name] = value;

	if (name == GetItemLabelType()) {
		TreeView1->Selected->Text = "        " + value;
	}

	if (Sender == Amount) {//collect the datetime right now as new date and user details need to be updated
		TDateTime dateNow = Now();  //what does LastMeasuredDate mean? The datetime an amount was set?
		String ds = dateNow.FormatString("yyyy-MM-dd HH:mm:ss");
		l->Values["LastMeasuredDate"] = ds;

	}

	if (selButton->Color != TAlphaColorRec::White) {//can't modify a new item until it is created!
		if (selButton->Color == TAlphaColorRec::Red) {//already being modified so lets ignore it
		}
		else {
			itemsTBU++;
			Update1->Enabled = true;
			UndoBut->Visible = true;
			selButton->Color = TAlphaColorRec::Red;
		}
	}
	UpdateItemsTBU(); //save changes
}
}
//---------------------------------------------------------------------------
String TForm1::getMapping(TComboBox *combo) {
String index = combo->Name;
//mapping for table joins often is required
if (combo->Name == "ContainerTypeId" || combo->Name == "ItemStateId" || combo->Name == "ItemTypeId") {
	index = "TypeId";
}
else if (combo->Name == "ItemSourceId") {
	index = "ContactId";
}
else if (combo->Name == "ScaleId") {
	index = "DeviceRegisterId";
}
else if (combo->Name == "StorageId") {
	index = "StorageId";
}
else if (combo->Name == "SpecimenId") {
	//index = "SpecimenName";
}
return index;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::EdKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar, TShiftState Shift) {
if (!TreeView1->Selected) {
    return;
}
TEdit *e = dynamic_cast<TEdit*>(Sender);
TMemo *m = dynamic_cast<TMemo*>(Sender);
String orig;
String current;
NodeData *data = (NodeData*)(TreeView1->Selected->Data);
if (e) {
	orig = data->final->Values[e->Name];
	current = e->Text;
	if (e == Amount) {
		ValidateEntry(e, KeyChar, orig, current);
		if (orig.ToDouble() == current.ToDouble()) {
			current = orig;
		}
	}
	else if (e == ItemBarcode) {
		Barcode1->InputText = ItemBarcode->Text;
		ShowBarcode(ItemBarcode);
	}
}
else {
	orig = data->final->Values[m->Name];
	current = m->Text;
}

if (current != orig) {
	EdChange(Sender);
}
if (e) {
	if (e == ItemBarcode) {
		Application->ProcessMessages();
		ItemBarcode->SetFocus();
	}
}
}
//---------------------------------------------------------------------------
void TForm1::ValidateEntry(TEdit *e, System::WideChar &KeyChar, String orig, String &current) {
if (isdigit((int)KeyChar) || KeyChar == '.') {
	//APPLE workaround

	if (KeyChar == '.' && e->Text.SubString(0, e->Text.Length() -1).Pos(".")) {  //two "."
		e->Text = e->Text.SubString(0, e->Text.Length() -1);
	}
	else {
		double x = e->Text.ToDouble();
		if (String(x * 1000).ToIntDef(-1) == -1) {
			ShowMessage(e->Name + " only supports values up to 3 DPs. Please try again.");
			e->Text = orig;
			return;
		}
	}
}
else {
	String a = String(KeyChar);
	if(String(KeyChar).Length() == 1 && KeyChar != '\0') {
		e->Text = e->Text.SubString(0, e->Text.Length() -1);
		current = e->Text;
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button6Click(TObject *Sender) {
LoginWn->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SaveNow(TObject *Sender)  {
Memo2->Lines->Clear();
Memo2->Lines->Add(LoginWn->DALBase->Text);
for (int j = 0; j < TreeView1->Count; j++) {
	TListBoxItem *t = TreeView1->ItemByIndex(j);
	//iterate over the items
	Memo2->Lines->Add(t->Text);
	//get the data and convert it to csv
	NodeData *d = (NodeData*)(t->Data);
	TStringList *l;
	for (int i=0; i < 2; i++) {
		if (!i) l = d->orig;
		else l = d->final;

		String csv = "";
		for (int i = 0; i < l->Count; i++) {
			csv += l->Strings[i] + ",";
		}
		Memo2->Lines->Add(csv);
	}
	//store button color
	TColorButton *but = GetButton(t);
	Memo2->Lines->Add(String(but->Color));
}
Memo2->Lines->SaveToFile(updateFile);
}
//---------------------------------------------------------------------------
void TForm1::LoadTables() {
for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
	TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
	if (combo) {
		if (combo != ItemOperation) {
			TButton *b = LoginWn->getButton(combo);
			LoginWn->LoadAddButton(b);
		}
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LoadNow(TObject *Sender)  {
ProgressBar1->Value = 0;
AniIndicator1->Visible = true;
CancelBut->Visible = true;
//Label1->Text = "Please wait...loading tables from file";
ProgressBar1->Max = 9;
Application->ProcessMessages();
for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
	TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
	if (combo) {
		if (combo != ItemOperation) {
			ProgressBar1->Value++;
			Application->ProcessMessages();
			LoginWn->LoadCombo(combo);
		}
	}
}
TListBoxItem *tmpSelNode = NULL;
TreeViewClear();
int icount = 0;
try {
	ProgressBar1->Value = 0;
	Label1->Text = "Please wait...loading shortlist from file";
	Application->ProcessMessages();
	Memo2->Lines->LoadFromFile(updateFile);
	ProgressBar1->Max = Memo2->Lines->Count;

	itemsTBU = 0;
	TListBoxItem *tNode = NULL;
	TListBoxItem* v = NULL;
	LoginWn->DALBase->Text = Memo2->Lines->Strings[0];
	TreeView1->BeginUpdate();
	for (int j = 1; j < Memo2->Lines->Count && CancelBut->Visible; j++) {
		String s = Memo2->Lines->Strings[j];
		if (icount % 20 == 0) {
			ProgressBar1->Value = j;
			TreeView1->EndUpdate();
			Application->ProcessMessages();
			TreeView1->BeginUpdate();
		}
		if (s.SubString(0,1) == " ") { //this is an item
			//read the next line to get the data
			NodeData *d = new NodeData;
			TStringList *l;
			for (int i=0; i < 2; i++) {
				if (!i) l = d->orig;
				else l = d->final;
				String dataS = Memo2->Lines->Strings[++j];
				//convert the csv into a stringlist data object
				int p = dataS.LastDelimiter(",");
				dataS = dataS.SubString(0, p - 1);
				while (p > 1) {
					p = dataS.LastDelimiter(",");
					l->Add(dataS.SubString(p + 1, dataS.Length()));
					dataS = dataS.SubString(0, p - 1);
				}
			}
			icount++;
			v = AddChildNode(0, d);
			//get the node state
			String sState = Memo2->Lines->Strings[++j];

			TColorButton *but = GetButton(v);
			double nState = sState.ToDouble();
			but->Color = (unsigned int)nState;
			if (but->Color == TAlphaColorRec::Yellow) {
				tmpSelNode = v;
			}
			else if (but->Color != TAlphaColorRec::Green) {
				if (!tmpSelNode) { //set the selected node to any modified node if it hasn't already been done
					tmpSelNode = v;
				}
				itemsTBU++;
			}
			//tNode->Expand();  //smooths loading but slow!
		}
	}

	if (!tmpSelNode) { //node always selected!
		tmpSelNode = v;
	}
	if (tmpSelNode) {
		tmpSelNode->IsSelected = true;
		TreeView1Click(TreeView1);
		ProgressBar1->Value++;
	}
	if (itemsTBU > 0) { //don't bother saving it!
		Caption =Application->Title + " (Items modified = " + String(itemsTBU) + ")";
	}
	TreeView1->EndUpdate();
	Label1->Text = String(icount) + " items from " + FileDateToDateTime(FileAge(updateFile)) + " loaded";
	ProgressBar1->Value = 0;
	AniIndicator1->Visible = false;
	CancelBut->Visible = false;
}
catch (...) {
	ShowMessage("There appears to be an error in your shortlist");
	ClearShortButClick(Sender);
	ProgressBar1->Value = 0;
	CancelBut->Visible = false;
	TreeView1->Visible = true;
	AniIndicator1->Visible = false;
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LogStatButClick(TObject *Sender) {
hintText = "";
if (LogOut->Visible) { //logged in so lets logout
	LoginWn->LogoutButClick(Sender);
}
else if (LogIn->Visible) {//logged out so lets login
	LoginWn->Visible = true;
	LoginWn->LoginButClick(0);
	SetUpLoggedInWindow();
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LogStatButMouseEnter(TObject *Sender) {
hintText = Label1->Text;
if (LogIn->Visible) {
	Label1->Text = "Currently logged out. Click to login";
}
else {
	Label1->Text = "Currently logged in. Click to logout";
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ButMouseLeave(TObject *Sender) {
if (AniIndicator1->Visible) {
	return;
}
Label1->Text = hintText;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Update1MouseEnter(TObject *Sender) {
hintText = Label1->Text;
TButton *b = reinterpret_cast<TButton*>(Sender);
Label1->Text = b->Text;
}
//---------------------------------------------------------------------------
void TForm1::SetUpLoggedInWindow (){
if (!LoginWn->LogoutBut->Enabled) { //didnt succeed
	Label1->Text = "Login failed"; //could show login window
	return;
}
SearchTab->Enabled = true;
Panel2->Repaint();
LogIn->Visible = false;
LogOut->Visible = true;
Label1->Text = "You have been logged in";
LogStat->Text = "Logged in as " + LoginWn->UserName->Text;
SaveLoggedStatus();
}
//---------------------------------------------------------------------------
void TForm1::SetUpLoggedOutWindow() {
//disable the search tab, etc until successful login
Label1->Text = "You have been logged out";
LogStat->Text = "You are not logged in";
SearchTab->Enabled = false;
LogIn->Visible = true;
LogOut->Visible = false;
SaveLoggedStatus();
}
//---------------------------------------------------------------------------
void TForm1::SaveLoggedStatus() {
int stat = 0;
if (LogOut->Visible) {
	stat = 1;
}
TStringList *settings4 = new TStringList(this);
settings4->Add(String(stat));
settings4->Add(String(TabControl1->TabIndex));
settings4->Add(String(TranslationCombo->ItemIndex));
settings4->Add(LoginWn->DALBase->Text);   //should we save this if login failed?
settings4->SaveToFile(SettingsWn->path + "settings4.txt");
delete settings4; settings4 = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LogTabClick(TObject *Sender) {
//kludge for getting the log to scroll on startup
if (!logTabSeen) {
	logTabSeen = true;
	log->ItemIndex = log->Count - 1;
}
}
//---------------------------------------------------------------------------
void TForm1::RemoveTreeItem(TListBoxItem *t2) {
NodeData *d = (NodeData*)t2->Data;
delete d; d = NULL;
TColorButton* b =GetButton(t2);
if (b->Color != TAlphaColorRec::Green && b->Color != TAlphaColorRec::Yellow) {
	itemsTBU--;
}
delete b; b = NULL;
delete t2;
}
//---------------------------------------------------------------------------
void TForm1::TreeViewClear() { //fix for intermittent treeview1->Clear() access violation bug
Label1->Text = "Please wait....clearing shortlist";
Application->ProcessMessages();
TreeView1->BeginUpdate();
for (int r = TreeView1->Count - 1; r >= 0  ; r--) {
	TListBoxItem *t = TreeView1->ItemByIndex(r);
	RemoveTreeItem(t);
}
TreeView1->EndUpdate();
selButton = NULL;
//TreeView1->Selected = NULL;
Label1->Text = "";
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ClearShortButClick(TObject *Sender)   {
TModalResult answer = mrYes;
if (Sender) {
	answer = MessageDlg("Remove your current shortlisted items?", TMsgDlgType::mtWarning,
			TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0);
}
if (answer == mrYes) {
	TreeViewClear();
	//if (mrYes == MessageDlg("Do you want to delete your snapshot of the metadata permanently?", TMsgDlgType::mtWarning, mbOKCancel, 0)) {
		DeleteFile(updateFile);
		ClearMetaData();
		//todo  logout
		ShortlistTab->Enabled = false;
		ClearShortBut->Enabled = false;
		#ifdef _DEBUG
		Label1->Text = "Shortlist and metadata removed";
		#endif
	/*}
	else {
		Memo2->Lines->Clear();
		Memo2->Lines->Add(LoginWn->DALBase->Text);
		Memo2->Lines->SaveToFile(updateFile);
		Label1->Text = "Shortlist removed";
	}*/
	SaveShortBut->Enabled = false;
	Update1->Enabled = false;
	UpdateAll->Enabled = false;
	itemsTBU = 0;

	Form1->TreeView1Click(Form1->TreeView1);
	Caption = Application->Title;
	if (sg) {
		delete sg;
		sg = NULL;
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SaveShortButClick(TObject *Sender) {
String rsPath;
bool chose = true;
char pathDelim = (char)System::Ioutils::TPath::DirectorySeparatorChar;
if (!Sender) {
	rsPath = SettingsWn->path + String(WizardWn->trialno);
	ForceDirectories(rsPath + pathDelim);
}
else {
	chose = mySelectDir(rsPath, "Select a folder to put your KDLogue archive", SettingsWn->path);
}
String newFN;
if (chose) {
	newFN = rsPath + pathDelim + "KDLoguetemp.txt";
	String origFN = updateFile;
	updateFile = newFN;
	SaveNow(Sender);
	for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
		TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
		if (combo) {
			if (combo != ItemOperation) {
				LoginWn->SaveCombo(combo);
				//TButton *b = LoginWn->getButton(combo);
				//LoginWn->SaveAddButton(b);
			}
		}
	}
	updateFile = origFN;
	Label1->Text = "Shortlist archived in " + rsPath;
	if (sg) {
		delete sg;
		sg = NULL;
	}
}
}
//---------------------------------------------------------------------------
void TForm1::UpdateButtonGraphic(TComboBox* combo) {
TButton *b = LoginWn->getButton(combo);
if (combo->ItemIndex != -1) {
	b->Text = "Details";
}
else {
	b->Text = "Add new";
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LoadShortButClick(TObject *Sender) {
LoadShortBut->Enabled = false;
/*if (TreeView1->Count) {
	if (mrYes != MessageDlg("If you have not yet archived your shortlist you will lose it. Are you sure?", TMsgDlgType::mtWarning, mbOKCancel, 0)) {
		LoadShortBut->Enabled = true;
		return;
	}
}
*/
String rsPath;
bool chose = true;
if (!Sender) {
	rsPath = SettingsWn->path + String(WizardWn->trialno);
}
else {
	chose = mySelectDir(rsPath, "Select your previously archived KDLogue folder", SettingsWn->path);
}
String newFN;
if (chose) {
	#ifdef _Windows
	newFN = rsPath + "\\KDLoguetemp.txt";
	#else
	newFN = rsPath + + "/" + "KDLoguetemp.txt";
	#endif

	if (!FileExists(newFN)) {
		ShowMessage(rsPath + " does not contain a valid archive. No archive will be loaded");
		LoadShortBut->Enabled = true;
		return;
	}
	ClearMetaData();
	//grab a copy of the archive data and put it in the cache
	String origFN = updateFile;
	updateFile = newFN;
	LoadNow(Sender);
	updateFile = origFN;
	SaveNow(Sender);
	//save metadata
	for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
		TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
		if (combo) {
			if (combo != ItemOperation) {
				LoginWn->SaveCombo(combo);
				//TButton *b = LoginWn->getButton(combo);
				//LoginWn->SaveAddButton(b);
			}
		}
	}
	ShortlistTab->Enabled = true;
	StartShortList();
	if (sg) {
		delete sg;
		sg = NULL;
	}
}
LoadShortBut->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ClearLogButtonClick(TObject *Sender) {
DeleteFile(LoginWn->logFileName);
log->Clear();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ssgClick(TObject *Sender) { //todo take into account the checkbox text (BUG!)
THeaderItem *h = reinterpret_cast<THeaderItem*>(Sender);
TStringGrid *ssg = (TStringGrid*)h->Parent->Parent->Parent->Parent;   //hack
THeader *header = dynamic_cast<THeader*>(ssg->FindStyleResource("header", false));
int w;
TStringColumn* col = NULL;
THeaderItem* headeritem;
for (w = 0; w < header->ChildrenCount - 1; w++) {
	THeaderItem* headeritem = dynamic_cast<THeaderItem*>(header->Children->Items[w]);
	if (headeritem == Sender) {
		// got column number/header text
		col = ((TStringColumn*)ssg->ColumnByIndex(w));
		break;
	}
}
if (!col) {
	ShowMessage("sort column error");
	return;
}
else if (w == 0 && ssg == sg) {//click shortlist 'button' rather than sort it
	ShortButClick(col);
	return;
}
TStringList *MySortCol = new TStringList();
TStringList *MyListCols = new TStringList();
if (col->Header.Pos("Date")) {
	MySortCol->CaseSensitive = true;
}
//use Sort and Sorted for standard sorts only
//MySortCol->Sort();
//MyListCols->Sorted = false;
int SortColumn = w;

ssg->BeginUpdate();
for (int row = 0; row < ssg->RowCount; row++) {
	MySortCol->AddObject(ssg->Cells[SortColumn][row], (TObject*)(row));
}
//MySortCol->Sorted = true;
//todo add sorts for fp and int columns
if (col->Tag==0) { //hack: using tag to pass sort order to/from the GUI
	MySortCol->WriteBOM = true; //hack: using WriteBom to pass sort order to/from sort
	MySortCol->CustomSort(mySort);
	col->Tag = 1;
}
else {
	MySortCol->WriteBOM = false;
	MySortCol->CustomSort(mySort);
	col->Tag = 0;
}
for (int row = 0; row < ssg->RowCount; row++) {
	ssg->Cells[SortColumn][row] = MySortCol->Strings[row];
}

for (int col = 0; col < ssg->ColumnCount ; col++ ) {
	if (col != SortColumn) {
		MyListCols->Clear();
		for (int row = 0; row < ssg->RowCount; row++ ) {
			MyListCols->Add(ssg->Cells[col][row]);
		}
		for (int rowx = 0; rowx < ssg->RowCount; rowx++) {
			ssg->Cells[col][rowx] = MyListCols->Strings[int(MySortCol->Objects[rowx])];
		}
	}
}

if (ssg == sg) { //must move the checkboxes around
	for (int row = 0; row < ssg->RowCount; row++) {
		int BeforeSortPos = int(MySortCol->Objects[row]); //the row position of the current row before sorting
		int AfterSortPos = int(MySortCol->Objects[BeforeSortPos]); //its position now
		//so lets set the appropriate checkbox
		bool found = false;
		int count = 0;
		for (count = 0; count <ssg->RowCount; count++) {
			if (itemR[count]->Tag == AfterSortPos) {  //the tag indicates the row no that the checkbox should be next to
				found = true;
				break;
			}
		}
		if (!found) {
			ShowMessage("checkbox sort error");
		}
		else {
			itemR[count]->Position->Y = 1 + (BeforeSortPos) * ssg->RowHeight;
		}
	}
	for (int row = 0; row < ssg->RowCount; row++) {//finally put on the correct tag
		itemR[row]->Tag = int((itemR[row]->Position->Y - 1) / ssg->RowHeight);
	}
}
ssg->EndUpdate();
delete MySortCol;
delete MyListCols;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::UndoButClick(TObject *Sender) {
itemsTBU--;
if (selButton->Color == TAlphaColorRec::White){//new item so lets remove it
	int index = TreeView1->Selected->Index;
	RemoveTreeItem(TreeView1->Selected);
	if (index > 0) {
		TreeView1->ItemByIndex(index - 1)->IsSelected = true;
	}
	else TreeView1->ItemByIndex(index)->IsSelected = true;
}
else {
	NodeData *data = (NodeData*)(TreeView1->Selected->Data);
	if (selButton->Color == TAlphaColorRec::Red){ //modified item so lets undo changes
		for (int i = 0; i < data->orig->Count; i++) {
			if (data->orig->Strings[i] != data->final->Strings[i]) {

				if (data->orig->Names[i] == GetItemLabelType()) {
					TreeView1->Selected->Text = "        " + data->orig->Values[data->orig->Names[i]];
				}
			}
		}
		if (!data->orig->Values["ItemGroupId"].IsEmpty() && data->orig->Values["ItemId"] != "new" && data->final->Values["Amount"] != "0") {//this is a associated destination item so will need to identify source
			String searchId = data->orig->Values["ItemGroupId"];
			double amounttoSubtract = data->final->Values["Amount"].ToDouble();
			//data->final->Clear();
			//data->final->AddStrings(data->orig);
			//selButton->Color = TAlphaColorRec::Yellow;

			for (int i = 0; i < TreeView1->Count; i++) {
				TListBoxItem* t = TreeView1->ItemByIndex(i);
				if (getTreeStrFromObject(t, "ItemId") == searchId) {//source item is found

					double curramount = getTreeStrFromObject(t, "Amount").ToDouble();
					double newamount = curramount + amounttoSubtract;
					TListBoxItem* tsel = TreeView1->Selected;
					t->IsSelected = true;  //select the source node
					TreeView1Click(TreeView1); //ensure its selected
					Amount->Text = String(newamount);
					EdChange(Amount);
					tsel->IsSelected = true;  //select the source node
					TreeView1Click(TreeView1); //ensure its selected
					break;
				}
			}
		}

		data->final->Clear();
		data->final->AddStrings(data->orig);
		selButton->Color = TAlphaColorRec::Yellow;

	}
	else if (selButton->Color == TAlphaColorRec::Black) {//deleted item so lets undelete it
		//check for it being modified by comparing final and orig strings
		bool changeFound = false;
		for (int i = 0; i < data->orig->Count; i++) {
			if (data->orig->Strings[i] != data->final->Strings[i]) { //the item has been modified
				selButton->Color = TAlphaColorRec::Red;
				itemsTBU++; //ignore items to be updated since update+delete = delete
				changeFound = true;
				break;
			}
		}
		if (!changeFound) {
			#ifdef _DEBUG
			selButton->Color = TAlphaColorRec::Yellow;
			#endif
		}
	}
}
if (!DontSaveYet) {
	TreeView1Click(TreeView1);
	UpdateItemsTBU();
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::UndoAllButClick(TObject *Sender) {
//addparent fn stub
//TStringList *sl = new TStringList();//TStringList*)combo->Items->Objects[index];
//TStringList *uplist = new TStringList();
//String s = "item/383/add/parent";
//if (!LoginWn->DoUpdate(uplist, sl, s));
DontSaveYet = true;
for (int i = 0; i < TreeView1->Count; i++) {
	TListBoxItem* t = TreeView1->ItemByIndex(i);
	TColorButton* b =GetButton(t);
	if (b->Color != TAlphaColorRec::Green && b->Color != TAlphaColorRec::Yellow) {//do the upload
		t->IsSelected = true;  //select the node
		TreeView1Click(TreeView1); //ensure its selected
		UndoButClick(0);      //call the update function
	}
}
DontSaveYet = false;
UpdateItemsTBU();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ShortListLocButClick(TObject *Sender) {
//get a list of storage locations/ids
String pstring = "list/storage";
bool success = LoginWn->DoQuery(pstring, true);
if (!success) {
	Label1->Text = pstring;
	return;
}
LoginWn->FillTable(pstring);
//extract storageIDs and populate tree
int id = GetCol("StorageId", MainWn->StringGrid1);
if (id == -1) {
	Label1->Text = "No locations in database!";
	return;
}
int fcount = 0;
for (int i = 0; i < MainWn->StringGrid1->RowCount; i++) {
	String sid = MainWn->StringGrid1->Cells[id][i];
	if (!sid.IsEmpty()) {
		bool found = false;
		for (int i = 0; i < TreeView1->Count; i++) {
			TListBoxItem* t = TreeView1->ItemByIndex(i);
			if (t->Text == sid) {
				found = true;
				break;
			}
		}
		if (!found) {
			TListBoxItem *tNode = new TListBoxItem(this);
			TStringList *stl = new TStringList();
			stl->Assign(GetSearchComboObject(sid));
			tNode->Data = stl;
			tNode->Text = GetSearchComboText(sid);
			tNode->Parent = TreeView1;
			fcount++;
		}
	}
}
SaveNow(0);
Label1->Text = "An extra " + String(fcount) + " locations have been shortlisted";
StartShortList();
}
//---------------------------------------------------------------------------
void TForm1::StartShortList()   {
////todo will have to populate the metadata combos
if (!ShortlistTab->Enabled) {
	ShortlistTab->Enabled = true;
	//AniIndicator1->Visible = true;
	Application->ProcessMessages();
	/*Label1->Text = "Please wait...creating new shortlist";
	Application->ProcessMessages();
	for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
		TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
		if (combo) { //found a combobox
			if (combo == Form1->ItemOperation) {
			}
			else {
				LoginWn->GetAddButtonDAL(combo);  //todo remove this call - isn't required as the table definitions should be shipped with the setup.exe
			}
		}
	}
	SaveNow(0); //todo check if need this?
	Label1->Text = "Descriptive metadata created";
	AniIndicator1->Visible = false;
	*/
}
ClearShortBut->Enabled = true;
SaveShortBut->Enabled = true;

}
//---------------------------------------------------------------------------
void __fastcall TForm1::ItemBarcodeMouseEnter(TObject *Sender) {
if (AniIndicator1->Visible) {
	return;
}
Label1->Text = "Required - Item barcode info";
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ComboMouseEnter(TObject *Sender) {
if (AniIndicator1->Visible) {
	return;
}
TComboBox *combo = reinterpret_cast<TComboBox*>(Sender);
TLabel *lab = NULL;
for (int i = 0; i < combo->Children->Count; i++) {
	lab = dynamic_cast<TLabel*>(combo->Children->Items[i]);
	if (lab) {
		break;
	}
}
String name = lab->Text + " (" + String(combo->Count) + ") ";
if (name.Pos("*")) {
	name = "Required - " + name.SubString(0, name.Length() - 1);
}
else name = "Optional - " + name;
Label1->Text = name + " info shown by " + combo->ListBoxResource;

}
//---------------------------------------------------------------------------
void __fastcall TForm1::ItemNoteMouseEnter(TObject *Sender){
if (AniIndicator1->Visible) {
	return;    
}
Label1->Text = "Optional - Enter comments here";
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AmountMouseEnter(TObject *Sender){
if (AniIndicator1->Visible) {
	return;    
}
Label1->Text = "Optional - Amount of the item in container";
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AddFieldButClick(TObject *Sender) {
//populate 'add new' form
TButton *b = reinterpret_cast<TButton*>(Sender);
TComboBox *combo = reinterpret_cast<TComboBox*>(b->ParentControl);
TStringList *sl;
if (Form3) { //todo - make intelligent decision on how many windows to allow
	//delete Form3;
	//Form3 = NULL;
}
Form3 = new TForm3(this);
if (combo->ItemIndex != -1) {
	sl = (TStringList*)combo->Items->Objects[combo->ItemIndex];
	Form3->Caption = "Details for " + combo->Name + "=" + sl->Values[getMapping(combo)];
	if (sl->Values[getMapping(combo)].ToIntDef(0) < 0) Form3->Button2->Visible = true;  //temp modify button. TODO update dal function for all metadata
}
else sl = new TStringList();



Form3->TagString = combo->Name;
TStringGrid *sg = (TStringGrid*)b->TagObject;
int countY = 10;
for (int i = 0; i < sg->RowCount; i++) {
	TLabel *l = new TLabel(Form3);
	l->Text = sg->Cells[GetCol("Name", sg)][i];
	l->Position->X = 10;
	l->Position->Y = countY;
	l->Width+=30;
	l->Parent = Form3->Layout1;

	if (sg->Cells[GetCol("ColSize", sg)][i]=="") {  //temp should use MCOL?
		TMemo *m = new TMemo(Form3);
		m->Text = sl->Values[l->Text];
		m->Name = l->Text;
		m->Enabled = false;
		m->Width = 150;
		m->Height = 32;
		m->ShowScrollBars = true;
		m->Position->X = 150;
		m->Position->Y = countY;
		m->Parent = Form3->Layout1;
		m->TagString = sg->Cells[GetCol("DataType", sg)][i];   //note bug i+1 here  - rowcount set incorrectly
		m->OnKeyUp = Form3->MemoKeyUp;
		m->Tag = sg->Cells[GetCol("Required", sg)][i].ToIntDef(0);
		countY += 35;
		m->OnMouseEnter = Form3->mEnter;
		m->OnMouseLeave = Form3->mLeave;
	}
	else {
		TEdit *e = new TEdit(Form3);
		e->Text = sl->Values[l->Text];
		e->Name = l->Text;
		e->Enabled = false;
		e->Position->X = 150;
		e->Position->Y = countY;
		e->Width = 150;
		e->Parent = Form3->Layout1;
		e->TagString = sg->Cells[GetCol("DataType", sg)][i];
		e->OnKeyUp = Form3->MemoKeyUp;
		e->Tag = sg->Cells[GetCol("Required", sg)][i].ToIntDef(0);
		countY += 25;
		if (b->Name.Pos("generaltype")) {
			if (!i) {
				e->ClipChildren = true;
				if(b->Name=="generaltype0") {
					e->Text = "container";
				}
				else if(b->Name=="generaltype1") {
					e->Text = "item";
				}
				else if(b->Name=="generaltype2") {
					e->Text = "state";
				}
			}
		}
		e->OnMouseEnter = Form3->mEnter;
		e->OnMouseLeave = Form3->mLeave;
	}
	if (sg->Cells[GetCol("Required", sg)][i] == "1") {
		l->Text = l->Text + "*";
	}
}
Form3->Label1->Position->Y = countY;
countY += 25;
Form3->Button2->Position->X = 70;
Form3->Button2->Position->Y = countY;
Form3->Button1->Position->X = 170;
Form3->Button1->Position->Y = countY;
if (combo == TrialUnitSpecimenId) {
	Form3->Button1->Enabled = false;
}

Form3->ClientHeight = countY + 30;
Form3->ClientWidth = 320;
if (combo->ItemIndex == -1) Form3->Button1Click(0);
if (!ignore) {
    Form3->Scale();
	Form3->Show();
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AddButMouseEnter(TObject *Sender) {
if (AniIndicator1->Visible) {
	return;
}
TButton *b = reinterpret_cast<TButton*>(Sender);
TComboBox *combo = reinterpret_cast<TComboBox*>(b->ParentControl);
if (combo->ItemIndex == -1) {
	Label1->Text = "Click to add a new " + combo->ListBoxResource + " record";
}
else {
    Label1->Text = "Click to view the selected " + combo->ListBoxResource  + " record details";
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::sFnBoxChange(TObject *Sender) {
String selectTxt = sFnBox->Items->Strings[sFnBox->ItemIndex];

/*TStringList *bits = new TStringList();
bits->Delimiter =  '/';
	bits->DelimitedText = Str;
	for (int i = 0; i < bits->Count; i++)

*/
int noparams = selectTxt.Pos("list/");
String tableN = selectTxt.SubString(noparams + 5, selectTxt.Length());
String s = tableN + "/list/field";

bool success = LoginWn->DoQuery(s, true);  //read ahead
if (!success) {
	Label1->Text = s;
	return;
}
LoginWn->FillTable(s);
sNameBox->Clear();
for (int j = 0; j < MainWn->StringGrid1->RowCount; j++) {
	String check = MainWn->StringGrid1->Cells[1][j]; //todo remove hardcoding of columns
	if (check.IsEmpty()) { //the main table row is not valid

	}
	else { //add a new row
		sNameBox->Items->Add(MainWn->StringGrid1->Cells[2][j]);
	}
}
Edit1->Visible = (noparams != 1);
Label7->Visible = (noparams != 1);
if (Edit1->Visible) {//lets populate for data entry

}
if (selectTxt == "list/item") {
    //itaertae over add buttons and grab all asociated fields for snamebox
}


}
//---------------------------------------------------------------------------
void __fastcall TForm1::CheckBox1Change(TObject *Sender) {
GroupBox3->Enabled = CheckBox1->IsChecked;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormResize(TObject *Sender) {
int cc = 0;
Panel1->Width = TabControl2->Width;
Panel1->Height = TabControl2->Height;
for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
	TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
	if (combo) {
		if (combo->Visible) {
			cc++;
		}
	}
}
int itemC = 0;
Layout1->Height = (ClientHeight)/Layout1->Scale->Y;
Layout1->Width = (ClientWidth)/Layout1->Scale->X;
ItemDetailBox->Height = Panel1->Height - 60;
int fact = (ItemDetailBox->Height - 168) / cc;
for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
	TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
	if (combo) {
		if (combo->Visible) {
			combo->Width = Panel1->Width - 184;
			combo->Position->Y = itemC * fact + 168;
			TButton *b = LoginWn->getButton(combo);
			b->Position->X = Panel1->Width - 170;
			itemC++;
		}
	}
}
ItemDetailBox->Width = Panel1->Width - 11;
ItemNote->Width = Panel1->Width - 184;
ItemBarcode->Width = Panel1->Width - 184;
Amount->Width = Panel1->Width - 184;
Button5->Position->X = Panel1->Width - 183;
reweighBut->Position->X = Panel1->Width - 183;
//CancelBut->Position->X = ClientWidth/Layout1->Scale->X - 30;
Label1->Width = ClientWidth/Layout1->Scale->X - 157;
ProgressBar1->Width = ClientWidth/Layout1->Scale->X - pstartpos;
//panel2
GroupBox1->Width = Panel1->Width - 11;
GroupBox3->Width = Panel1->Width - 5;
//SearchBut->Position->X = Panel1->Width * 0.8 - SearchBut->Width/2;
ShortListLocBut->Position->X = SearchBut->Position->X + 104;

if (MainWn->first_activated) {
	for (int r = 0; r < GroupBox3->Tag; r++) {
		sValue[r]->Width = Panel1->Width - 308;
	}
	SaveWindowSizeSettings();
}
ResizeGrid(offStringGrid);
ResizeGrid(onStringGrid);
if (sg) {
	ResizeGrid(sg);
}
//Button12->Position->X = Panel1->Width - 84;
//Button13->Position->X = Panel1->Width - 52;
//Image4->Width = TreeView1->Width;
//Image4->Height = TreeView1->Height;
}
//---------------------------------------------------------------------------
void TForm1::SaveWindowSizeSettings() {//save window size to settings
if (ItemDetailBox->Parent == Panel1) {
	TStringList *settings3 = new TStringList(this);
	settings3->Add(String(Height));
	settings3->Add(String(Width));
	settings3->Add(String(TreeView1->Width));
	settings3->Add(String(Layout1->Scale->X));
	settings3->Add(String(Layout1->Scale->Y));
	settings3->Add(String(Left));
	settings3->Add(String(Top));
	settings3->SaveToFile(SettingsWn->path + "settings3.txt");
	delete settings3; settings3 = NULL;
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TreeView1Resize(TObject *Sender) { //resize using splitter only
Image4->Width = TreeView1->Width;
Image4->Height = TreeView1->Height;
if (MainWn->first_activated) {
	GroupBox3->Width = Panel1->Width - 5;
	SaveWindowSizeSettings();
}
if (drag) {
	FormResize(Sender);
	WizardWn->treeWidth = TreeView1->Width;
}
}
//---------------------------------------------------------------------------
String TForm1::GetItemLabelType() {
	return ItSet->Items->Strings[ItSet->ItemIndex];
}
//---------------------------------------------------------------------------
String TForm1::GetStorageLabelType() {
	return StSet->Items->Strings[StSet->ItemIndex];
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ItSetChange(TObject *Sender) {
//need to delete search results?
iNameList->Lines->Clear();
iNameList->Lines->Add(ItSet->Items->Strings[ItSet->ItemIndex]);
for (int i = 0; i < ItSet->Count ; i++) {
	visF[i]->Parent = NULL;
	if (i != ItSet->ItemIndex) {
		iNameList->Lines->Add(ItSet->Items->Strings[i]);
	}
}
SeSettings->Repaint();
AddCheckBoxes();
String search = GetItemLabelType();
TreeView1->BeginUpdate();
for (int r = 0; r < TreeView1->Count; r++) {
	TListBoxItem *t  = TreeView1->ItemByIndex(r);
	t->Text = "        " + getTreeStrFromObject(t, search);
}
TreeView1->EndUpdate();
SaveSettings2();
}
//---------------------------------------------------------------------------
void TForm1::AddCheckBoxes() {
TStringList *settings = new TStringList(this);
if (FileExists(SettingsWn->path + "settings.txt")) {
	settings->LoadFromFile(SettingsWn->path + "settings.txt");
}

for (int i = 0; i < iNameList->Lines->Count; i++) {
	visF[i] = new TCheckBox(this);
	visF[i]->Parent = SeSettings;
    String sid = iNameList->Lines->Strings[i];
	if (!i) { //special case
		visF[i]->Enabled = false;
		visF[i]->IsChecked = true;
	}
	else if (settings->Count) {
		for (int q = 0; q < settings->Count ; q++) {
			int pos = settings->Strings[q].LastDelimiter(",");
			if (sid == settings->Strings[q].SubString(0, pos - 1)) {
				visF[i]->IsChecked = settings->Strings[q].SubString(pos + 1, 1).ToIntDef(1); //loaded from settings file
				break;
			}
		}
	}
	else visF[i]->IsChecked = true;
	visF[i]->Position->Y = (i + 1) * 18;
	visF[i]->Position->X = 8;
	visF[i]->Text = sid;
	visF[i]->Tag = i;
	visF[i]->OnChange = SettingsChange;
}
delete settings; settings = NULL;
}
//---------------------------------------------------------------------------
void TForm1::SaveSettings2() {
TStringList *settings2 = new TStringList(this);
settings2->Add(StSet->ItemIndex);
settings2->Add(ItSet->ItemIndex);

for (int i = 0; i < GroupBox4->ChildrenCount; i++) {
	TCheckBox *c = dynamic_cast<TCheckBox*>(GroupBox4->Children->Items[i]);
	if (c) {
		settings2->Add(String((int)c->IsChecked));
	}
}
settings2->SaveToFile(SettingsWn->path + "settings2.txt");
delete settings2; settings2 = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::HelpButClick(TObject *Sender) {
//todo check that pdf reader exists!
#ifndef __APPLE__
String exePath = ExtractFilePath(ParamStr(0));
ShellExecute(NULL, String("open").c_str(), String(exePath + "Readme.pdf").c_str(), NULL, String(exePath).c_str(), SW_SHOWNORMAL);
#endif
#ifdef __APPLE__
//workaround for ParamStr(0) not working in APPLE
char16_t buf[200];
int bufsize = 200;
GetModuleFileName(NULL, buf, bufsize);
String exePath = ExtractFilePath(String(buf));
system(UTF8String(String("open \"" + String(exePath + "Readme.pdf") + "\"").c_str()).c_str());
#endif
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ShortlistDispChange(TObject *Sender) {
TCheckBox *c = reinterpret_cast<TCheckBox*>(Sender);
String search = c->Text;
for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
	TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
	if (combo) {

		TLabel *lab = NULL;
		for (int i = 0; i < combo->Children->Count; i++) {
			lab = dynamic_cast<TLabel*>(combo->Children->Items[i]);
			if (lab) {
				break;
			}
		}
		String name = lab->Text;
		if (name == search) {
			combo->Visible = c->IsChecked;
		}
	}
}
FormResize(Sender);   //Todo check if this is required
SaveSettings2();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button8Click(TObject *Sender) {
if (Button8->Text == "Hide Debug") {
    MainWn->Hide();
	Button8->Text = "Show Debug";
}
else {
	MainWn->BorderStyle = TFmxFormBorderStyle::bsSizeable;
	MainWn->Image1->Visible = false;
	Application->ProcessMessages();
	MainWn->TabControl1->Visible = true;
	MainWn->TabControl2->Visible = true;
	MainWn->StatusBar1->Visible = true;
    MainWn->Show();
	MainWn->BringToFront();

	if (LogIn->Visible) {//logged out so lets login
		LoginWn->Visible = true;
		LoginWn->LoginButClick(0);
		SetUpLoggedInWindow();
	}
	SettingsWn->Button1Click(Sender);
	Button8->Text = "Hide Debug";
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button10Click(TObject *Sender) {
Form4->Visible = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button24Click(TObject *Sender) {
while (Button13->Enabled) {
	Button13Click(0);
}
sNameBox->ItemIndex = -1;
ComboBox1->Clear();
ComboBox1->ItemIndex = -1;
sOpBox->Clear();
sOpBox->ItemIndex = -1;
valEdit->Clear();
valEdit->Text = "";
SortChoice->ItemIndex = 0;
ItSort->ItemIndex = -1;
DeleteFile(SettingsWn->path + "KDLoguetemp.src");
Button24->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button12Click(TObject *Sender) {
GroupBox3->Height += 25;
Panel2->Height += 25;
SearchBut->Position->Y += 25;
ShortListLocBut->Position->Y += 25;
GroupBox6->Position->Y += 25;
GroupBox9->Position->Y += 25;

sName[GroupBox3->Tag] = new TComboBox(GroupBox3);
CopyControl(sName[GroupBox3->Tag], sNameBox);
sName[GroupBox3->Tag]->OnChange = sNameBoxChange;

sOperator[GroupBox3->Tag] = new TComboBox(GroupBox3);
CopyControl(sOperator[GroupBox3->Tag], sOpBox);
sOperator[GroupBox3->Tag]->OnChange = sOpBoxChange;

sField[GroupBox3->Tag] = new TComboBox(GroupBox3);
CopyControl(sField[GroupBox3->Tag], ComboBox1);
sField[GroupBox3->Tag]->OnChange = ComboBox1Change;

sValue[GroupBox3->Tag] = new TComboEdit(GroupBox3);
sValue[GroupBox3->Tag]->Width = valEdit->Width;
sValue[GroupBox3->Tag]->Enabled = false;
sValue[GroupBox3->Tag]->Position->Y = valEdit->Position->Y + 25 * (GroupBox3->Tag);
sValue[GroupBox3->Tag]->Position->X = valEdit->Position->X;
sValue[GroupBox3->Tag]->Parent = GroupBox3;
sValue[GroupBox3->Tag]->Tag = GroupBox3->Tag;
sValue[GroupBox3->Tag]->OnKeyUp = valEditKeyUp;
GroupBox3->Tag++;
EnabBut();
}
//---------------------------------------------------------------------------
void TForm1::CopyControl(TControl* c1, TControl * c2) {
TComboBox *combo1 = dynamic_cast<TComboBox * > (c1);
TComboBox *combo2 = dynamic_cast<TComboBox * > (c2);
combo1->Position->Y = valEdit->Position->Y + 25 * (GroupBox3->Tag);
combo1->Position->X = combo2->Position->X;
combo1->Tag = GroupBox3->Tag;
combo1->Width = combo2->Width;
combo1->Parent = GroupBox3;
combo1->Items->Assign(combo2->Items);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button13Click(TObject *Sender) {
GroupBox3->Tag--;
delete sName[GroupBox3->Tag]; sName[GroupBox3->Tag] = NULL;
delete sOperator[GroupBox3->Tag]; sOperator[GroupBox3->Tag] = NULL;
delete sValue[GroupBox3->Tag]; sValue[GroupBox3->Tag] = NULL;
delete sField[GroupBox3->Tag]; sField[GroupBox3->Tag] = NULL;

GroupBox3->Height -= 25;
Panel2->Height -= 25;
SearchBut->Position->Y -= 25;
ShortListLocBut->Position->Y -= 25;
GroupBox6->Position->Y -= 25;
GroupBox9->Position->Y -= 25;
EnabBut();
}
//---------------------------------------------------------------------------
void TForm1::EnabBut() {
Button13->Enabled = true;
Button12->Enabled = true;
if (GroupBox3->Tag == 1) {
	Button13->Enabled = false;
}
else if (GroupBox3->Tag == 19) {
	Button12->Enabled = false;
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::sOpBoxChange(TObject *Sender) {
TComboBox *combo = reinterpret_cast<TComboBox*>(Sender);
if (combo->ItemIndex == -1) return;
if (combo->ItemIndex == 6) {
	Label1->Text = "LIKE supports MySQL wildcard characters (% and ?)";
}
else if (combo->ItemIndex == 7) {
	Label1->Text = "IN requires comma separated values inside parentheses";
}
else if (combo->ItemIndex == 0 && combo->Items->Strings[combo->ItemIndex] == "contains") { //todo - make the client and server side searches consistent
	Label1->Text = "for CONTAINS simply type the text you want to find";
}
sValue[combo->Tag]->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::sNameBoxChange(TObject *Sender) {
Button24->Enabled = true;
TComboBox *comboSender = reinterpret_cast<TComboBox*>(Sender);
if (comboSender->ItemIndex == -1) return;
String tableName = comboSender->Items->Strings[comboSender->ItemIndex]; //eg SpecimenId
if (tableName == "AddedByUserId" || tableName == "LastMeasuredUserId") {
	tableName = "ItemSourceId";
}
sValue[comboSender->Tag]->Items->Clear();
sValue[comboSender->Tag]->Text = "";
sField[comboSender->Tag]->Items->Clear();
sField[comboSender->Tag]->ItemIndex = -1;
bool comboF = false;
for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
	TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
	if (combo) {
		if (combo != ItemOperation) {
			if (combo->Name == tableName) {
				TButton *b = LoginWn->getButton(combo);
				TStringGrid *sg = (TStringGrid*)b->TagObject;
				if (!sg) {
					//if (mrYes == MessageDlg("To access this option you will need to download descriptive metadata. Do it now?", TMsgDlgType::mtConfirmation,
					//		TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
						try {
							StartShortList(); //start here access violation bug! Due to new controls not being shown?
						} catch (...) {
					//	}

						sg = (TStringGrid*)b->TagObject;
					}
				}
				if (sg) {
					for (int i = 0; i < sg->RowCount; i++) {
						String fieldname = sg->Cells[GetCol("Name", sg)][i];
						if (fieldname != "Class") {
							sField[comboSender->Tag]->Items->Add(fieldname);
						}
					}
					comboF = true;
					sField[comboSender->Tag]->Visible = true;

					break;
				}
			}
		}
	}
}
if (!comboF) {
	sField[comboSender->Tag]->Visible = false;

	if (tableName == "ItemOperation") {
		sValue[comboSender->Tag]->Items->Add("");
		sValue[comboSender->Tag]->Items->Add("subsample");
		sValue[comboSender->Tag]->Items->Add("group");
		//sValue[comboSender->Tag]->Items->Add("\"subsample\"");
		//sValue[comboSender->Tag]->Items->Add("\"group\"");
	}
}
int selval = sOperator[comboSender->Tag]->ItemIndex;
sOperator[comboSender->Tag]->Items->Clear();
AddDALOperators(sOperator[comboSender->Tag]);
sOperator[comboSender->Tag]->ItemIndex = selval;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ComboBox1Change(TObject *Sender) {
TComboBox *comboSender = reinterpret_cast<TComboBox*>(Sender);
if (comboSender->ItemIndex == -1) return;
int selval = sOperator[comboSender->Tag]->ItemIndex;
sOperator[comboSender->Tag]->Items->Clear();
sOperator[comboSender->Tag]->Items->Add("=");
sOperator[comboSender->Tag]->Items->Add("contains");
if (selval <= 1) {
	sOperator[comboSender->Tag]->ItemIndex = selval;
}
String tableName = sName[comboSender->Tag]->Items->Strings[sName[comboSender->Tag]->ItemIndex]; //eg SpecimenId
if (tableName == "AddedByUserId" || tableName == "LastMeasuredUserId") {
	tableName = "ItemSourceId";
}
sValue[comboSender->Tag]->Items->Clear();
sValue[comboSender->Tag]->Text = "";
String fieldName = comboSender->Items->Strings[comboSender->ItemIndex];
//populate the edit box to help the user select a known value
for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {
	TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
	if (combo) {
		if (combo != ItemOperation) {
			if (combo->Name == tableName) {
				if (!ignore) {
					Label1->Text = "Getting " + String(combo->Count) + " metadata records from " + tableName;
					AniIndicator1->Visible = true;
					Application->ProcessMessages();
				}
				String id = Form1->getMapping(combo);
				int max = combo->Count;
				TStringList *nsl = new TStringList();
				nsl->Sorted = true;
				nsl->Duplicates = TDuplicates::dupIgnore;    //only show unique items
				int i;
				for (i = 0; i < max; i++) {
					TStringList* sl = (TStringList*)combo->Items->Objects[i];
					String item = sl->Values[id];
					if (item.ToIntDef(0) < 0) {
					}
					else {
						nsl->Add(sl->Values[fieldName]);
					}
				}
				Application->ProcessMessages();
				sValue[comboSender->Tag]->Items->Assign(nsl);
				delete nsl;
				if (!ignore) {
					if (i == max) Label1->Text = "Select from the " + String(sValue[comboSender->Tag]->Count) + " values found or type some text";
					else Label1->Text = "User interrupted lookup";
					AniIndicator1->Visible = false;
					Application->ProcessMessages();
				}
				break;
			}
		}
	}
}
}
//---------------------------------------------------------------------------
void TForm1::AddDALOperators(TComboBox *comb) {
for (int i = 1; i <= Oplist->Count ; i++) {
	String sid = Oplist->Items->Strings[i - 1];
	comb->Items->Add(sid);
}
}
//---------------------------------------------------------------------------
void TForm1::ClearMetaData() {
Label1->Text = "Please wait....clearing metadata";
Application->ProcessMessages();
for (int ri = 0; ri < ItemDetailBox->ChildrenCount; ri++) {//do a clear for all fields
	TComboBox *combo = dynamic_cast<TComboBox*>(ItemDetailBox->Children->Items[ri]);
	if (combo) {
		if (combo != Form1->ItemOperation) {
			String index = combo->Name;
			DeleteFile(updateFile +  "." + index);
			for (int i = 0; i < combo->Count ; i++) {
				TStringList *l = (TStringList*)combo->Items->Objects[i];
				delete l; l = NULL;
			}
			combo->Items->Clear();

			//TButton *b = LoginWn->getButton(combo);
			//delete b->TagObject; b->TagObject = NULL;
			//DeleteFile(updateFile +  "." + b->Name);
		}
	}
}
Label1->Text = "";
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SortChange(TObject *Sender) {
ItSort->Visible = (SortChoice->ItemIndex != 0);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::UpdateButClick(TObject *Sender) {
#ifndef __APPLE__
// specify the source URL
const String url("http://software.kddart.com/KDLog/test2.txt");
// specify the fully-qualified target filename
String filename = SettingsWn->path + "update2.txt";
// download the file from the URL to APP_DATA directory (try twice)
HRESULT hRes = URLDownloadToFile(NULL, url.c_str(), filename.c_str(), NULL, NULL);
if (hRes != S_OK) hRes = URLDownloadToFile(NULL, url.c_str(), filename.c_str(), NULL, NULL);

if (hRes != S_OK) ShowMessage("KDLogue can't access the internet. Please check your connection and try again");
else {
	TMemo *nm=new TMemo(this);
	nm->Parent=this;
	nm->Visible=false;
	nm->Lines->LoadFromFile(filename);
	DeleteFile(filename);
	String version=nm->Lines->Strings[0];
	int pos = version.LastDelimiter(".");
	String lastpart = version.SubString(pos + 1, version.Length() - pos + 1);
	int minorv = lastpart.TrimRight().ToIntDef(0);
	int newver = version.SubString(1,1).ToIntDef(0)*10000 + version.SubString(3,1).ToIntDef(0)*1000 + version.SubString(5,1).ToIntDef(0)*100 + minorv;
	pos = MainWn->KDVersion.LastDelimiter(".");
	lastpart = MainWn->KDVersion.SubString(pos + 1, MainWn->KDVersion.Length() - pos + 1);
	minorv = lastpart.TrimRight().ToIntDef(0);
	int cuver = MainWn->KDVersion.SubString(1,1).ToIntDef(0)*10000 + MainWn->KDVersion.SubString(3,1).ToIntDef(0)*1000 + MainWn->KDVersion.SubString(5,1).ToIntDef(0)*100 + minorv;
	if (newver > cuver) {
		if (mrYes == MessageDlg("An update version " + version + " is available. Do you want to download and replace this version " + MainWn->KDVersion, TMsgDlgType::mtConfirmation,
				TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
			const String url("http://software.kddart.com/KDLog/KDLog.exe");
			// specify the fully-qualified target filename
			String filename = SettingsWn->path + "KDLog.exe";
			LoginWn->WriteToLog("Update to " + version + " attempted");
			String filenz=ExtractFilePath(ParamStr(0)) + "Updater.exe" ;
			if (!FileExists(filenz)) {
				ShowMessage("No updater found at " + filenz);
				LoginWn->WriteToLog("No updater found at " + filenz);
				return;
			}

			// download the file from the URL to our temp location
			Label1->Text = "Please wait...downloading update from server";
			AniIndicator1->Visible = true;
			Application->ProcessMessages();
			HRESULT hRes = URLDownloadToFile(NULL, url.c_str(), filename.c_str(), NULL, NULL);
			if (hRes != S_OK) hRes = URLDownloadToFile(NULL, url.c_str(), filename.c_str(), NULL, NULL);
			AniIndicator1->Visible = false;
			if (hRes != S_OK) {
				ShowMessage("Error: can't download the update.");
				LoginWn->WriteToLog("Error: can't download the update");
				Label1->Text = "Error: can't download the update";
			}
			else {
				Label1->Text = "Download completed";
				STARTUPINFO si;
				PROCESS_INFORMATION pi;
				ZeroMemory( &si, sizeof(si) );
				si.cb = sizeof(si);

				String mess="cmd.exe /c \"" + filenz  + "\" " + String(getpid()) + " " + SettingsWn->path + + " " + String(cuver) + " " + String(newver);
				//ShowMessage(mess);
				if( !CreateProcess( NULL, // No module name (use command line).
					(mess).c_str(), // Command line.
					NULL,             // Process handle not inheritable.
					NULL,             // Thread handle not inheritable.
					FALSE,            // Set handle inheritance to FALSE.
					0,                // No creation flags.
					NULL,             // Use parent's environment block.
					NULL,             // Use parent's starting directory.
					&si,              // Pointer to STARTUPINFO structure.
					&pi )) {             // Pointer to PROCESS_INFORMATION structure.

					ShowMessage( "CreateProcess failed");
					LoginWn->WriteToLog("CreateProcess failed using " + mess);
					return;
				}
				else {
					updating = true;
                }
			}
		}
	}
	else {
		ShowMessage("No updates are available at this time");
		return;
	}
	delete nm;
	DeleteFile(filename);
	Close();
} //end download ok
#endif
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action) {
TModalResult answer = mrYes;
#ifndef _DEBUG
if (!updating) {
	if (Action != TCloseAction::caNone) {//the program is to be closed in non-debug mode
		answer = MessageDlg("Are you sure you want to exit KDLogue?", TMsgDlgType::mtConfirmation,
			TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0);
	}
}
#endif
if (answer != mrYes) {
	Action = TCloseAction::caNone;
	MainWn->Visible = false;
}
else {
	//if (LogOut->Visible) LoginWn->LogoutButClick(Sender);
	Form3 = new TForm3(this);
	Form3->Caption = "Please wait...";
	MainWn->Image1->Parent = Form3;
	MainWn->LabelDaRT->Text = "Closing KDLogue";
	MainWn->LabelDaRT->Align = TAlignLayout::alBottom;
	MainWn->Image1->Visible = true;
    Application->ProcessMessages();
	Form3->ClientHeight = 50;
	Form3->ClientWidth = 100;
	Form3->Scale();
	Form3->Show();
	Application->ProcessMessages();
	TreeViewClear();
	MainWn->Close();
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button19Click(TObject *Sender) {
TMemo* res = new TMemo(this);
if (!sg) {
	ShowMessage("No results to export");
	return;
}
SaveDialog1->Title = "Enter a name for your results (.csv)";
if (SaveDialog1->Execute()) {
	if (!SaveDialog1->FileName.Pos(".csv")) {
		SaveDialog1->FileName = SaveDialog1->FileName + ".csv";
	}
	for (int i = 0; i < sg->RowCount; i++) {//iterate over the main table rows
		String lineTxt = "";
		for (int j = 1; j < sg->ColumnCount; j++) {    //iterate over the current search results headers
			String s1;
			if (i==0) s1 = ((TStringColumn*)sg->ColumnByIndex(j))->Header;
			else s1 = sg->Cells[j][i];
			lineTxt += s1 + ",";
		}
		res->Lines->Add(lineTxt);
	}
}
res->Lines->SaveToFile(SaveDialog1->FileName);
Label1->Text = "Results exported to " + SaveDialog1->FileName;
delete res; res = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TreeView1MouseMove(TObject *Sender, TShiftState Shift, float X, float Y) {
TListBoxItem *t = TreeView1->ItemByPoint(X, Y);
if (t) {
	exptxt = t->Text.TrimLeft();
	
	/*if (t->Level() == 1) {
		Label1->Text = GetStorageLabelType() + " " + t->Text + " containing " + String(t->Count) + " items";
	}
	else*/
		TColorButton *col = GetButton(t);
		String status = " (not modified)";
		if (col->Color == TAlphaColorRec::Black) {
			status = " (to be deleted)";
		}
		else if (col->Color == TAlphaColorRec::White) {
			status = " (to be added)";
		}
		else if (col->Color == TAlphaColorRec::Red) {
			status = " (locally modified)";
		}
		Label1->Text = GetItemLabelType() + " " + t->Text.TrimLeft() + status;


}
else {
	Label1->Text = hintText;
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TreeView1MouseEnter(TObject *Sender) {
//hintText = Label1->Text;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TreeView1MouseLeave(TObject *Sender) {
TFmxObject *l1;
TPointF p = Screen->MousePos();
IControl *a = ObjectAtPoint(p);
if (a) {
	l1 = a->GetObject();
	//TFmxObject *l2 = l1->Parent
	if (l1->ClassName() == "TSpeedButton") { //todo don't use classname, derive ListBoxItem as parent and use its isexpanded property
		Label1->Text = "Click to " + exptxt;
	}
	else Label1->Text = hintText;
}
else Label1->Text = hintText;
}
//---------------------------------------------------------------------------
/*  https://forums.embarcadero.com/thread.jspa?messageID=629061&#629061
void __fastcall TForm1::Button15Click(TObject *Sender)
{
//TRestThread* Thrd = new TRestThread(2);
//Thrd->OnTerminate = &RestThreadTerminated;
//Thrd->Start();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::RestThreadTerminated(TObject *Sender)
{
TRestThread *Thrd = (TRestThread*) Sender;
ShowMessage(String(Thrd->PString));
if (1==1)//Thrd->Terminate())// ReturnValue == 1)
{
// do something...
}

else
{
// do something else...
}
}
*/
//---------------------------------------------------------------------------
void __fastcall TForm1::LoadResButClick(TObject *Sender) {
//todo add database name to metadata files as metadata needs to match db, eg check for db + metadata during login)
bool result = true;
if (Sender) {
	OpenDialog1->Title = "Select your previously saved search (.src) file";
	result = OpenDialog1->Execute();
}
if (result) {
	if (Sender) {
		if (!Login()) return;
	}
	String fn = OpenDialog1->FileName;
	if (!Sender) {
		fn = SettingsWn->path + "KDLoguetemp.src";
	}
	if (!FileExists(fn)) {
		return;
	}
	TMemo *m = new TMemo(this);
	m->Lines->LoadFromFile(fn);
	while (Button13->Enabled)  {
		Button13Click(Sender);
	}
	while (GroupBox3->Tag < m->Lines->Count - 2) {
		Button12Click(Sender);
	}
	SortChoice->ItemIndex = m->Lines->Strings[0].ToIntDef(-1);
	ItSort->ItemIndex = m->Lines->Strings[1].ToIntDef(-1);
	ignore = true;
	for (int j = 2; j < m->Lines->Count; j++) {
		String dataS = m->Lines->Strings[j];
		int p = dataS.Pos(";");
		int count = 0;
		while (p >= 1 ) {
			count++;
			String extract = dataS.SubString(0 , p - 1);
			if (count == 1) {
				sName[j - 2]->ItemIndex = extract.ToIntDef(-1);
			}
			else if (count == 2) {

				sOperator[j - 2]->ItemIndex = extract.ToIntDef(-1);
			}
			else if (count == 3) {
				if (sField[j - 2]->Count > extract.ToIntDef(-1)) sField[j - 2]->ItemIndex = extract.ToIntDef(-1);
			}

			dataS = dataS.SubString(p + 1, dataS.Length() - p);
			p = dataS.Pos(";");
			if (!p) {
				sValue[j - 2]->Text = dataS;
			}
		}
	}
	delete m; m = NULL;
	ignore = false;
	if (Sender) {
		Label1->Text = "Saved search loaded from " + fn;
    	TabControl1->TabIndex = 0;
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SaveResButClick(TObject *Sender) {
TMemo* res = new TMemo(this);
bool result = true;
if (Sender) {
	SaveDialog1->Title = "Enter name for your search (.src)";
	result = SaveDialog1->Execute();
	if (!SaveDialog1->FileName.Pos(".src")) {
		SaveDialog1->FileName = SaveDialog1->FileName + ".src";
	}
}
if (result) {
	int count = 0;
	for (int r = 0; r < GroupBox3->Tag; r++) {
		if (sOperator[r]->ItemIndex != -1 && sName[r]->ItemIndex != -1 && sValue[r]->Text != "") {
			String lineTxt  = String(sName[r]->ItemIndex) + ";"
							+ String(sOperator[r]->ItemIndex) + ";"
							+ String(sField[r]->ItemIndex) + ";"
							+ sValue[r]->Text;
			res->Lines->Add(lineTxt);
			count++;
		}
	}
	if (count) {
		res->Lines->Insert(0, String(ItSort->ItemIndex));
		res->Lines->Insert(0, String(SortChoice->ItemIndex));
		String fn = SaveDialog1->FileName;
		if (!Sender) {
			fn = SettingsWn->path + "KDLoguetemp.src";
		}
		res->Lines->SaveToFile(fn);

		if (Sender) Label1->Text = "Current search saved to " + fn;
	}
	else if (Sender) ShowMessage("Nothing to save");
}
delete res; res = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::valEditKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar, TShiftState Shift) {
//2 up key calls in a row means the motorola barcode scanner has been used as input (todo: test other scanners and/or id proper way!)
//Repup++;
//if (Repup > 1) {
/*if (BarcodeSearch.Pos(".")) {  ////todo use |* to start, to end  *|
		double val = BarcodeSearch.ToDouble();
		TDateTime d1 = (val / 86400.0);
		unsigned short year, month, day;
		DecodeDate(d1, year, month, day);
		if (year >= 2014) { //crc
			BarcodeI = true;
			//give the timer some msec to grab the rest of the barcode (after the .
			Label1->Text = "Reading " + BarcodeSearch;
			Timer1->Enabled = true;
			Label1->Repaint();
	}
}
*/
/*if (Key == VK_DOWN || Key == VK_UP) {
	TreeView1Click(TreeView1);
}
else*/ if (BarcodeSearch.Pos("|*") && BarcodeSearch.Pos("*|")) {  //use |* to start, to end  *|
	BarcodeI = true;
	//give the timer some msec to grab the rest of the barcode (after the |*
	Label1->Text = "Reading " + getBarcodeType(BarcodeSearch) + BarcodeSearch;
	Timer1->Enabled = true;
	Label1->Repaint();
}


if (Key == 13 && valEdit->IsFocused) {
	SearchButClick(Sender);
}
}
//---------------------------------------------------------------------------
String TForm1::getBarcodeType(String Search) {
if (Search.Pos("|*I")) {
	return "Item";
}
else if (Search.Pos("|*S")) {
	return "Storage";
}
else {
	return "Unknown";
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::valEditKeyDown(TObject *Sender, WORD &Key, System::WideChar &KeyChar, TShiftState Shift) {
if (KeyChar != '\0') {//todo use |* to start, to end  *|
	BarcodeSearch = BarcodeSearch + KeyChar;
}

}
//---------------------------------------------------------------------------
void __fastcall TForm1::TabControl1Change(TObject *Sender) {
if (TabControl1->Tabs[TabControl1->TabIndex]->Text == "Trials") {
	//WizardWn->treeWidth = TreeView1->Width;
	//TreeView1->Width = 1;
	ResizeGrid(offStringGrid);
	ResizeGrid(onStringGrid);
}
//else TreeView1->Width = WizardWn->treeWidth;
SaveLoggedStatus();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button17Click(TObject *Sender) {
TButton *a = reinterpret_cast<TButton*>(Sender);
UseCase = reinterpret_cast<TComboBox*>(a->ParentControl);
if (!UseCase) {
	UseCase = a;
}
while (Scan->wait) Application->ProcessMessages();
if (Button5->Text == "Re-scan") {
	Scan->Start();
}
else {
	Button5->Text = "Re-scan";
	Scan->StopCamera = true;
}
/*
 */
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TreeView1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y) {
if (Button == TMouseButton::mbRight) {
	TListBoxItem *tRight = TreeView1->ItemByPoint(X, Y);
	if (tRight) {
		tRight->IsSelected = true;
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::RemoveNodeClick(TObject *Sender) { //todo update search results?
TListBoxItem *t = TreeView1->Selected;
if (!t) {
	return;
}
if (mrYes == MessageDlg("Are you sure you want to remove " + t->Text.TrimLeft() + " from your shortlist?", TMsgDlgType::mtWarning,
	TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
	Label1->Text = "  Item "+ t->Text.TrimLeft() + " removed from shortlist";
	RemoveTreeItem(t);

	selButton = NULL;
	//TreeView1->Selected = NULL;
	TreeView1Click(TreeView1);
	UpdateItemsTBU();
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ZoomClick(TObject *Sender) {
Form5 = new TForm5(this);
Form5->Layout1->Scale->X = Layout1->Scale->X;
Form5->Layout1->Scale->Y = Layout1->Scale->Y;
Form5->ClientHeight = Form5->ClientHeight * Layout1->Scale->Y;
Form5->ClientWidth = Form5->ClientWidth * Layout1->Scale->X;
Form5->Show();
}
//---------------------------------------------------------------------------
void TForm1::ShowBarcode(TObject *control) {
if (control == ItemBarcode) {

	if (im2) {
		delete im2; im2 = NULL;
	}
	im2 = new TImage(this);
	im2->Parent = ItemBarcode;
	im2->Position->X = -64;
	im2->Height = 40;
	im2->Width = 40;
	if (!Scan->wait) ConvertImageToBitmap(im2);


}
else {
	

}
}
//---------------------------------------------------------------------------
void TForm1::ConvertImageToBitmap(TImage *image) {
if (Barcode1->InputText.IsEmpty()) {
	return;
}
try {
	image->Bitmap->Assign(Barcode1->Bitmap);
	image->Scale->Y =  image->Height/(double)Barcode1->Bitmap->Height;
	image->Scale->X =  image->Width/(double)Barcode1->Bitmap->Width;
}
catch (const EBarcodeError *e) {     //// use E.ErrorCode, E.Message, and E.ErrorMessage as needed...
		LoginWn->WriteToLog(e->Message);
		Label1->Text = e->Message;
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ComboBoxSymbologyChange(TObject *Sender) {
Barcode1->SymbologyName = ComboBoxSymbology->Items->Strings[ComboBoxSymbology->ItemIndex] ;
ShowBarcode(ItemBarcode);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::PrintIBarButClick(TObject *Sender) {
PrintWn = new TPrintWn(this);
PrintWn->Scale();

TButton *b = reinterpret_cast<TButton*>(Sender);
PrintWn->PrintSetup(b->Name);
PrintWn->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::reweighButClick(TObject *Sender) {
reweighBut->Enabled = false;
Application->ProcessMessages();
String units;
String final_val = ReadScale(units);
if (!final_val.IsEmpty()) {
	Amount->Text = final_val.Trim();
	EdChange(Amount);
	ItemUnitId->ItemIndex = ItemUnitId->Items->IndexOf(units);  //todo remove harcoded
	ScaleId->ItemIndex = ScaleId->Items->IndexOf("El Batan Seed Scale 1");
	if (ItemUnitId->ItemIndex == -1 || ScaleId->ItemIndex == -1) {
		ShowMessage("Weighing error: scales or units are not known in database");
		reweighBut->Enabled = true;
		return;
	}
	UpdateSeedProgress();
	//todo window size at startup - call resize
	//save every address entry to file
}
reweighBut->Enabled = true;
}
//---------------------------------------------------------------------------
String TForm1::ReadScale(String &units) {

if (ScalesSetting->Text.Pos("cimmyt")) {
	const String url(ScalesSetting->Text);
	// specify the fully-qualified target filename
	String filename = SettingsWn->path + "scalesread.txt";

	TFileStream *dlf;
	dlf = new TFileStream(filename, fmCreate);
	try {
		IdHTTP1->Get(url,dlf);
		delete dlf;  dlf = NULL;
	}
	catch (...) {
			//todo automatic detection of the scale connection
			Label1->Text = "No scale reading. Ensure scales are turned ON + connected";
			if (dlf) {
				delete dlf;  dlf = NULL;
			}
			return "";
	}
	TMemo *nm=new TMemo(this);
	nm->Parent=this;
	nm->Visible=false;
	nm->Lines->LoadFromFile(filename);
	if (!DeleteFile(filename)) ShowMessage("error deleting " + filename);
	//validate that the value is a double
	String cutoffStr = " kg GR";
	if (nm->Lines->Strings[0].Pos("-")) {
		Label1->Text = "Negative scale reading ignored. Please calibrate your scales";
		return "";
	}
	int po = nm->Lines->Strings[0].Pos(cutoffStr);
	String final_val;
	if (!po) { //vibra
		final_val = nm->Lines->Strings[0];
		while (final_val[1] == '0' && final_val[2] != '.') final_val.Delete(1,1);

		units = "grams";
	}
	else {
		final_val = nm->Lines->Strings[0].SubString(0, po);
		units = "kilograms";
	}
	Label1->Text = "Scales read: '" + nm->Lines->Strings[0] + "' = " + final_val + " " + units;

	delete nm; nm = NULL;
	return final_val;
}
else {
	Label1->Text = "Scales setting invalid: needs to contain 'cimmyt'";
}
return "";
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SubsampleClick(TObject *Sender) {
TListBoxItem* tsel = TreeView1->Selected;
String capText = "Seed Preparation";
if (WizardWn->tstate == 2) capText = "Seed Harvesting";
if (!getTreeStrFromObject(tsel, "ItemGroupId").IsEmpty()) {
	ShowMessage("You can't " + Form1->MenuItem2->Text + " destination items when in " + capText + " mode!");
	return;
}

SampleWn = new TSampleWn(this);
SampleWn->GroupBox1->Text = capText;
SampleWn->AutoWeigh->IsChecked = WizardWn->autoweigh;
/*if (WizardWn->tstate == 2) { //HARVEST
	SampleWn->TabControl1->Tabs[0]->Visible = true;
	SampleWn->TabControl1->TabIndex = 0;
}
else {
	SampleWn->TabControl1->Tabs[0]->Visible = false;
	SampleWn->TabControl1->TabIndex = 1;
}*/
SampleWn->Scale();

String searchId = getTreeStrFromObject(TreeView1->Selected, "ItemId");
for (int i = TreeView1->Count - 1; i >=0 ; i--) {
	TListBoxItem* t = TreeView1->ItemByIndex(i);
	NodeData *origd = (NodeData*)(t->Data);
	//if (WizardWn->tstate == 2) searchId = searchId + " Harvest";
	if (origd->final->Values["ItemGroupId"] == searchId) {//add to the prepare seed list.
	//Start here - for harvest change the itemId from "new" to the "Range3|Plot4".
	//ItemGroupId should have "Harvest" removed but then need another way to set it to green
	//If leave "Harvest" in the itemgroup just use .Pos("Harvest")==0 to turn it green and set the comparison to remove "Harvest"
        NodeData *d = new NodeData();
		d->orig->Assign(origd->orig);
		d->final->Assign(origd->final);
		SampleWn->ListBox1->Items->AddObject(d->final->Values[GetItemLabelType()], (TObject*)d);
	}
}
SampleWn->ListBox1->ItemIndex = SampleWn->ListBox1->Items->Count - 1;

SampleWn->ProgressBar1->Max = SampleWn->ListBox1->Count;
SampleWn->ProgressBar1->Value = SampleWn->CalcMeasured();

SampleWn->Caption = Form1->MenuItem2->Text + " from " + SampleWn->getselText();
SampleWn->ShowModal();
if (SampleWn->ModalResult == mrYes) {
	bool mustupdatesource = false;
	for (int i = 0; i < TreeView1->Count; i++) {
		TListBoxItem* t = TreeView1->ItemByIndex(i);
		if (getTreeStrFromObject(t, "ItemGroupId") == searchId) {
			/*if (getTreeStrFromObject(t, "ItemId") == "new") {//"new" item found in shortlist so search trial setup
				bool founditem = false;
				for (int r = SampleWn->ListBox2->Items->Count - 1; r >= 0; r--) {
					TListBoxItem* lb = SampleWn->ListBox2->ItemByIndex(r);
					if (getListStrFromObject(lb, "DateAdded") == getTreeStrFromObject(t, "DateAdded")) {
						founditem = true;
						SampleWn->ListBox2->Items->Delete(r); //this item has been dealt with
						break;
					}
				}
				if (!founditem) { //the item no longer exists in the trial setup
					t->Parent = NULL;  //so lets delete it from the shortlist
					itemsTBU--;
				}
			}
			else*/  {//a "previously prepared" item found in the shortlist so verify if the amount has changed, etc
				for (int r = SampleWn->ListBox1->Items->Count - 1; r >= 0; r--) {
					TListBoxItem* lb = SampleWn->ListBox1->ItemByIndex(r);
					if (getListStrFromObject(lb, "ItemBarcode") == getTreeStrFromObject(t, "ItemBarcode")) { //item is found
						if (getListStrFromObject(lb, "Amount") != getTreeStrFromObject(t, "Amount")) {
                            NodeData *d = (NodeData*)t->Data;
							d->final->Values["ItemUnitId"] = getListStrFromObject(lb, "ItemUnitId");
							d->final->Values["ScaleId"] = getListStrFromObject(lb, "ScaleId");
							d->final->Values["Measured"] = "1";
							mustupdatesource = true;
							t->IsSelected = true;  //select the node
							TreeView1Click(TreeView1); //ensure its selected
							Amount->Text = getListStrFromObject(lb, "Amount");
							EdChange(Amount);

						}
						break; //ignore cos item amount has not changed
					}
				}
			}
		}
	}
	/*for (int r = SampleWn->ListBox2->Items->Count - 1; r >= 0; r--) {
		NodeData *d = (NodeData*)SampleWn->ListBox2->ItemByIndex(r)->Data;
		itemsTBU++;
		Update1->Enabled = true;
		TListBoxItem* v = AddChildNode(tsel->Index + 1, d);
		TColorButton *but = GetButton(v);
		but->Color =  TAlphaColorRec::White;
		v->IsSelected = true;
		TreeView1Click(TreeView1);
		UpdateItemsTBU();
	}*/
	if (mustupdatesource) {
		tsel->IsSelected = true;  //select the source node
		TreeView1Click(TreeView1); //ensure its selected
		Amount->Text = SampleWn->AmountBox->Text;
		EdChange(Amount);
	}
}
delete SampleWn; SampleWn = NULL;
UpdateSeedProgress();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::MenuItem3Click(TObject *Sender) {
PrepareWn = new TPrepareWn(this);
PrepareWn->Scale();
PrepareWn->Setup();
PrepareWn->ShowModal();
}
//---------------------------------------------------------------------------
void TForm1::FindInTree(String searchStr) {
if (BarcodeI) { //currently only handle barcode input and searching. Todo search by other means?
	BarcodeI = false;
	BarcodeSearch = "";
	String byStr;
	String find = "";
	String btype = getBarcodeType(searchStr);
	if (btype == "Item" || btype == "Storage") {
		if (btype == "Item") {
			byStr = "ItemBarcode";
		}
		else if (btype == "Storage") {
			byStr = "StorageId";
			//iterate over storageid combo to retrieve the storageid of the matching storage barcode
			//note return the first item only
			for (int i = 0; i < StorageId->Count; i++) {
				TStringList *stl = (TStringList*)StorageId->Items->Objects[i];
				if (stl->Values["StorageBarcode"] == searchStr) {
					searchStr = stl->Values["StorageId"];
					find = "true";
					break;
				}
			}
			if (find.IsEmpty()) {
				ShowMessage("Storage location is not known in KDLogue. Please update your metadata");
				Scan->Beep(1);
				return;
			}
		}
	}
	else {
		Label1->Text = "Nothing implemented for " + btype + " barcode (" + searchStr + ")";
		Scan->Beep(1);
		return;
	}
	bool itemfound = false;
	for (int i = 0; i < TreeView1->Count; i++) {
		TListBoxItem* t = TreeView1->ItemByIndex(i);
		find = getTreeStrFromObject(t, byStr);
		if (find == searchStr) {////todo wrt storageId what happens with multiple items?
			t->IsSelected = true;  //select the found node
			TreeView1Click(TreeView1); //ensure its selected
			itemfound = true;
			Label1->Text = btype +" Barcode (" + searchStr + ") found in the shortlist";
			break;
		}
	}
	if (!itemfound) {
		Label1->Text = "No " + btype + " barcode (" + searchStr + ") exists in the shortlist";
		Scan->Beep(1);
		return;
		//todo for harvest could set the current item's storage
	}
    if (WizardWn->currentpage == 6) {//Validate order
		if (btype != "Storage") {
			if (WizardWn->tstate == 0) {//PREPARE
				for (int i = 0; i < TreeView1->Count; i++) {
					TListBoxItem *t = TreeView1->ItemByIndex(i);
					if (t->Visible) {
						TColorButton *but = GetButton(t);
						if (but->Color == TAlphaColorRec::White) { //find first white one
							if (TreeView1->Selected == t) {
								but->Color = TAlphaColorRec::Green;
								Scan->Beep(0);
								return;
							}
							break;
						}
					}

				}
				Scan->Beep(1);
				return;
			}
		}
	}
	else if (WizardWn->currentpage == 4 || TabControl1->TabIndex == 2) {
		if (btype != "Storage") {
			Scan->Beep(0);
			SubsampleClick(0);
			return;
		}
	}
	Scan->Beep(0);
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer1Timer(TObject *Sender) {
Timer1->Enabled = false;
if (BarcodeI) {
	Label1->Text = "Barcode read as " + BarcodeSearch;
	if (!SampleWn) {
		if (TabControl1->TabIndex == 0) { //todo move this into a new function
			String btype = getBarcodeType(BarcodeSearch);
			if (btype == "Item" || btype == "Storage") {
				//reset the search GUI and then set the search fields to search for itembarcode = val
				Scan->Beep(0);
				Button24Click(0);
				if (btype == "Item") {
					sNameBox->ItemIndex = sNameBox->Items->IndexOf("ItemBarcode");
				}
				else if (btype == "Storage") {
					sNameBox->ItemIndex = sNameBox->Items->IndexOf("StorageId");
					ComboBox1->ItemIndex = ComboBox1->Items->IndexOf("StorageBarcode");
				}
			}
			else {//can't do anything
				Scan->Beep(1);
				BarcodeI = false;
				BarcodeSearch = "";
				return;
			}

			sOpBox->ItemIndex = sOpBox->Items->IndexOf("=");
			valEdit->Text = BarcodeSearch;
			SearchButClick(Sender);
		}
		else if (TabControl1->Tabs[1]->Enabled) {//todo do this search if shortlist tab was selected
			FindInTree(BarcodeSearch);
		}
		else {//can't do anything
			BarcodeI = false;
			BarcodeSearch = "";
		}
	}
	else  {
		SampleWn->FindInListbox(BarcodeSearch);
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button21Click(TObject *Sender) {
//simple way of subsampling every single item once in the shortlist (temp simulation)
/*for (int r = 0; r< TreeView1->Count; r++) {
	TListBoxItem *t = TreeView1->ItemByIndex(r);
	NodeData *origd = (NodeData*)t->Data;
	NodeData *d = new NodeData();
	d->orig->Assign(origd->orig);
	d->final->Assign(origd->final);
	TStringList *l;
	//collect the datetime right now
	TDateTime dateNow = Now();
	for (int i=0; i < 2; i++) {
		if (!i) l = d->orig;
		else l = d->final;
		String ds = dateNow.FormatString("yyyy-MM-dd HH:mm:ss");
		l->Values["DateAdded"] = ds;
		l->Values["ItemGroupId"] = l->Values["ItemId"];
		l->Values["ItemId"] = "new";
		l->Values["Amount"] = "0";
		l->Values["ItemBarcode"] = "|*I" + String((double)dateNow * 86400) + "*|";
		//	+ FormatFloat("000", Random(999)); ;
	}
	itemsTBU++;
	Update1->Enabled = true;
	TListBoxItem* v = AddChildNode(t->Index + 1, d);
	TColorButton *but = GetButton(v);
	but->Color =  TAlphaColorRec::White;
	Caption = Caption + String(r);
	TreeView1->Selected = v;
	TreeView1Click(TreeView1);
	UpdateItemsTBU();
}
*/
//code to update all shortlisted barcodes
/*for (int r = 0; r< TreeView1->Count; r++) {
	TListBoxItem *t = TreeView1->ItemByIndex(r);
	NodeData *d = (NodeData*)t->Data;

	TDateTime dateNow = Now();
	TStringList *l;
	for (int i=0; i < 2; i++) {
		if (!i) l = d->orig;
		else l = d->final;
		String ds = dateNow.FormatString("yyyy-MM-dd HH:mm:ss");
		l->Values["ItemBarcode"] = "|*I" + String((double)dateNow * 86400) + "*|";
		//	+ FormatFloat("000", Random(999)); ;
	}
	itemsTBU++;
	Update1->Enabled = true;
	TColorButton *but = GetButton(t);
	but->Color =  TAlphaColorRec::Red;
	Caption = Caption + String(r);
	TreeView1->Selected = t;
	TreeView1Click(TreeView1);
}
UpdateItemsTBU();*/
//segregate items evenly throughout "shelf" storage locations
/*for (int r = 0; r< TreeView1->Count; r++) {
	TListBoxItem *t = TreeView1->ItemByIndex(r);
	TreeView1->Selected = t;
	TreeView1Click(TreeView1);
	if (r < TreeView1->Count/4) {
		StorageId->ItemIndex = 4;
	}
	else if (r < TreeView1->Count/2) {
		StorageId->ItemIndex = 5;
	}
	else if (r < (TreeView1->Count/3)*4) {
		StorageId->ItemIndex = 6;
	}
	else StorageId->ItemIndex = 7;
}
UpdateItemsTBU();*/
//set the source items to have a nice label
for (int r = 0; r< TreeView1->Count; r++) {
	TListBoxItem *t = TreeView1->ItemByIndex(r);
	t->IsSelected = true;
	TreeView1Click(TreeView1);
	ItemNote->Text = "Source " + getTreeStrFromObject(t, "SpecimenId");
	EdChange(ItemNote);
}
UpdateItemsTBU();
}
//---------------------------------------------------------------------------
void TForm1::extractxy(int &x, int &y, String &s) {
int pos = s.Pos("|");
int spos = pos, epos = s.Length() + 1;
while (x != -1) {
	spos--;
	x = s.SubString(spos, pos - spos).ToIntDef(-1);

}
int temp = s.Pos(" Harvest");
if (temp) {
	s = s.SubString(0, temp - 1) + "0";
	epos = s.Length() + 1;
}
while (y != -1) {
	epos--;
	y = s.SubString(epos, s.Length() + 1 - epos).ToIntDef(-1);
}
spos++;
epos++;
x = s.SubString(spos, pos - spos).ToIntDef(0);
y = s.SubString(epos, s.Length() + 1 - epos).ToIntDef(0);

}
//---------------------------------------------------------------------------
int TForm1::SpecialSort(String s1, String s2) {
int result = 0;
int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
extractxy(x1, y1, s1);
extractxy(x2, y2, s2);
/*if (x1 == 0 || y1 == 0 || x2 == 0 || y2 == 0) {
int rrr=3;
rrr++;

}
*/
/*if (x1 != 0 && y1 != 0 && x2 != 0 && y2 != 0) {
int rrr=3;
rrr++;
}
//String res1 = rx1 + ry1;
//String res2 = rx2 + ry2;
//return result = CompareStr(res1, res2);

*/
String rx1 = FormatFloat("00000", x1);  //todo ensure max x and max y < 100000
String rx2 = FormatFloat("00000", x2);
String ry1 = FormatFloat("00000", y1);
String ry2 = FormatFloat("00000", y2);

bool switchoddeven = (ComboBox3->ItemIndex == 2);
if (x1 == x2) {
	if (x1 % 2 == switchoddeven) {
		result = CompareStr(ry1, ry2);
	}
	else {
		result = CompareStr(ry2, ry1);
	}
}
else {
	result = CompareStr(rx1, rx2);
}
return result;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TreeView1Compare(TListBoxItem *Item1, TListBoxItem *Item2, int &Result) {
if (Item1 == Item2) {
    Result = 0;
	return;
}
if (ShSort->ItemIndex == 6) {
	String s1 = getTreeStrFromObject(Item1, "ItemNote");
	String s2 = getTreeStrFromObject(Item2, "ItemNote");

	Result = SpecialSort(s1, s2);
}
else {
	String byStr = ShSort->Items->Strings[ShSort->ItemIndex];
	if (byStr.Pos("Id")) {
		String i1 = FormatFloat("000000", getTreeStrFromObject(Item1, byStr).ToIntDef(0)); //todo ensure max Id <1mil
		String i2 = FormatFloat("000000", getTreeStrFromObject(Item2, byStr).ToIntDef(0));
		Result = CompareStr(i1, i2);
		if (!Result) {
			String s1 = getTreeStrFromObject(Item1, "ItemNote");
			String s2 = getTreeStrFromObject(Item2, "ItemNote");
			Result = SpecialSort(s1, s2);
		}
	}
	else Result = CompareStr(getTreeStrFromObject(Item1, byStr), getTreeStrFromObject(Item2, byStr));

	if (ComboBox3->ItemIndex == 2) {
		Result = - Result;
	}
	if (!Result) {
		Result = CompareStr(getTreeStrFromObject(Item1, "ItemId"), getTreeStrFromObject(Item2, "ItemId"));
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ComboBox3Change(TObject *Sender) {
ShSort->Visible = (ComboBox3->ItemIndex != 0);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button22Click(TObject *Sender) {
Button22->Enabled = false;
Application->ProcessMessages();
TreeView1->Sorted = true;
TreeView1->Sorted = false;
if (TreeView1->Parent == Layout1) {
	TreeView1->Width--;
	TreeView1->Width++;
}
else {
	WizardWn->Width--;
	WizardWn->Width++;
}
Label1->Text = "Sort finished";
SaveNow(Sender);
Button22->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ShSortChange(TObject *Sender) {
Button22->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button23Click(TObject *Sender) {
if (Sender) Button23->Enabled = false;
if (LogIn->Visible) {//logged out so lets login
		LoginWn->Visible = true;
		LoginWn->LoginButClick(0);
		SetUpLoggedInWindow();
}
TreeViewClear();
if (WizardWn->trialno == -1) {
	ShowMessage("trialno is not integer error");
	return;
}

String pstring = "trial/" + String(WizardWn->trialno) + "/list/trialunit";
Label1->Text = "Getting details for trial " + String(WizardWn->trialno) + "...";
Application->ProcessMessages();
bool success = LoginWn->DoQuery(pstring, true);
if (!success) {
	Form1->Label1->Text = pstring;
	return;
}
LoginWn->FillTable(pstring);
TStringList *spId = new TStringList();

ProgressBar1->Max = MainWn->StringGrid1->RowCount;
TreeView1->BeginUpdate();

//this code is temporary - once off just to create source items for the trial (in real life they will always exist!)
//for the test we need to use kdlog to print some dummy labels on the dummy source bags containing crap seed
/*
Label1->Text = "Creating source items for trial 3...";
for (int i = 0; i < MainWn->StringGrid1->RowCount ; i++) {
	String spID = MainWn->StringGrid1->Cells[GetCol("SpecimenId", MainWn->StringGrid1)][i];
	//iterate over the [specimenlist] and check the shortlist for items with each specimenId - if none are found
	// 	then create a new item with this specimen and other details
	if (spId->IndexOf(spID) == -1) {
		spId->Add(MainWn->StringGrid1->Cells[GetCol("SpecimenId", MainWn->StringGrid1)][i]); //note bug rowcount needs to be rowcount + 1;
		AddClick(0);
		String unitposText = MainWn->StringGrid1->Cells[GetCol("UnitPositionText", MainWn->StringGrid1)][i];
		String TrialUnitSpecimenIdText = MainWn->StringGrid1->Cells[GetCol("TrialUnitSpecimenId", MainWn->StringGrid1)][i];
		NodeData *data = (NodeData*)(TreeView1->Selected->Data);
		data->final->Values["SpecimenId"] = spID;
		data->orig->Values["SpecimenId"] = spID;
		data->orig->Values["UnitPosText"] = unitposText;
		data->final->Values["UnitPosText"] = unitposText;
		data->orig->Values["TrialUnitSpecimenId"] = TrialUnitSpecimenIdText;
		data->final->Values["TrialUnitSpecimenId"] = TrialUnitSpecimenIdText;

		if (i % 20 == 0) {
			ProgressBar1->Value = i;
			Application->ProcessMessages();
		}
	}
}
TreeView1->EndUpdate();
Label1->Text = "Finished creating source items for trial 3";
ProgressBar1->Value = 0;
return;
*/

Label1->Text = "Creating destination items for trial " + String(WizardWn->trialno) + "...";
DontSaveYet = true;
for (int i = 0; i < MainWn->StringGrid1->RowCount ; i++) {
	String spID = MainWn->StringGrid1->Cells[GetCol("SpecimenId", MainWn->StringGrid1)][i];
	if (spId->IndexOf(spID) == -1) { //used in adding source items
		spId->Add(MainWn->StringGrid1->Cells[GetCol("SpecimenId", MainWn->StringGrid1)][i]); //note bug rowcount needs to be rowcount + 1;
	}
	//iterate over the [specimenlist] and check the shortlist for items with each specimenId - if none are found
	// 	then create a new item with this specimen and other details
	AddClick(0);
	String unitposText = MainWn->StringGrid1->Cells[GetCol("UnitPositionText", MainWn->StringGrid1)][i];
	String tusId = MainWn->StringGrid1->Cells[GetCol("TrialUnitSpecimenId", MainWn->StringGrid1)][i];
	NodeData *data = (NodeData*)(TreeView1->Selected->Data);
	data->final->Values["SpecimenId"] = spID;
	data->orig->Values["SpecimenId"] = spID;
//	data->orig->Values["UnitPosText"] = unitposText;
//	data->final->Values["UnitPosText"] = unitposText;
	data->orig->Values["ItemNote"] = unitposText;
	data->final->Values["ItemNote"] = unitposText;
	//data->orig->Values["TrialUnitSpecimenId"] = tusId;  //this only occurs during harvest
	//data->final->Values["TrialUnitSpecimenId"] = tusId;
	data->orig->Values["ContainerTypeId"] = "6";
	data->final->Values["TrialUnitSpecimenId"] = tusId;
	data->orig->Values["ContainerTypeId"] = "6";//hardcoded values = envelope
	data->final->Values["ContainerTypeId"] = "6";//todo set this properly (lookup envelope and if it doesnt exist then create it!)
	data->orig->Values["ItemUnitId"] = "13";//kernels
	data->final->Values["ItemUnitId"] = "13";
	if (i % 50 == 0) {
		ProgressBar1->Value = i;
		Application->ProcessMessages();
	}
}
DontSaveYet = false;
ProgressBar1->Value = 0;
//set the correct labels
ItSet->ItemIndex = ItSet->Items->IndexOf("ItemNote");
ItSetChange(Sender);
Label1->Text = "Adding source items for trial 3...";
Application->ProcessMessages();
//identify all specimenIds
String searchVal = "(";
for (int i = 0; i <spId->Count; i++) {
	if (i == 0) {
		searchVal += spId->Strings[0];
	}
	else {
		searchVal = searchVal + "," + spId->Strings[i];
	}
}
searchVal += ")";
Button24Click(0); //reset the search db tab
sNameBox->ItemIndex = sNameBox->Items->IndexOf("SpecimenId");
sOpBox->ItemIndex = sOpBox->Items->IndexOf("IN ");
valEdit->Text = searchVal;
ComboBox2->ItemIndex = 0;
//search for all items with these SpecimenIds [specimenlist] and add them to the shortlist
//for demo only one item exists per specimen
//todo remove duplicate sources using business rules such as selecting the newest item at the specified storage with amount > 400
SearchButClick(0);
//simple and temp validation as source items may be missing or >1 for each source
if (!sg) {
	ShowMessage("Can't continue load of trial - validation reports no source items");
	return;
}
if (spId->Count != sg->RowCount) {
	ShowMessage("Can't continue load of trial - validation reports source items may be missing or >1 for each specimen. According to trial SpecimenId list has " + String(spId->Count) + " but db indicates there are " + String(sg->RowCount) + " items");
	TreeViewClear();
	return;
}
TStringColumn* col = ((TStringColumn*)sg->ColumnByIndex(0));
ShortButClick(col);
//sort by genotype
ComboBox3->ItemIndex = 2; //descending
ShSort->ItemIndex = ShSort->Items->IndexOf("SpecimenId");
Button22Click(0);
//now put temp flag on each destination item so we can easily subsample
String sourceId;
for (int i = 0; i < TreeView1->Count; i++) {
	TListBoxItem *t = TreeView1->ItemByIndex(i);
	if (getTreeStrFromObject(t, "ItemId") != "new") {
		sourceId = getTreeStrFromObject(t, "ItemId");
	}
	else {//its a destination
		NodeData *d = (NodeData*)t->Data;
		d->orig->Values["ItemGroupId"] = sourceId;
		d->final->Values["ItemGroupId"] = sourceId;
		String bcextract = d->final->Values["ItemBarcode"];
		String newbc = bcextract.Insert(sourceId + "|", 4);
		d->orig->Values["ItemBarcode"] = newbc;
		d->final->Values["ItemBarcode"] = newbc;

	}
}
SaveNow(Sender);
if (Sender) {
	ShowMessage("Trial has been successfully loaded, sorted and grouped. Seed preparation mode has been activated");
	WizardWn->SetTState(0);
	Button23->Enabled = true;
}
//message dlg . Would you like to print the entry list barcodes?
// Connect your scanner and scales and commence sub sampling
}
//---------------------------------------------------------------------------
void __fastcall TForm1::StSetChange(TObject *Sender) {
StorageId->ListBoxResource = StSet->Items->Strings[StSet->ItemIndex];
for (int i = 0; i < StorageId->Count; i++) {
	TStringList *stl = (TStringList*)StorageId->Items->Objects[i];
	StorageId->Items->Strings[i] = stl->Values[StorageId->ListBoxResource];
}
SaveSettings2();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::PrintSetupButClick(TObject *Sender) {
//PrinterSetupDialog1->Execute();
TMemo* res = new TMemo(this);
if (!TreeView1->Count) {
	ShowMessage("Nothing in your shortlist to export");
	return;
}
SaveDialog1->Title = "Enter a name for your print file (.csv)";
if (SaveDialog1->Execute()) {
	if (!SaveDialog1->FileName.Pos(".csv")) {
		SaveDialog1->FileName = SaveDialog1->FileName + ".csv";
	}
	//sort by planting order
	//ComboBox3->ItemIndex = 2; //descending
	//ShSort->ItemIndex = ShSort->Items->IndexOf("ItemNote");
	//Button22Click(0);

	for (int i = 0; i < TreeView1->Count; i++) {//iterate over the main table rows
		String lineTxt = "";
		TListBoxItem *t = TreeView1->ItemByIndex(i);
		if (getTreeStrFromObject(t, "ItemId") == "new") {
		for (int j = 0; j < 5; j++) {    //iterate over the current search results headers
			String s1;
			/*if (j == -1) {
				s1 = "TM14DRTRE4-SI";
			}
			else*/
			if (j == 0) {
				s1 = getTreeStrFromObject(t, "ItemBarcode");
			}
			else if (j <= 2) {
				String iN = getTreeStrFromObject(t, "ItemNote");
				int x1 = 0, y1 = 0;
				extractxy(x1, y1, iN);
				s1 = String(x1) + "," + String(y1);
				j++;
			}
			else if (j == 3) s1 = getTreeStrFromObject(t, "SpecimenId");
			else if (j == 4) {
				String bc = getTreeStrFromObject(t, "ItemBarcode");
				String extract = bc.SubString(4, bc.Length());
				int p = extract.Pos("|");
				s1 = extract.SubString(0, p - 1);
			}
			lineTxt += s1 + ",";
		}
		res->Lines->Add(lineTxt);
	}
}
res->Lines->SaveToFile(SaveDialog1->FileName);
Label1->Text = "Results exported to " + SaveDialog1->FileName;
delete res; res = NULL;
}
}
//---------------------------------------------------------------------------//---------------------------------------------------------------------------
void __fastcall TForm1::Button11Click(TObject *Sender) {
//  upload barcodes to trialunitspecimen (marks the planting prep has ended)
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button18Click(TObject *Sender) {
//check that the trial has been planted (trialunit barcodes exist)
//validate that trait82 exists. Note that there will be one saample measuremenaurement per trailunit
//validate that planting has occured by checking the trialunitbarcode.
//recreate trial destination items by tracing the source itemid from the trialbarcode
//create >=0 new source bags based using selection counts for a specific trait
//set harvest mode = ON and other modes = OFF
//print new source bag barcodes
//commence scanning and weighing as in seed preparation
//except this time we upload the new bags to DAL
Button10->Enabled = false;
Application->ProcessMessages();
WizardWn->SetTState(2);//HARVEST
Button23Click(0);
//Button20Click(Sender); //remove source

ShowMessage("Trial has been successfully loaded, sorted and grouped. Harvesting mode has been activated");
ShowMessage("Temporararily setting the selection to 2 for each harvested item");
for (int i = TreeView1->Count - 1; i >= 0; i--) {
	TListBoxItem *t = TreeView1->ItemByIndex(i);
	String text = getTreeStrFromObject(t, "ItemNote");

	for (int j = 0; j < 2; j++) {
		AddClick(0);
		TListBoxItem *tsel = TreeView1->Selected;
		NodeData *d = (NodeData*)t->Data;
		d->final->Values["ItemNote"] = text + " Harvest";
		tsel->Text = "        " + text + " Harvest";
		i--;
	}

}
ComboBox3->ItemIndex = 2; //descending
ShSort->ItemIndex = ShSort->Items->IndexOf("ItemNote");
Button22Click(0);
SaveNow(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::WizardButClick(TObject *Sender) {
if (Sender) {
	if (WizardWn->currentpage == -999) {
		WizardWn->currentpage = 0;
		TStringList *info = new TStringList(this);
		String ifile = SettingsWn->path + "Localtriallist.txt";
		if (FileExists(ifile)) {
			info->LoadFromFile(ifile);
		}
		String selText;
		for (int i = 0; i < 5; i++) {
			if (!i) {
				selText = onStringGrid->Cells[i][onStringGrid->Selected];
			}
			else {
				selText = selText + "," + onStringGrid->Cells[i][onStringGrid->Selected];
			}
		}
		selText = selText + "," + WizardWn->stepStr[WizardWn->currentpage + 1]->Text;
		info->Insert(0, selText);
		info->SaveToFile(ifile);
		delete info; info = NULL;
	}
	else if (WizardWn->currentpage != 0) {
		LoadShortButClick(0);
	}
}
//AniIndicator1->Visible = true;
//Label1->Text = "Please wait...launching Wizard";
//Application->ProcessMessages();
WizardWn->Memo1->Lines->Clear();
WizardWn->Left = Form1->Left;
WizardWn->Top = Form1->Top;
WizardWn->Height = Form1->Height;
WizardWn->Width = Form1->Width;
//WizardWn->treeWidth = TreeView1->Width;
WizardWn->origcurrentpage = WizardWn->currentpage;
WizardWn->SetStages();
WizardWn->Show();
WizardWn->WizButClick(0);
#ifndef _DEBUG
WizardWn->ShowModal();
#endif
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ComboBox4Change(TObject *Sender) {
Application->ProcessMessages();
AniIndicator1->Visible = true;
Application->ProcessMessages();
TreeView1->BeginUpdate();
int count = 0;
for (int i = TreeView1->Count - 1; i >= 0; i--) {
	TListBoxItem *t = TreeView1->ItemByIndex(i);
	if (ComboBox4->ItemIndex == 1) { //view source
		if (getTreeStrFromObject(t, "ItemId") == "new") {
			t->Enabled = false;
			count++;
		}
		else t->Enabled = true;
	}
	else if (ComboBox4->ItemIndex == 2) { //view destinations
		if (getTreeStrFromObject(t, "ItemId") != "new") {
			t->Enabled = false;
			count++;
		}
		else t->Enabled = true;
	}
	else if (ComboBox4->ItemIndex == 0) t->Enabled = true;
	else { //white only
		TColorButton *but = GetButton(t);
		if (but->Color == TAlphaColorRec::White) {
			t->Enabled = true;
		}
		else t->Enabled = false;
    }
	if (i % 20 == 0) {
		//ProgressBar1->Value = j;
		TreeView1->EndUpdate();
		Application->ProcessMessages();
		TreeView1->BeginUpdate();
	}
}
TreeView1->EndUpdate();

Label1->Text = String(count) + " items hidden, " + String(TreeView1->Count) + " items total";
TreeView1->EndUpdate();

AniIndicator1->Visible = false;
}
//---------------------------------------------------------------------------
void TForm1::SetProgressBar() {
	ProgressBar1->Parent = Form1->StatusBar1;
	ProgressBar1->Align = TAlignLayout::alNone;
    ProgressBar1->Value = 0;
	ProgressBar1->Width = ClientWidth/Layout1->Scale->X - pstartpos;
	ProgressBar1->Position->X = pstartpos;
}
//---------------------------------------------------------------------------
void TForm1::UpdateSeedProgress() {
//if (WizardWn->tstate == 0) { //PREPARE
	int emptycount = 0;
	ProgressBar1->Max = CountDestinationItems(emptycount);
	ProgressBar1->Value = ProgressBar1->Max - emptycount;
	Label1->Text = String(ProgressBar1->Max) + " destination envelopes total, " +String(emptycount) + " empty";
//}
}
//---------------------------------------------------------------------------
int TForm1::CountDestinationItems(int &empty) {
int total = 0;
for (int i = TreeView1->Count - 1; i >= 0; i--) {
	TListBoxItem *t = TreeView1->ItemByIndex(i);
	if (getTreeStrFromObject(t, "ItemId") == "new") {
		total++;
		if (getTreeStrFromObject(t, "Amount").ToDouble() == 0) {
			empty++;
		}
	}
}
return total;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::cbStylesClick(TObject *Sender) {
TComboBox *b = (TComboBox*)Sender;
String stylef = SettingsWn->path + b->Items->Strings[b->ItemIndex] + ".Style";

if (FileExists(stylef)) {
	StyleBook1->Resource->LoadFromFile(stylef);
	TStyleManager::SetStyle(StyleBook1->Style);     //b->Items[b->ItemIndex]
}
else {
	TStyleManager::SetStyle(NULL);
}
TStringList *settings5 = new TStringList(this);
settings5->Add(String(b->ItemIndex));
settings5->SaveToFile(SettingsWn->path + "settings5.txt");
delete settings5; settings5 = NULL;
Label1->Text = b->Items->Strings[b->ItemIndex] + " style applied";
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TabControl2Change(TObject *Sender) {
PrButa->Visible = false;
DelBut->Visible = false;
WizardBut->Visible = false;
ToolBar3->Enabled = false;
seEdit->Text = "";
String tText = TabControl2->Tabs[TabControl2->TabIndex]->Text;
Label1->Text = "Getting " + tText + " trials...";
//get offline info for use later
TStringList *info2 = new TStringList(this);
String ifile2 = SettingsWn->path + "Localtriallist.txt";
if (FileExists(ifile2)) {
	info2->LoadFromFile(ifile2);
}
if (TabControl2->TabIndex == 1) {  //online
	//login
	if (!CheckandLogin("You can't see the list of online trials without first logging in")) return;

	onStringGrid->RowCount = 0; //hack to unselect row
	onStringGrid->Enabled = false;
	onStringGrid->Visible = true;
	SetResource(onStringGrid);
	xLabel3->Text = "Getting list of available trials...";
	Application->ProcessMessages();
	String pstring = "list/trial/10000/page/1";
	bool success = LoginWn->DoQuery(pstring, true);
	if (!success) {
		Form1->Label1->Text = pstring;
		return;
	}
	xLabel3->Text = "Select a trial to continue";
	LoginWn->FillTable(pstring);

	onStringGrid->Selected = NULL;
	onStringGrid->BeginUpdate();
	int count = 0;
	for (int i = 0; i < MainWn->StringGrid1->RowCount; i++) {//iterate over the main table rows
		//a check to prevent double loading (uses trialno)
		bool found = false;
		int searchtno = MainWn->StringGrid1->Cells[GetCol("TrialId", MainWn->StringGrid1)][i].ToIntDef(-1);
		for (int qi = 0; qi < info2->Count; qi++) {
			String line = info2->Strings[qi];
			int p = line.Pos(",");
			int val = line.SubString(0, p - 1).ToIntDef(-1);
			if (val != -1)
			if (val == searchtno) {
				found = true;
				break;
			}
		}
		if (!found) {
			onStringGrid->RowCount = count + 1;
			for (int j = 0; j < onStringGrid->ColumnCount; j++) {    //iterate over the current search results headers
				String s1 = ((TStringColumn*)onStringGrid->ColumnByIndex(j))->Header;
				int id = GetCol(s1, MainWn->StringGrid1);    //get the corresponding main column
				Application->ProcessMessages();
				int idcol = GetCol(Form1->ItSet->Items->Strings[Form1->ItSet->ItemIndex], MainWn->StringGrid1);
				onStringGrid->Cells[j][count] = MainWn->StringGrid1->Cells[id][i];
			}
			count++;
		}
	}
	onStringGrid->EndUpdate();
	onStringGrid->Enabled = true;

	TStringList *info = new TStringList(this);
	String ifile = SettingsWn->path + "KDDArTtriallist.txt";
	for (int row = 0; row < onStringGrid->RowCount; row++) {
		String selText = "";
		for (int col =0; col < onStringGrid->ColumnCount; col++) {
			if (!col) {
				selText = onStringGrid->Cells[col][row];
			}
			else {
				selText = selText + "," + onStringGrid->Cells[col][row];
			}
		}
		info->Add(selText);
	}
	info->SaveToFile(ifile);
	delete info; info = NULL;
	Label1->Text = String(onStringGrid->RowCount) + " " + tText + " trials available";
}
else { //offline 
	offStringGrid->RowCount = 0;      //hack to unselect row
	offStringGrid->Enabled = false;
	Application->ProcessMessages();
	offStringGrid->Selected = NULL;
	
    SetResource(offStringGrid);

	offStringGrid->BeginUpdate();
	for (int i = 0; i < info2->Count; i++) {
		AddLineToGrid(info2->Strings[i], offStringGrid);
	}

	offStringGrid->EndUpdate();
	offStringGrid->Enabled = true;
	tLabel3->Text = "Select a trial to continue";
	Label1->Text = String(offStringGrid->RowCount) + " " + tText + " trials available";

}
ToolBar3->Enabled = true;
delete info2; info2 = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::onStringGridSelChanged(TObject *Sender) {
if (onStringGrid->Selected != -1) {
	Label1->Text = "You have selected trial " + onStringGrid->Cells[0][onStringGrid->Selected];
	WizardWn->currentpage = -999;
	WizardWn->trialno = onStringGrid->Cells[0][onStringGrid->Selected].ToIntDef(-1);
	WizardBut->Text = "Start";
	WizardWn->SetTState(-1);  //set it to unknown until we know otherwise
	xLabel3->Text = "Start trial: " + onStringGrid->Cells[1][onStringGrid->Selected];
}
else xLabel3->Text = "Select a trial to continue";
WizardBut->Visible = (onStringGrid->Selected != -1);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::offStringGridSelChanged(TObject *Sender) {
if (offStringGrid->Selected != -1) {
	WizardWn->currentpage = offStringGrid->Cells[5][offStringGrid->Selected].SubString(0, 1).ToIntDef(0) - 1;
	WizardWn->SetTState(0);
	if (offStringGrid->Cells[5][offStringGrid->Selected].Pos("(HARVEST)")) {
		WizardWn->SetTState(2);
	}
	else if (offStringGrid->Cells[5][offStringGrid->Selected].Pos("(PREPARE)")) {
		WizardWn->SetTState(0);
	}
	else WizardWn->SetTState(-1);
	WizardWn->trialno = offStringGrid->Cells[0][offStringGrid->Selected].ToIntDef(-1);
	Label1->Text = "You have selected trial " + String(WizardWn->trialno);
	WizardBut->Text = "Resume";
	tLabel3->Text = "Resume or delete trial: " + offStringGrid->Cells[1][offStringGrid->Selected];
}
else tLabel3->Text = "Select a trial to continue";
WizardBut->Visible = (offStringGrid->Selected != -1);
DelBut->Visible = (offStringGrid->Selected != -1);
PrButa->Visible = (offStringGrid->Selected != -1 && WizardWn->currentpage > 1);
}
//---------------------------------------------------------------------------
void TForm1::AddLineToGrid(String line, TStringGrid *ssg) {
ssg->RowCount++;
int p = line.Pos(",");
int col = 0;
while (p) {
	ssg->Cells[col][ssg->RowCount - 1] = line.SubString(0, p - 1);
	line = line.SubString(p + 1, line.Length() - p + 1);
	col++;
	p = line.Pos(",");
}
ssg->Cells[col][ssg->RowCount - 1] = line;
}
//---------------------------------------------------------------------------
void TForm1::SetResource(TStringGrid *ssg) {
ssg->ApplyStyleLookup(); //does this need to go here?
THeader *header = dynamic_cast<THeader*>(ssg->FindStyleResource("header", false));
for (int w = 0; w < header->ChildrenCount - 1; w++) {
	THeaderItem* headeritem = dynamic_cast<THeaderItem*>(header->Children->Items[w]);
	headeritem->DragMode = TDragMode::dmManual;
	headeritem->OnClick = ssgClick;
}
}
//---------------------------------------------------------------------------
bool TForm1::CheckandLogin(String message) {
if (LogIn->Visible) {//logged out
	LoginWn->Visible = true;
	LoginWn->LoginButClick(0);
	SetUpLoggedInWindow();

	if (LogIn->Visible) {
		AniIndicator1->Visible = false;
		ShowMessage(message);
		return 0;
	}
}
return 1;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DelButClick(TObject *Sender) {
#ifndef _DEBUG
if (mrYes != MessageDlg("Are you sure you want to remove trial " + String(WizardWn->trialno) + " from your computer?", TMsgDlgType::mtWarning,
	TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
	ShowMessage("Trial " + String(WizardWn->trialno) + " deletion cancelled");
	return;
}
#endif
//remove from the "row" from file
TStringList *info = new TStringList(this);
String ifile = SettingsWn->path + "Localtriallist.txt";
if (FileExists(ifile)) {
	info->LoadFromFile(ifile);
}
for (int i = 0; i < info->Count; i++) {
	String line = info->Strings[i];
	int p = line.Pos(",");
	int val = line.SubString(0, p - 1).ToIntDef(-1);
	if (val != -1) {
		if (val == WizardWn->trialno) {
			info->Delete(i);
			break;
		}
	}
}
info->SaveToFile(ifile);
delete info; info = NULL;
//cleanup associated files and directories (optional)
int iAttributes = 0;
//iAttributes |= faDirectory;
TSearchRec sr;
String DirName = SettingsWn->path + String(WizardWn->trialno);
char pathDelim = (char)System::Ioutils::TPath::DirectorySeparatorChar;
String Path = DirName + pathDelim + "*.*";
if (FindFirst(Path, iAttributes, sr) == 0) {
	do {
		if (sr.Name != "." && sr.Name != "..") DeleteFile(DirName + pathDelim + sr.Name);
	} while (FindNext(sr) == 0);  //iterate through the directory until no file is found
}
//RemoveDir(DirName); //can cause access denied later on
DeleteFile(SettingsWn->path + "t" + String(WizardWn->trialno) + ".trial");
DeleteFile(SettingsWn->path + "t" + String(WizardWn->trialno) + ".trait");
TabControl2Change(0);  //update the offline list
if (offStringGrid->RowCount) {
 	offStringGrid->Selected = 0;
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::seEditKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar, TShiftState Shift) {
TStringGrid *ssg;
String ifile;
if (TabControl2->TabIndex == 0) {
	ssg = offStringGrid;
	ifile = SettingsWn->path + "Localtriallist.txt";
}
else {
	ssg = onStringGrid;
	ifile = SettingsWn->path + "KDDArTtriallist.txt";
}
ssg->Enabled = false;
TStringList *info = new TStringList();

if (FileExists(ifile)) {
	info->LoadFromFile(ifile);
}


ssg->RowCount = 0;      //hack to unselect row
ssg->Enabled = false;
ssg->Selected = NULL;

ssg->BeginUpdate();
for (int i = 0; i < info->Count; i++) {
	String line = info->Strings[i];
	int p = 1;
	if (seEdit->Text.Length()) p = line.Pos(seEdit->Text);
	if (p) {
		AddLineToGrid(line, ssg);
	}
}
ssg->EndUpdate();
ssg->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::setSButClick(TObject *Sender) {
Form8 = new TForm8(this);
Form8->Scale(Form1->GroupBox11, Sender);
Form8->Show();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::PrButaClick(TObject *Sender) {
LoadShortButClick(0);
Form8 = new TForm8(this);
Form8->Scale(Form1->GroupBox10, Sender);
Form8->Caption = "Choose your reprint option for Trial " + String(WizardWn->trialno);
Form8->ShowModal();
ClearShortButClick(0);
}

//---------------------------------------------------------------------------

void __fastcall TForm1::modeComboBox5Change(TObject *Sender)
{
if (modeComboBox5->ItemIndex == 0) {
    pstartpos = 30;
	TabControl1->Visible = false;
	TabControl2->Parent = Layout1;
	ToolBar3->Parent = Layout1;
}
else {
	TabControl1->Visible = true;
	TabControl2->Parent = TabItem2;
	GroupBox4->Visible = true;
	GroupBox2->Visible = true;
	Update1->Visible = true;
	UpdateAll->Visible = true;
	pstartpos = 90;
}
SetProgressBar();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Splitter1MouseEnter(TObject *Sender) {
drag = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Splitter1MouseLeave(TObject *Sender) {
drag = false;
}
//---------------------------------------------------------------------------

