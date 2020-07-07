// ---------------------------------------------------------------------------

#pragma hdrstop
#include <vcl.h>
#include "Unit2.h"
#include <process.h>
#include "Unit1.h"
#include <queue>
#include "SimpleThreads.h"
#include <iostream>
// #include <fstream>
using namespace std;
#include <boost/pool/singleton_pool.hpp>
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma classgroup "Vcl.Controls.TControl"
#pragma link "DBAccess"
#pragma link "SQLiteUniProvider"
#pragma link "Uni"
#pragma link "UniProvider"
#pragma link "DAScript"
#pragma link "MemDS"
#pragma link "UniScript"
#pragma resource "*.dfm"

typedef wchar_t wurl[256];

enum data_type {
	txt_, img_, video_
};

struct by93_data {
	int u_id;
	// 0-txt, 1-img, 2-video
	int data_type;
	// 0-level 1, 1-level 2
	int level;
	// city_index
	int i_c;

	// union {
	// wchar_t *_string;
	// unsigned char *_binary;
	// };

	wurl url;
	// UnicodeString url;
};

extern bool extracting_mode;

const UnicodeString url_prefix = L"http://www.byou93.com/member.asp?id=%d";
const UnicodeString city_format =
	L"http://www.byou93.com/%s/index.asp?currentpage=%d&s=";
const UnicodeString single_user_item = L"zby_rbox";
const UnicodeString usr_id_prefix = L"id=";

enum city_index {
	beijing = 1, shanghai, guangzhou, shenzhen, nanjing, hangzhou, chengdu,
	wuhan, chongqing, shenyang, xian, qingdao, suzhou, changsha, zhengzhou,
	jinan, taiyuan, hefei, fuzhou, shijiazhuang, haerbin, xiamen, ningbo,
	kunming, changchun, changzhou, foshan, nanchang, nanning, wenzhou, dalian,
	haikou, quanzhou, yantai, guiyang, othercity
};

const wchar_t illegal_chars[9] = {
	L'\\', L'/', L':', L'*', L'?', L'\"', L'<', L'>', L'|'};

const UnicodeString illegal_chars_substitute[9] = {
	L"『backslash』", L"『slash』", L"『colon』", L"『asterisk』", L"『question mark』",
	L"『double quotes』", L"『less than』", L"『greater than』", L"『vertical bar』"};

const UnicodeString city_pinyin[] = {
	L"", L"beijing", L"shanghai", L"guangzhou", L"shenzhen", L"nanjing",
	L"hangzhou", L"chengdu", L"wuhan", L"chongqing", L"shenyang", L"xian",
	L"qingdao", L"suzhou", L"changsha", L"zhengzhou", L"jinan", L"taiyuan",
	L"hefei", L"fuzhou", L"shijiazhuang", L"haerbin", L"xiamen", L"ningbo",
	L"kunming", L"changchun", L"changzhou", L"foshan", L"nanchang", L"nanning",
	L"wenzhou", L"dalian", L"haikou", L"quanzhou", L"yantai", L"guiyang"};

const UnicodeString city_chinese[] = {
	L"", L"北京", L"上海", L"广州", L"深圳", L"南京", L"杭州", L"成都", L"武汉", L"重庆", L"沈阳",
	L"西安", L"青岛", L"苏州", L"长沙", L"郑州", L"济南", L"太原", L"合肥", L"福州", L"石家庄",
	L"哈尔滨", L"厦门", L"宁波", L"昆明", L"长春", L"常州", L"佛山", L"南昌", L"南宁", L"温州",
	L"大连", L"海口", L"泉州", L"烟台", L"贵阳"};

enum key_index {
	myphoto, nickname, video, gender, age, birthday, horoscope, height, weight,
	somatotype, educationalbackground, maritalstatus, profession, currentcity,
	birthplace, email, phone, qq, wechat, escortregion, hobby, personalprofile,
	imgs_flag, other
};

const UnicodeString info_keys[] = {
	L"myphoto", L"昵　　称", L"查看视频", L"性　　别", L"年　　龄", L"出生年月", L"星　　座", L"身　　高",
	L"体　　重", L"体　　形", L"学　　历", L"婚　　姻", L"职　　业", L"居住城市", L"籍　　贯", L"邮箱", L"手机",
	L"Q&nbsp;&nbsp;Q", L"微信", L"伴游范围", L"兴趣爱好", L"心情留言", L"byxc_bb_im"};

const UnicodeString page_order_keyword = L">页";
const UnicodeString person_number_keyword = L"总数";
const UnicodeString default_avatar = L"457164.jpg";
const UnicodeString by93_url = L"http://www.byou93.com";
const UnicodeString space = L"&nbsp;";
const UnicodeString l_single_quote = L"『BotBead』";
const UnicodeString r_single_quote = L"『.com』";
const UnicodeString album_flag = L"clearfix";

bool login;
bool hide_response;
bool stop;
volatile bool census_end;
TDataModule2 *DataModule2;
TEncoding *encd = 0;
void *thread = 0;

void *event_census_sending = 0;
void *event_census_recving = 0;
void *thread_ecs = 0;
void *thread_ecr = 0;

void *thread_sending = 0;
void *thread_recving = 0;

// *extracing begin
// breadth-first
int global_media_id;

unsigned thread_id_create_queue_request_level_1;
void *thread_create_queue_request_level_1;

unsigned thread_id_extractor;
void *thread_extractor = 0;

unsigned thread_id_downloading;
void *thread_dowloading = 0;

void *thread_extracting_ary[3] = {0};

void *event_infos_extracting_completed = 0;

queue<by93_data*>q_by93_l1;
queue<by93_data*>q_by93_l2;

typedef boost::singleton_pool < struct by93_data_tag {
}, sizeof(by93_data) > by93_data_pool;

unsigned __stdcall create_q_r_l1(void*);
unsigned __stdcall extracting_infos(void*);
unsigned __stdcall download_media(void *);
unsigned __stdcall wait_extracting_end(void *);

void start_extracting();
void parse_data_level_1(UnicodeString *, int, int);
void parse_data_level_2(UnicodeString *, int, int, int);
// *extracing end

UnicodeString str_content_recved;

static UnicodeString content_census;

unsigned thread_id;

unsigned thread_id_census_s;
unsigned thread_id_census_r;

int cores_num;
volatile int city_i;
void *thread_ary[2] = {0};
UnicodeString peoplepath;
static UnicodeString URL(L"");

thread_common *tc_data_handler = 0;
thread_common tc_crawling_u_id;

// SRWLOCK srwlock = SRWLOCK_INIT;
// CONDITION_VARIABLE condivar = CONDITION_VARIABLE_INIT;

void create_city_tables();
void create_cities_dirs();

void start_census();
void end_census();
unsigned __stdcall data_hanlder_func(void*);
unsigned __stdcall media_handler_func(void*);
unsigned __stdcall crawling_u_id_func(void*);
void start_crawling_u_id();
void end_crawling_u_id();

// ---------------------------------------------------------------------------
__fastcall TDataModule2::TDataModule2(TComponent* Owner) : TDataModule(Owner) {
	global_media_id = 0;
	login = false;
	stop = false;
	hide_response = true;
	census_end = false;
	encd = TEncoding::GetEncoding(936); // gbk
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule2::DataModuleCreate(TObject *Sender) {
	UniConnection1->ProviderName = "SQLite";
	UniConnection1->SpecificOptions->Values["Direct"] = "True";
	UniConnection1->SpecificOptions->Values["ForceCreateDatabase"] = "True";
	UniConnection1->Database = ExtractFilePath(Application->ExeName) +
		"byou93.db";
	UniConnection1->Connect();
	UniScript1->Connection = UniConnection1;
	UniQuery1->Connection = UniConnection1;
	UniSQL1->Connection = UniConnection1;
	UniTable_census->Connection = UniConnection1;
	UniTable_census->TableName = L"user_stats";
	UniTable_census->RefreshOptions << roAfterInsert << roAfterUpdate;
	UniTable_census->SmartFetch->Enabled = true;
	UniTable_census->SmartFetch->LiveBlock = false;
	UniTable_traversal_ids->Connection = UniConnection1;
	UniTable_traversal_ids->TableName = L"traversal_u_id";
	UniTable_traversal_ids->RefreshOptions << roAfterInsert << roAfterUpdate;
	UniTable_traversal_ids->SmartFetch->Enabled = true;
	UniTable_traversal_ids->SmartFetch->LiveBlock = false;
	UniScript1->SQL->Clear();
	UniScript1->SQL->Add("CREATE TABLE IF NOT EXISTS user_stats (\
city_pinyin TEXT,\
city_chinese TEXT,\
url text,\
total integer,\
page_num intger,\
rowid_2th INTEGER PRIMARY KEY AUTOINCREMENT,\
unique(city_pinyin)\
);CREATE TABLE IF NOT EXISTS traversal_u_id (\
city_pinyin TEXT,\
current_url text default '',\
people_num_crawled integer default 0,\
current_page_num intger default 1,\
traversal_completed_once bool default false,\
rowid_2th INTEGER PRIMARY KEY AUTOINCREMENT,\
unique(city_pinyin)\
);CREATE TABLE IF NOT EXISTS media_url (\
usr_id INTEGER,\
city_index INTEGER,\
media_url TEXT,\
downloaded bool default false,\
rowid_2th INTEGER PRIMARY KEY AUTOINCREMENT,\
unique(usr_id, media_url)\
);");
	UniScript1->Execute();
	create_city_tables();

	peoplepath = ExtractFileDir(Application->ExeName) + "\\peoplepath";
	if (DirectoryExists(peoplepath));
	else
		ForceDirectories(peoplepath);

	create_cities_dirs();

	NetHTTPRequest_login->Client = NetHTTPClient1;
	NetHTTPRequest_login->Asynchronous = true;
	NetHTTPRequest_census->Client = NetHTTPClient1;
	NetHTTPRequest_census->Asynchronous = true;
	NetHTTPRequest_ids->Client = NetHTTPClient1;
	NetHTTPRequest_ids->Asynchronous = false;

	UniTable_census->Active = true;
	UniTable_traversal_ids->Active = true;

	event_census_sending = CreateEvent(0, 0, 1, 0);
	event_census_recving = CreateEvent(0, 0, 0, 0);

	get_cpus_num(&cores_num);
}
// ---------------------------------------------------------------------------

void __fastcall TDataModule2::NetHTTPRequest_loginRequestCompleted
	(TObject * const Sender, IHTTPResponse * const AResponse) {
	UnicodeString content = AResponse->ContentAsString(encd);
	if (hide_response);
	else {
		Form1->Memo1->Lines->Clear();
		Form1->Memo1->Lines->Text = content;
	}
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule2::DataModuleDestroy(TObject *Sender) {
	CloseHandle(event_census_recving);
	CloseHandle(event_census_sending);
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule2::NetHTTPRequest_censusRequestCompleted
	(TObject * const Sender, IHTTPResponse * const AResponse) {
	if (200 != AResponse->StatusCode);
	else {
		content_census = AResponse->ContentAsString(encd);
		SetEvent(event_census_recving);
	}
}

// ---------------------------------------------------------------------------
void create_city_tables() {
	int i = beijing;
	UnicodeString create_tables_cmd(L"");
	UnicodeString temp(L"");
	for (; i < othercity; ++i) {
		temp = Sysutils::Format("CREATE TABLE IF NOT EXISTS %s (\
usr_id INTEGER,\
name TEXT,\
age TEXT,\
birthday TEXT,\
gender text,\
currentcity TEXT,\
horoscope TEXT,\
height TEXT,\
weight TEXT,\
somatotype TEXT,\
educationalbackground TEXT,\
maritalstatus TEXT,\
profession TEXT,\
birthplace TEXT,\
email text,\
phone text,\
qq text,\
wechat text,\
hobby text,\
personality text,\
personalprofile text,\
escortregion text,\
albumpath text,\
invalidation bool default false,\
crawling_completed bool default false,\
rowid_2th INTEGER PRIMARY KEY AUTOINCREMENT,\
UNIQUE (usr_id));", ARRAYOFCONST((city_pinyin[i])));
		create_tables_cmd += temp;
		temp.SetLength(0);
	}
	DataModule2->UniScript1->SQL->Clear();
	DataModule2->UniScript1->SQL->Add(create_tables_cmd);
	DataModule2->UniScript1->Execute();
}

// ---------------------------------------------------------------------------
unsigned __stdcall census_sending(void* param) {
	city_i = beijing;
	while (city_i < othercity) {
		WaitForSingleObject(event_census_sending, INFINITE);
		URL = Sysutils::Format(city_format,
			ARRAYOFCONST((city_pinyin[city_i], 1)));
		DataModule2->NetHTTPRequest_census->URL = URL;
		DataModule2->NetHTTPRequest_census->MethodString = "GET";
		DataModule2->NetHTTPRequest_census->Execute();
	}
	census_end = true;
	return 0;
}

// ---------------------------------------------------------------------------
unsigned __stdcall census_recving(void* param) {
	int i;
	int j;
	int k;
	int page_num = 0;
	int total = 0;
	Variant *p;
	UnicodeString x;
	Variant v;
	while (1) {
		WaitForSingleObject(event_census_recving, INFINITE);
		if (census_end)
			break;
		if (content_census.IsEmpty());
		else {
			k = j = i = content_census.Pos(page_order_keyword);
			if (i) {
				--i;
				while (content_census[i] != L'>') {
					if (content_census[i] == L'<')
						j = i;
					--i;
				}
				page_num = content_census.SubString(i + 1, j - i - 1).ToInt();
				i = 0;
				++k;
				do {
					if (L'>' != content_census[k]);
					else
						++i;
					k++;
				}
				while (3 != i);
				i = k;
				while (L'<' != content_census[k])
					k++;
				total = content_census.SubString(i, k - i).ToInt();
				if (k = DataModule2->UniTable_census->RecordCount) {
					v = DataModule2->UniTable_census->Lookup(L"city_pinyin",
						city_pinyin[city_i], L"total;page_num");
					if (VarIsArray(v)) {
						p = (Variant*)VarArrayLock(v);
						if ((total != (int)p[0]) || (page_num != (int)p[1])) {
							if (DataModule2->UniConnection1->InTransaction);
							else {
								DataModule2->UniConnection1->StartTransaction();
								DataModule2->UniSQL1->SQL->Clear();
								DataModule2->UniSQL1->SQL->Add
									(Sysutils::Format
									("update user_stats set total = %d, page_num = %d where city_pinyin = '%s';",
									ARRAYOFCONST((total, page_num,
									city_pinyin[city_i]))));
								try {
									DataModule2->UniSQL1->Execute();
									DataModule2->UniConnection1->Commit();
								}
								catch (...) {
									DataModule2->UniConnection1->Rollback();
								}
							}
						}
						VarArrayUnlock(v);
					}
				}
				else {
					if (DataModule2->UniConnection1->InTransaction);
					else {
						DataModule2->UniConnection1->StartTransaction();
						DataModule2->UniSQL1->SQL->Clear();
						DataModule2->UniSQL1->SQL->Add
							(Sysutils::Format
							("insert into user_stats (city_pinyin, city_chinese, url, total, page_num) values('%s', '%s', '%s', %d, %d);",
							ARRAYOFCONST((city_pinyin[city_i],
							city_chinese[city_i], URL, total, page_num))));
						DataModule2->UniSQL1->SQL->Add
							(Sysutils::Format
							("insert into traversal_u_id (city_pinyin, current_url) values('%s', '%s');",
							ARRAYOFCONST((city_pinyin[city_i], URL))));
						try {
							DataModule2->UniSQL1->Execute();
							DataModule2->UniConnection1->Commit();
						}
						catch (...) {
							DataModule2->UniConnection1->Rollback();
						}
					}
				}
			}
		}
		++city_i;
		Sleep(250);
		SetEvent(event_census_sending);
	}
	return 0;
}

// ---------------------------------------------------------------------------
unsigned __stdcall census_end_func(void* param) {
	if (WAIT_OBJECT_0 == WaitForMultipleObjects(2, thread_ary, true, INFINITE))
	{
		CloseHandle(thread_ecs);
		CloseHandle(thread_ecr);
		thread_ary[0] = thread_ary[1] = 0;
		Form1->Button5->Enabled = true;
		Form1->StatusBar1->Panels->operator[](1)->Text = L"Crawling Stoped!";
	}
#ifdef _DEBUG
	Form1->Memo1->Lines->Add(DataModule2->UniTable_census->RecordCount);
#endif
	return 0;
}

// ---------------------------------------------------------------------------
void start_census() {
	thread_ary[0] = thread_ecs = (void *)_beginthreadex(0, 0, &census_sending,
		0, 0, &thread_id_census_s);
	thread_ary[1] = thread_ecr = (void *)_beginthreadex(0, 0, &census_recving,
		0, 0, &thread_id_census_s);
	end_census();
}
// ---------------------------------------------------------------------------

void end_census() {
	CloseHandle((HANDLE)_beginthreadex(0, 0, &census_end_func, 0, 0,
		&thread_id));
}
// ---------------------------------------------------------------------------

void create_cities_dirs() {
	int i = beijing;
	UnicodeString temp("");
	while (i < othercity) {
		temp = peoplepath + L"\\" + city_chinese[i];
		if (DirectoryExists(temp));
		else
			ForceDirectories(temp);
		i++;
	}
}

// ---------------------------------------------------------------------------
unsigned __stdcall data_hanlder_func(void* param) {
	// by93_data q_data_unit;
	// thread_common *tc = (thread_common*)param;
	// while (is_tc_go_on(tc)) {
	// lock(&q_by93_lock);
	// while (!q_by93.empty()) {
	// MoveMemory(&q_data_unit, &(q_by93.front()), sizeof(q_by93.front()));
	// q_by93.pop();
	// unlock(&q_by93_lock);
	// do something

	// lock(&q_by93_lock);
	// }
	// unlock(&q_by93_lock);
	// }

	return 0;
}

// ---------------------------------------------------------------------------
unsigned __stdcall crawling_u_id_func(void* param) {
	bool traversal_completed_once;
	bool delta_action;
	int i, j;
	int single_city_people_num, single_city_pages_num;
	int people_num_crawled, current_page_num;
	TUniTable *table_census = DataModule2->UniTable_census;
	TUniTable *table_traversal = DataModule2->UniTable_traversal_ids;
	Variant *p = 0;
	UnicodeString city_pinyin1, city_chinese, city_first_url, current_url;
	UnicodeString content, usr_id, sql_cmd;
	Variant v;
	thread_common *tc = (thread_common*)param;
	while (is_tc_go_on(tc)) {
		table_census->First();
		while (!(table_census->Eof)) {
			city_pinyin1 = table_census->FieldByName(L"city_pinyin")->AsString;
			single_city_people_num = table_census->FieldByName(L"total")
				->AsInteger;
			single_city_pages_num = table_census->FieldByName(L"page_num")
				->AsInteger;
			city_first_url = table_census->FieldByName(L"url")->AsString;

			v = DataModule2->UniTable_traversal_ids->Lookup(L"city_pinyin",
				city_pinyin1,
				L"current_url;people_num_crawled;current_page_num;traversal_completed_once");
			if (VarIsArray(v)) {
				p = (Variant*)VarArrayLock(v);
				current_url = p[0].operator UnicodeString();
				people_num_crawled = p[1].operator int();
				current_page_num = p[2].operator int();
				traversal_completed_once = p[3].operator bool();
				VarArrayUnlock(v);
			}
			delta_action = false;
			if (traversal_completed_once) {

				if (people_num_crawled < single_city_people_num) {
					delta_action = true;
					current_url = city_first_url;
					current_page_num = 1;
				NEXT_PAGE_DELTA:

					try {
						DataModule2->NetHTTPRequest_ids->URL = current_url;
						DataModule2->NetHTTPRequest_ids->MethodString = "GET";
						content = DataModule2->NetHTTPRequest_ids->Execute()
							->ContentAsString(encd);
					}
					catch (ENetHTTPException &e) {
						Form1->StatusBar1->Panels->operator[](1)->Text =
							e.Message;
					}

					i = content.Pos(single_user_item);
					while (i) {
						content.Delete(1, i + single_user_item.Length());
						i = content.Pos(usr_id_prefix);
						content.Delete(1, i + usr_id_prefix.Length() - 1);
						j = content.Pos(L"\"");
						usr_id = content.SubString(1, j - 1);

						DataModule2->UniQuery1->SQL->Clear();
						DataModule2->UniQuery1->SQL->Add
							(Sysutils::Format
							("select usr_id from %s where usr_id = %d;",
							ARRAYOFCONST((city_pinyin1, usr_id.ToInt()))));
						if (DataModule2->UniConnection1->InTransaction);
						else {
							DataModule2->UniConnection1->StartTransaction();
							try {
								DataModule2->UniQuery1->Execute();
								DataModule2->UniConnection1->Commit();
							}
							catch (...) {
								DataModule2->UniConnection1->Rollback();
							}
						}

						if (DataModule2->UniQuery1->RecordCount);
						else {
							DataModule2->UniSQL1->SQL->Clear();
							DataModule2->UniSQL1->SQL->Add
								(Sysutils::Format
								("insert into %s (usr_id) values (%d);",
								ARRAYOFCONST((city_pinyin1, usr_id.ToInt()))));
							if (DataModule2->UniConnection1->InTransaction);
							else {
								DataModule2->UniConnection1->StartTransaction();
								try {
									DataModule2->UniSQL1->Execute();
									DataModule2->UniConnection1->Commit();
								}
								catch (...) {
									DataModule2->UniConnection1->Rollback();
								}
							}
							++people_num_crawled;
							if (people_num_crawled == single_city_people_num)
								goto DELTA_END;
						}

						i = content.Pos(single_user_item);
					}

					++current_page_num;
					current_url =
						Sysutils::Format(city_format,
						ARRAYOFCONST((city_pinyin1, current_page_num)));
					Sleep(250);
					if (current_page_num < single_city_pages_num + 1)
						goto NEXT_PAGE_DELTA;

				}
			DELTA_END:
				if (delta_action) {
					DataModule2->UniSQL1->SQL->Clear();
					DataModule2->UniSQL1->SQL->Add
						(Sysutils::Format
						("update traversal_u_id set current_url = '%s', people_num_crawled = %d, current_page_num = %d where city_pinyin = '%s';",
						ARRAYOFCONST((current_url, people_num_crawled,
						current_page_num, city_pinyin1))));
					if (DataModule2->UniConnection1->InTransaction);
					else {
						DataModule2->UniConnection1->StartTransaction();
						try {
							DataModule2->UniSQL1->Execute();
							DataModule2->UniConnection1->Commit();
						}
						catch (...) {
							DataModule2->UniConnection1->Rollback();
						}
					};
				}
			}
			else {
			NEXT_PAGE_FIRST_TIME:

				DataModule2->UniSQL1->SQL->Clear();

				try {
					DataModule2->NetHTTPRequest_ids->URL = current_url;
					DataModule2->NetHTTPRequest_ids->MethodString = "GET";
					content = DataModule2->NetHTTPRequest_ids->Execute()
						->ContentAsString(encd);
				}
				catch (ENetHTTPException &e) {
					Form1->StatusBar1->Panels->operator[](1)->Text = e.Message;
				}

#ifdef _DEBUG
				Form1->Memo1->Lines->Text = content;
#endif

				i = content.Pos(single_user_item);
				while (i) {
					content.Delete(1, i + single_user_item.Length());
					i = content.Pos(usr_id_prefix);
					content.Delete(1, i + usr_id_prefix.Length() - 1);
					j = content.Pos(L"\"");
					usr_id = content.SubString(1, j - 1);
					DataModule2->UniSQL1->SQL->Add
						(Sysutils::Format
						("insert into %s (usr_id) values (%d);",
						ARRAYOFCONST((city_pinyin1, usr_id.ToInt()))));
					++people_num_crawled;

					i = content.Pos(single_user_item);
				}

				++current_page_num;
				current_url = Sysutils::Format(city_format,
					ARRAYOFCONST((city_pinyin1, current_page_num)));
				DataModule2->UniSQL1->SQL->Add
					(Sysutils::Format
					("update traversal_u_id set city_pinyin = '%s', current_url = '%s', people_num_crawled = %d, current_page_num =%d where city_pinyin = '%s';",
					ARRAYOFCONST((city_pinyin1, current_url, people_num_crawled,
					current_page_num, city_pinyin1))));

				if (DataModule2->UniConnection1->InTransaction);
				else {
					DataModule2->UniConnection1->StartTransaction();
					try {
						DataModule2->UniSQL1->Execute();
						DataModule2->UniConnection1->Commit();
					}
					catch (...) {
						DataModule2->UniConnection1->Rollback();
					}
				}
				Sleep(250);
				if (current_page_num < single_city_pages_num + 1)
					goto NEXT_PAGE_FIRST_TIME;

			}
			if (traversal_completed_once);
			else {
				DataModule2->UniSQL1->SQL->Clear();
				DataModule2->UniSQL1->SQL->Add
					(Sysutils::Format
					("update traversal_u_id set traversal_completed_once = %d where city_pinyin = '%s';",
					ARRAYOFCONST((1, city_pinyin1))));
				if (DataModule2->UniConnection1->InTransaction);
				else {
					DataModule2->UniConnection1->StartTransaction();
					try {
						DataModule2->UniSQL1->Execute();
						DataModule2->UniConnection1->Commit();
					}
					catch (...) {
						DataModule2->UniConnection1->Rollback();
					}
				}
			}
			table_census->Next();
		}
		Form1->StatusBar1->Panels->operator[](1)->Text =
			L"ID crawling Completed!";
	}
	return 0;
}

// ---------------------------------------------------------------------------
void start_crawling_u_id() {
	init_thread_common(&tc_crawling_u_id);
	start_thread_common(&tc_crawling_u_id, crawling_u_id_func,
		&tc_crawling_u_id);
	active_thread_common(&tc_crawling_u_id);
}

// ---------------------------------------------------------------------------
void end_crawling_u_id() {
	end_thread_common(&tc_crawling_u_id);
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule2::NetHTTPRequest_censusRequestError
	(TObject * const Sender, const UnicodeString AError) {
	Form1->StatusBar1->Panels->operator[](1)->Text = AError;
	Sleep(60000);
	SetEvent(event_census_sending);
}

// ---------------------------------------------------------------------------
void start_extracting() {
	event_infos_extracting_completed = CreateEvent(0, 0, 0, 0);
	thread_create_queue_request_level_1 =
		(void *)_beginthreadex(0, 0, &create_q_r_l1, 0, 0,
		&thread_id_create_queue_request_level_1);

	thread_dowloading = (void *)_beginthreadex(0, 0, &download_media, 0, 0,
		&thread_id_downloading);
	thread_extracting_ary[0] = thread_create_queue_request_level_1;
	thread_extracting_ary[2] = thread_dowloading;
}

// ---------------------------------------------------------------------------
unsigned __stdcall create_q_r_l1(void* param) {
	bool crawling_completed;
	bool starting_extracting;
	int i_city;
	int uid;
	by93_data* by93_d = 0;
	UnicodeString temp_str;
	UnicodeString city;

	starting_extracting = false;
	i_city = 1;
	while (i_city < othercity) {
		city = city_pinyin[i_city];
		DataModule2->UniQuery1->SQL->Clear();
		if (extracting_mode) {
			DataModule2->UniQuery1->SQL->Add
				(Sysutils::Format(L"select usr_id from %s;",
				ARRAYOFCONST((city))));
		}
		else {
			DataModule2->UniQuery1->SQL->Add
				(Sysutils::Format
				(L"select usr_id from %s where crawling_completed = 0;",
				ARRAYOFCONST((city))));
		}

		if (DataModule2->UniConnection1->InTransaction);
		else {
			DataModule2->UniConnection1->StartTransaction();
			try {
				DataModule2->UniQuery1->Execute();
				DataModule2->UniConnection1->Commit();
			}
			catch (...) {
				DataModule2->UniConnection1->Rollback();
			}
		}

		if (DataModule2->UniQuery1->RecordCount) {
			DataModule2->UniQuery1->First();
			while (!(DataModule2->UniQuery1->Eof)) {
				uid = DataModule2->UniQuery1->FieldByName("usr_id")->AsInteger;

				by93_d = (by93_data*)by93_data_pool::malloc();
				temp_str = (AnsiString)Sysutils::Format(url_prefix,
					ARRAYOFCONST((uid)));
				SecureZeroMemory(by93_d->url, 256 * sizeof(wchar_t));
				MoveMemory(by93_d->url, temp_str.c_str(),
					temp_str.Length()*sizeof(wchar_t));
				by93_d->u_id = uid;
				by93_d->level = 0;
				by93_d->data_type = txt_;
				by93_d->i_c = i_city;

				q_by93_l1.push(by93_d);
				if ((q_by93_l1.size() > 10) && !starting_extracting) {

					thread_extractor =
						(void *)_beginthreadex(0, 0, &extracting_infos, 0, 0,
						&thread_id_extractor);
					thread_extracting_ary[1] = thread_extractor;
					CloseHandle((HANDLE)_beginthreadex(0, 0,
						&wait_extracting_end, 0, 0, &thread_id));
					starting_extracting = true;
				}

				DataModule2->UniQuery1->Next();
			}
		}

		++i_city;
	}
	if (!starting_extracting) {
		thread_extractor = (void *)_beginthreadex(0, 0, &extracting_infos, 0, 0,
			&thread_id_extractor);
		thread_extracting_ary[1] = thread_extractor;
		CloseHandle((HANDLE)_beginthreadex(0, 0, &wait_extracting_end, 0, 0,
			&thread_id));
		starting_extracting = true;
	}
	return 0;
}

// ---------------------------------------------------------------------------
unsigned __stdcall extracting_infos(void* param) {
	by93_data *by93_d = 0;
	UnicodeString str_responded;
	// UnicodeString test_url = L"http://www.byou93.com/member.asp?id=946812";
	while (!q_by93_l1.empty()) {
		by93_d = q_by93_l1.front();

		try {
			DataModule2->NetHTTPRequest_extracting->URL =
				UnicodeString(by93_d->url);
			// DataModule2->NetHTTPRequest_extracting->URL = test_url;
			DataModule2->NetHTTPRequest_extracting->MethodString = L"GET";
			str_responded = DataModule2->NetHTTPRequest_extracting->Execute()
				->ContentAsString(encd);

			parse_data_level_1(&str_responded, by93_d->u_id, by93_d->i_c);
		}
		catch (ENetHTTPRequestException &e) {
			Form1->StatusBar1->Panels->operator[](1)->Text =
				L"Request ERROR" + e.Message;
		}
		catch (ENetHTTPResponseException &e) {
			Form1->StatusBar1->Panels->operator[](1)->Text =
				L"Response ERROR" + e.Message;
		}
		catch (ENetHTTPException &e) {
			Form1->StatusBar1->Panels->operator[](1)->Text =
				L"HTTP ERRRO" + e.Message;
		}
		catch (...) {
			Form1->StatusBar1->Panels->operator[](1)->Text = L"Other ERRRO";
		}

		by93_data_pool::free(by93_d);

		q_by93_l1.pop();
		Sleep(250);
	}
	Form1->StatusBar1->Panels->operator[](1)->Text =
		L"Parsing data level 1 completed!";
	while (!q_by93_l2.empty()) {
		by93_d = q_by93_l2.front();

		try {
			DataModule2->NetHTTPRequest_extracting->URL =
				UnicodeString(by93_d->url);
			DataModule2->NetHTTPRequest_extracting->MethodString = L"GET";
			str_responded = DataModule2->NetHTTPRequest_extracting->Execute()
				->ContentAsString(encd);

			parse_data_level_2(&str_responded, by93_d->u_id, by93_d->i_c,
				by93_d->data_type);
		}
		catch (ENetHTTPRequestException &e) {
			Form1->StatusBar1->Panels->operator[](1)->Text =
				L"Request ERROR" + e.Message;
		}
		catch (ENetHTTPResponseException &e) {
			Form1->StatusBar1->Panels->operator[](1)->Text =
				L"Response ERROR" + e.Message;
		}
		catch (ENetHTTPException &e) {
			Form1->StatusBar1->Panels->operator[](1)->Text =
				L"HTTP ERRRO" + e.Message;
		}
		catch (...) {
			Form1->StatusBar1->Panels->operator[](1)->Text = L"Other ERRRO";
		}

		by93_data_pool::free(by93_d);
		q_by93_l2.pop();
		Sleep(250);
	}
	SetEvent(event_infos_extracting_completed);
	return 0;
}

// ---------------------------------------------------------------------------

void parse_data_level_2(UnicodeString *data, int uid, int city_ind,
	int data_type) {
	int pos;
	int url_pos;
	UnicodeString tmp_str;
	UnicodeString sqlcmd(L"");

	if (img_ == data_type) {
		pos = data->Pos(album_flag);
		if (pos) {
			data->Delete(1, pos + album_flag.Length());
			pos = data->Pos(L"</ul>");
			*data = data->SubString(1, pos);
			while (pos = data->Pos(L"src=")) {
				data->Delete(1, pos + UnicodeString(L"src=").Length());
				pos = data->Pos(L"\"");
				tmp_str = data->SubString(1, pos - 1);
				tmp_str = by93_url + tmp_str;
				sqlcmd +=
					Sysutils::Format
					("insert into media_url (usr_id, city_index, media_url) values (%d, %d, '%s');",
					ARRAYOFCONST((uid, city_ind, tmp_str)));
			}
			DataModule2->UniSQL1->SQL->Clear();
			DataModule2->UniSQL1->SQL->Add(sqlcmd);
			if (DataModule2->UniConnection1->InTransaction);
			else {
				DataModule2->UniConnection1->StartTransaction();
				try {
					DataModule2->UniSQL1->Execute();
					DataModule2->UniConnection1->Commit();
				}
				catch (...) {
					DataModule2->UniConnection1->Rollback();
				}
			}
		}
	}
	else if (video_ == data_type) {
		pos = data->Pos(L"source");
		if (pos) {
			data->Delete(1, pos + UnicodeString(L"source").Length());
			pos = data->Pos(L"\"");
			data->Delete(1, pos);
			pos = data->Pos(L"\"");
			tmp_str = data->SubString(1, pos - 1);
			tmp_str = by93_url + L"/" + tmp_str;
			sqlcmd +=
				Sysutils::Format
				("insert into media_url (usr_id, city_index, media_url) values (%d, %d, '%s');",
				ARRAYOFCONST((uid, city_ind, tmp_str)));
			DataModule2->UniSQL1->SQL->Clear();
			DataModule2->UniSQL1->SQL->Add(sqlcmd);
			if (DataModule2->UniConnection1->InTransaction);
			else {
				DataModule2->UniConnection1->StartTransaction();
				try {
					DataModule2->UniSQL1->Execute();
					DataModule2->UniConnection1->Commit();
				}
				catch (...) {
					DataModule2->UniConnection1->Rollback();
				}
			}
		}
	}
}

unsigned __stdcall download_media(void *param) {
	int uid, cid;
	int i;
	TMemoryStream* mstrm = 0;
	UnicodeString url;
	UnicodeString save_path;

	WaitForSingleObject(event_infos_extracting_completed, INFINITE);
	DataModule2->UniQuery1->SQL->Clear();
	DataModule2->UniQuery1->SQL->Add
		(UnicodeString(L"select * from media_url where downloaded = 0;"));
	if (DataModule2->UniConnection1->InTransaction);
	else {
		DataModule2->UniConnection1->StartTransaction();
		try {
			DataModule2->UniQuery1->Execute();
			DataModule2->UniConnection1->Commit();
		}
		catch (...) {
			DataModule2->UniConnection1->Rollback();
		}
	}
	if (DataModule2->UniQuery1->RecordCount) {
		mstrm = new TMemoryStream();
		DataModule2->UniQuery1->First();
		while (!(DataModule2->UniQuery1->Eof)) {
			uid = DataModule2->UniQuery1->FieldByName(L"usr_id")->AsInteger;
			cid = DataModule2->UniQuery1->FieldByName(L"city_index")->AsInteger;
			url = DataModule2->UniQuery1->FieldByName(L"media_url")->AsString;
			save_path = peoplepath + L"\\" + city_chinese[cid] + L"\\" +
				IntToStr((int)uid) + L"\\";
			try {
				DataModule2->NetHTTPRequest_extracting->URL =
					UnicodeString(url);
				DataModule2->NetHTTPRequest_extracting->MethodString = L"GET";
				if (DataModule2->NetHTTPRequest_extracting->Execute()
					->StatusCode != 200);
				else {
					mstrm->Clear();
					mstrm->LoadFromStream
						(DataModule2->NetHTTPRequest_extracting->Execute()
						->ContentStream);

					i = url.Length();
					while (L'.' != url.operator[](i))
						i--;
					save_path += IntToStr((int)global_media_id) +
						url.Delete(1, i - 1);
					mstrm->SaveToFile(save_path);
					++global_media_id;
				}
			}
			catch (ENetHTTPRequestException &e) {
				Form1->StatusBar1->Panels->operator[](1)->Text =
					L"Request ERROR" + e.Message;
			}
			catch (ENetHTTPResponseException &e) {
				Form1->StatusBar1->Panels->operator[](1)->Text =
					L"Response ERROR" + e.Message;
			}
			catch (ENetHTTPException &e) {
				Form1->StatusBar1->Panels->operator[](1)->Text =
					L"HTTP ERRRO" + e.Message;
			}
			catch (...) {
				Form1->StatusBar1->Panels->operator[](1)->Text = L"Other ERRRO";
			}

			DataModule2->UniQuery1->Next();
			Sleep(250);
		}
		delete mstrm;
	}
	return 0;
}

// ---------------------------------------------------------------------------
unsigned __stdcall wait_extracting_end(void *param) {
	if (WAIT_OBJECT_0 == WaitForMultipleObjects(3, thread_extracting_ary, true,
		INFINITE)) {
		CloseHandle(thread_extracting_ary[0]);
		CloseHandle(thread_extracting_ary[1]);
		CloseHandle(thread_extracting_ary[2]);
		CloseHandle(event_infos_extracting_completed);
		Form1->StatusBar1->Panels->operator[](1)->Text =
			L"Extracting completed!";
		Form1->Button6->Enabled = true;
		Form1->Button7->Enabled = true;
	}
	return 0;
}

// ---------------------------------------------------------------------------
void parse_data_level_1(UnicodeString *data, int uid, int city_ind) {
	wchar_t c;
	int i = myphoto;
	int s, e;
	by93_data* by93_d = 0;
	TStringList *txt_list = 0;
	UnicodeString tmp_str, tmp_str1;
	UnicodeString infos[23] = {L""};
	int pos = data->Pos(info_keys[myphoto]);
	if (pos) {
		tmp_str1 = data->SubString(pos + info_keys[myphoto].Length(), 1);
		if (tmp_str1.operator == (UnicodeString(L'-'))) {
			tmp_str1 =
				Sysutils::Format
				(L"update %s set invalidation = 1, crawling_completed = 1 where usr_id = %d;",
				ARRAYOFCONST((city_pinyin[city_ind], uid)));
			DataModule2->UniSQL1->SQL->Clear();
			DataModule2->UniSQL1->SQL->Add(tmp_str1);
			if (DataModule2->UniConnection1->InTransaction);
			else {
				DataModule2->UniConnection1->StartTransaction();
				try {
					DataModule2->UniSQL1->Execute();
					DataModule2->UniConnection1->Commit();
				}
				catch (...) {
					DataModule2->UniConnection1->Rollback();
				}
			}
			return;
		}
		data->Delete(1, pos + UnicodeString(info_keys[myphoto]).Length());
		pos = data->Pos(L"\"");
		data->Delete(1, pos);
		e = data->Pos(L"\"");
		tmp_str = data->SubString(1, e - 1);

		if (L'/' != tmp_str.operator[](1)) {
			tmp_str.Insert(L"/", 1);
		}

		s = tmp_str.Length();
		while (L'/' != tmp_str[s])
			--s;
		tmp_str1 = tmp_str.SubString(s + 1, tmp_str.Length() - s);
		if (tmp_str1 != default_avatar) {
			// tmp_str = by93_url + tmp_str;
			tmp_str.Insert(by93_url, 1);
			DataModule2->UniSQL1->SQL->Clear();
			DataModule2->UniSQL1->SQL->Add
				(Sysutils::Format
				("insert into media_url (usr_id, city_index, media_url) values (%d, %d, '%s');",
				ARRAYOFCONST((uid, city_ind, tmp_str))));
			if (DataModule2->UniConnection1->InTransaction);
			else {
				DataModule2->UniConnection1->StartTransaction();
				try {
					DataModule2->UniSQL1->Execute();
					DataModule2->UniConnection1->Commit();
				}
				catch (...) {
					DataModule2->UniConnection1->Rollback();
				}
			}
		}
	}
	else
		return;

	pos = data->Pos(info_keys[nickname]);
	if (pos) {
		data->Delete(1, pos + UnicodeString(info_keys[nickname]).Length());
		e = data->Pos(L"<");
		tmp_str = data->SubString(1, e - 1);
		if (i = tmp_str.Pos(UnicodeString(L'\''))) {
			tmp_str.Delete(i, 1);
			tmp_str.Insert(l_single_quote, i);
		}
		infos[nickname] = tmp_str;
		data->Delete(1, e);
	}

	pos = data->Pos(info_keys[video]);
	if (pos) {
		s = e = 0;
		i = pos;
		while (L'<' != data->operator[](i)) {
			if (L'\"' == data->operator[](i)) {
				if (!e) {
					e = i;
				}
				else {
					s = i;
					break;
				}
			}
			i--;
		}
		tmp_str = by93_url + L"/" + data->SubString(s + 1, e - s - 1);
		by93_d = (by93_data*)by93_data_pool::malloc();
		by93_d->u_id = uid;
		by93_d->data_type = video_;
		by93_d->level = 1;
		by93_d->i_c = city_ind;
		SecureZeroMemory(by93_d->url, 256 * sizeof(wchar_t));
		MoveMemory(by93_d->url, tmp_str.c_str(),
			tmp_str.Length()*sizeof(wchar_t));
		q_by93_l2.push(by93_d);
	}

	for (i = gender; i < imgs_flag; ++i) {
		pos = data->Pos(info_keys[i]);
		if (pos) {
			data->Delete(1, pos + String(info_keys[i]).Length());
			s = data->Pos(L">");
			e = s + 1;
			while (L'<' != data->operator[](e))
				e++;
			if (1 != e - s) {
				tmp_str = data->SubString(s + 1, e - s - 1).Trim();
				infos[i] = tmp_str;
				data->Delete(1, e);
			}
		}
	}

	if (infos[height] != UnicodeString(L"保留"))
		infos[height] = infos[height].SubString(1, infos[height].Length() - 2);
	if (infos[weight] != UnicodeString(L"保留"))
		infos[weight] =
			UnicodeString(infos[weight].SubString(1, infos[weight].Length() - 2)
		.ToInt() * 2);
	pos = infos[currentcity].Pos(space);
	if (pos) {
		infos[currentcity] = infos[currentcity].Delete(1,
			pos + space.Length() - 1);
	}

	pos = data->Pos(info_keys[imgs_flag]);
	if (pos) {
		data->Delete(1, pos + String(info_keys[imgs_flag]).Length());
		pos = data->Pos(L"\"");
		data->Delete(1, pos);
		pos = data->Pos(L"\"");
		tmp_str = by93_url + L"/" + data->SubString(1, pos - 1);
		by93_d = (by93_data*)by93_data_pool::malloc();
		by93_d->u_id = uid;
		by93_d->data_type = img_;
		by93_d->level = 1;
		by93_d->i_c = city_ind;
		SecureZeroMemory(by93_d->url, 256 * sizeof(wchar_t));
		MoveMemory(by93_d->url, tmp_str.c_str(),
			tmp_str.Length()*sizeof(wchar_t));
		q_by93_l2.push(by93_d);
	}

	i = -1;
	tmp_str = infos[personalprofile];
	while (pos = tmp_str.Pos(UnicodeString(L'\''))) {
		tmp_str.Delete(pos, 1);
		++i;
		if (i % 2) {
			tmp_str.Insert(r_single_quote, pos);
		}
		else {
			tmp_str.Insert(l_single_quote, pos);
		}
	}
	infos[personalprofile] = tmp_str;

	tmp_str = peoplepath + L"\\" + city_chinese[city_ind] + L"\\" +
		IntToStr((int)uid);
	if (DirectoryExists(tmp_str));
	else
		ForceDirectories(tmp_str);

	i = infos[nickname].Length();
	while (i) {
		s = 0;
		c = infos[nickname].operator[](i);
		while (s < 9) {
			if (c == illegal_chars[s]) {
				infos[nickname].Delete(i, 1);
				infos[nickname].Insert(illegal_chars_substitute[s], i);
				break;
			}
			s++;
		}
		--i;
	}
	tmp_str1 = tmp_str + L"\\" + infos[nickname] + L".txt";
	if (FileExists(tmp_str1));
	else {
		txt_list = new TStringList;

		for (i = gender; i < imgs_flag; ++i) {
			tmp_str = info_keys[i];
			tmp_str += L" = ";
			tmp_str += infos[i];
			txt_list->Add(tmp_str);
		}
		tmp_str = txt_list->operator[](qq - 3);
		tmp_str.Delete(2, tmp_str.Length() - infos[qq].Length() -
			UnicodeString(L" = ").Length() - 2);
		tmp_str.Insert(L"  ", 2);
		txt_list->Delete(qq - 3);
		txt_list->Insert(qq - 3, tmp_str);
		txt_list->SaveToFile(tmp_str1, TEncoding::Unicode);
		delete txt_list;
	}

	tmp_str = L"update ";
	tmp_str += city_pinyin[city_ind];
	tmp_str += L" set name='";
	tmp_str += infos[nickname];
	tmp_str += L"',gender='";
	tmp_str += infos[gender];
	tmp_str += L"',age='";
	tmp_str += infos[age];
	tmp_str += L"',birthday='";
	tmp_str += infos[birthday];
	tmp_str += L"',horoscope='";
	tmp_str += infos[horoscope];
	tmp_str += L"',height='";
	tmp_str += infos[height];
	tmp_str += L"',weight='";
	tmp_str += infos[weight];
	tmp_str += L"',somatotype='";
	tmp_str += infos[somatotype];
	tmp_str += L"',educationalbackground='";
	tmp_str += infos[educationalbackground];
	tmp_str += L"',maritalstatus='";
	tmp_str += infos[maritalstatus];
	tmp_str += L"',profession='";
	tmp_str += infos[profession];
	tmp_str += L"',currentcity='";
	tmp_str += infos[currentcity];
	tmp_str += L"',birthplace='";
	tmp_str += infos[birthplace];
	tmp_str += L"',email='";
	tmp_str += infos[email];
	tmp_str += L"',phone='";
	tmp_str += infos[phone];
	tmp_str += L"',qq='";
	tmp_str += infos[qq];
	tmp_str += L"',wechat='";
	tmp_str += infos[wechat];
	tmp_str += L"',escortregion='";
	tmp_str += infos[escortregion];
	tmp_str += L"',hobby='";
	tmp_str += infos[hobby];
	tmp_str += L"',personalprofile='";
	tmp_str += infos[personalprofile];
	tmp_str += L"',crawling_completed=";
	tmp_str += IntToStr((int)1);
	tmp_str += L" where usr_id=";
	tmp_str += IntToStr((int)uid);
	tmp_str += L";";
#ifdef _DEBUG
	OutputDebugStringW(city_pinyin[city_ind].c_str());
	OutputDebugStringW(tmp_str.c_str());
#endif
	DataModule2->UniSQL1->SQL->Clear();
	DataModule2->UniSQL1->SQL->Add(tmp_str);
	if (DataModule2->UniConnection1->InTransaction);
	else {
		DataModule2->UniConnection1->StartTransaction();
		try {
			DataModule2->UniSQL1->Execute();
			DataModule2->UniConnection1->Commit();
		}
		catch (...) {
			DataModule2->UniConnection1->Rollback();
		}
	}
}
// ---------------------------------------------------------------------------
