//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "Unit3.h"
#include "Unit1.h"
#include "Login.h"
#include "settings.h"
#include "wizard.h"
#include "main.h"
//always maintain an offline listing of the associated tables (because users may need to add to them when offline)
//this means if they are altered (eg new container types added/removed to/from type table) problems may arise on synchronisation

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm3 *Form3;
//---------------------------------------------------------------------------
__fastcall TForm3::TForm3(TComponent* Owner) : TForm(Owner) {//enable the add button when all the fields are correctly populated
}
//---------------------------------------------------------------------------
void __fastcall TForm3::Button1Click(TObject *Sender) {
//Will need to call DAL to add the field or tag the field as new so it can be added to DAL before updating
if (Button1->Text == "Add new") {
	Button1->Text = "OK";
	Button1->Enabled = false;
	Caption = "Add new " + TagString;
	for (int i = 0; i < Layout1->Children->Count; i++) {
		TEdit *e = dynamic_cast<TEdit*>(Layout1->Children->Items[i]);
		if (e) {
			e->Enabled = true;
			if (e->Name == "Class") {
				e->Enabled = false;
			}
			else e->Text = "";
		}
		else {
			TMemo *m = dynamic_cast<TMemo*>(Layout1->Children->Items[i]);
			if (m) {
				m->Text = "";
				m->Enabled = true;
			}
		}
	}
}
else {
	TStringList *s = new TStringList();
	for (int i = 0; i < Layout1->Children->Count; i++) {
		TEdit *e = dynamic_cast<TEdit*>(Layout1->Children->Items[i]);
		if (e) {
			if (e->Text != "") {
				s->Values[e->Name] = e->Text;
			}
		}
		else {
			TMemo *m = dynamic_cast<TMemo*>(Layout1->Children->Items[i]);
			if (m) {
				if (m->Text != "") {
					if (m->Name == "GenotypeId") {
					//tmp - messy creation of xml
						TMemo* m2 = new TMemo(this);
						//m2->WordWrap = true;
						m2->Lines->Add("<DATA>");
						for (int i = 0; i < m->Lines->Count; i++) {
							String ss;
							ss.sprintf(L"<genotypespecimen GenotypeId=\"%s\"/>", m->Lines->Strings[i].c_str());
							m2->Lines->Add(ss);
						}
						m2->Lines->Add("</DATA>");
						String txt = m2->Text;
						txt = StringReplace(txt, sLineBreak, "", TReplaceFlags() << rfReplaceAll);
						m2->Lines->Clear();
						m2->Lines->Add(txt);
						m2->Lines->SaveToFile(SettingsWn->path + s->Values["SpecimenName"] + ".xml");
						s->Values[m->Name] = m2->Text;
					}
					else {
						s->Values[m->Name] = m->Text;
					}
				}
			}
		}
	}
	int pos = Caption.LastDelimiter(" ");
	String sid = Caption.SubString(pos + 1, Caption.Length() - pos + 1);
	pos = sid.LastDelimiter("=");
	if (pos != 0) {
		sid = sid.SubString(1, pos - 1);
	}

	TComboBox *combo;
	for (int ri = 0; ri < Form1->ItemDetailBox->ChildrenCount; ri++) {
		combo = dynamic_cast<TComboBox*>(Form1->ItemDetailBox->Children->Items[ri]);
		if (combo) {

			if (combo->Name == sid) {
				int index = -1;
				if (!modify) {
					//find lowest -ve value in combo and set the id to one below (can't assume they are sorted)
					String id = Form1->getMapping(combo);
					int lowestval = 0;
					for (int i = 0; i < combo->Count ; i++) {
						String item = NULL;
						if (combo != Form1->ItemOperation) {
							TStringList *l = (TStringList*)combo->Items->Objects[i];
							item = l->Values[id];
							int newlowestval = item.ToIntDef(0);
							if (newlowestval < lowestval) lowestval = newlowestval;
						}
					}
					lowestval--;
					s->Values[Form1->getMapping(combo)] = String(lowestval);//set the id to <0 until it has been commited
					index = combo->Items->AddObject("new - " + s->Values[combo->ListBoxResource], s);
				}
				else {
					modify = false;
					int pos = Caption.LastDelimiter("=");
					String origID = Caption.SubString(pos + 1, Caption.Length() - pos + 1);
					s->Values[Form1->getMapping(combo)] = origID;
					index = Form1->getIndex(combo, origID);
					TStringList *l = (TStringList*)combo->Items->Objects[index];
					delete l;
					combo->Items->Objects[index] = s;
					combo->Items->Strings[index] = "new - " + s->Values[combo->ListBoxResource];
				}
				Application->ProcessMessages();
				combo->ItemIndex = index;
				LoginWn->SaveCombo(combo);   //save this list automatically
				if (!Form1->TreeView1->Count) Form1->AddLocBlankShortList();
				break;
			}
		}
	}
	if (Layout1->Parent != WizardWn->Rectangle5) Close();
}
}
//---------------------------------------------------------------------------
void __fastcall TForm3::MemoKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar, TShiftState Shift) {
String type;
String text;
TEdit *e = dynamic_cast<TEdit*>(Sender);
TMemo *m = dynamic_cast<TMemo*>(Sender);
if (!e) {
	type = m->TagString;
	text = m->Text;
}
else {
	type = e->TagString;
	text = e->Text;
}
//ShowMessage(type);
bool isOK = false;
if (text !="") {
	if (type == "integer") {
		int x = text.ToIntDef(-1);
		if (x != -1) {
			isOK = true;
		}
		else {
			if (e) ShowMessage(e->Name + " must be of type <" + type + ">. Please try again.");
			else isOK = true;// temp for ease of working with xml ShowMessage(m->Name + " must be of type <" + type + ">. Please try again.");
		}
	}
	else if (type == "decimal") {
		try {
			text.ToDouble();
			isOK = true;
		}
		catch(EConvertError *err) {
			ShowMessage(e->Name + " must be of type <" + type + ">. Please try again.");
		}
	}
	else {  //todo implement validation for all types
		isOK = true;
	}

}
if (!isOK) {
	if (!e) {
		m->ClipChildren = false;
	}
	else {
		e->ClipChildren = false;
	}
}
else {
	if (!e) {
		m->ClipChildren = true;
	}
	else {
		e->ClipChildren = true;
	}
}

for (int i = 0; i < Layout1->Children->Count; i++) {
	TEdit *e = dynamic_cast<TEdit*>(Layout1->Children->Items[i]);
	if (e) {
		if (e->Tag) { //it is required
			if (!e->ClipChildren) {
                Button1->Enabled = false;
				return;
			}
		}
	}
}
Button1->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm3::mEnter(TObject *Sender) {
TEdit *e = dynamic_cast<TEdit*>(Sender);
	if (e) {
		Label1->Text = "Please enter a value of type <" + e->TagString + ">";
	}
	else {
		TMemo *m = dynamic_cast<TMemo*>(Sender);
		Label1->Text = "Please enter a value of type <" + m->TagString + ">";
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm3::mLeave(TObject *Sender) {
Label1->Text = "";
}
//---------------------------------------------------------------------------
void TForm3::Scale() {
Layout1->Scale->X = Form1->Layout1->Scale->X;
Layout1->Scale->Y = Form1->Layout1->Scale->Y;
ClientHeight = ClientHeight * Layout1->Scale->Y;
ClientWidth = ClientWidth * Layout1->Scale->X;
Left = (Form1->Left) + (Form1->Width - Width)/2;
Top = (Form1->Top) + (Form1->Height - Height)/2;
}
//---------------------------------------------------------------------------
void __fastcall TForm3::FormResize(TObject *Sender) {
for (int i = 0; i < Layout1->Children->Count; i++) {
	TEdit *e = dynamic_cast<TEdit*>(Layout1->Children->Items[i]);
	if (e) {
		e->Width = ClientWidth/Layout1->Scale->X - 170 ;
	}
	else {
		TMemo *m = dynamic_cast<TMemo*>(Layout1->Children->Items[i]);
		if (m) m->Width = ClientWidth/Layout1->Scale->X - 170 ;
	}
}
}
//---------------------------------------------------------------------------
void __fastcall TForm3::Button2Click(TObject *Sender) {
Button2->Visible = false;
for (int i = 0; i < Layout1->Children->Count; i++) {
	TEdit *e = dynamic_cast<TEdit*>(Layout1->Children->Items[i]);
	if (e) {
		if (e->Name != "Class") {
			e->Enabled = true;
			e->ClipChildren = true;
		}
	}
	else {
		TMemo *m = dynamic_cast<TMemo*>(Layout1->Children->Items[i]);
		if (m) {
			m->Enabled = true;
			m->ClipChildren = true;
			if (m->Name == "GenotypeId") {
				String txt = m->Text;
				m->Lines->Clear();
				int pos = txt.Pos("\"");
				while (pos) {
					m->Lines->Add(LoginWn->ExtractValue("GenotypeId", txt));
					txt = txt.SubString(pos + 1, txt.Length() - pos);
					pos = txt.Pos("\"");
					txt = txt.SubString(pos + 1, txt.Length() - pos);
					pos = txt.Pos("\"");
				}
			}
		}
	}
}
Button1->Text = "OK";
Button1->Enabled = true;
modify = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm3::FormClose(TObject *Sender, TCloseAction &Action) {
Action = TCloseAction::caFree;
}
//---------------------------------------------------------------------------

