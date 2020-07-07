// ---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "Unit2.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
// false--extracting delta, true--extracting brand new
bool extracting_mode = false;
const UnicodeString name = L"天空立法者";
const UnicodeString pwd = L"why1983316";

// ---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner) {
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender) {

	TStringList *strings = new TStringList();
	UnicodeString name, pwd;
	if (Edit1->Text != L"")
		name = Edit1->Text;
	else
		name = ::name;
	if (Edit2->Text != L"")
		pwd = Edit2->Text;
	else
		pwd = ::pwd;
	strings->AddPair("username", name);
	strings->AddPair("password", pwd);
	DataModule2->NetHTTPRequest_login->Post
		("http://www.byou93.com/passport/loginsubmit.asp", strings, 0, encd);
	delete strings;
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::Button2Click(TObject *Sender) {
	hide_response = !hide_response;
	if (hide_response) {
		Button2->Caption = L"Show Response";
	}
	else {
		Button2->Caption = L"Hide Response";
	}
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender) {
	Button3->Enabled = false;
	Button4->Enabled = true;
	start_crawling_u_id();
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender) {
	end_crawling_u_id();
	Button3->Enabled = true;
	Button4->Enabled = false;
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::FormCreate(TObject *Sender) {
	Button3->Enabled = true;
	Button4->Enabled = false;
	Button2->Caption = L"Show Response";
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::Button5Click(TObject *Sender) {
	Button5->Enabled = false;
	start_census();
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::Button6Click(TObject *Sender) {
	extracting_mode = true;
	start_extracting();
	Button6->Enabled = false;
	Button7->Enabled = false;
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::FormConstrainedResize(TObject *Sender, int &MinWidth,
	int &MinHeight, int &MaxWidth, int &MaxHeight) {
	MinWidth = 957;
	MinHeight = 607;
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::Button7Click(TObject *Sender) {
	extracting_mode = false;
	start_extracting();
	Button6->Enabled = false;
	Button7->Enabled = false;
}
// ---------------------------------------------------------------------------

