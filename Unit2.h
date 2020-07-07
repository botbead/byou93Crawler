// ---------------------------------------------------------------------------

#ifndef Unit2H
#define Unit2H
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include "DBAccess.hpp"
#include "SQLiteUniProvider.hpp"
#include "Uni.hpp"
#include "UniProvider.hpp"
#include <Data.DB.hpp>
#include "DAScript.hpp"
#include "MemDS.hpp"
#include "UniScript.hpp"
#include <System.Net.HttpClient.hpp>
#include <System.Net.HttpClientComponent.hpp>
#include <System.Net.URLClient.hpp>
#include <Vcl.ExtCtrls.hpp>

// ---------------------------------------------------------------------------
class TDataModule2 : public TDataModule {
__published: // IDE-managed Components
	TSQLiteUniProvider *SQLiteUniProvider1;
	TUniConnection *UniConnection1;
	TUniQuery *UniQuery1;
	TUniScript *UniScript1;
	TNetHTTPClient *NetHTTPClient1;
	TNetHTTPRequest *NetHTTPRequest_login;
	TNetHTTPRequest *NetHTTPRequest_census;
	TUniSQL *UniSQL1;
	TUniTable *UniTable_census;
	TNetHTTPRequest *NetHTTPRequest_ids;
	TUniTable *UniTable_traversal_ids;
	TNetHTTPRequest *NetHTTPRequest_solo;
	TNetHTTPRequest *NetHTTPRequest_extracting;

	void __fastcall DataModuleCreate(TObject *Sender);
	void __fastcall NetHTTPRequest_loginRequestCompleted(TObject * const Sender,
		IHTTPResponse * const AResponse);
	void __fastcall DataModuleDestroy(TObject *Sender);
	void __fastcall NetHTTPRequest_censusRequestCompleted(TObject * const Sender,
		IHTTPResponse * const AResponse);
	void __fastcall NetHTTPRequest_censusRequestError(TObject * const Sender, const UnicodeString AError);





private: // User declarations
public: // User declarations
	__fastcall TDataModule2(TComponent* Owner);
};

// ---------------------------------------------------------------------------
extern bool login;
extern bool hide_response, stop;
extern PACKAGE TDataModule2 *DataModule2;
extern UnicodeString peoplepath;
extern TEncoding *encd;
extern void *event_activate_recving;

extern void start_census();
extern void start_crawling_u_id();
extern void end_crawling_u_id();
// extern void wait_for_timer_end();
//extern void start_crawling_solo();
//extern void end_crawling_solo();
extern void start_extracting();
// ---------------------------------------------------------------------------
#endif
