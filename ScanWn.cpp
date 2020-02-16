//---------------------------------------------------------------------------

#include <fmx.h>
#include "unit1.h"
#pragma hdrstop
#include "ScanWn.h"
#include "Sample.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//#pragma link "Winsoft.FireMonkey.Obr"
#pragma link "Winsoft.FireMonkey.Barcode"
//#pragma link "Winsoft.FireMonkey.Barcode"
//#pragma link "Winsoft.FireMonkey.Obr"
//#pragma link "Winsoft.FireMonkey.Obr"
#pragma resource "*.fmx"
TScan *Scan;
//---------------------------------------------------------------------------
__fastcall TScan::TScan(TComponent* Owner) : TForm(Owner) {
//todo fix intermittent av https://forums.embarcadero.com/thread.jspa?messageID=627621&#627621
}
//---------------------------------------------------------------------------
void __fastcall TScan::CameraComponent1SampleBufferReady(TObject *Sender, const __int64 ATime) {
wait = true;
if (!found) {
	#ifdef _Windows
	//bug in TImage! todo Could try clearing it or streaming it below
	//http://delphihaven.wordpress.com/2012/02/02/copying-and-pasting-a-firemonkey-tbitmap-new-and-improved/
	TBitmap *b = new TBitmap(100,100);
	CameraComponent1->SampleBufferToBitmap(b, true);
	b->SaveToFile("tmp.bmp");
	delete b; b= NULL;
	im1->Bitmap->LoadFromFile("tmp.bmp");
	#else
	CameraComponent1->SampleBufferToBitmap(im1->Bitmap, true);
	#endif

	CameraComponent1->SampleBufferToBitmap(FObr1->Picture, true);
	FObr1->Scan();
}
wait = false;
}
//---------------------------------------------------------------------------
void TScan::Start() {
wait = true;
Timer1->Enabled = true;
FObr1->Active = true;
Edit1->Text = "";
Memo1->Lines->Clear();
CameraComponent1 = new TCameraComponent(this);
CameraComponent1->Kind = Fmx::Media::TCameraKind::ckBackCamera;
CameraComponent1->OnSampleBufferReady = CameraComponent1SampleBufferReady;
CameraComponent1->Active = true;
Form1->Button5->Text = "Stop Scan";
Memo->Lines->Clear();
im1 = new TImage(this);
im1->Align = TAlignLayout::alClient;
im1->Parent = ScrollBox1;
ShowModal();
}
//---------------------------------------------------------------------------
void TScan::Stop() {
if (FObr1->Active) FObr1->Active = false;
CameraComponent1->OnSampleBufferReady = NULL;
StopCamera = true;
Form1->Button5->Text = "Re-scan";
if (im1) {
	delete im1; im1 = NULL;
}
Close();
}
//---------------------------------------------------------------------------
void TScan::Beep(bool error) {
if (!error) {
	if (FileExists("beep 06.mp3")) {
		MediaPlayer1->FileName = "beep 06.mp3";
		MediaPlayer1->Play();
	}
	else ShowMessage("missing 'beep 06.mp3' so can't beep!");
}
else {
	if (FileExists("beep 03.mp3")) {
		MediaPlayer1->FileName = "beep 03.mp3";
		MediaPlayer1->Play();
	}
	else ShowMessage("missing 'beep 03.mp3' so can't beep!");
}
}
//---------------------------------------------------------------------------
void __fastcall TScan::FObr1BarcodeDetected(TObject *Sender) {
TObrSymbol* RecognisedSymbol;
String Line;

for (int i = 0; i < FObr1->BarcodeCount; i++) {
	found = true;
	RecognisedSymbol = FObr1->Barcode[i];
	Line = RecognisedSymbol->SymbologyName + RecognisedSymbol->SymbologyAddonName + " " + RecognisedSymbol->OrientationName;
	Edit1->Text = RecognisedSymbol->Data;
	if (i < Memo->Lines->Count) Memo->Lines->Strings[i] = Line;
	else {
		Memo->Lines->Add(Line);
	}
}
if (found) {
    found = false;
	while (Memo->Lines->Count > FObr1->BarcodeCount) Memo->Lines->Delete(Memo->Lines->Count - 1);
	String symbology = RecognisedSymbol->SymbologyName;
	if (symbology == "QR-Code") { //todo fix the mismatch between the recognised symbol name and the Barcode component
		symbology = "QR Code";
	}
	else if (symbology == "CODE-128") {
		symbology = "Code 128 (Subset B)";
	}
	int matchfound = Form1->ComboBoxSymbology->Items->IndexOf(symbology);

	if (matchfound != -1) {
		Form1->ComboBoxSymbology->ItemIndex = matchfound;
	}
	else {
		Memo1->Lines->Add("No match for " + symbology + " found");
	}
	Form1->Barcode1->InputText = RecognisedSymbol->Data;
	Beep(0);
	Stop();
	//what should we do with the barcode? depends on the use case!
	if (Form1->UseCase == Form1->ItemBarcode) {
		Form1->ComboBoxSymbologyChange(Sender);
		Form1->ShowBarcode(Form1->ItemBarcode);
		Form1->ItemBarcode->Text = Form1->Barcode1->InputText;
		WORD Key;
		System::WideChar KeyChar;
		TShiftState Shift;
		Form1->EdKeyUp(Form1->ItemBarcode, Key, KeyChar, Shift);
	}
	else {
		String findStr = Form1->Barcode1->InputText;
		/*if (!Form1->FindInTree(findStr)) {
			ShowMessage(findStr + " is not in the shortlist. Do you want to create a new item or search the database for this item?");
		}
		else {	//do subsample op by grabbing scale weight
				//create new item using scanner, and set the new item details including amount
				// and subtracting this amount from the original item
		} */
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TScan::Timer1Timer(TObject *Sender) {
if (StopCamera) {
	StopCamera = false;
	Timer1->Enabled = false;
	CameraComponent1->Active = false;
	delete CameraComponent1; //apparently we are not supposed to do this http://docwiki.embarcadero.com/Libraries/XE5/en/FMX.Media.TCaptureDevice
	CameraComponent1 = NULL;
	if (FileExists("tmp.bmp")) {
		DeleteFile("tmp.bmp");
	}
	//repaint controls
	if (Form1->im2) {
		Form1->ConvertImageToBitmap(Form1->im2);
		Form1->im2->Repaint();
	}
	if (im1) {
		delete im1; im1 = NULL;
	}
	Form1->Button5->Repaint();
	Form1->ComboBoxSymbology->Repaint();
	Form1->TreeView1->Repaint();
	Memo1->SetFocus();
	Memo->SetFocus();
}
}
//---------------------------------------------------------------------------
void __fastcall TScan::FormClose(TObject *Sender, TCloseAction &Action) {
while (wait) Application->ProcessMessages();
Form1->Button5->Text = "Re-scan";
StopCamera = true;
}
//---------------------------------------------------------------------------
void TScan::Scale() {
Layout1->Scale->X = Form1->Layout1->Scale->X;
Layout1->Scale->Y = Form1->Layout1->Scale->Y;
ClientHeight = ClientHeight * Layout1->Scale->Y;
ClientWidth = ClientWidth * Layout1->Scale->X;
Left = (Form1->Left) + (Form1->Width - Width)/2;
Top = (Form1->Top) + (Form1->Height - Height)/2;
}
//---------------------------------------------------------------------------

