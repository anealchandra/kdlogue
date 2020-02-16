//---------------------------------------------------------------------------
#include <fmx.h>
#pragma hdrstop
#include "login.h"
#include "settings.h"
#include "main.h"
#include "unit1.h"
#include <Classes.hpp>
#include <IdGlobal.hpp>
#include "md5.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TLoginWn *LoginWn;
//---------------------------------------------------------------------------
__fastcall TLoginWn::TLoginWn(TComponent* Owner)
	: TForm(Owner)
{//pointer to Brians v. http://192.168.9.193:40112/dal/
UserName->Text = "admin";
Edit2->Text = "kdd@rt";
Edit3->Text = "0";//0
XMLDoc = new TXMLDocument(this);
#ifdef _DEBUG
//DALBase->Text = "http://kddart-t.diversityarrays.com/dal/";
#endif
logFileName = GetHomePath() + (char)System::Ioutils::TPath::DirectorySeparatorChar + "log.txt"; //todo move this into kdlog directory (remember to forcedirectory)
//http://kddart-t.diversityarrays.com/dal/
}
//---------------------------------------------------------------------------
void __fastcall TLoginWn::LoginButClick(TObject *Sender)
{
UserID->Text = "";
/*for (int i = 0; i < DALBase->Items->C; i++) {

} */
LoginBut->Enabled = false;
GroupBox1->Enabled = false;
AniIndicator1->Visible = true;
Application->ProcessMessages();
data = new TStringList();
data2 = new TIdMultiPartFormDataStream();
lIOHandler = new TIdSSLIOHandlerSocketOpenSSL();
if (XMLDoc) {
	delete XMLDoc; XMLDoc = NULL;
}
XMLDoc = new TXMLDocument(this);
Memo1->Lines->Clear();

//1. Get the HMACSHA1 hash of the users login name and password
Memo1->Lines->Add(hash_hmac_SHA1(UserName->Text, Edit2->Text));
//2. Create a string of random data
Randomize();
String randStr = String(Random(MaxInt));

//3. Create a string containing the DAL URL you are connecting to
String url = DALBase->Text + "login/" + UserName->Text + "/yes";  //no = dont send extra data;

//4. Get the HMACSHA1 hash of the Random string from step 2 and the hash from step 1
Memo1->Lines->Add(hash_hmac_SHA1(randStr, Memo1->Lines->Strings[0].LowerCase()));

//5. Get the HMACSHA1 hash of the DAL URL from step 3 and the hash from step 4
Memo1->Lines->Add(hash_hmac_SHA1(url, Memo1->Lines->Strings[1].LowerCase()));

/*6. The make a http post request to kddart with the 'form values' to login
	"rand_num",<String from step 1>
	"url",<URL from step 3>
	"signature",<signature hash from step 5>*/
data->Values["rand_num"]=randStr;
data->Values["url"] = url;
data->Values["signature"] = Memo1->Lines->Strings[2].LowerCase();
IdHTTP1->Request->ContentType="application/x-www-form-urlencoded; charset=UTF-8";

String findStr = "WriteToken Value";
String findStr2 = "UserId";
bool loggedIn = false;
String otv = "error";
while (!loggedIn) {
	try  {
		IdHTTP1->IOHandler = lIOHandler;
		otv = IdHTTP1->Post(url, data);
		int pos = otv.Pos(findStr);
		otv = otv.SubString(pos + findStr.Length() + 2, otv.Length() - pos);
		int endpos = otv.Pos("\"");
		writetoken = otv.SubString(0, endpos - 1); //keep the writetoken for updating via dal
		pos = otv.Pos(findStr2);
		otv = otv.SubString(pos + findStr2.Length() + 2, otv.Length() - pos);
		endpos = otv.Pos("\"");
		UserID->Text = otv.SubString(0, endpos - 1);
		loggedIn = true;
	}
	catch (const Exception &e) {     //// use E.ErrorCode, E.Message, and E.ErrorMessage as needed...
		String err = "Error logging in: " + e.Message;
		WriteToLog(err);
		if (mrYes != MessageDlg("Database has reported '" + err + "' Try logging in again?", TMsgDlgType::mtError,
			TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
			//loggedIn = true;
			AniIndicator1->Visible = false;
			if (Form1) {
				LoginBut->Enabled = true;
				GroupBox1->Enabled = true;
				Form1->SaveLoggedStatus();
				return; //needs more testing
			}
			else {
				exit(0);
			}

        }
	}
}
WriteToLog("Logged in to " + DALBase->Text + " as " + UserName->Text);
AniIndicator1->Visible = false;
LogoutBut->Enabled = true;
//7. set the group

String s = IdHTTP1->Post(DALBase->Text + "switch/group/" + Edit3->Text, data);

//8. get the list of functions and create a treeview showing operations
if (!Form1) {//dont need to do this for KDLog
	String s = "list/operation";
	bool success = DoQuery(s, true);

	if (!success) {
		return;
	}
	FillTable(s);
}
else {
	Form1->SetUpLoggedInWindow();
}
Close();
}
//---------------------------------------------------------------------------
String TLoginWn::hash_hmac_SHA1(String AData, String AKey) {  //still working with 69 chars max
TIdHMACSHA1 *HMACSHA1;
_di_IIdTextEncoding DesiredTextEncoding = NULL;
TIdBytes KeyBytes;
UTF8String akey = AKey;
KeyBytes = ToBytes(akey, DesiredTextEncoding);
//KeyBytes.Length = AKey.Length() / 2;
//KeyBytes.Length = HexToBin(AKey.c_str(), &KeyBytes[0], KeyBytes.Length);
UTF8String adata = AData;


	TIdBytes DataBytes = ToBytes( adata, DesiredTextEncoding);

//TIdBytes DataBytes;
TIdBytes ResBytes;

HMACSHA1 = new TIdHMACSHA1();
try {
	//UTF8String akey = AKey;
	HMACSHA1->Key =KeyBytes;
	//UTF8String adata = AData;
	//DataBytes = ToBytes(AData);
	ResBytes = HMACSHA1->HashValue(DataBytes);
}
catch (const Exception& E) {
ShowMessage("hmac error");
}
delete HMACSHA1;
return(ToHex(ResBytes));
}

//---------------------------------------------------------------------------
String TLoginWn::getSignature(String url, String randomNumber, String atomicData, String uploadFileMd5) {
	String wholeString2Sign = "";
	wholeString2Sign = url + randomNumber + atomicData + uploadFileMd5;
	String hmacString = hash_hmac_SHA1(wholeString2Sign, writetoken);
	return hmacString.LowerCase();
}
//---------------------------------------------------------------------------
string TLoginWn::url_encode(const string &value) {//not used
	ostringstream escaped;
	escaped.fill('0');
	escaped << hex;

	for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		string::value_type c = (*i);
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
		}
		else if (c == ' ')  {
			escaped << '+';
		}
		else {
			escaped << '%' << setw(2) << ((int) c) << setw(0);
		}
	}
	return escaped.str();
}
//---------------------------------------------------------------------------
void TLoginWn::WriteToLog(AnsiString message)  {
TDateTime d = Now();

AnsiString Stime = d.DateString() +  " " + d.TimeString() + " ";
//Stime.sprintf("%02d-%02d-%04d %02d:%02d:%02d ", d. wDay, d.wMonth, d.wYear, d.wHour, d.wMinute, d.wSecond);
AnsiString username = "Apple_user";
wchar_t rqstr [21];
ULONG rqstrLngth = 20;
#ifndef __APPLE__
if (GetUserName(rqstr , (ULONG *)&rqstrLngth)) username = AnsiString(rqstr);
#endif
Stime = Stime + username + " " + message;

fstream fs(AnsiString(logFileName).c_str(), ios::app| ios::out);
if (fs.is_open()) {
	// known bug with codeguard http://stackoverflow.com/questions/8629384/c-ostream-error-when-passing-argument-in-cbuilder-2010
	fs<<Stime.c_str()<<endl;
}
fs.close();

if (Form1) {
	int id = Form1->log->Items->Add(Stime);
	//scroll to last item
	if (Form1->logTabSeen) { //kludge for getting the log to scroll on startup
		Form1->log->ItemIndex = id;
	}
}
}
//---------------------------------------------------------------------------
bool TLoginWn::DoValidation(String offlineDateStr, String iid) {
//need to do a DAL validation search before updating to ensure this data hasn't been changed by another user
//convert this to a datetime object
TFormatSettings fmt;
fmt.ShortDateFormat= "yyyy-MM-dd";
fmt.DateSeparator  = '-';
fmt.LongTimeFormat = "HH:mm:ss";
fmt.TimeSeparator  = ':';


String s = "list/item/1/page/1?Filtering=ItemId=" + iid;
bool success = DoQuery(s, true);  //read ahead
if (!success) {
	Form1->Label1->Text = s;
	return false;
}
FillTable(s);
String onlineDateStr = MainWn->StringGrid1->Cells[GetCol("LastMeasuredDate", MainWn->StringGrid1)][1];

if (!onlineDateStr.IsEmpty() && onlineDateStr != "(empty)" && onlineDateStr != "0000-00-00 00:00:00") {  //dont need to proceed any further, since the online copy has never been measured
	TDateTime onlineDate = StrToDateTime(onlineDateStr, fmt);
	if (offlineDateStr != "0000-00-00 00:00:00" && !offlineDateStr.IsEmpty() && offlineDateStr != "(empty)") {
		TDateTime offlineDate = StrToDateTime(offlineDateStr, fmt);
		if (offlineDate < onlineDate) {  //db date needs to be less than our date
			String onlineUser = MainWn->StringGrid1->Cells[GetCol("LastMeasuredUserId", MainWn->StringGrid1)][1];
			String amount = MainWn->StringGrid1->Cells[GetCol("Amount", MainWn->StringGrid1)][1];
			if (mrYes != MessageDlg("Database reports that the item " + iid + " has already been updated [by UserID: "
				+ onlineUser + " at " + onlineDate + " to an amount of " + amount +
				"]. Would you like to overwrite this value?", TMsgDlgType::mtWarning,
				TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
					return false;
			}
		}
	}
	else { //should never happen!
		String onlineUser = MainWn->StringGrid1->Cells[GetCol("LastMeasuredUserId", MainWn->StringGrid1)][1];
		if (mrYes != MessageDlg("The item " + iid + " has already been measured [by UserID: " + onlineUser + " at " + onlineDate +
			"]. Do you wish to remove this measurement user and date?", TMsgDlgType::mtWarning,
			TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo << TMsgDlgBtn::mbCancel, 0)) {
				return false;
		}
	}
}

return true;
}
//---------------------------------------------------------------------------
bool TLoginWn::DoUpdate(TStringList *uplist, TStringList *l, String &s) {
//apply the updated object to the dal
data->Clear();
data2->Clear();
String randStr = String(Random(MaxInt));
int fileupload = uplist->IndexOf("GenotypeId"); //temp - simple determination of whether upload file is required based on genotypeid

if (fileupload != -1) { //upload file
	IdHTTP1->Request->ContentType = "multipart/form-data;boundary=" + randStr;
	TIdFormDataField* d = data2->AddFormField("rand_num", randStr, "ISO-8859-1");
	d->ContentTransfer = "8bit";
}
else {//no upload file
	IdHTTP1->Request->ContentType = "";
	data->Values["rand_num"] = randStr;
}

concatPString = "";//order of names
concatVString = "";//order of values
String filePath = "";
for (int i = 0; i < uplist->Count; i++) {
	if (i) {
		concatPString += String(",");
	}
	String name = uplist->Strings[i];
	String value = l->Values[name];
	if (value.IsEmpty()) {
		ShowMessage("Error finding field " + name);
		return false;
	}
	if (fileupload != -1) {
		if (name == "GenotypeId") {
			filePath = SettingsWn->path + l->Values["SpecimenName"] + ".xml";
			if (FileExists(filePath)) {
				data2->AddFile("uploadfile", filePath, "text/xml");
			}
			else {
				ShowMessage("Error: no xml file exists");
			}
			//concatPString += name;
			//concatVString += value;
		}
		else {
			concatPString += name;
			concatVString += value;
			TIdFormDataField* d = data2->AddFormField(name, value);
			d->ContentTransfer = "8bit";
		}
	}
	else {
		concatPString += name;
		concatVString += value;
		data->Values[name] = value;
	}
}


if (fileupload != -1) {
	std::ifstream t(AnsiString(filePath).c_str());  //todo verify why can't add specimen in APPLE
	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	std::string fileContent(size, ' ');
	t.seekg(0);
	t.read(&fileContent[0], size);
	//string fileContent = "<DATA><genotypespecimen GenotypeId=\"1792\"/></DATA>";
	String sig = getSignature(DALBase->Text + s, randStr, concatVString, String(md5(fileContent).c_str()));
	TIdFormDataField* d = data2->AddFormField("param_order",concatPString, "ISO-8859-1");
	d->ContentTransfer = "8bit";
	d = data2->AddFormField("url", DALBase->Text + s, "ISO-8859-1");
	d->ContentTransfer = "8bit";
	d = data2->AddFormField("signature", sig, "ISO-8859-1");
	d->ContentTransfer = "8bit";
}
else {
	String sig = getSignature(DALBase->Text + s, randStr, concatVString, String(""));
	data->Values["url"] = DALBase->Text + s;
	data->Values["param_order"] = concatPString;
	data->Values["signature"] = sig;
}
bool success = DoQuery(s, false);
if (fileupload != -1) {
	data2->Clear();
	if (success) {
		DeleteFile(filePath);   //todo create file in this function, making this statement redundant
	}
}
if (!success) {
	Form1->Label1->Text = s;
	return false;
}

return true;
}
//---------------------------------------------------------------------------
String TLoginWn::ExtractValue(String findStr, String otv) {
int pos = otv.Pos(findStr);
otv = otv.SubString(pos + findStr.Length() + 2, otv.Length() - pos);
int endpos = otv.Pos("\"");
return otv.SubString(0, endpos - 1);
}
//---------------------------------------------------------------------------
bool TLoginWn::DoQuery(String &PString, bool get) {//call a DAL operation
//BUG should do this in a new thread to stop it locking up GUI
String functionCall = DALBase->Text + PString;
if (functionCall.Pos("export_samplemeasurement")) {
    functionCall = PString;
}
MainWn->Caption = "crDAL Tester v0.2.5 - " + functionCall;
WriteToLog(PString);

try {
	if (!get) {

		if (data->Count==0) {
			PString = IdHTTP1->Post(/*TIdURI::URLEncode(*/functionCall/*)*/, data2);
		}
		else PString = IdHTTP1->Post(/*TIdURI::URLEncode(*/functionCall/*)*/, data);
	}
	else {
		PString = IdHTTP1->Get(/*TIdURI::URLEncode(*/functionCall/*)*/);
	}
}
catch (const EIdHTTPProtocolException& E) {     //// use E.ErrorCode, E.Message, and E.ErrorMessage as needed...
	String otv = E.ErrorMessage;
	String findStr = "Error Message";
	int pos = otv.Pos(findStr);
	int endpos = 1;
	if (pos) {
		otv = otv.SubString(pos + findStr.Length() + 2, otv.Length() - pos);
		endpos = otv.Pos("\"");
		PString = "DAL Error: " + otv.SubString(0, endpos - 1);
	}
	else {
		findStr = "Error";
		pos = otv.Pos(findStr);
		if (pos) {
			otv = otv.SubString(pos + findStr.Length() + 1, otv.Length() - pos);
			endpos = otv.Pos("DATA");
			if (!endpos) {
				pos = E.ErrorMessage.Pos("<h1>");
				endpos = E.ErrorMessage.Pos("</h1>");
				PString = "DAL Error: " + E.ErrorMessage.SubString(pos + 4, endpos - pos - 4);
			}
			else PString = "DAL Error: " + otv.SubString(0, endpos - 7);
		}
		else {
			findStr = "DATA";
			pos = otv.Pos(findStr);
			if (!pos) {//still can't decifer error so just show it all
				PString = E.Message;
			}
			else {
				otv = otv.SubString(pos + findStr.Length() + 2, otv.Length() -  pos );
				pos = otv.Pos("\"");
				otv = otv.SubString(pos + 1, otv.Length() -  pos );
				endpos = otv.Pos("\"");
				PString = "Internal DAL Server Error: " + otv.SubString(0, endpos - 1);
			}
		}
	}
	if (errChk->IsChecked) ShowMessage(PString);
	WriteToLog(PString);
	return false;
}
catch (const Exception& E/*const EIdHTTPProtocolException& E*/) {
	PString = E.Message;
	if (errChk->IsChecked) ShowMessage(PString);
	WriteToLog(PString);
	return false;
}
return true;
}

//---------------------------------------------------------------------------
void TLoginWn::FillTable(String s) {
XMLDoc->XML->Clear();
XMLDoc->XML->Add(s);

DeleteCols(MainWn->StringGrid1);
MainWn->StringGrid1->RowCount = 0;
MainWn->rowcount=0;
MainWn->Memo1->Lines->Clear();
MainWn->Memo1->Lines->Add(s);
//MainWn->Visible = true;
IXMLNode* iNode;
XMLDoc->Active = true;
String s1 = XMLDoc->DocumentElement->NodeName;

TTreeViewItem *t = new TTreeViewItem(this);
t->Text = s1;
/*if (!MainWn->TreeView1->Selected) {
	t->Parent = MainWn->TreeView1;
}
else {
	t->Parent = MainWn->TreeView1->Selected;
} */
//add xml to table

MainWn->addTreeNode(XMLDoc->DocumentElement, t);


int n = MainWn->StringGrid1->ColumnCount;
//sort alphabetically
quick(MainWn->StringGrid1,0,n-1);
//force a repaint
MainWn->Width++;
MainWn->Width--;
t->Expand();
}
//---------------------------------------------------------------------------
void __fastcall TLoginWn::LogoutButClick(TObject *Sender)
{
String s = "logout";
DoQuery(s, true);
//reset for logout
Memo1->Lines->Clear();
delete data;
delete lIOHandler;
//delete XMLDoc;
LoginBut->Enabled = true;
LogoutBut->Enabled = false;
GroupBox1->Enabled = true;
if (Form1) {
	Form1->SetUpLoggedOutWindow();
}
}
//---------------------------------------------------------------------------
void __fastcall TLoginWn::IdHTTP1Connected(TObject *Sender) //not required?
{
TIdHTTP *b = reinterpret_cast<TIdHTTP*>(Sender);
b->IOHandler->DefStringEncoding=CharsetToEncoding("UTF-8");
}
//---------------------------------------------------------------------------
void TLoginWn::SaveCombo(TComboBox *combo) {
if (combo == Form1->ItemOperation) {
	return;
}
Memo->Lines->Clear();
for (int k = 0; k < combo->Count; k++) {
	TStringList *l = (TStringList*)(combo->Items->Objects[k]);
	String csv = "";
	for (int i = 0; i < l->Count; i++) {
		csv += l->Strings[i] + ",";
	}
	Memo->Lines->Add(csv);
}
String index = combo->Name;
Memo->Lines->SaveToFile(Form1->updateFile +  "." + index);
}

//---------------------------------------------------------------------------
void TLoginWn::SaveAddButton(TButton *b) {
Memo->Lines->Clear();

TStringGrid *sg = (TStringGrid*)(b->TagObject);
String csv = "";
if (!sg) {
	ShowMessage("add button definition error");
}
for (int i = 0; i < sg->ColumnCount; i++) {
	csv += ((TStringColumn*)sg->ColumnByIndex(i))->Header + ",";
}
Memo->Lines->Add(csv);

for (int i = 0; i < sg->RowCount; i++) {
	String csv = "";
	for (int j = 0; j < sg->ColumnCount ; j++) {
		csv += sg->Cells[j][i] + ",";
	}
	Memo->Lines->Add(csv);
}
String index = b->Name;
Memo->Lines->SaveToFile(Form1->updateFile +  "." + index);
}
//---------------------------------------------------------------------------
void TLoginWn::LoadAddButton(TButton *b) {
String index = b->Name;
if (!FileExists(Form1->updateFile + "." + index)) {
	ShowMessage("Table data corrupted. Button file missing at " + Form1->updateFile + "." + index);
	return;
}
Memo->Lines->LoadFromFile(Form1->updateFile + "." + index);
TStringGrid *sg = new TStringGrid(this);
sg->RowCount = Memo->Lines->Count - 1;

for (int j = 0; j < Memo->Lines->Count; j++) {
	String dataS = Memo->Lines->Strings[j];
	//convert the csv into a stringlist data object
	int p = dataS.LastDelimiter(",");
	dataS = dataS.SubString(0, p - 1);

	int i = 0;
	while (p > 1) {
		p = dataS.LastDelimiter(",");
		if (sg->ColumnCount <= i) {
			TStringColumn* sc = new TStringColumn(sg);
			sc->Header = dataS.SubString(p + 1 , dataS.Length() - p);
			sc->Parent = sg;
		}
		else {
			sg->Cells[i][j - 1] = dataS.SubString(p + 1 , dataS.Length() - p);
		}
		i++;
		dataS = dataS.SubString(0, p - 1);
	}
}
sg->Visible = false;
b->TagObject = sg;
}
//---------------------------------------------------------------------------
void TLoginWn::LoadCombo(TComboBox *combo) {
String index = combo->Name;
if (!FileExists(Form1->updateFile + "." + index)) {
	WriteToLog("Warning: " + index + " item metadata missing - " + Form1->updateFile + "." + index);
	return;
	//todo make it contact DAL for the missing table data
	/*ClearShortButClick(Sender);
	ProgressBar1->Value = 0;
	AniIndicator1->Visible = false;
	*/
}
//start here
//https://forums.embarcadero.com/thread.jspa?messageID=554159&#554159
//https://forums.embarcadero.com/thread.jspa?messageID=621524&#621524
/*if (index == "SpecimenId") {
	Memo->Lines->LoadFromFile(Form1->updateFile + "." + index);

	combo->Items->Add(Memo->Lines->Strings[0]);
	TStringGrid *c = new TStringGrid(this);
	c->BeginUpdate();
	c->Items->Assign(Memo->Lines);

}
else {
*/
Memo->Lines->LoadFromFile(Form1->updateFile + "." + index);
combo->BeginUpdate();
for (int j = 0; j < Memo->Lines->Count; j++) {
	TStringList *l = new TStringList();
	String dataS = Memo->Lines->Strings[j];
	//convert the csv into a stringlist data object
	int p = dataS.LastDelimiter(",");
	dataS = dataS.SubString(0, p - 1);
	while (p > 1) {
		p = dataS.LastDelimiter(",");
		String d = dataS.SubString(p + 1, dataS.Length());
		l->Add(d);
		dataS = dataS.SubString(0, p - 1);
	}
	String id = Form1->getMapping(combo);
	String item = l->Values[id];
    String disptxt;
	if (item.ToIntDef(0) < 0) {
		disptxt = "new - " + l->Values[combo->ListBoxResource];
	}
	else disptxt = l->Values[combo->ListBoxResource];
	combo->Items->AddObject(disptxt, l);
}
combo->EndUpdate();
//}
}
//---------------------------------------------------------------------------
void TLoginWn::GetFastCombosFromDAL() {
GetCombosFromDAL("ContainerTypeId", "");
GetCombosFromDAL("ItemTypeId", "");
//GetCombosFromDAL("StorageId", ""); //not required for harvesting (yet!)
GetCombosFromDAL("ItemStateId", "");
GetCombosFromDAL("ItemUnitId", "");
GetCombosFromDAL("ScaleId", "");
}
//---------------------------------------------------------------------------
void TLoginWn::GetCombosFromDAL(String tablename, String query) {
TStringList *functionCalls = new TStringList();
if (tablename == "ContainerTypeId") {
	functionCalls->Values[tablename] = "list/type/container/active";
}
else if (tablename == "ItemTypeId") {
		functionCalls->Values[tablename] = "list/type/item/active";
}
else if (tablename == "StorageId") {
		functionCalls->Values[tablename] = "list/storage";
}
else if (tablename == "ItemStateId") {
		functionCalls->Values[tablename] = "list/type/state/active";
}
else if (tablename == "ItemUnitId") {
		functionCalls->Values[tablename] = "list/itemunit";
}
else if (tablename == "ScaleId") {
		functionCalls->Values[tablename] = "list/deviceregistration";
}
else if (tablename == "SpecimenId") { //todo use getmapping here!
	functionCalls->Values[tablename] = "list/specimen/10000/page/1?Filtering=" + tablename + "+IN+" + query;        //SpecimenName
}
else if (tablename == "ItemSourceId") {
	functionCalls->Values[tablename] = "list/contact/10000/page/1?Filtering=ContactId+IN+" + query;  	   //ContactFirstName + ContactLastName
}
else if (tablename == "TrialUnitSpecimenId") {
	functionCalls->Values[tablename] = "list/trialunitspecimen/10000/page/1?Filtering=" + tablename + "+IN+" + query;
}
else {
	ShowMessage("GetCombosFromDAL error - called with " + tablename);
	return;
}

//iterate over all the item details fields
for (int ri = 0; ri < Form1->ItemDetailBox->ChildrenCount; ri++) {
	TComboBox *combo = dynamic_cast<TComboBox*>(Form1->ItemDetailBox->Children->Items[ri]);
	if (combo) { //found a combobox
		if (combo == Form1->ItemOperation) {

		}
		else {
			String indexName = combo->Name;
			bool specialCase = false;//(combo->Name == "ItemSourceId");
			//get a list of container and item types
			String pstring = functionCalls->Values[indexName];
			if (!pstring.IsEmpty()) {
				Form1->Label1->Text = "Updating " + indexName;
				bool success = LoginWn->DoQuery(pstring, true);
				if (!success) {
					Form1->Label1->Text = pstring;
					return;
				}
				LoginWn->FillTable(pstring);
				int id = GetCol(combo->ListBoxResource, MainWn->StringGrid1);
				//combo->Items->Clear();   //todo check whether this is correct! When disabled every search will increase the metadata
				combo->BeginUpdate();
				int currIndex = 0;
				for (int i = 0; i < MainWn->StringGrid1->RowCount; i++) {
					String sid;
					if (specialCase) {
						sid = MainWn->StringGrid1->Cells[id][i] + " " + MainWn->StringGrid1->Cells[id][i+2];
					}
					else sid = MainWn->StringGrid1->Cells[id][i];
					if (sid.IsEmpty()) { //mcol metadata!
						for (int j = 0; j < MainWn->StringGrid1->ColumnCount; j++) {
							String extraval = MainWn->StringGrid1->Cells[j][i];
							if (extraval.IsEmpty()) {
							}
							else {
								String colHeader = ((TStringColumn*)MainWn->StringGrid1->ColumnByIndex(j))->Header;
								TStringList *s = (TStringList*)combo->Items->Objects[currIndex];
								s->Values[colHeader] = s->Values[colHeader] + "," + extraval;   //use csv for mcol
							}
						}
					}
					else {
						TStringList *s = new TStringList();
						for (int j = 0; j < MainWn->StringGrid1->ColumnCount; j++) {//iterate over the main table rows
							String colHeader = ((TStringColumn*)MainWn->StringGrid1->ColumnByIndex(j))->Header;
							s->Values[colHeader] = MainWn->StringGrid1->Cells[j][i];
						}
						currIndex = combo->Items->AddObject(sid, s);
					}
				}
				combo->EndUpdate();
				SaveCombo(combo);
				break;
			}
		}
	}
}
delete functionCalls; functionCalls = NULL;
}
//---------------------------------------------------------------------------
TButton* TLoginWn::getButton(TComboBox *combo) {
TButton *b = NULL;
for (int i = 0; i < combo->Children->Count; i++) {
	b = dynamic_cast<TButton*>(combo->Children->Items[i]);
	if (b) {
		break;
	}
}
if (!b) {
	ShowMessage("error finding button " + combo->Name);
}
return b;
}
//---------------------------------------------------------------------------
void TLoginWn::GetAddButtonDAL(TComboBox *combo) {
TButton *b = getButton(combo);
String tableQ = b->Name;
if (b->Name.Pos("generaltype")) {
	tableQ = "generaltype";
}
String pstring = tableQ + "/list/field";
bool success = LoginWn->DoQuery(pstring, true);
if (!success) {
	Form1->Label1->Text = pstring;
	return;
}
TStringGrid *sg = new TStringGrid(this);
sg->Visible = false;

sg->RowCount = 0;
LoginWn->FillTable(pstring);
//int reversecol = 3;
for (int j = 0; j < 4; j++) {
	TStringColumn *Col = new TStringColumn(sg);
	Col->Header = ((TStringColumn*)MainWn->StringGrid1->ColumnByIndex(j))->Header;
	Col->Parent = sg;
}
for (int j = 0; j < MainWn->StringGrid1->RowCount; j++) {
	String check = MainWn->StringGrid1->Cells[1][j]; //todo remove hardcoding of columns
	if (check.IsEmpty()) { //the main table row is not valid

	}
	else { //add a new row
		sg->RowCount++;
		for (int i = 0; i < 4; i++) {//iterate over the 1st 4 main table columns columns
			sg->Cells[i][sg->RowCount - 1] = MainWn->StringGrid1->Cells[i][j];
		}
	}
}

b->TagObject = (sg);
SaveAddButton(b);
}
//---------------------------------------------------------------------------
void __fastcall TLoginWn::DALBaseKeyUp(TObject *Sender, WORD &Key, System::WideChar &KeyChar, TShiftState Shift) {
if (Key == 13) {
	LoginButClick(Sender);
}
}
//---------------------------------------------------------------------------
void TLoginWn::Scale() {
Layout1->Scale->X = Form1->Layout1->Scale->X;
Layout1->Scale->Y = Form1->Layout1->Scale->Y;
ClientHeight = ClientHeight * Layout1->Scale->Y;
ClientWidth = ClientWidth * Layout1->Scale->X;
Left = (Form1->Left) + (Form1->Width - Width)/2;
Top = (Form1->Top) + (Form1->Height - Height)/2;
}
//---------------------------------------------------------------------------





