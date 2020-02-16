//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop
#include "settings.h"
#include "unit1.h"
#include "unit3.h"
#include "Unit4.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm4 *Form4;
//---------------------------------------------------------------------------
__fastcall TForm4::TForm4(TComponent* Owner)
	: TForm(Owner)
{
sgI = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm4::Button2Click(TObject *Sender) {
Form1->OpenDialog1->Title = "Select the csv file to be imported";
Form1->OpenDialog1->InitialDir = SettingsWn->path;
//Form1->OpenDialog1->FileName = SettingsWn->path + "1021x.csv";
if (Form1->OpenDialog1->Execute()) {
	TMemo *m = new TMemo(this);
	m->Lines->LoadFromFile(Form1->OpenDialog1->FileName);
	sgI = new TStringGrid(this);
	sgI->RowCount = m->Lines->Count - 1;

	ProgressBar1->Max = m->Lines->Count - 1;
	int row = 0;
	for (int j = 0; j < m->Lines->Count; j++) {
		String dataS = m->Lines->Strings[j];
		int p = dataS.Pos(",");
		bool success = false;
		if (j > 23) {
			sgI->Cells[0][row] = String(row + 1);
		}
		int i = 0;
		while (p >= 1 ) {
			int p2 = dataS.Pos(",\"");
			if (p == p2) {  //special case extract has " " surrounding
				i++;
				int p3 = dataS.SubString(p + 1, dataS.Length() - p).Pos("\",");
				if (!p3) { //found erroneous data traversing two lines so stop loop (ignore data)
					//p = 1;
					//dataS = "a";
					//improve import by getting next line
					sgI->Cells[i][row] = dataS;
					j++;
					dataS = m->Lines->Strings[j];
					p = 0;
				}
				else {
					p = p3 + 1;
					sgI->Cells[i][row] = dataS.SubString(2 , p - 1);
					success = true;
					i--;
				}
			}
			else {
				String extract = dataS.SubString(0 , p - 1);
				if (!extract.IsEmpty()) {
					if (sgI->ColumnCount <= i) {
						TStringColumn* sc = new TStringColumn(sgI);
						if (sgI->ColumnCount == 0) sc->Header = "Row No";
						else sc->Header = extract;
						sc->Parent = sgI;
					}
					else {
						if (extract == "/\"" || extract == "\"") {  //found erroneous data traversing two lines so stop loop (ignore data)
							dataS = dataS.SubString(extract.Length()+2, dataS.Length() - extract.Length());
                            p = dataS.Pos(",");
							//p = 1;
							//dataS = "a";
						}
						else {
							sgI->Cells[i][row] = extract;
							success = true;
						}
					}

				}
			}
			i++;
			dataS = dataS.SubString(p + 1, dataS.Length() - p);
			p = dataS.Pos(",");
		}
		if (j == 1) { //skip a sub heading row
			j = 23;
		}
		if (success) {
			row++;
			if (row%40==0) {
				ProgressBar1->Value = row;
				Application->ProcessMessages();
			}
		}
	}
	sgI->RowCount = row;
	sgI->Parent = Layout1;
	sgI->Align = TAlignLayout::alClient;
	Button1->Enabled = true;
	delete m; m = NULL;
	ProgressBar1->Value = 0;
}
}
//---------------------------------------------------------------------------
void __fastcall TForm4::Button1Click(TObject *Sender) {
if (!Form1->SpecimenId->Items->Count) {
	ShowMessage("You must have at least 1 Specimen in your metadata to continue. Refresh metadata to continue");
	return;
}
//TTreeViewItem *tNode = Form1->AddLocBlankShortList();
//Form1->TreeView1->Selected = tNode;
if (!Form1->TreeView1->Selected) {
	ShowMessage("Select an item to continue");
	return;
}
int startRow = Edit1->Text.ToIntDef(1) - 1;
int endRow = Edit2->Text.ToIntDef(20) - 1;
TStringList *s = (TStringList*)Form1->SpecimenId->Items->Objects[0];

for (int i = startRow; i <= endRow; i++) {
	TListBoxItem *n = Form1->TreeView1->Selected;
	Form1->AddClick(Sender);
	Form1->ItemNote->Text = sgI->Cells[2][i];
	Form1->EdChange(Form1->ItemNote);
	Form1->ItemBarcode->Text = sgI->Cells[1][i];
	Form1->EdChange(Form1->ItemBarcode);
	Form1->ItemNote->Text = "Item " + sgI->Cells[0][i] + " imported from " + Form1->OpenDialog1->FileName;
	Form1->EdChange(Form1->ItemNote);
	Form1->ItemTypeId->ItemIndex = 0;
	Form1->EdChange(Form1->ItemTypeId);
	Form1->ignore = true;
	Form1->AddFieldButClick(Form1->specimen); //create new specimen dialog
    Form1->ignore = false;
	for (int j = 0; j < Form3->Layout1->Children->Count; j++) {
		TEdit *e = dynamic_cast<TEdit*>(Form3->Layout1->Children->Items[j]);
		if (e) {
			if (e->Name == "BreedingMethodId") {
				e->Text = 0;
			}
			else if (e->Name == "SpecimenName") {
				e->Text = "GID" + String(sgI->Cells[9][i].ToIntDef(0)*-1);
			}
			else if (e->Name == "Pedigree") {
				e->Text = StringReplace(sgI->Cells[3][i],"\"","", TReplaceFlags() << rfReplaceAll);
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
					m->Text = s->Values["GenotypeId"];
				}
			}
		}
	}
	Form3->Button1Click(0);
	Form1->EdChange(Form1->SpecimenId);
	delete Form3;
	Form3 = NULL;
}
ShowMessage("Rows " + String(startRow) + " to " + String(endRow) + " have been added to your shortlist");
}
//---------------------------------------------------------------------------
void __fastcall TForm4::FormClose(TObject *Sender, TCloseAction &Action) {
if (sgI) {
	delete sgI;
	sgI = NULL;
}
}
//---------------------------------------------------------------------------
void TForm4::Scale() {
Layout1->Scale->X = Form1->Layout1->Scale->X;
Layout1->Scale->Y = Form1->Layout1->Scale->Y;
ClientHeight = ClientHeight * Layout1->Scale->Y;
ClientWidth = ClientWidth * Layout1->Scale->X;
Left = (Form1->Left) + (Form1->Width - Width)/2;
Top = (Form1->Top) + (Form1->Height - Height)/2;
}
//---------------------------------------------------------------------------

