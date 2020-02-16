//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "Sample.h"
#include "scanwn.h"
#include "wizard.h"
#include "unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TSampleWn *SampleWn;
//---------------------------------------------------------------------------
__fastcall TSampleWn::TSampleWn(TComponent* Owner) : TForm(Owner) {
}
//---------------------------------------------------------------------------
void TSampleWn::Scale() {
Layout1->Scale->X = Form1->Layout1->Scale->X;
Layout1->Scale->Y = Form1->Layout1->Scale->Y;
ClientHeight = ClientHeight * Layout1->Scale->Y;
ClientWidth = ClientWidth * Layout1->Scale->X;
Left = (Form1->Left) + (Form1->Width - Width)/2;
Top = (Form1->Top) + (Form1->Height - Height)/2;
}
//---------------------------------------------------------------------------
void __fastcall TSampleWn::AddButClick(TObject *Sender) {
TListBoxItem* t = Form1->TreeView1->Selected;
NodeData *origd = (NodeData*)(t->Data);

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
}
//---------------------------------------------------------------------------
String TSampleWn::getselText() {
TListBoxItem* t = Form1->TreeView1->Selected;
NodeData *dorig = (NodeData*)(t->Data);
String selText = dorig->final->Values[Form1->GetItemLabelType()];
return selText;
}
//---------------------------------------------------------------------------
void __fastcall TSampleWn::FormClose(TObject *Sender, TCloseAction &Action) {
if (!update) {
	if (ProgressBar1->Value) {//todo do a proper check orig vs final so we dont see this if nothing has changed
		if (mrYes == MessageDlg("Are you sure you want to remove " + String(ProgressBar1->Value) + " destination envelopes from "  + getselText() + "? Click 'OK' to apply any changes", TMsgDlgType::mtWarning,
			TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
			ModalResult = mrYes;
		}
	}
}
else ModalResult = mrYes;
}
//---------------------------------------------------------------------------
void __fastcall TSampleWn::ListBox1Change(TObject *Sender) {
double aggregtotal = 0.0;
SelBox->Enabled = true;
if (WizardWn->tstate == 0) Label5->Text = getselText() + " estimate";
else Label5->Text = getselText() + " total";

//calc agregrated total in grams
for (int i = ListBox1->Items->Count - 1; i >= 0; i--) {
	NodeData *d = (NodeData*)ListBox1->ItemByIndex(i)->Data;
	if (ListBox1->ItemByIndex(i)->IsSelected) {
		Label3->Text = d->final->Values[Form1->GetItemLabelType()] + " weight";
		SelBox->Text = d->final->Values["Amount"];
	}
	String searchid = d->final->Values["ItemUnitId"];
	if (!searchid.IsEmpty()) {
		int index = Form1->getIndex(Form1->ItemUnitId, searchid);
		if (index == -1 ) {
			ShowMessage("error: it appears the item " + getselText() + " has invalid units as no local units corresponding to " + searchid);
		}
		else {
			TStringList *l = (TStringList*)Form1->ItemUnitId->Items->Objects[index];
			if (Sender) {
				unitlabel->Text = l->Values["ItemUnitName"];//Form1->ItemUnitId->Items->Strings[index];
			}
			double convertedamount = d->final->Values["Amount"].ToDouble();
			double mult = l->Values["GramsConversionMultiplier"].ToDouble();
			convertedamount = convertedamount * mult;
			aggregtotal += convertedamount;
		}
	}
}
//get orig amount in grams
NodeData *dorig = (NodeData*)(Form1->TreeView1->Selected->Data);
String osearchid = dorig->final->Values["ItemUnitId"];
if (osearchid.IsEmpty()) {
	return;
}
int oindex = Form1->getIndex(Form1->ItemUnitId, osearchid);
if (oindex == -1 ) {
	ShowMessage("error: no local units corresponding to " + osearchid);
}
else {
	TStringList *ol = (TStringList*)Form1->ItemUnitId->Items->Objects[oindex];
	ounitlabel->Text = ol->Values["ItemUnitName"];
	double mult = ol->Values["GramsConversionMultiplier"].ToDouble();
	double origamount = dorig->orig->Values["Amount"].ToDouble() * mult;
	//cal remaining amount and show in orig units
	double remaining = origamount - aggregtotal;
	if (WizardWn->tstate == 2) {
    	remaining = origamount + aggregtotal;
	}
	double remgrams = remaining / mult;
	AmountBox->Text = FormatFloat("0.000", remgrams); //todo get this from the units gram conversion
}
}
//---------------------------------------------------------------------------
void __fastcall TSampleWn::SelBoxKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar, TShiftState Shift) {
Form1->valEditKeyUp(Sender, Key, KeyChar, Shift);
if (!Form1->BarcodeSearch.IsEmpty()) {
	NodeData *d = (NodeData*)ListBox1->Selected->Data;
	SelBox->Text = d->final->Values["Amount"];
	ListBox1->SetFocus();  //note this is changing the focus away from the amount box (user has an extra click)
	return;
}
//if (!SelBox->ReadOnly) {
	NodeData *d = (NodeData*)ListBox1->Selected->Data;
	String orig = d->final->Values["Amount"];
	String current = SelBox->Text;
	Form1->ValidateEntry(SelBox, KeyChar, orig, current);
	if (current.IsEmpty()) {
		current = "0";
	}
	if (orig.ToDouble() != current.ToDouble()) {
		d->final->Values["Amount"] = current; //todo check do we need to be modifying orig, final or both?
		d->final->Values["ItemUnitId"] = d->orig->Values["ItemUnitId"];
		d->final->Values["Measured"] = 1;
		d->final->Values["ScaleId"] = NULL;
		ProgressBar1->Value = CalcMeasured();
		ListBox1Change(0);
	}
//}
}
//---------------------------------------------------------------------------
void TSampleWn::FindInListbox(String searchStr) {
if (Form1->BarcodeI) { //currently only handle barcode input and searching. Todo search by other means?
	Form1->BarcodeI = false;
	Form1->BarcodeSearch = "";
	bool itemfound = false;
	NodeData *d = NULL;
	for (int i = ListBox1->Items->Count - 1; i >= 0; i--) {
		d = (NodeData*)ListBox1->ItemByIndex(i)->Data;
		if (d->final->Values["ItemBarcode"] == searchStr) {
			ListBox1->ItemIndex = i;
			ListBox1Change(ListBox1);
			itemfound = true;
			break;
		}
	}
	if (!itemfound) {
		Scan->Beep(1);
		ShowMessage("No such barcode (" + searchStr + ") exists in your destination items!");
		//todo search the database?
	}
	else {
		//if settings allow (use scales here checkbox), check scales
		Scan->Beep(0);

		if (AutoWeigh->IsChecked) {
			String units;
			String final_val = Form1->ReadScale(units);
			if (!final_val.IsEmpty()) {
				//update the scale and units (note destination units label need updating on every selection)
				if (d->final->Values["Measured"] == "1") {
					if (mrYes != MessageDlg(Label3->Text + " has previously been measured. DO you want to to update anyway?", TMsgDlgType::mtWarning,
						TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
						ShowMessage("Scales reading ignored");
						return;
					}
				}
				SelBox->Text = final_val.Trim();

				unitlabel->Text = units;
				//d->final->Values["ScaleId"] = Form1->ScaleId->Items->IndexOf();
				d->final->Values["ItemUnitId"] = Form1->getIdOfTitle(Form1->ItemUnitId, units);
				d->final->Values["ScaleId"] = Form1->getIdOfTitle(Form1->ScaleId, "El Batan Seed Scale 1"); //todo remove harcoded
				if (d->final->Values["ItemUnitId"] == -1 || d->final->Values["ScaleId"] == -1) {
					ShowMessage("Weighing error: scales or units are not known in database");
					return;
				}
				d->final->Values["Measured"] = "1";
				ProgressBar1->Value = CalcMeasured();
			}
			else return;
		}
		else {
			SelBox->Text = NumberBox1->Text;   //40 kernels todo make this a setting for maize trials
			d->final->Values["ItemUnitId"] = Form1->getIdOfTitle(Form1->ItemUnitId, ComboBox1->Items->Strings[ComboBox1->ItemIndex]);
			d->final->Values["ScaleId"] = NULL;
			d->final->Values["Measured"] = "1";
			ProgressBar1->Value = CalcMeasured();
		}
		WORD Key;
		String s = SelBox->Text;


		System::WideChar KeyChar = s[s.Length()];
		TShiftState Shift;
		SelBoxKeyUp(Form1->ItemBarcode, Key, KeyChar, Shift);
		GroupBox1->SetFocus();
		if (CalcMeasured() == ListBox1->Count) {
			if (mrYes == MessageDlg("You have now prepared all " + String(ListBox1->Items->Count) +" items. Would you like save your results now?", TMsgDlgType::mtConfirmation,
				TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
				update = true;
				Close();
			}

		}
	}
}
}
//---------------------------------------------------------------------------
int TSampleWn::CalcMeasured() {
int count = 0;
for (int i = ListBox1->Items->Count - 1; i >= 0; i--) {
	NodeData *d = (NodeData*)ListBox1->ItemByIndex(i)->Data;
	if (d->final->Values["Measured"] == "1") {
		count++;
	}
}
return count;
}
//---------------------------------------------------------------------------
void __fastcall TSampleWn::CheckBox2Change(TObject *Sender) {
//todo implement hiding of the measured items
}
//---------------------------------------------------------------------------
void __fastcall TSampleWn::FormKeyDown(TObject *Sender, WORD &Key, System::WideChar &KeyChar, TShiftState Shift) {
if (this->Active) { //probably not required
	//Caption = Caption + Name;
	if (Form1->BarcodeSearch.Pos("|") || KeyChar == '|') {
		Form1->valEditKeyDown(Sender, Key, KeyChar, Shift);
	}
	else {
		NodeData *d = (NodeData*)ListBox1->Selected->Data;
		d->orig->Values["Amount"] = SelBox->Text ;
		d->orig->Values["ItemUnitId"] = Form1->getIdOfTitle(Form1->ItemUnitId, unitlabel->Text);
    }
}
}
//---------------------------------------------------------------------------
void __fastcall TSampleWn::FormKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar, TShiftState Shift) {
if (this->Active) Form1->valEditKeyUp(Sender, Key, KeyChar, Shift);
}
//---------------------------------------------------------------------------
void __fastcall TSampleWn::AutoWeighChange(TObject *Sender) {
if (AutoWeigh->IsChecked) {
	Label2->Text = "Add seed from source, place on scales and then barcode scan the destination item to automatically weigh it";
	SelBox->ReadOnly = true;
}
else {
	Label2->Text = "Add seed and then either barcode scan the destination item to assign the preset amount or click the item in the list and enter a weight value";
	ComboBox1->BeginUpdate();
	ComboBox1->Items->Assign(Form1->ItemUnitId->Items);
	ComboBox1->EndUpdate();
	ComboBox1->ItemIndex = ComboBox1->Items->IndexOf("kernel");
	SelBox->ReadOnly = false;

}
WizardWn->autoweigh = AutoWeigh->IsChecked;
NumberBox1->Visible = !AutoWeigh->IsChecked;
Label6->Visible = !AutoWeigh->IsChecked;
ComboBox1->Visible = !AutoWeigh->IsChecked;
}
//---------------------------------------------------------------------------
void __fastcall TSampleWn::ComboBox1Change(TObject *Sender) {
unitlabel->Text = ComboBox1->Items->Strings[ComboBox1->ItemIndex];
}
//---------------------------------------------------------------------------
void __fastcall TSampleWn::AutoWeighKeyDown(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift)
{
if (Form1->BarcodeSearch.Pos("|") || KeyChar == '|') {
	Form1->valEditKeyDown(Sender, Key, KeyChar, Shift);
}
}
//---------------------------------------------------------------------------

