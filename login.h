//---------------------------------------------------------------------------

#ifndef loginH
#define loginH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Edit.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdHTTP.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>

#include <Xml.XMLDoc.hpp>
#include <Xml.xmldom.hpp>
#include <Xml.XMLIntf.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdHTTP.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include <IdSSLOpenSSL.hpp>
#include <IdMultipartFormData.hpp>
#include <IdHMACSHA1.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Memo.hpp>
#include <IdIntercept.hpp>
#include <IdLogBase.hpp>
#include <IdLogFile.hpp>

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
using namespace std;
//---------------------------------------------------------------------------
void exch(TStringGrid *a, int i,int j){
TStringColumn *s=(TStringColumn*)a->ColumnByIndex(i);
TStringColumn *s2=(TStringColumn*)a->ColumnByIndex(j);
s->Index = j;
s2->Index = i;
}
//---------------------------------------------------------------------------
int partition(TStringGrid* a,int l,int h){
int i=l;//-1;
int j=h;
String s2 = ((TStringColumn*)a->ColumnByIndex(h))->Header;
String s1 = ((TStringColumn*)a->ColumnByIndex(i))->Header;
while(true){
	while( CompareStr(s1, s2)<0) {
		i++;
		s1 = ((TStringColumn*)a->ColumnByIndex(i))->Header;
	}
	j--;
	s1 = ((TStringColumn*)a->ColumnByIndex(j))->Header;
	while(CompareStr(s1, s2)>0) {

		if (j==i)  break;
		j--;
		s1 = ((TStringColumn*)a->ColumnByIndex(j))->Header;
	}
	if (i>=j) break;
	exch(a,i,j);
}
exch(a,i,h);
return i;
}
//---------------------------------------------------------------------------
void quick(TStringGrid *a,int l,int h){
if (h<=l) return;
int j=partition(a,l,h);
quick(a,l,j-1);
quick(a,j+1,h);
}
//---------------------------------------------------------------------------
void ResizeGrid(TStringGrid *a) {
bool vis = a->Visible;
a->Visible = true;
int idcount = 0;
int totcount = 0;
for (int j = 0; j < a->ColumnCount; j++) {    //iterate over the current search results headers
	TStringColumn* sc = (TStringColumn*)a->ColumnByIndex(j);
	if (sc->Visible) {
		totcount++;
		if (sc->Header.Pos("Id") || sc->Header == "Use") {
			idcount++;
			sc->Width = 38;
		}
	}
}
int remainingwidth = a->Width - (38 * idcount);// - a->ColumnCount + 2;
for (int j = 0; j < a->ColumnCount; j++) {    //iterate over the current search results headers
	TStringColumn* sc = (TStringColumn*)a->ColumnByIndex(j);
	if (sc->Visible) {
		//if (!sc->Header.Pos("Id") && sc->Header != "Use") {
		if (sc->Header.Pos("Id") || sc->Header == "Use") ;
		else	sc->Width = (remainingwidth/* - a->ColumnCount*/) / (totcount - idcount);
		//}
	}
}
a->Visible = vis;
}
//---------------------------------------------------------------------------
int GetCol(String searchtxt, TStringGrid* a) {
bool found = false;
int i;
for (i = 0; i < a->ColumnCount; i++) {
	TStringColumn *col = (TStringColumn*)a->ColumnByIndex(i);
	if (col->Header == searchtxt) {
		found = true;
		break;
	}
}
if (found) {
	return i;
}
else return -1;
}

//---------------------------------------------------------------------------
void DeleteCols(TStringGrid* a) {
for (int i = a->ColumnCount - 1; i >=0  ; i--) {
	//could use MainWn->StringGrid1->Children to identify child component buttons and delete these first but is there any memory leak?
	delete a->ColumnByIndex(i);
	a->ColumnByIndex(i)->Free();
}
a->RowCount=0;
}
//---------------------------------------------------------------------------
class TLoginWn : public TForm
{
__published:	// IDE-managed Components
	TEdit *UserName;
	TEdit *Edit2;
	TEdit *Edit3;
	TButton *LoginBut;
	TIdHTTP *IdHTTP1;
	TMemo *Memo1;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TAniIndicator *AniIndicator1;
	TButton *LogoutBut;
	TLabel *Label4;
	TIdLogFile *IdLogFile1;
	TCheckBox *errChk;
	TLabel *UserID;
	TMemo *Memo;
	TLayout *Layout1;
	TComboEdit *DALBase;
	TGroupBox *GroupBox1;
	void __fastcall LoginButClick(TObject *Sender);
	void __fastcall LogoutButClick(TObject *Sender);
	void __fastcall IdHTTP1Connected(TObject *Sender);
	void __fastcall DALBaseKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar,
          TShiftState Shift);


private:	// User declarations
String writetoken;
String hash_hmac_SHA1(String AData, String AKey);
TIdSSLIOHandlerSocketOpenSSL* lIOHandler;
TXMLDocument *XMLDoc;

public:		// User declarations
void Scale();
void SaveAddButton(TButton *b);
void LoadAddButton(TButton *b);
void LoadCombo(TComboBox *combo);
void SaveCombo(TComboBox *combo);
TButton* getButton(TComboBox *combo);
void GetAddButtonDAL(TComboBox *combo);
void GetCombosFromDAL(String tablename, String query);
void GetFastCombosFromDAL();
void FillTable(String s);
void WriteToLog(AnsiString message);
String getSignature(String url, String randomNumber, String atomicData, String uploadFileMd5);
TStringList *data;
TIdMultiPartFormDataStream* data2;
String concatPString, concatVString;
bool DoQuery(String &PString, bool get);
string url_encode(const string &value);
String logFileName;
bool DoUpdate(TStringList *uplist, TStringList *l, String &s);
bool DoValidation(String validationDate, String iid);
String ExtractValue(String findStr, String otv);
	__fastcall TLoginWn(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TLoginWn *LoginWn;
//---------------------------------------------------------------------------
#endif
