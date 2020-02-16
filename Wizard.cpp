//---------------------------------------------------------------------------
#include <fmx.h>
#pragma hdrstop
#include "unit1.h"
#include "login.h"
#include "main.h"
#include "unit3.h"
#include "prepare.h"
#include "Wizard.h"
#include "Settings.h"
#include "unit8.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
#define PREPARE 0
#define HARVEST 2
TWizardWn *WizardWn;
//---------------------------------------------------------------------------
__fastcall TWizardWn::TWizardWn(TComponent* Owner) : TForm(Owner) {
TraitHarvestList = new TStringList();
autoweigh = true;
for (int i = 0; i < 10; i++) {
	stepStr[i] = NULL;
}
for (int i = 0; i < Rectangle1->ChildrenCount; i++) {
	TLabel *lab = dynamic_cast<TLabel*>(Rectangle1->Children->Items[i]);
	if (lab) {
		int no = lab->Text.SubString(0, 1).ToIntDef(-1);
		if (no != -1) {
			stepStr[no] = lab;
		}
	}
}
}
//---------------------------------------------------------------------------
void TWizardWn::UpdateLabels() {
TFontStyles AStyle(1);//bold

for (int i = 0; i < 10; i++) {
	if (stepStr[i] != NULL) {
		if (stepStr[i]->Font->Style.Contains(TFontStyle::fsBold)) {
			stepStr[i]->Font->Style = stepStr[i]->Font->Style - AStyle;
			Application->ProcessMessages();
		}
		if (currentpage + 1 == i) {
			stepStr[i]->Font->Style = stepStr[i]->Font->Style + AStyle;
		}
	}
}
stageLabel->Text = stepStr[currentpage + 1]->Text.SubString(7, stepStr[currentpage + 1]->Text.Length()) +
	" (" + stepStr[currentpage + 1]->Text.SubString(0,1) + " of 8)";
}
//---------------------------------------------------------------------------
void TWizardWn::SetStages() {
String tType = "Trial Inventory Wizard (trial " + String(trialno);
if (tstate == HARVEST) {
	s5->Text = "5.     Harvest";
	s3->Text = "3.     Define selections";
	s6->Text = "6.     Set other fields";
	s7->Text = "7.     Validation";
	Caption = tType + " in HARVEST mode)";
}
else {
	s5->Text = "5.     Seed preparation";
	s3->Text = "3.     Sorting";
	s6->Text = "6.     Re-sorting";
	s7->Text = "7.     Validation";
	if (tstate == PREPARE) Caption = tType + " in PREPARE mode)";
	else Caption = tType + ")";
}
}
//---------------------------------------------------------------------------
void TWizardWn::SetTState(int mode) {
if (mode != HARVEST) {
	Form1->MenuItem2->Text = "Prepare";
	tstate = mode;
}
else  {
	Form1->MenuItem2->Text = "Harvest";
	tstate = HARVEST;
}
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::WizButClick(TObject *Sender) {
TButton *butSender = reinterpret_cast<TButton*>(Sender);

if (butSender == BackBut) {
	if (currentpage == 2) {
		if (mrYes != MessageDlg("If you go back and download the trial you will lose any changes you made. Do you want to reload the trial anyway?", TMsgDlgType::mtWarning,
						TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
			return;
		}
		else {
			somethingChanged = true; //todo understand user requirement for tracking changes
			Form1->ClearShortButClick(0);
        }
	}
	currentpage--;

}
else if (butSender == NextBut) {
	somethingChanged = true; //todo understand user requirement for tracking changes
	currentpage++;
}
else {
    origFormWidth = Form1->Width;

}
WriteWizardPos(trialno, currentpage);
SetAmountDefaultPos();
UpdateLabels();
UpdateOffLineTrialStatus();

Label5->Visible = false;
eButton4->Visible = false;
hpButton->Visible = false;
BackBut->Enabled = (currentpage != 0);
NextBut->Enabled = (currentpage != 7);
aaGroupBox1->Visible = false;
Label5->Visible = false;
Memo1->Visible = (currentpage <= 1);
if (currentpage >= 1 && currentpage != 3) {
	Form1->ProgressBar1->Parent = Rectangle5;
	Form1->ProgressBar1->Align = TAlignLayout::alMostBottom;
	Form1->ProgressBar1->Value = 0;
	Form1->Label1->Text = "";
}
else {
	Form1->SetProgressBar();
}
if (PrepareWn) {
	//PrepareWn->Layout1->Parent = PrepareWn;
	if (Form1->sg) Form1->sg->Parent = Form1->SearchTab;
	delete PrepareWn;
	PrepareWn = NULL;
	Application->ProcessMessages(); //temp?
}

Form1->GroupBox8->Align = TAlignLayout::alNone;
Form1->GroupBox8->Parent = Form1->SettingsTab;
Form1->GroupBox10->Parent = Form1->SettingsTab;
Form1->GroupBox10->Align = TAlignLayout::alNone;
Form1->TreeView1->Parent = Form1->Layout1;
Form1->TreeView1->Align = TAlignLayout::alMostLeft;
Form1->Width = origFormWidth;
Form1->ItemDetailBox->Align = TAlignLayout::alNone;
Form1->ItemDetailBox->Parent = Form1->Panel1;

if (Form3) {
	delete Form3; Form3 = NULL;
}
Button1->Visible = false;

if (currentpage == 0) {
	Memo1->Lines->Add("Introduction to the wizard");
	Memo1->Lines->Add("==========================");
	Memo1->Lines->Add("");
	Memo1->Lines->Add("You will be presented with several steps. Each step makes important changes to your trial data");
	Memo1->Lines->Add("Next and last steps requires internet access, all others can be performed offline");
	Memo1->Lines->Add("");
	Memo1->Lines->Add("You can exit the wizard at any time and optionally save changes so you can resume from where you left off");
	Memo1->Lines->Add("");
	Memo1->Lines->Add("Press 'Next' to start loading your selected trial from the database");
	Label3->Text = "Welcome to the Trial Inventory Wizard";
	Label4->Text = "You have selected trial " + String(trialno);
}
else if (currentpage == 1) {  //get trial from db
	TStringList *spId = new TStringList();
	Label3->Text = "Select source bags for destination items";
	if (Sender == NextBut) {
		Label4->Text = "Automatically preselecting bags for trial " + String(trialno);
		NextBut->Enabled = false;
		Memo1->Lines->Clear();
		Memo1->Visible = true;
		Form1->TreeViewClear();
		if (!Form1->CheckandLogin("You can't download a trial without first logging in")) return;
		TModalResult answer = mrYes;
		while (answer = mrYes && !GetTrialUnits(trialno)) {
			answer = MessageDlg("There is a problem getting the trial's details. Try again?", TMsgDlgType::mtError,
			TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0);
			if (answer != mrYes) {
				Memo1->Lines->Add("Trial can't be loaded. Contact DAL administrator");
				return;
			}
		}

		Memo1->Lines->Strings[Memo1->Lines->Count - 1] + String(MainWn->StringGrid1->RowCount) + " destination items found";
		Form1->ProgressBar1->Max = MainWn->StringGrid1->RowCount;

		Form1->DontSaveYet = true;
		SetTState(PREPARE);
	}
	else { //reloading trial specimen list from file
		reloadTrial(trialno);
	}
	for (int i = 0; i < MainWn->StringGrid1->RowCount ; i++) {
		String spID = MainWn->StringGrid1->Cells[GetCol("SpecimenId", MainWn->StringGrid1)][i];
		if (spId->IndexOf(spID) == -1) { //used in adding source items
			spId->Add(MainWn->StringGrid1->Cells[GetCol("SpecimenId", MainWn->StringGrid1)][i]); //note bug rowcount needs to be rowcount + 1;
		}
		String tuBC = MainWn->StringGrid1->Cells[GetCol("TrialUnitBarcode", MainWn->StringGrid1)][i];
		if (!tuBC.IsEmpty()) {
			if (tstate == PREPARE) {
				Memo1->Lines->Add("trial has been planted...");
				SetTState(HARVEST);
			}
		}
		else { //not empty
			if (tstate != PREPARE) { //could only occur if KDLog was interrupted during step 8
				if (tstate == -1) {
					SetTState(PREPARE);
				}
				else {
					String unitposText = MainWn->StringGrid1->Cells[GetCol("UnitPositionText", MainWn->StringGrid1)][i];
					Memo1->Lines->Add("Error detected. TrialUnitBarcode is missing from planted trial at " + unitposText);
					Memo1->Lines->Add("Trial is in intermediate state. Serious risk of corruption. Loading cancelled");
					SetTState(PREPARE);
					Form1->AniIndicator1->Visible = false;
					return;
				}
			}
			//Memo1->Lines->Add("Trial has not been planted");
		}
	}

	if (tstate == PREPARE) {
		Memo1->Lines->Add("Trial " + String(trialno) + " can be prepared");
	}
	else {
		Memo1->Lines->Add("Trial " + String(trialno) + " can be harvested");
		SetTState(HARVEST);
	}
	SetStages();
	if (tstate == PREPARE) {
		//Memo1->Lines->Strings[Memo1->Lines->Count - 1] = Memo1->Lines->Strings[Memo1->Lines->Count - 1] + "completed";
		Memo1->Lines->Add(String(spId->Count) + " unique specimens found in trial setup");
		if (!spId->Count) {
			Memo1->Lines->Add("Error detected. Trial cannot have 0 specimens");
			Form1->AniIndicator1->Visible = false;
			return;
		}

		//search for all items with these SpecimenIds [specimenlist] and add them to the shortlist
		//set the correct labels
		Form1->ItSet->ItemIndex = Form1->ItSet->Items->IndexOf("ItemNote");
		Form1->ItSetChange(Sender);

		if (!Form1->CheckandLogin("You can't download a trial without first logging in")) return;
		Memo1->Lines->Add("Searching database for matching source items...");
		Application->ProcessMessages();
		//search for items with the trial specimenIds
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
		Form1->Button24Click(0); //reset the search db tab
		//todo set the search results columns = itemnote, itemid, amount, date added, itembarcode
		Form1->sNameBox->ItemIndex = Form1->sNameBox->Items->IndexOf("SpecimenId");
		Form1->sOpBox->ItemIndex = Form1->sOpBox->Items->IndexOf("IN ");
		Form1->valEdit->Text = searchVal;
		Form1->ComboBox2->ItemIndex = 0;
		Form1->SortChoice->ItemIndex = 1;
		Form1->ItSort->ItemIndex = Form1->ItSort->Items->IndexOf("SpecimenId");
		Form1->SearchButClick(0);
		Memo1->Visible = false;
		//use preparewn to park our list of specimens
		PrepareWn = new TPrepareWn(this);
		//PrepareWn->Scale();
		//PrepareWn->Setup();
		PrepareWn->Layout1->Parent = Rectangle5;
		//PrepareWn->ListBox1->BeginUpdate();
		int i = -1, specCol = -1;
		if (Form1->sg) {
			i = Form1->sg->RowCount - 1;
			specCol = GetCol("Specimen", Form1->sg);
		}

		//sg is now populated so now we create a list of specimenIds in green, yellow and red
		int greenCount = 0, redCount = 0, yellowCount = 0;

		Form1->DontSaveYet = true;
		Form1->TreeView1->BeginUpdate();
		while (spId->Count) {
			String sid;
			String specId;
			if (i >= 0) {
				sid = Form1->sg->Cells[specCol][i];
				//index = Form1->getIndex(Form1->SpecimenId, sid);
				//TStringList* sx = (TStringList*)Form1->SpecimenId->Items->Objects[index];
				//specId = sx->Values["SpecimenName"];
				specId = Form1->getIdOfTitle(Form1->SpecimenId, sid);
				spId->Delete(spId->IndexOf(specId));
			}
			else {
				specId = spId->Strings[0];
				spId->Delete(0);
			}
			bool found = false;

			//find last item with same specimen
			while (i >= 0) { // -1 means no no item with this specimen exists
				if (!Form1->sg->Cells[specCol][i].IsEmpty() && sid == Form1->sg->Cells[specCol][i]) {
					found = true;
					break;
				}
				i--;
			}

			TDateTime youngest = 0;
			int youngestindex = -1;
			int j = i - 1;
			if (found) {//get other items with same specimen
				while (j >= 0 && (sid == Form1->sg->Cells[specCol][j] || Form1->sg->Cells[specCol][j].IsEmpty())) {
					j--;
				}
				//remove duplicate sources using business rules such as selecting the newest item at the specified storage with amount > 400
				j++;
				for (int r = j; r <= i; r++) {
					String amount = Form1->sg->Cells[GetCol("Amount", Form1->sg)][r];
					if (amount != "(empty)" && !amount.IsEmpty()) {
						if (amount.ToDouble() > 0) {   //should be >0
							TFormatSettings fmt;
							fmt.ShortDateFormat= "yyyy-MM-dd";
							fmt.DateSeparator  = '-';
							fmt.LongTimeFormat = "HH:mm:ss";
							fmt.TimeSeparator  = ':';
							String D1 = Form1->sg->Cells[GetCol("DateAdded", Form1->sg)][r];
							TDateTime d1 = StrToDateTime(D1, fmt);
							if (d1 > youngest) {
								youngest = d1;
								youngestindex = r;
							}
						}
					}
				}
			}
			//use the specimen id for a new item in the listbox
			TListBoxItem *Item = new TListBoxItem(this);
			Item->Text = specId;
			if (youngestindex != -1) {
				//select the item
				Form1->itemR[youngestindex]->IsChecked = true;  //add item to shortlist!
				if (j - i == 0) { //=1 (green)
					Item->Tag = 0;
					greenCount++;
				}
				else {  //>1 (yellow)
					Item->Tag = 1;
					yellowCount++;
				}
			}
			else { // =0 (red)
				Item->Tag = 2;
				redCount++;
			}
			Item->Parent = PrepareWn->ListBox1;
			// this set our style to new item
			Item->OnApplyStyleLookup = PrepareWn->ListBox1ApplyStyleLookup;
			Item->StyleLookup = "listboxitemstyle";
			i = j - 1;  //note optimised search for speed
		}
		Form1->TreeView1->EndUpdate();
		//PrepareWn->ListBox1->EndUpdate();
		Label4->Text = "Trial defined: " + String(greenCount + yellowCount) + " source items found with the following specimens, " + String(redCount) + " missing";
		if (redCount == 0 ) {
			NextBut->Enabled = true;
			if (!yellowCount) Form1->Label1->Text = "Click Next to continue";
			else Form1->Label1->Text = "Optionally review " + String(yellowCount) + " items or click 'Next' to continue";
		}
		else {
			Form1->Label1->Text = "No items exist for " + String(redCount) + " specimens. You can't continue";
			NextBut->Enabled = false;
		}
	}
	else {//harvest mode
	//0 DO LAST present the user a list of traits that have been used in this trail
	//	could be done from the list of export_samplemeasurements
	// #TrialUnitId,TraitId,OperatorId,MeasureDateTime,InstanceNumber,SampleTypeId,TraitValue
	// make a list of unique traitids by checking all trialunitIds in the selected trial against the export
	// then ask the user to select 1 trait from a list of traits (id + name + other using list/trait/_nperpage/_num) present in this trial
	//1 get TraitValue corresponding to the selected traitId from export list [=82 Number_Ears_Advanced]. Each value becomes the max_value for each harvested bag as shown in step 3)
	//2 for each value - add to the metadata the new specimens with the parentspecimenid = harvested specimenid
	//3 create item and assign appropriate specimen to each
		String pstring = "export/samplemeasurement/csv"; //trailunitcsv = or traitid =

		Memo1->Lines->Add("Getting full list of traits from database...");
		Application->ProcessMessages();
		bool success = LoginWn->DoQuery(pstring, true);
		if (!success) {
			Memo1->Lines->Add(pstring);
			NextBut->Enabled = false;
			return ;
		}
		int csvstart = pstring.Pos("csv") + 5;
		pstring = pstring.SubString(csvstart, pstring.Length() - csvstart - 11);
		//get trait values
		success = LoginWn->DoQuery(pstring, true);
		if (!success) {
			Memo1->Lines->Add(pstring);
			NextBut->Enabled = false;
			return ;
		}
		TMemo *m2 = new TMemo(this);
		m2->Text = pstring;
		m2->Lines->SaveToFile(SettingsWn->path + "t" + String(trialno) + ".trait"); //todo move this and .trial into the correct subdirectory
		delete m2; m2 = NULL;
		Memo1->Lines->Strings[Memo1->Lines->Count - 1] = Memo1->Lines->Strings[Memo1->Lines->Count - 1] + "completed";
		//get specimengroup values
		Memo1->Lines->Add("Getting full list of specimen groups from database...");
		pstring = "list/specimengroup/10000/page/1";
		success = LoginWn->DoQuery(pstring, true);
		if (!success) {
			Memo1->Lines->Add(pstring);
			NextBut->Enabled = false;
			return ;
		}
		LoginWn->FillTable(pstring);
		TStringList* notFoundList = new TStringList();
		String seArch = "Harvest_Trial_" + String(trialno);
		int idcol = GetCol("SpecimenGroupName", MainWn->StringGrid1);
		//find SpecimenGroupName = Harvest_Trial_ + String(trialno)
		for (int i = 0; i < MainWn->StringGrid1->RowCount ; i++) {
			String itemInfo = MainWn->StringGrid1->Cells[idcol][i];
			if (itemInfo == seArch) {//when the first found, loop adding the SpecimenId where removeSpecimen starts the same ie "specimengroup/1"
				int ridcol = GetCol("removeSpecimen", MainWn->StringGrid1);
				String sameGroup = MainWn->StringGrid1->Cells[ridcol][i].SubString(1, 30);
				do {
					int sidcol = GetCol("SpecimenId", MainWn->StringGrid1);
					notFoundList->Add(MainWn->StringGrid1->Cells[sidcol][i]);
					i++;
				} while (sameGroup == MainWn->StringGrid1->Cells[ridcol][i].SubString(1, 30));

			}
        }
		//add new specimens to offline data
		String searchVal = "";
		for (int wi = 0; wi < notFoundList->Count; wi++) {
			if (searchVal.IsEmpty()) searchVal = "(" + notFoundList->Strings[wi];
			else searchVal = searchVal + "," + notFoundList->Strings[wi];
		}
		if (!searchVal.IsEmpty()) {
			searchVal = searchVal + ")";
			//cutdown for SpecimenId, TrialUnitSpecimenId & ItemSourceId so it doesn't grab the big tables
			LoginWn->GetCombosFromDAL("SpecimenId", searchVal);
			//now need to set these specimen's parent so we can find them in step 3
			for (int i = 0; i < Form1->SpecimenId->Count ; i++) {
				TStringList *sl = (TStringList*)Form1->SpecimenId->Items->Objects[i];
				String sid = sl->Values[Form1->getMapping(Form1->SpecimenId)];
				int sidcol = GetCol("SpecimenId", MainWn->StringGrid1);
				for (int j = 0; j < MainWn->StringGrid1->RowCount ; j++) {
					if (MainWn->StringGrid1->Cells[sidcol][j] == sid) {//found matching specimenid
						int pidcol = GetCol("ParentSpecimenId", MainWn->StringGrid1);
						sl->Values["ParentSpecimenId"] = MainWn->StringGrid1->Cells[pidcol][j];
						break;
					}
				}
			}
			Memo1->Lines->Strings[Memo1->Lines->Count - 1] = Memo1->Lines->Strings[Memo1->Lines->Count - 1] + "completed";
			Memo1->Lines->Add("Getting full list of metadata from database...");
			LoginWn->GetFastCombosFromDAL();
			Memo1->Lines->Strings[Memo1->Lines->Count - 1] = Memo1->Lines->Strings[Memo1->Lines->Count - 1] + "completed";
			Form1->Label1->Text = "Press 'Next' to continue";
			NextBut->Enabled = true;
		}
		else {
			Memo1->Lines->Strings[Memo1->Lines->Count - 1] = Memo1->Lines->Strings[Memo1->Lines->Count - 1] + "error no specimens have been found for harvested items";
			//Form1->Label1->Text = "Press 'Next' to continue";
			NextBut->Enabled = false;
        }

		//not required --- 2. list/pedigree to find all the group all SpecimenIds by ParentSpecimenId
		//not required ---  use private map of vectors
		//3. to use iterate over Specimen combobox checking that parentSpecimenId = searchSpecimenId and that it hasn't already been used.
		 //not required?
	}
	Form1->AniIndicator1->Visible = false;
	Form1->DontSaveYet = false;
	Application->ProcessMessages();
	Form1->SaveNow(Sender);
}
else if (currentpage == 2)  {
	if (Sender == NextBut) {
		Label4->Text = "Please wait...";
		Application->ProcessMessages();
		reloadTrial(trialno);
		TMemo *trait = NULL;
		int traitId = -1; //todo move to global (obtained in previous step)
		if (tstate != PREPARE) {
			trait = new TMemo(this);
			Memo1->Lines->Add("Cross-referencing trait 82 to find Number_Ears_Advanced for each plot...");
			trait->Lines->LoadFromFile(SettingsWn->path + "t" + String(trialno) + ".trait");
			traitId = 82; //todo allow user to choose trait!
		}

		Form1->TreeView1->BeginUpdate();
		for (int i = 0; i < MainWn->StringGrid1->RowCount ; i++) {
			Form1->AddClick(0);
			String unitposText = MainWn->StringGrid1->Cells[GetCol("UnitPositionText", MainWn->StringGrid1)][i];
			String tuBC = MainWn->StringGrid1->Cells[GetCol("TrialUnitBarcode", MainWn->StringGrid1)][i];
			String tuId = MainWn->StringGrid1->Cells[GetCol("TrialUnitId", MainWn->StringGrid1)][i];
			NodeData *data = (NodeData*)(Form1->TreeView1->Selected->Data);
			if (!tuBC.IsEmpty()) { //harvest!
				data->orig->Values["ItemBarcode"] = tuBC;
				data->final->Values["ItemBarcode"] = tuBC;
				data->orig->Values["ItemId"] = unitposText;
				data->final->Values["ItemId"] = unitposText;
				bool tfound = false;
				for (int q = 1; q < trait->Lines->Count && !tfound; q++) {
					int p = trait->Lines->Strings[q].Pos(",");
					String sid = trait->Lines->Strings[q].SubString(0, p - 1);
					if (sid == tuId) {//we have found the correct plot so now verify trait = sel trait
						String shortText = trait->Lines->Strings[q].SubString(p + 1, trait->Lines->Strings[q].Length());
						p = shortText.Pos(",");
						String tid = shortText.SubString(1, p - 1);
						if (tid == traitId) {
							p = shortText.LastDelimiter(",");
							data->orig->Values["TraitValue"] = shortText.SubString(p + 1, shortText.Length());
							//data->orig->Values["TraitHarvestCount"] = "0";
							//TraitHarvestList[specId] = String(fcount);
							tfound = true;
							break;
						}
					}
				}
			}
			else {
				if (tstate != PREPARE) { //could only occur if KDLog was interrupted during step 8
					Memo1->Lines->Add("Error detected. TrialUnitBarcode is missing from planted trial at " + unitposText);
					Memo1->Lines->Add("Trial is in intermediate state. Serious risk of corruption. Loading cancelled");
					Form1->TreeView1->EndUpdate();
					SetTState(PREPARE);
					Form1->AniIndicator1->Visible = false;
					return;
				}
				//Memo1->Lines->Add("Trial has not been planted");
			}
		//			String tusId = MainWn->StringGrid1->Cells[GetCol("TrialUnitSpecimenId", MainWn->StringGrid1)][i];

			String repNo = MainWn->StringGrid1->Cells[GetCol("ReplicateNumber", MainWn->StringGrid1)][i];
			String upNo = MainWn->StringGrid1->Cells[GetCol("UnitPositionId", MainWn->StringGrid1)][i];
			String tusNo = MainWn->StringGrid1->Cells[GetCol("TrialUnitSpecimenId", MainWn->StringGrid1)][i];
			String spID = MainWn->StringGrid1->Cells[GetCol("SpecimenId", MainWn->StringGrid1)][i];
			data->final->Values["TrialUnitSpecimenId"] = spID;
			data->orig->Values["TrialUnitSpecimenId"] = spID;
			data->final->Values["SpecimenId"] = spID;
			data->orig->Values["SpecimenId"] = spID;
			data->final->Values["TrialUnitId"] = tuId;
			data->orig->Values["TrialUnitId"] = tuId;
		//	data->orig->Values["UnitPosText"] = unitposText;
		//	data->final->Values["UnitPosText"] = unitposText;
			data->orig->Values["ItemNote"] = unitposText;
			data->final->Values["ItemNote"] = unitposText;
			Form1->TreeView1->Selected->Text = "        " + unitposText;
			data->orig->Values["ReplicateNumber"] = repNo;
			data->final->Values["ReplicateNumber"] = repNo;
			data->orig->Values["UnitPositionId"] = upNo;
			data->final->Values["UnitPositionId"] = upNo;
			//data->orig->Values["TrialUnitSpecimenId"] = tusId;  //this only occurs during harvest
			//data->final->Values["TrialUnitSpecimenId"] = tusId;
			data->orig->Values["ContainerTypeId"] = "6";
			//data->final->Values["TrialUnitSpecimenId"] = tusId;
			data->orig->Values["ContainerTypeId"] = "6";//hardcoded values = envelope
			data->final->Values["ContainerTypeId"] = "6";//todo set this properly (lookup envelope and if it doesnt exist then create it!)
			data->orig->Values["ItemUnitId"] = "13";//kernels
			data->final->Values["ItemUnitId"] = "13";
			if (i % 50 == 0) {
				Form1->ProgressBar1->Value = i;
				Form1->Label1->Text = "Created " + String(i) + " items of " + String(Form1->ProgressBar1->Max);
				Application->ProcessMessages();
			}
		}
		if (tstate == HARVEST) {
        	for (int i = Form1->TreeView1->Count - 1; i >= 0; i--) {
				TListBoxItem *t = Form1->TreeView1->ItemByIndex(i);
				NodeData *data = (NodeData*)(t->Data);
				String itid = data->orig->Values["ItemId"];

				if (!itid.Pos("Harvest")) {  //temp setting to make this a source item
					TColorButton *but = Form1->GetButton(t);
					but->Color = TAlphaColorRec::Green;
				}
				else {
					Form1->RemoveTreeItem(t); //probably don't need to call this as there are no orig source items?
				}
			}
			for (int i = Form1->TreeView1->Count - 1; i >= 0; i--) {
				TListBoxItem *t = Form1->TreeView1->ItemByIndex(i);
				if (t->Visible) {
					NodeData *data = (NodeData*)(t->Data);
					int maxval = data->orig->Values["TraitValue"].ToIntDef(0);
					for (int qq = 0; qq < maxval; qq++) {
						AddHItem(t, qq);
					}
				}
			}
		}
		Form1->TreeView1->EndUpdate();
	}
	//Form1->DontSaveYet = false;

	if (tstate == HARVEST) {
		//aaGroupBox1->Visible = true;   //disabled this functionality
		//Label5->Visible = true;
		Label3->Text = "Review selections for each plot";
		Label4->Text = "Selections for harvesting have been automatically set";

		StartShow();
		Form1->GroupBox8->Parent = Rectangle5;
		Form1->GroupBox8->Align = TAlignLayout::alTop;
		SetTreeItemsVisible(0);   //view both
		CheckEnableNext();
	}
	else if (tstate == PREPARE) {
		Label5->Visible = false;
		Label3->Text = "Sort the shortlist";
		Label4->Text = "Sort by 'ItemNote' for single reps or by 'SpecimenId' for multiples";
		StartShow();
		Form1->GroupBox8->Parent = Rectangle5;
		Form1->GroupBox8->Align = TAlignLayout::alTop;

		SetTreeItemsVisible(0);
		//sort by genotype
		Form1->ComboBox3->ItemIndex = 1; //ascending
		Form1->ShSort->ItemIndex = Form1->ShSort->Items->IndexOf("SpecimenId");
		Form1->Button22Click(0);
		//now put temp flag on each destination item so we can easily subsample
		String sourceId;
		for (int i = 0; i < Form1->TreeView1->Count; i++) {
			TListBoxItem *t = Form1->TreeView1->ItemByIndex(i);
			if (Form1->getTreeStrFromObject(t, "ItemId") != "new") {
				sourceId = Form1->getTreeStrFromObject(t, "ItemId");
			}
			else {//its a destination
				NodeData *d = (NodeData*)t->Data;
				d->orig->Values["ItemGroupId"] = sourceId;
				d->final->Values["ItemGroupId"] = sourceId;
				String newbc = d->final->Values["ItemBarcode"];

				String bcextract = newbc.SubString(2, newbc.Length()-2);
				if (!bcextract.Pos("|")) {//only set the barcodes if not already set
					newbc = newbc.Insert(sourceId + "|", 4);
				}
				d->orig->Values["ItemBarcode"] = newbc;
				d->final->Values["ItemBarcode"] = newbc;
			}
		}
	}
	EndShow();
	Form1->SaveNow(Sender);
}
else if (currentpage == 3)  {
	Label3->Text = "Print the shortlist";
	Label4->Text = "Export to file (e.g. envelopes/tickets) or print barcode labels";
	Form1->GroupBox10->Parent = Rectangle5;
	Form1->GroupBox10->Align = TAlignLayout::alTop;
}
else if (currentpage == 4)  {
	Label4->Text = "Selected item:";
    hpButton->Visible = true;
	if (tstate == PREPARE) {
		Label3->Text = "Barcode scan or click 'prepare' button to prepare destination(s)";
		hpButton->Text = "Prepare";
	}
	else {
		Label3->Text = "Barcode scan or click 'harvest' button to harvest destination(s)";
		hpButton->Text = "Harvest";
	}
	Form1->Amount->Parent = Label4;
	Form1->Amount->Width = 50;
	Form1->Amount->Position->Y = 3;
	Form1->reweighBut->Position->X = Form1->Amount->Width + 5 ;
	Form1->Amount->Position->X = Label4->Width - Form1->Amount->Width - Form1->reweighBut->Width - eButton4->Width - 10;
	StartShow();
	if (tstate == PREPARE) SetTreeItemsVisible(0);  //view both
	EndShow();
	//if (tstate == PREPARE) {
		eButton4->Visible = true;
		Form1->UpdateSeedProgress();
	//}
}
else if (currentpage == 5)  {
	StartShow();
	if (tstate == HARVEST) {
		Label3->Text = "Set other fields";
		Label4->Text = "Enter values (fields marked * are compulsory)";

		ShowSpec();

		SetTreeItemsVisible(3);//view white only
	}
	else {
		UndoValidation();
		Label3->Text = "Re-sort the destination items for planting";
		Label4->Text = "Sort by 'Ascending' or 'Descending' to suit";
		Form1->GroupBox8->Parent = Rectangle5;
		Form1->GroupBox8->Align = TAlignLayout::alTop;
		SetTreeItemsVisible(2);//view destinations
	}

	//sort by entry
	Form1->ComboBox3->ItemIndex = 1; //descending
	Form1->ShSort->ItemIndex = Form1->ShSort->Items->IndexOf("ItemNote");
	Form1->Button22Click(0);
	EndShow();
}
else if (currentpage == 6)  {
	StartShow();
	if (tstate == HARVEST) {
		Label3->Text = "Set harvest items storage by barcodes (not implemented)";
		Label4->Text = "Remove empty items by right-click";
		SetTreeItemsVisible(3);//view white only
	}
	else {
		Label3->Text = "Validate the destination envelope order";
		Label4->Text = "Barcode scan the destination barcodes in the correct sequence";
		SetTreeItemsVisible(2);//view destinations
	}
	EndShow();

	//to do validate that all the bags have amounts stored
}
else if (currentpage == 7)  {
	UndoValidation();
	Label3->Text = "Update the database with your changes";
	Label4->Text = "Click the button to apply the changes";
	StartShow();

	if (tstate == HARVEST) {
		SetTreeItemsVisible(3);//view white only
	}
	else {
		SetTreeItemsVisible(0);//view both
	}
	Button1->Visible = true;
	EndShow();
	//Form1->ProgressBar1->Max = Form1->TreeView1->Count;
}
}
//---------------------------------------------------------------------------
void TWizardWn::UndoValidation() {
for (int i = Form1->TreeView1->Count - 1; i >= 0; i--) {
	TListBoxItem *t = Form1->TreeView1->ItemByIndex(i);
	if (Form1->getTreeStrFromObject(t, "ItemId") == "new") {
		TColorButton *but = Form1->GetButton(t);
		but->Color = TAlphaColorRec::White;
	}
}
}
//---------------------------------------------------------------------------
void TWizardWn::ShowSpec() {
/*Form1->ignore = true;
Form1->AddFieldButClick(Form1->specimen); //create new specimen dialog
Form1->ignore = false;
if (Form1->SpecimenId->ItemIndex == -1) {

	for (int j = 0; j < Form3->Layout1->Children->Count; j++) {
		TEdit *e = dynamic_cast<TEdit*>(Form3->Layout1->Children->Items[j]);
		if (e) {
			if (e->Name == "BreedingMethodId") {
				e->Text = 0;
			}
			else if (e->Name == "SpecimenName") {
				TDateTime dateNow = Now();//temp (todo remember to delete xml files that aren't required or create them just when we need them)
				e->Text = "GID " + String((double)dateNow * 86400);
			}
			else if (e->Name == "Pedigree") {
				//e->Text = StringReplace(sgI->Cells[3][i],"\"","", TReplaceFlags() << rfReplaceAll);
			}
			else if (e->Name == "IsActive") {
				e->Text = "1";
			}
		}
		else {
			TMemo *m = dynamic_cast<TMemo*>(Form3->Layout1->Children->Items[j]);
			if (m) {
				if (m->Name == "GenotypeSpecimenType") {
					m->Text = "0";
				}
				else if (m->Name == "GenotypeId") {
					TStringList *s = (TStringList*)Form1->SpecimenId->Items->Objects[0];
					m->Text = s->Values["GenotypeId"];
				}
			}
		}
	}
} */
//Form1->Panel1->Parent = Rectangle5; //start here

int origw = Form1->ItemDetailBox->Width;//Form3->Layout1->Width;
Form1->ItemDetailBox->Parent = Rectangle5;
Form1->ItemDetailBox->Align = TAlignLayout::alNone;
Form1->TreeView1->Align = TAlignLayout::alNone;
Form1->TreeView1->Width = 200;//origw - 100 - Label3->Position->X;
Form1->TreeView1->Position->X = Label3->Position->X;
Form1->ItemDetailBox->Align = TAlignLayout::alClient;
Form1->TreeView1->Align = TAlignLayout::alLeft;
Form1->Width = Rectangle5->Width - Form1->TreeView1->Width;

//Form1->ItemDetailBox->Position->X = Form1->TreeView1->Width + Label3->Position->X;
//Form1->ItemDetailBox->Position->Y = Label4->Position->Y + Label4->Height - 10;
Form1->TreeView1->Position->Y = Label4->Position->Y + Label4->Height;
Form1->TreeView1->Height = Rectangle5->Height - Form1->TreeView1->Position->Y;
}
//---------------------------------------------------------------------------
void TWizardWn::SetTreeItemsVisible(int newindex){
int oldindex = Form1->ComboBox4->ItemIndex;
Form1->ComboBox4->ItemIndex = newindex;
if (oldindex == Form1->ComboBox4->ItemIndex) {
	Form1->ComboBox4Change(0); //ensure things are hidden/shown
}
}
//---------------------------------------------------------------------------
bool TWizardWn::GetTrialUnits(int tno) {
String pstring = "trial/" + String(tno) + "/list/trialunit";
Memo1->Lines->Add("Getting details from database...");
Application->ProcessMessages();
bool success = LoginWn->DoQuery(pstring, true);
if (!success) {
	Memo1->Lines->Add(pstring);
	return false;
}
TMemo *m2 = new TMemo(this);
m2->Lines->Add(pstring);
m2->Lines->SaveToFile(SettingsWn->path + "t" + String(tno) + ".trial");
delete m2; m2 = NULL;
LoginWn->FillTable(pstring);
return true;
}
//---------------------------------------------------------------------------
void TWizardWn::reloadTrial(int trialno) {
TMemo* m2 = new TMemo(this);
m2->Lines->LoadFromFile(SettingsWn->path + "t" + String(trialno) + ".trial");
LoginWn->FillTable(m2->Text);
delete m2; m2 = NULL;
}
//---------------------------------------------------------------------------
void TWizardWn::EndShow() {
//Form1->AniIndicator1->Parent = Form1->StatusBar1;
Form1->AniIndicator1->Visible = false;
Form1->TreeView1->Visible = true;

for (int i = 0; i < Form1->TreeView1->Count; i++) {
	if (Form1->TreeView1->ItemByIndex(i)->Visible) {
		Form1->TreeView1->ItemByIndex(i)->IsSelected = true;
		Form1->TreeView1Click(Form1->TreeView1);
		break;
	}
}
Width++;
Width--;
}
//---------------------------------------------------------------------------
void TWizardWn::StartShow() {
//Form1->AniIndicator1->Parent = Rectangle4;
Form1->AniIndicator1->Visible = true;
Form1->TreeView1->Visible = false;
Form1->TreeView1->Parent = Rectangle5;
Form1->TreeView1->Align = TAlignLayout::alClient;

Application->ProcessMessages();
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::FinishButClick(TObject *Sender) {
if (mrYes == MessageDlg("You have now finished the seed preparation for trial " + String(trialno) + ". Click 'OK' to exit the wizard?", TMsgDlgType::mtConfirmation,
	TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
	Close();
}
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::CancButClick(TObject *Sender) {
Close();
}
//---------------------------------------------------------------------------
void TWizardWn::Scale() {
Layout1->Scale->X = Form1->Layout1->Scale->X;
Layout1->Scale->Y = Form1->Layout1->Scale->Y;
//ClientHeight = ClientHeight * Layout1->Scale->Y;
//ClientWidth = ClientWidth * Layout1->Scale->X;
FormResize(0);
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::FormResize(TObject *Sender) {
Rectangle3->Width = (ClientWidth)/Layout1->Scale->X - Rectangle1->Width;
Rectangle3->Height = (ClientHeight - ToolBar1->Height) / Layout1->Scale->Y;
ToolBar1->Position->Y = Rectangle3->Height;
Image4->Width = Rectangle1->Width;
Image4->Height = (ClientHeight)/Layout1->Scale->Y;
Rectangle1->Height = (ClientHeight)/Layout1->Scale->Y;
if (Form1) {
	if (Form1->ItemDetailBox->Parent == Rectangle5) {
		Form1->Width = Rectangle5->Width - Form1->TreeView1->Width;
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::FormClose(TObject *Sender, TCloseAction &Action) {
TModalResult answer = mrYes;
if (somethingChanged) {
	answer = MessageDlg("You have chosen to exit the wizard at step " + String(currentpage + 1) +
	". Would you like save changes to trial " + String(trialno) + "?", TMsgDlgType::mtWarning,
	TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0);
}
if (answer == mrCancel) {
	Action = TCloseAction::caNone;
}
else {
	Form1->Width = origFormWidth;
	Form1->ItemDetailBox->Align = TAlignLayout::alNone;
	Form1->ItemDetailBox->Parent = Form1->Panel1;
	Form1->ItemDetailBox->Position->X = 5;
	Form1->ItemDetailBox->Position->Y = 48;
	//Form1->ItemDetailBox->Width = 105;
	//Form1->ItemDetailBox->Height = 148;
	Form1->GroupBox8->Align = TAlignLayout::alNone;
	Form1->GroupBox8->Parent = Form1->SettingsTab;
	Form1->GroupBox8->Position->X = 8;
	Form1->GroupBox8->Position->Y = 212;
	Form1->GroupBox8->Width = Form1->GroupBox5->Width;
	Form1->GroupBox8->Anchors <<(TAnchorKind::akRight);
	Form1->GroupBox10->Parent = Form1->SettingsTab;
	Form1->GroupBox10->Align = TAlignLayout::alNone;
	Form1->GroupBox10->Position->X = 8;
	Form1->GroupBox10->Position->Y = 152;
	Form1->GroupBox10->Width = Form1->GroupBox5->Width;
	Form1->GroupBox10->Anchors <<(TAnchorKind::akRight);
	Form1->TreeView1->Parent = Form1->Layout1;
	Form1->TreeView1->Align = TAlignLayout::alMostLeft;
	Form1->TreeView1->Width = treeWidth;
	SetAmountDefaultPos();
	Form1->SetProgressBar();
	if (currentpage == 5 && tstate == PREPARE) UndoValidation();
	if (PrepareWn) {
		if (Form1->sg) Form1->sg->Parent = Form1->SearchTab;
		delete PrepareWn;
		PrepareWn = NULL;
	}
	if (answer == mrYes) {
		Form1->SaveShortButClick(0);

	}
	else { //restore the wizards position if the user decides not to apply changes
		currentpage = origcurrentpage;
		UpdateOffLineTrialStatus();
	}
	somethingChanged = false;

	if (Form1->TabControl2->TabIndex != 0) Form1->TabControl2->TabIndex = 0;
	else Form1->TabControl2Change(0);
	Form1->ClearShortButClick(0); //enabling this option makes app startup quick as no metadata is ever stored in the home dir
	DeleteFile(SettingsWn->path + "tmppos.txt");
}
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::Button1Click(TObject *Sender) {
//update trialunit table with destination barcodes
//GetTrialUnits(10);//temp
Button1->Enabled = false;
for (int j = 0; j < Form1->TreeView1->Count; j++) {
	TListBoxItem *t = Form1->TreeView1->ItemByIndex(j);
	t->IsSelected = true;  //select the node
	Form1->TreeView1Click(Form1->TreeView1); //ensure its selected
	if (Form1->getTreeStrFromObject(t, "ItemId") == "new")  {//its a destination so get trialunit
		NodeData *data = (NodeData*)(t->Data);
		String tuid = data->final->Values["TrialUnitId"];//Find(data->final->Values["TrialUnitId"], "TrialUnitId", 1);
		if (tuid == "-1") {
			ShowMessage("error finding UnitId");
		}
		String s = "update/trialunit/" + String(tuid);
		TStringList *uplist = new TStringList();
		TStringList *sl = new TStringList();
		uplist->Add("ReplicateNumber");
		String rep = data->orig->Values["ReplicateNumber"];//Find(tuid, "ReplicateNumber", 0);
		if (rep == "-1") {
			ShowMessage("error finding RepNo");
		}
		sl->Values["ReplicateNumber"] = rep;
		uplist->Add("UnitPositionId");
		sl->Values["UnitPositionId"] = data->orig->Values["UnitPositionId"];//Find(tuid, "UnitPositionId", 0);
		uplist->Add("TrialUnitBarcode");
		sl->Values["TrialUnitBarcode"] = data->orig->Values["ItemBarcode"];
		//todo optional change of entry order
		uplist->Add("UnitPositionText");
		sl->Values["UnitPositionText"] = data->orig->Values["ItemNote"];
		Label4->Text = "Updating database trialunit " + String(tuid);
		Form1->ProgressBar1->Value++;
		Application->ProcessMessages();
		if (!LoginWn->DoUpdate(uplist, sl, s)) return;
		delete sl; sl = NULL;
		delete uplist; uplist = NULL;
	}
	else {
		Form1->Update1Click(Sender);
	}
}
Form1->ProgressBar1->Value = 0;
FinishBut->Enabled = true;
}
//---------------------------------------------------------------------------
void TWizardWn::SetAmountDefaultPos() {
Form1->Amount->Parent = Form1->ItemDetailBox;
Form1->reweighBut->Position->X = Form1->Panel1->Width - 183;
Form1->Amount->Width = Form1->Panel1->Width - 184;
Form1->Amount->Position->X = 80;
Form1->Amount->Position->Y = 56;
}
//---------------------------------------------------------------------------
String TWizardWn::Find(String value, String col, bool return_tuid) { //temp function
for (int i = 0; i < MainWn->StringGrid1->RowCount ; i++) {
	String ID;
	if (!return_tuid) ID = MainWn->StringGrid1->Cells[GetCol("TrialUnitId", MainWn->StringGrid1)][i];
	else ID = MainWn->StringGrid1->Cells[GetCol(col, MainWn->StringGrid1)][i];
	if (ID == value) { //used in adding source items
		if (!return_tuid) return MainWn->StringGrid1->Cells[GetCol(col, MainWn->StringGrid1)][i];
		else return MainWn->StringGrid1->Cells[GetCol("TrialUnitId", MainWn->StringGrid1)][i];
	}
}
//ShowMessage("not found");
return "-1";
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::aaComboBox1Change(TObject *Sender) {
aaComboBox1->Enabled = false;
Form1->TreeView1->Parent = Form1->Layout1;
Form1->TreeView1->Align = TAlignLayout::alMostLeft;
//Form1->TreeView1->Width = treeWidth;
Application->ProcessMessages();
Form1->TreeView1->BeginUpdate();
for (int i = Form1->TreeView1->Count - 1; i >= 0; i--) {
	TListBoxItem *t = Form1->TreeView1->ItemByIndex(i);
	RemoveHItem(t);
}
Form1->TreeView1->EndUpdate();
StartShow();
Form1->DontSaveYet = true;

for (int i = Form1->TreeView1->Count - 1; i >= 0; i--) {
	TListBoxItem *t = Form1->TreeView1->ItemByIndex(i);
	if (t->Visible) {
		for (int j = 0; j < aaComboBox1->ItemIndex; j++) {
			AddHItem(t, j);
			Application->ProcessMessages();
		}
	}
}


//Form1->ComboBox3->ItemIndex = 1; //descending
//Form1->ShSort->ItemIndex = Form1->ShSort->Items->IndexOf("TrialUnitSpecimenId");
//Form1->Button22Click(0);
Form1->DontSaveYet = false;
//Form1->SaveNow(Sender);
EndShow();
CheckEnableNext();
aaComboBox1->Enabled = true;

}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::aaButton3Click(TObject *Sender) {
String specId = Form1->getTreeStrFromObject(Form1->TreeView1->Selected, "SpecimenId");
AddHItem(Form1->TreeView1->Selected, TraitHarvestList->Values[specId].ToIntDef(0) + 1);
CheckEnableNext();
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::aaButton2Click(TObject *Sender) {
int index = Form1->TreeView1->Selected->Index;
TListBoxItem *t = Form1->TreeView1->ItemByIndex(index + 1);
if (t) RemoveHItem(t);
CheckEnableNext();
}
//---------------------------------------------------------------------------
void TWizardWn::AddHItem(TListBoxItem *t, int spacing) {
//find appropriate specimen from metadata (where parentspecimenid == this specimenId)
String specId = Form1->getTreeStrFromObject(t, "SpecimenId");
String nspecId = "-1";
int fcount = 0;
bool found = false;
int usedsofar = TraitHarvestList->Values[specId].ToIntDef(0);
for (int i = 0; i < Form1->SpecimenId->Count ; i++) {
	TStringList *sl = (TStringList*)Form1->SpecimenId->Items->Objects[i];
	String psid = sl->Values["ParentSpecimenId"];
	if (psid == specId) {
		fcount++;
		if (fcount - 1 == usedsofar) {
			nspecId = sl->Values["SpecimenId"];
			found = true;
			break;
		}
	}
}

if (!found) {
	specId = " missing " + specId;
}
//NodeData *dd = (NodeData*)t->Data;
//dd->final->Values["TraitHarvestCount"] = String(fcount);
TraitHarvestList->Values[specId] = String(fcount);
String text = Form1->getTreeStrFromObject(t, "ItemNote");
String tui = Form1->getTreeStrFromObject(t, "TrialUnitSpecimenId");
//todo check for missing fields
NodeData *d = new NodeData();
d->orig->Values["ItemNote"] = text + " Harvest";
d->final->Values["ItemNote"] = text + " Harvest";
d->orig->Values["ItemGroupId"] = text;
d->final->Values["ItemGroupId"] = text;
d->orig->Values["TrialUnitSpecimenId"] = tui;
d->final->Values["TrialUnitSpecimenId"] = tui;
d->orig->Values["ItemId"] = "new";
d->final->Values["ItemId"] = "new";
d->orig->Values["SpecimenId"] = nspecId;
d->final->Values["SpecimenId"] = nspecId;
d->orig->Values["Amount"] = "0";
d->final->Values["Amount"] = "0";
TDateTime dateNow = Now();
String ds = dateNow.FormatString("yyyy-MM-dd HH:mm:ss");
d->orig->Values["DateAdded"] = ds;
d->final->Values["DateAdded"] = ds;
d->orig->Values["ItemBarcode"] = "|*I" + String((double)dateNow * 86400) + "*|";
d->final->Values["ItemBarcode"] = d->orig->Values["ItemBarcode"];
Sleep(1);
TListBoxItem* v = Form1->AddChildNode(t->Index + 1 + spacing, d);//prevents access violations
TColorButton *but = Form1->GetButton(v);
but->Color =  TAlphaColorRec::White;
#ifndef _DEBUG
v->Text = "        " + text + " Harvest";
#endif
#ifdef _DEBUG
v->Text = "        " + text + " Harvest " + String(fcount) + " " + String(specId);
#endif
}
//---------------------------------------------------------------------------
void TWizardWn::RemoveHItem(TListBoxItem *t) {
String text = Form1->getTreeStrFromObject(t, "ItemNote");
if (text.Pos(" Harvest")) {
	Form1->RemoveTreeItem(t);
}
}
//---------------------------------------------------------------------------
void TWizardWn::CheckEnableNext() {
int count = 0;
bool found = false;
for (int i = Form1->TreeView1->Count - 1; i >= 0; i--) {
	TListBoxItem *t = Form1->TreeView1->ItemByIndex(i);
	TColorButton *but = Form1->GetButton(t);
	if (but->Color ==  TAlphaColorRec::White) {
		found = true;
		count++;
	}
}
NextBut->Enabled = found;
aaGroupBox1->Text = "Selections total: "+ String(count);
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::FormKeyDown(TObject *Sender, WORD &Key, System::WideChar &KeyChar, TShiftState Shift) {
if (currentpage == 4 || currentpage == 6) {
	if (this->Active) {
		//Caption = Caption + Name;
		Form1->valEditKeyDown(Sender, Key, KeyChar, Shift);
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::FormKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar, TShiftState Shift) {
if (currentpage == 4 || currentpage == 6) {
	if (this->Active) Form1->valEditKeyUp(Sender, Key, KeyChar, Shift);
}
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::eButton4Click(TObject *Sender) {
int startpos = Form1->TreeView1->Selected->Index + 1;
if (startpos == Form1->TreeView1->Count) {//start again
	startpos = 1;
}
for (int i = startpos; i < Form1->TreeView1->Count; i++) {
	TListBoxItem *t = Form1->TreeView1->ItemByIndex(i);
	if (Form1->getTreeStrFromObject(t, "ItemId") == "new" && Form1->getTreeStrFromObject(t, "Amount").ToDouble() == 0) {
		t->IsSelected = true;
		Form1->TreeView1Click(Form1->TreeView1);
		return;
	}
}
Form1->Label1->Text = "All destinations have been filled. Click 'Next' to continue";
}
//---------------------------------------------------------------------------
void TWizardWn::UpdateOffLineTrialStatus() {
TStringList *info = new TStringList(this);
String ifile = SettingsWn->path + "Localtriallist.txt";
if (FileExists(ifile)) {
	info->LoadFromFile(ifile);
}
String mmode;
if (tstate == PREPARE) mmode = " (PREPARE)";
else if (tstate == HARVEST) mmode = " (HARVEST)";
else mmode = " (N/A)";
for (int i = 0; i < info->Count; i++) {
	String line = info->Strings[i];
	int p = line.Pos(",");
	int val = line.SubString(0, p - 1).ToIntDef(-1);
	if (val != -1) {
		if (val == trialno) {
			p = line.LastDelimiter(",");
			String oldText = info->Strings[i].SubString(0, p);
			info->Delete(i);
			info->Insert(0, oldText + stepStr[currentpage + 1]->Text + mmode);
			break;
		}
	}
}
info->SaveToFile(ifile);
delete info; info = NULL;
}
//---------------------------------------------------------------------------
void TWizardWn::WriteWizardPos(int trialno, int currentpos) {
TStringList *tmpos = new TStringList(this);
tmpos->Add(String(trialno));
tmpos->Add(String(currentpos));
tmpos->Add(String(tstate));
tmpos->SaveToFile(SettingsWn->path + "tmppos.txt");
delete tmpos; tmpos = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::setSBut2Click(TObject *Sender) {
Form8 = new TForm8(this);
if (currentpage == 5) {
	Form8->Scale(Form1->GroupBox4, Sender);
}
else Form8->Scale(Form1->SeSettings, Sender);
Form8->Show();
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::HelpButClick(TObject *Sender) {
Form1->HelpButClick(Sender); //todo support different locations in help doco for different help clicks!
}
//---------------------------------------------------------------------------
void __fastcall TWizardWn::hpButtonClick(TObject *Sender) {
Form1->SubsampleClick(Sender);
}
//---------------------------------------------------------------------------

