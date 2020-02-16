//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop
#include "FMX.Printer.hpp"
#include "Print.h"
#include "wizard.h"
#include "unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TPrintWn *PrintWn;
//---------------------------------------------------------------------------
__fastcall TPrintWn::TPrintWn(TComponent* Owner)
	: TForm(Owner) {
labelBox->ItemIndex = 0;
sb[0] = ScrollBox1;
for (int i = 0; i < 5000; i++) {
	im[i] = NULL;
}
for (int i = 1; i < 300; i++) {
	sb[i] = NULL;
}
}
//---------------------------------------------------------------------------
void TPrintWn::createsb(int pageno) {
t[pageno] = new TTabItem(TabControl1);
t[pageno]->Parent = TabControl1;
t[pageno]->Text = "Page " + String(pageno + 1);
sb[pageno] = new TScrollBox(this);
sb[pageno]->Parent = t[pageno];
sb[pageno]->Position->X = sb[0]->Position->X;
sb[pageno]->Position->Y = sb[0]->Position->Y;
sb[pageno]->Width = sb[0]->Width;
sb[pageno]->Height = sb[0]->Height;
//TabControl1->TabIndex = pageno;
}
//---------------------------------------------------------------------------
void __fastcall TPrintWn::ShowPrintPreview(TObject *Sender)  {
CleanUpObjs();
int itemsperpage = ComboBox1->Items->Strings[ComboBox1->ItemIndex].ToIntDef(1) *
	ComboBox2->Items->Strings[ComboBox2->ItemIndex].ToIntDef(1);
int barcodeNo = 0;
for (int pageno = 0; pageno <= ListBox2->Count / itemsperpage; pageno++) {
	int startpos = pageno * itemsperpage;
	if (pageno) {
    	createsb(pageno);
	}
	for (int i = startpos; i < startpos + itemsperpage; i++) {
		if (barcodeNo > ListBox2->Count -1) {
			break;
		}
		TStringList* pl = (TStringList*)ListBox2->ItemByIndex(barcodeNo)->Data;
		String val = pl->Values["ItemBarcode"];
		if (val.IsEmpty()) {
			val = pl->Values["StorageBarcode"];
		}
		Form1->Barcode1->InputText = val;
		if (!Form1->Barcode1->InputText.IsEmpty()) {
			im[i] = new TImage(this);
			im[i]->Parent = sb[pageno];
			int xcount = ComboBox1->Items->Strings[ComboBox1->ItemIndex].ToIntDef(1);
			int xpos = (i - startpos) % xcount;
			int ypos = (i - startpos) / xcount;
			im[i]->Bitmap->Assign(Form1->Barcode1->Bitmap);

			//Form1->ConvertImageToBitmap(im[barcodeNo]);
			int ixsize = sb[pageno]->Width / xcount;    //image size determined by the barcode per width
			int xborder = (Border->Text.ToIntDef(0) * ypos) / 2;
			int yborder = (Border->Text.ToIntDef(0) * xpos) / 2;
			im[i]->Position->X = ixsize * xpos + yborder;  //barcodeNo * im[barcodeNo]->Width;
			im[i]->Position->Y = ixsize * ypos + xborder;
			im[i]->Height = ixsize - Border->Text.ToIntDef(0);
			im[i]->Width = ixsize - Border->Text.ToIntDef(0);

			if (labelBox->Items->Strings[labelBox->ItemIndex] != "None") { //amend a label
				/*
				im[barcodeNo]->Canvas->Font->Size = 15;
				im[barcodeNo]->Canvas->Font->Family   = "Arial";
				TFontStyles fs;
				fs << TFontStyle::fsBold;
				im[barcodeNo]->Canvas->Font->Style  = fs;
				im[barcodeNo]->Canvas->Fill->Color  = claBlack;
				im[barcodeNo]->Canvas->Fill->Kind = TBrushKind::bkSolid;
				int l,t,r,b;
				String s;
				im[barcodeNo]->Canvas->BeginScene();
				s = ComboBox3->Items->Strings[ComboBox3->ItemIndex];
				l = GroupBox2->Position->X + im[barcodeNo]->Position->X;
				t = GroupBox2->Position->Y + im[barcodeNo]->Position->Y + im[barcodeNo]->Height;
				r = l + (im[barcodeNo]->Canvas->TextWidth(s));
				b = t + (im[barcodeNo]->Canvas->TextHeight(s));

				TRectF textRect(l, t, r, b);
				TFillTextFlags tf;
				tf<<TFillTextFlag::ftRightToLeft;
				im[barcodeNo]->Canvas->FillText(textRect, s, true, 255, tf, TTextAlign::taLeading, TTextAlign::taCenter);
				im[barcodeNo]->Canvas->EndScene();
				*/

				la[i] = new TLabel(this);
				la[i]->AutoSize = true;
				la[i]->Parent = sb[pageno];
				la[i]->Scale->X = (ComboBox4->ItemIndex + 1) / 5.0;
				la[i]->Scale->Y = (ComboBox4->ItemIndex + 1) / 5.0;
				la[i]->Visible = true;
				String stype = labelBox->Items->Strings[labelBox->ItemIndex];
				TStringList* pl = (TStringList*)ListBox2->ItemByIndex(i)->Data;
				if (stype == "ItemNote") {

				}
				la[i]->Text =  pl->Values[stype];

				la[i]->Position->X = im[i]->Position->X;// +
					//(im[barcodeNo]->Width * im[barcodeNo]->Scale->X  - la[barcodeNo]->Width)/2;
				la[i]->Position->Y = im[i]->Position->Y +
					(im[i]->Height - Border->Text.ToIntDef(0) / 2) * im[i]->Scale->Y;
			}

		}
		barcodeNo++;
	}
}
}
//---------------------------------------------------------------------------
void TPrintWn::PrintBarCode() {
int l,t,r,b;
String s;
int itemsperpage = ComboBox1->Items->Strings[ComboBox1->ItemIndex].ToIntDef(1) *
	ComboBox2->Items->Strings[ComboBox2->ItemIndex].ToIntDef(1);
int origH = sb[0]->Height;
int origW = sb[0]->Width;
if (PrintDialog1->Execute()) {
	//Set default DPI for the printer. The SelectDPI routine defaults
	//to the closest available resolution as reported by the driver.
	Fmx::Printer::Printer()->ActivePrinter->SelectDPI(1200, 1200);
	Fmx::Printer::Printer()->BeginDoc();
	for (int xx = 0; xx < TabControl1->TabCount; xx++) {
		int startpos = xx * itemsperpage;

		if (xx) {
			Fmx::Printer::Printer()->NewPage();
		}
		//ScrollBox1->Parent = this;
		sb[xx]->Height = Fmx::Printer::Printer()->PageHeight / 10;
		sb[xx]->Width = Fmx::Printer::Printer()->PageWidth / 10;
		Caption = "Printing page " + String(xx + 1) + "...";
		ShowPrintPreview(0);
		TabControl1->TabIndex = xx;
		Application->ProcessMessages();
		//print from bitmap.
		TRectF source(0, 0, sb[xx]->Width, sb[xx]->Height);
		l = 0;
		t = 0;
		r = l + Fmx::Printer::Printer()->PageWidth;
		b = t + Fmx::Printer::Printer()->PageHeight;
		TRectF dest(l, t, r, b);
		TBitmap* ss;
		//without the labels
		for (int i = startpos; i < startpos + itemsperpage; i++) {
			if (la[i]) la[i]->Parent = this;
		}
		ss = sb[xx]->MakeScreenshot();
		Fmx::Printer::Printer()->Canvas->DrawBitmap(ss, source, dest, true);

		if (labelBox->Items->Strings[labelBox->ItemIndex] != "None") {
			Fmx::Printer::Printer()->Canvas->Font->Size = 10;
			Fmx::Printer::Printer()->Canvas->Font->Family   = "Arial";
			TFontStyles fs;
			fs << TFontStyle::fsBold;
			Fmx::Printer::Printer()->Canvas->Font->Style  = fs;
			Fmx::Printer::Printer()->Canvas->Fill->Color  = claBlack;
			Fmx::Printer::Printer()->Canvas->Fill->Kind = TBrushKind::bkSolid;

			for (int i = startpos; i < startpos + itemsperpage; i++) {
				if (la[i]) {
					String s = la[i]->Text;
					int l, t, r, b;
					l = la[i]->Position->X * Fmx::Printer::Printer()->PageWidth/sb[xx]->Width;
					t = la[i]->Position->Y * Fmx::Printer::Printer()->PageHeight/sb[xx]->Height;
					r = l + la[i]->Width * Fmx::Printer::Printer()->PageWidth/sb[xx]->Width;
					b = t + la[i]->Height * Fmx::Printer::Printer()->PageHeight/sb[xx]->Height;
					TRectF textRect(l, t, r, b);
					TFillTextFlags tf;
					tf<<TFillTextFlag::ftRightToLeft;
					Fmx::Printer::Printer()->Canvas->FillText(textRect, s, true, 255, tf, TTextAlign::taLeading, TTextAlign::taCenter);
					/*l = im[i]->Position->X * Fmx::Printer::Printer()->PageWidth/ScrollBox1->Width;
					t = im[i]->Position->Y * Fmx::Printer::Printer()->PageHeight/ScrollBox1->Height;
					r = l + im[i]->Width * Fmx::Printer::Printer()->PageWidth/ScrollBox1->Width;
					b = t + im[i]->Height * Fmx::Printer::Printer()->PageHeight/ScrollBox1->Height;
					TRectF imageRect(l, t, r, b);
					TRectF source(0, 0, im[i]->Width+10, im[i]->Height+10);
					Fmx::Printer::Printer()->Canvas->DrawBitmap(im[i]->Bitmap, source, imageRect, true);
					*/
				}
			}
		}
		/*except
		  on Exception do
		  begin
			//Abort if an error occurs while printing.             7332431038226
			Fmx::Printer::Printer()->Abort;

			if Fmx::Printer::Printer()->Printing then
			   Fmx::Printer::Printer()->EndDoc;

			Raise;
		  end;
		end;
		end;*/
	}
	Fmx::Printer::Printer()->EndDoc();
	Caption = "Printing completed";
}
else {
	Caption = "Printing aborted";
}

sb[0]->Height = origH;
sb[0]->Width = origW;
ShowPrintPreview(0);
ShowMessage(Caption);
}
//---------------------------------------------------------------------------
void TPrintWn::Scale() {
Layout1->Scale->X = Form1->Layout1->Scale->X;
Layout1->Scale->Y = Form1->Layout1->Scale->Y;
ClientHeight = ClientHeight * Layout1->Scale->Y;
ClientWidth = ClientWidth * Layout1->Scale->X;
Left = (Form1->Left) + (Form1->Width - Width)/2;
Top = (Form1->Top) + (Form1->Height - Height)/2;
}
//---------------------------------------------------------------------------
void __fastcall TPrintWn::priButton1Click(TObject *Sender) {
PrintBarCode();
}
//---------------------------------------------------------------------------
void __fastcall TPrintWn::ChooseButClick(TObject *Sender) {
TListBox *b1;
TListBox *b2;
if (Sender == rAllBut || Sender == rOneBut) {
	b2 = ListBox1;
	b1 = ListBox2;
}
else {
	b1 = ListBox1;
	b2 = ListBox2;
}
if (Sender == rAllBut || Sender == aAllBut) {
	b1->SelectAll();
}
Application->ProcessMessages();
b1->BeginUpdate();
b2->BeginUpdate();
for (int i = b1->Items->Count - 1; i >= 0; i--) {
	if (b1->ItemByIndex(i)->IsSelected) {
		TStringList* pl = (TStringList*)b1->ItemByIndex(i)->Data;
		b2->Items->AddObject(b1->ItemByIndex(i)->Text, pl);
		b1->Items->Delete(i);
	}
}
b1->EndUpdate();
b2->EndUpdate();
aOneBut->Enabled = ListBox1->Count;
aAllBut->Enabled = ListBox1->Count;
rOneBut->Enabled = ListBox2->Count;
rAllBut->Enabled = ListBox2->Count;
priButton1->Enabled = ListBox2->Count;
ShowPrintPreview(0);
Label1->Text = "Available: " + String(ListBox1->Count);
Label2->Text = "To print: " + String(ListBox2->Count);
}
//---------------------------------------------------------------------------
void __fastcall TPrintWn::FormClose(TObject *Sender, TCloseAction &Action) {
CleanUpObjs();
ListClear(ListBox1);
ListClear(ListBox2);
Action = TCloseAction::caFree;
}
//---------------------------------------------------------------------------
void TPrintWn::PrintSetup(String type) {
Type = type;
TStringList *BCList = new TStringList();
for (int i = 0; i < Form1->TreeView1->Count; i++) {
	TListBoxItem* t = Form1->TreeView1->ItemByIndex(i);
	if (type == "PrintSBarBut") {//lets print the storage locations for this itemlist
		String stBC = Form1->getTreeStrFromObject(t, "StorageId");
		if (!stBC.IsEmpty()) { //storage has been assigned to the item
			if (BCList->IndexOf(stBC) == -1) { //used in adding source items
				BCList->Add(stBC);
			}
		}
	}
	else {
		NodeData *d = (NodeData*)(t->Data);
		if (WizardWn->tstate == 2) { //HARVEST
			if (t->Visible) {
				TColorButton *but = Form1->GetButton(t);
				if (but->Color == TAlphaColorRec::White) {
					ListBox1->Items->AddObject(t->Text, d->final);
				}
			}
		}
		else { //todo require REPRINT mode for printing of source bags?
			ListBox1->Items->AddObject(t->Text, d->final);
		}
	}
	aOneBut->Enabled = true;
	aAllBut->Enabled = true;
}
if (type == "PrintSBarBut") {
	for (int i = 0; i < BCList->Count; i++) {
		String searchid = BCList->Strings[i];
		int index = Form1->getIndex(Form1->StorageId, searchid);
		TStringList *l = (TStringList*)Form1->StorageId->Items->Objects[index];
		if (i == 0) {
			for (int i = 0; i < l->Count; i++) labelBox->Items->Add(l->Names[i]);
			labelBox->ItemIndex = labelBox->Items->IndexOf("StorageLocation");
		}
		ListBox1->Items->AddObject(l->Values["StorageLocation"], l);
	}
}
else {
	labelBox->Items = Form1->iNameList->Lines;
	labelBox->Items->Insert(0, "None");
	labelBox->ItemIndex = labelBox->Items->IndexOf("ItemNote");
}
delete BCList; BCList = NULL;
Label1->Text = "Available: " + String(ListBox1->Count);
}
//---------------------------------------------------------------------------
void __fastcall TPrintWn::ScrollBox1MouseMove(TObject *Sender, TShiftState Shift, float X, float Y) {
//Caption = IntToStr((int)X) + ":"+ IntToStr((int)Y);
}
//---------------------------------------------------------------------------
void __fastcall TPrintWn::ListBox1Change(TObject *Sender) {
aOneBut->Enabled = (ListBox1->Selected);
}
//---------------------------------------------------------------------------
void __fastcall TPrintWn::ListBox2Change(TObject *Sender) {
rOneBut->Enabled = (ListBox2->Selected);
}
//---------------------------------------------------------------------------
void TPrintWn::CleanUpObjs() {
for (int i = 0; i < 5000; i++) {
	if (im[i]) {
		delete im[i]; im[i] = NULL;
		delete la[i]; la[i] = NULL;
	}
}
for (int i = 1; i < 300; i++) {
	if (sb[i]) {
		delete sb[i]; sb[i] = NULL;
	}
	if (t[i]) {
		delete t[i]; t[i] = NULL;
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TPrintWn::CheckBox1Change(TObject *Sender) {
CheckBox1->Enabled = false;
Application->ProcessMessages();
if (CheckBox1->IsChecked) {
	ListBox1->BeginUpdate();
	ListBox1->Sorted = true;
	ListBox1->EndUpdate();
	int pos =  ListBox1->Items->AddObject("tmp", NULL);  //kludge to get it to repaint
	ListBox1->Items->Delete(pos);
}
else {
	ChooseButClick(rAllBut);
	ListBox1->Sorted = false;
	ListBox1->Clear();
	PrintSetup(Type); //todo improve this as it will reload the entire list

}
CheckBox1->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TPrintWn::FormResize(TObject *Sender) {
TabControl2->Height = ClientHeight/Layout1->Scale->Y - 50;
TabControl2->Width = ClientWidth/Layout1->Scale->X;
for (int pageno = 0; pageno < TabControl1->TabCount; pageno++) {
	sb[pageno]->Width = TabControl2->Width;
	sb[pageno]->Height = TabControl2->Height;
}
ListBox1->Width = (ClientWidth/Layout1->Scale->X - 40 - Panel1->Width) / 2;
Panel1->Position->X = ListBox1->Width + 20;
Panel1->Height = ListBox1->Height;
ListBox2->Width = ListBox1->Width;
ListBox2->Position->X = Panel1->Position->X + Panel1->Width + 10;
ListBox2->Height = ListBox1->Height;
priButton1->Position->X = (ClientWidth /Layout1->Scale->X - priButton1->Width) / 2;
priButton1->Position->Y = TabControl2->Height + 20;
Label2->Position->X = ListBox2->Position->X;
}
//---------------------------------------------------------------------------
void TPrintWn::ListClear(TListBox *lb) {
//not required as these objects are the same as the treeviews
/*lb->BeginUpdate();
for (int r = lb->Count - 1; r >= 0  ; r--) {
	TListBoxItem *t2 = lb->ItemByIndex(r);
	NodeData *d = (NodeData*)t2->Data;
	delete d; d = NULL;
	delete t2; t2 = NULL;
}
lb->EndUpdate();*/
}
