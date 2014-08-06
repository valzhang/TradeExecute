#include "IDMPTradeApi.h"
#include <iomanip>
#include <iostream>
#include <fstream>
//#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <hash_map>
#include <string>
#include <list>
#include <afxmt.h>
#include <queue>

const char SERVER_IP[16] = "196.168.0.163";
const int SERVER_PORT = 8080;
const char USER_NAME[20] = "8111";
const char USER_PASSWORD[20] = "111111";

const int FUND_ID = 100;	//�ʲ��˻�����
const int CELL_ID = 1001;	//�ʲ���Ԫ����
const i64 PROFL_ID = 10011;	//�ʲ���ϱ���
const int DIRECTION = 0;	//���׷���
const int EXCHANGE = 6;	//���������
const int OFFSET = 1;		//��ƽ����
const int PJ = 0;			//�Ƿ�ƽ��
const char SYMBOL[8] = "IF1409";	//��Ʊ���ڻ�����
const int INI_PRICE = 0;
const int INI_VOL = 4;

const int FUND_SIZE = 100;
const int HOLD_SIZE = 300;
const int ORDER_SIZE = 300;
const int N_TIME = 3;
const int MAX_SECONDS = 600;
const int N_TICKS = 1;
const double FINAL_RATIO = 0.05;
const int MAX_INSERT_SECS = 5;	//�µ���ȴ�ʱ��
const string LOG_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\log.txt";
const string TRANS_LOG_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\transaction_log.txt";
const string ACTION_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\action_log.txt";
const string SYMBOL_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\symbol.txt";
//const bool DEBUG = false;
const bool PROCED_DEBUG = false;
const bool INSERT_DEBUG = true;
const bool CANCEL_DEBUG = true;
const int UPDATE_WAIT_MILSECS = 100;		//������¼��������
FILE *stream = NULL;
//ȫ�ֱ���
int global_trade_vol = 0;		//�ۼ��ѽ�����
double global_trade_mount = 0;		//�ۼ��ѽ��׽��
int global_order_vol = 0;		//�����л�δ�ɽ���
double global_last_price = 0.0;		//���¼�
int global_last_order_no = 0;
typedef hash_map <string, pair<double, double> > HashMapPriceData;
HashMapPriceData global_symbol_price;		//���µļ۸�
vector<string> global_symbol;				//����Symbol
list<int> global_order_NO;				//���е�orderNo
typedef hash_map <int, int> HashMapVolData;
HashMapVolData global_volumn;
CRITICAL_SECTION global_cs;				//������
bool global_price_ready;
bool global_vol_ready;
bool global_trade_ready;
//typedef hash_map <string, pair<CEvent, CEvent> > HashMapEventData;
//HashMapEventData global_event;
CEvent PriceChangeEvent(FALSE, FALSE, NULL, NULL);
CEvent TradeCommandEvent(FALSE, FALSE, NULL, NULL);
CEvent TradeReadyEvent(FALSE, FALSE, NULL, NULL);
CEvent PrintLogEvent(FALSE, FALSE, NULL, NULL);
char price_change_symbol[8];
int price_change_direction;
CRITICAL_SECTION PriceChangeCriticalSection;
CRITICAL_SECTION CommandCriticalSection;	//ָ�������
typedef struct TraderCommand{
	int fund_id;
	int cell_id;
	int profl_id;
	int exchange_code;
	char symbol[8];
	int direction;
	int offset_flag_type;
	int order_volumn;
	int pj;
	int max_seconds;
} TraderCommand;
TraderCommand trader_command;


typedef struct TraderThreadParameter{
	CIDMPTradeApi* pTrader;
	CIDMP_ORDER_REQ* orderReq;
	int maxSecs;
	int nTime;
	double finalRatio;
	double minGap;
	int orderStatus;
} TraderThreadParameter;

enum CommandStatus{
	INSERT_SEND,
	INSERT_RETURN,
	CANCEL_SEND,
	CANCEL_RETURN,
	UPDATE_ORDER
};
typedef struct LogMessage{
	int server_no;
	CommandStatus cmd_status;
	int cmd_return;
	int server_status;
	int direction;
	int offset;
	int trade_vol;
	int order_vol;
	int cancel_vol;
	double price;
}LogMessage;
queue<LogMessage> logMsgQueue;

DWORD WINAPI TraderCore( void *para );
DWORD WINAPI TraderThread( void *para );
DWORD WINAPI UpdateOrderVol( void *para );
DWORD WINAPI UpdatePrice( void *para );
DWORD WINAPI LogPrint( void *para );
//��ӡ��ί��������Ϣ
inline void PrintOrderMsg( CIDMP_ORDER_REQ &orderReq );
int OrderCancel( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE *stream, int action_no );
inline bool UpdateOrder( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE* stream, int action_no );
int OrderInsert( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE *stream, int action_no );
inline void PrintLogMessage( CIDMPTradeApi *pTader, CIDMP_ORDER_REQ *pOrder, CIDMP_ORDER_INFO *pInfo, FILE *stream, int actNo, bool insert );
//��ȡ���½�����Ϣ
//inline bool GetRealTrade( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, CIDMP_ORDER_INFO &orderInfo );
inline void GetCharStatus( int sta, char* data );
int GotoFinalStep( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, int maxSeconds, int nTime, double finalRatio, double minGap, FILE *stream, int action_no );
int main()
{

	CIDMPTradeApi trader;
	printf("�Ƿ��¼���׷�������(Y/y��¼����������)\n");
	//��¼
	printf("***���ڵ�¼�� %s, %d, %s...\n", SERVER_IP, SERVER_PORT, USER_NAME);
	char serv_ip[20];
	strcpy( serv_ip, SERVER_IP );
	int serv_port;
	serv_port = SERVER_PORT;
	char ur_name[20];
	strcpy( ur_name, USER_NAME );
	char ur_pw[20];
	strcpy( ur_pw, USER_PASSWORD );
	int logSuccess = trader.Connect( serv_ip, serv_port, ur_name, ur_pw, 0 );
	if ( logSuccess != 1 ){
		printf("��¼ʧ�ܣ�\n");
		printf("%d\n", logSuccess);
		getchar();
		return 0;
	}
	printf("***��¼�ɹ���\n������IP��%s,\t�������˿ڣ�%d,\t�û�����%s...\n", SERVER_IP, SERVER_PORT, USER_NAME);

	//��ȡ�ʲ��˻��б�
	printf("***��ȡ�ʲ��˻��б���\n");
	CIDMP_FUND_ACCOUNT_INFO accountInfo[FUND_SIZE];
	int accountSize = trader.GetFundAccount( accountInfo, FUND_SIZE );
	if (accountSize < 0){
		printf("��ȡ�ʲ��˻��б�ʧ�ܣ�\n");
		getchar();
		return 0;
	}
	printf("[���]\t����\t����\n");
	for(int i = 0;i < accountSize; i++)
	{
		printf("%d\t%d\t%s\n", i+1, accountInfo[i].id, accountInfo[i].name);
	}

	//���ָ���ʲ��˻����ʲ���Ԫ�б�
	printf("\n***����ʲ��˻�[%d]���ʲ���Ԫ�б�...\n", FUND_ID);
	CIDMP_FUND_CELL_INFO CellInfo[FUND_SIZE];	
	int cellSize = trader.GetFundCell( FUND_ID, CellInfo, FUND_SIZE);
	if(cellSize == -1) {
		printf("��ȡ�ʲ���Ԫ�б�ʧ��\n");
		getchar();
		return 0;
	}	 
	printf("[���]\t����\t����\n");
	for(int i = 0;i < cellSize; i++)
	{
		printf("%d\t%d\t%s\n", i+1, CellInfo[i].id, CellInfo[i].name);
	}

	//����ʲ���Ԫ��Ͷ������б�
	printf("\n***����ʲ��˻�[%d]���ʲ���Ԫ[%d]��Ͷ������б�...\n", FUND_ID, CELL_ID);
	CIDMP_FUND_PROFL_INFO ProfInfo[FUND_SIZE];	
	int proSize = trader.GetFundProfl( FUND_ID, CELL_ID, ProfInfo, FUND_SIZE);
	if(proSize == -1) {
		printf("��ȡͶ������б�ʧ��\n");
		getchar();	
		return 0;
	}	
	printf("[���]\t����\t����\n");
	for(int i = 0; i < proSize; i++)
	{
		printf("%d\t%I64d\t%s\n", i+1, ProfInfo[i].id, ProfInfo[i].name);		
	}
	
	//��ѯ�ʽ���Ϣ
	printf("\n***����ʲ��˻�[%d]���ʽ���Ϣ...\n", FUND_ID, CELL_ID);
	CIDMP_FUND_INFO fInfo;
	int nSuccess = trader.GetFund( FUND_ID, fInfo);
	if(nSuccess != 1) {
		printf("��ѯ�ʲ��˻����ʽ���Ϣʧ��\n");
		printf("%d\n", nSuccess);
		getchar();
		return 0;
	}

	printf("�ֻ����ʽ�\t: %6.3f\n�ֻ�����\t: %6.3f\n�ڻ����ʽ�\t: %6.3f\n�ڻ�����\t: %6.3f\n\n",
		fInfo.stkTotal,fInfo.stkAvailable, fInfo.futureTotal, fInfo.futureAvailable);

		
	printf("\n***����ʲ��˻�[%d]���ʲ���Ԫ[%d]���ʽ���Ϣ...\n", FUND_ID, CELL_ID);
	CIDMP_FUND_INFO cInfo;
	nSuccess = trader.GetFund( FUND_ID, CELL_ID, cInfo);
	
	if(nSuccess != 1) {
		printf("��ѯ�ʲ���Ԫ���ʽ���Ϣʧ��\n");
		getchar();
		return 0;
	}
	printf("�ֻ����ʽ�\t: %6.3f\n�ֻ�����\t: %6.3f\n�ڻ����ʽ�\t: %6.3f\n�ڻ�����\t: %6.3f\n\n",
		cInfo.stkTotal, cInfo.stkAvailable, cInfo.futureTotal, cInfo.futureAvailable);
	
	//��ѯ�ֲ���Ϣ
	printf("\n***����ʲ��˻�[%d]�ĳֲ���Ϣ...\n", FUND_ID);
	CIDMP_HOLD_INFO holdInfo[HOLD_SIZE];
	int nSize = trader.GetHold( FUND_ID, 0, "", 2, holdInfo, HOLD_SIZE );
	if(nSize < 0) {
		printf("��ѯ�ʲ��˻��ֲ���Ϣʧ��\n");
		getchar();
		return 0;
	}
	printf("[���]\t����\t����������\t�ֲ�����\t��������\t������\t��ƽ���\t��ƽ���\n");
	for(int i = 0; i < nSize && i < HOLD_SIZE; i ++) {
		printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\n",
			i+1, holdInfo[i].symbol, holdInfo[i].exchgcode, holdInfo[i].holdType, holdInfo[i].available, holdInfo[i].total, 
			holdInfo[i].available_jr, holdInfo[i].available_zr);
	}

	//��ѯ�ʲ���Ԫ�ֲ���Ϣ
	printf("\n***����ʲ��˻�[%d]���ʲ���Ԫ[%d]�ĳֲ���Ϣ...\n", FUND_ID, CELL_ID);
	CIDMP_HOLD_INFO holdInfo1[HOLD_SIZE];
	nSize = trader.GetHold( FUND_ID, CELL_ID, 0, "", 2, holdInfo1, HOLD_SIZE );

	if(nSize < 0) {
		printf("��ѯ�ʲ���Ԫ�ֲ���Ϣʧ��\n");
		getchar();
		return 0;
	}
	printf("[���]\t����\t����������\t�ֲ�����\t��������\t������\t��ƽ���\t��ƽ���\n");
	for(int i = 0; i < nSize && i < HOLD_SIZE; i ++) {
		printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\n",
			i+1, holdInfo1[i].symbol, holdInfo1[i].exchgcode, holdInfo1[i].holdType, holdInfo1[i].available,holdInfo1[i].total, 
			holdInfo1[i].available_jr, holdInfo1[i].available_zr);
	}
	
	//��ѯͶ����ϳֲ���Ϣ
	printf("\n***����ʲ��˻�[%d]���ʲ���Ԫ[%d]��Ͷ�����[%I64d]�ĳֲ���Ϣ...\n", FUND_ID, CELL_ID, PROFL_ID);
	CIDMP_HOLD_INFO holdInfo2[HOLD_SIZE];
	nSize = trader.GetHold( FUND_ID, CELL_ID, PROFL_ID, 0, "", 2, holdInfo2, HOLD_SIZE );

	if(nSize < 0) {
		printf("��ѯͶ����ϳֲ���Ϣʧ��\n");
		getchar();
		return 0;
	}
	
	printf("[���]\t����\t����������\t�ֲ�����\t��������\t������\t��ƽ���\t��ƽ���\n");
	for(int i = 0; i < nSize && i < HOLD_SIZE; i ++) {
		printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\n",
			i+1, holdInfo2[i].symbol, holdInfo2[i].exchgcode, holdInfo2[i].holdType, holdInfo2[i].available, holdInfo2[i].total,
			holdInfo2[i].available_jr, holdInfo2[i].available_zr);
	}
#if 0
	//������������
	CIDMP_ORDER_REQ orderReq;
	orderReq.fundId = FUND_ID;
	orderReq.cellId = CELL_ID;
	orderReq.proflId = PROFL_ID;
	orderReq.direction = DIRECTION;
	orderReq.exchgcode = EXCHANGE;
	orderReq.offsetFlagType = OFFSET;
	orderReq.pj = PJ;
	strcpy(orderReq.symbol, SYMBOL);
	orderReq.orderPrice = INI_PRICE;
	orderReq.orderVol = INI_VOL;
	int riskNum;
#endif
	//Ԥ�������
	//��ȡSymbolList
	printf("***��ȡSymbol List...\n");
	ifstream in;
	in.open(SYMBOL_PATH);
	if (!in){
		printf("***Symbol List ��ȡʧ�ܣ�\n");
		return -1;
	}
	string tmp;
	while (getline(in, tmp))
	{
		
		if (tmp.length() > 3){
			global_symbol.push_back(tmp);
			global_symbol_price[tmp] = make_pair(0, 0);
			//global_event[tmp].first = CEvent(FALSE, FALSE, NULL, NULL);
			//global_event[tmp].second = CEvent(FALSE, FALSE, NULL, NULL);
		}
	}
	CIDMP_STOCK_INFO *info = new CIDMP_STOCK_INFO[6000];
	int stkinfo2 = trader.GetSysStockInfo(false,info,6000);
	CIDMP_FUTRUE_INFO *ftinfo = new CIDMP_FUTRUE_INFO[3000];
	int ftinfoSize = trader.GetSysFutrueInfo(false,ftinfo,3000);

	//��ʼ���ٽ���
	InitializeCriticalSection(&CommandCriticalSection);
	InitializeCriticalSection(&PriceChangeCriticalSection);
	InitializeCriticalSection(&global_cs);
	global_price_ready = false;
	global_vol_ready = false;
	global_trade_ready = false;
	HANDLE priceHandler = CreateThread( NULL, 0, UpdatePrice, NULL, 0, NULL );
	HANDLE volumeHandler = CreateThread( NULL, 0, UpdateOrderVol, NULL, 0, NULL );
	HANDLE logPrintHandler = CreateThread( NULL, 0, LogPrint, NULL, 0, NULL );
	printf("***�ȴ���������׼������...\n");
	while(!global_vol_ready || !global_price_ready){
		Sleep(500);
	}
//	printf("***���н���׼��������\n");

	 TradeCommandEvent.ResetEvent();
		
		//��ʼ������һ�����׽���
		HANDLE traderHandler = CreateThread(NULL, 0, TraderThread, NULL, 0, NULL);
		
		//�ȴ����׽���׼��������Ϣ
		//printf("***Wait For Trade Ready Signal...\n");
		printf("***�ȴ������߳�׼�������ź�\n");
		TradeReadyEvent.ResetEvent();
		WaitForSingleObject(TradeReadyEvent, INFINITE);
		//printf("***Get Trade Ready Signal...\n");
		printf("***���ս����߳�׼�������ź�\n");
		TradeReadyEvent.ResetEvent();
	//	printf("***Trade Thread Get Ready!\n");
		while(true){
			printf("***����Y/y������һ������ָ���������������������...\n");
			char flag;
			scanf("%c", &flag);
			if ( flag != 'Y' && flag != 'y' ){
				break;
			}
			printf("***���������ָ�(���������� ���� �������� ��ƽ���� �������� �Ƿ�ƽ�� ����ʱ��/secs)\n");
			getchar();

			//debug
			//	scanf( "%d%s%d%d%d%d%d", &exchange, &symbol, &direction, &offset, &volumn, &pj, &trade_secs );
			EnterCriticalSection(&CommandCriticalSection);
			trader_command.fund_id = FUND_ID; trader_command.cell_id = CELL_ID; trader_command.profl_id = PROFL_ID;
			trader_command.exchange_code = 6; strcpy(trader_command.symbol, "IF1409"); trader_command.direction = 0; trader_command.offset_flag_type = 0; trader_command.order_volumn = 1; trader_command.pj = 0; trader_command.max_seconds = 600;
			LeaveCriticalSection(&CommandCriticalSection);
			//���ͽ���ָ��
			printf("***���ͽ���ָ���ź�\n");
			TradeCommandEvent.SetEvent();
			CreateThread(NULL, 0, TraderThread, NULL, 0, NULL);
			printf("***�ȴ������߳�׼�������ź�\n");
			WaitForSingleObject(TradeReadyEvent, INFINITE);
			TradeReadyEvent.ResetEvent();
		}
#if 0
		CIDMP_ORDER_REQ order;
		order.fundId = FUND_ID;
		order.cellId = CELL_ID;
		order.proflId = PROFL_ID;
		order.exchgcode = exchange;
		strcpy(order.symbol, symbol);
		order.direction = direction;
		order.offsetFlagType = offset;
		order.orderPrice = 0;
		order.orderVol = volumn;
		order.pj = pj;
		TraderThreadParameter trader_para;
		trader_para.pTrader = &trader;
		trader_para.orderReq = &order;
		trader_para.maxSecs = trade_secs;
		trader_para.nTime = N_TIME;
		int minGap = 0;
		if (exchange < 3){
			for (int i = 0; i < stkinfo2; i++){
				if ( strcmp( info[i].symbol, orderReq.symbol ) == 0 ){
					minGap = info[i].priceUnit;
					break;
				}
			}
		}else{
			for (int i = 0; i < ftinfoSize; i++){
				if ( strcmp( ftinfo[i].symbol, orderReq.symbol ) == 0 ){
					minGap = ftinfo[i].priceUnit;
					break;
				}
			}
		}
		if (direction == 1){
			minGap = 0 - minGap;
		}
		trader_para.minGap = minGap;
		trader_para.finalRatio = FINAL_RATIO;

		HANDLE traderHandler = CreateThread( NULL, 0, TraderThread, &trader_para, 0, NULL );
#endif
	fclose(stream);
	delete []ftinfo;
	delete []info;
#if 0 
	printf("�Ƿ�ʼ�µ���(ȷ��������Y/y�س���������������\n)");
	char start_order = 0;
	scanf("%c", &start_order);
	if ( start_order != 'Y' && start_order != 'y' ){
		return 0;
	}
	//��������
	printf( "\n***�������㣺 �ʲ��˻�[%d],�ʲ���Ԫ[%d],Ͷ�����[%I64d]\n",FUND_ID,CELL_ID,PROFL_ID );
	nSuccess = trader.RiskTest( orderReq, riskNum );
	printf("ί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", orderReq.symbol, orderReq.direction, orderReq.offsetFlagType, orderReq.orderPrice,
		orderReq.orderVol);
	printf("�ӿڷ���ֵ:%d\n",nSuccess);
	printf("���������Ŀ:%d\n",riskNum);

	if ( nSuccess < 0 ){
		return 0;
	}




	TraderThreadParameter thread_para;
	thread_para.pTrader = &trader;
	thread_para.orderReq = &orderReq;
	//*(thread_para.pTrader) = trader;
	//*(thread_para.orderReq) = orderReq;
	thread_para.maxSecs = MAX_SECONDS;
	thread_para.nTime = N_TIME;
	thread_para.finalRatio = FINAL_RATIO;
	thread_para.minGap = minGap;
	thread_para.orderStatus = 0;
	


	HANDLE traderHandler = CreateThread( NULL, 0, TraderCore, &thread_para, 0, NULL );

//	nSuccess = TraderCore( trader, orderReq, MAX_SECONDS, N_TICKS, FINAL_RATIO, minGap );
	while (thread_para.orderStatus == 0){
		Sleep(5000);
	}
	switch (thread_para.orderStatus){
	case -1:
		printf("***����ʧ�ܣ�δ֪����\n");
		break;;
	case 1:
		printf("***����ʧ�ܣ��µ���ʱ��\n");
		break;
	case 2:
		printf("***���׳ɹ���\n");
		break;
	default:
		break;
	}
	//CloseHandle(TraderCore);

/*	if (nSuccess < 0){
		printf("***����ʧ�ܣ�ʧ��ԭ�򣺷�ؼ��ʧ�ܣ�\n");
	}else if (nSuccess == 0){
		printf("***����ʧ�ܣ�ʧ��ԭ�򣺽��׳�ʱ��\n");
	}else{
		printf("***���׳ɹ���\n");
	}
	CIDMP_ORDER_INFO orderInfo;*/
//	GetRealTrade( trader, orderReq, orderInfo );
//	printf("***����������������µ�����\n");
#endif 
	system("PAUSE");
	return 0;
}
DWORD WINAPI TraderThread( void *para )
{
	printf("***%d\tTrader Thread Begin...\n", GetCurrentThreadId());
	//��½������
	char* serverIP = new char[16];
	strcpy(serverIP, SERVER_IP);
	int serverPort = SERVER_PORT;
	char* userID = new char[10];
	strcpy(userID, USER_NAME);
	char* userPW = new char[10];
	strcpy(userPW, USER_PASSWORD);
	CIDMPTradeApi trader;
	CIDMP_ORDER_INFO info;
	int nSuccess = trader.Connect(serverIP, serverPort, userID, userPW, 0);
	if (nSuccess != 1){
		printf("***%d\tTrader Thread Login Failed!\n", GetCurrentThreadId());
		getchar();
		return -1;
	}
	printf("***%d\tTrader Thread Login Done...\n", GetCurrentThreadId());
	printf("***%d\t�����߳�׼�������ź�...\n", GetCurrentThreadId());
	TradeReadyEvent.SetEvent();
	
	//�ȴ������ź�
	printf("***%d\t�ȴ�����ָ���ź�\n", GetCurrentThreadId());
	WaitForSingleObject(TradeCommandEvent, INFINITE);
	printf("***%d\t���ս���ָ���ź�\n", GetCurrentThreadId());
	TradeCommandEvent.ResetEvent();
	
	//�����ٽ�������ȡ����ָ��
	EnterCriticalSection(&CommandCriticalSection);
	CIDMP_ORDER_REQ orderReq;
	orderReq.fundId = trader_command.fund_id;
	orderReq.cellId = trader_command.cell_id;
	orderReq.proflId = trader_command.profl_id;
	orderReq.exchgcode = trader_command.exchange_code;
	strcpy(orderReq.symbol, trader_command.symbol);
	orderReq.direction = trader_command.direction;
	orderReq.offsetFlagType = trader_command.offset_flag_type;
	orderReq.orderVol = trader_command.order_volumn;
	orderReq.pj = trader_command.pj;
	int maxSecs = trader_command.max_seconds;
	LeaveCriticalSection(&CommandCriticalSection);

	//��ȡ���½��׼۸�
	orderReq.orderPrice = orderReq.direction == 0 ? global_symbol_price[orderReq.symbol].first : global_symbol_price[orderReq.symbol].second;
	printf("***%d\t��ʼ����...\n", GetCurrentThreadId());
	//�µ�
	int riskNum;

	LogMessage oneMsg;
	oneMsg.server_no = -1; oneMsg.cmd_status = INSERT_SEND; oneMsg.server_status = -1; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = 0; oneMsg.cancel_vol = 0; oneMsg.price = orderReq.orderPrice;
	logMsgQueue.push(oneMsg);
	//֪ͨprint���̴�ӡ
	PrintLogEvent.SetEvent();
	
	nSuccess = trader.OrderInsert(orderReq, riskNum);

	trader.GetOrderByOrderNo(orderReq.orderNo, info);
	oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = INSERT_RETURN; oneMsg.cmd_return = nSuccess, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = 0; oneMsg.cancel_vol = 0; oneMsg.price = orderReq.orderPrice;
	logMsgQueue.push(oneMsg);
	PrintLogEvent.SetEvent();

	if (nSuccess < 0){
		printf("***%d\t�µ�ʧ�ܣ�\n", GetCurrentThreadId());
		printf("***%s\n", trader.GetLastOrderInsertError());
		return -1;
	}
	//ȷ���µ��ɹ�
	trader.GetOrderByOrderNo(orderReq.orderNo, info);
	while ( info.orderStatus < 3 ){
		Sleep(100);
		trader.GetOrderByOrderNo(orderReq.orderNo, info);
	}
	
	oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = UPDATE_ORDER, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = info.tradeVol; oneMsg.cancel_vol = info.canceledVol; oneMsg.price = orderReq.orderPrice;
	logMsgQueue.push(oneMsg);
	PrintLogEvent.SetEvent();

	if (info.orderStatus == 3){
		printf("***%d\t�µ�ʧ�ܣ�\n", GetCurrentThreadId());
		printf("***%s\n", trader.GetLastOrderInsertError());
		return -1;
	}
	printf("***%d\t�µ��ɹ����µ��۸� = %3.7f\n", GetCurrentThreadId(), orderReq.orderPrice);
	global_order_NO.push_back(orderReq.orderNo);
	global_volumn[orderReq.orderNo] = orderReq.orderVol;

	//��ʼ����ѭ��
//	CEvent& priceEvent = orderReq.direction == 0 ? global_event[orderReq.symbol].first : global_event[orderReq.symbol].second;
	clock_t start_time = clock();
	bool getTrade = false;
	PriceChangeEvent.ResetEvent();
	while (clock() - start_time <= maxSecs * 1000){
		if (WaitForSingleObject(PriceChangeEvent, 3000) == WAIT_TIMEOUT ){
			printf("Heart Beat...%d\n", clock());
			//trader.GetOrderByOrderNo(orderReq.orderNo, info);
			if (global_volumn[orderReq.orderNo] == 0){
				printf("***%d\t�����ɽ�!\n", GetCurrentThreadId());
				getTrade = true;
				break;
			}
			continue;
		}else{
			if (global_volumn[orderReq.orderNo] == 0){
				printf("***%d\t�����ɽ�!\n", GetCurrentThreadId());
				getTrade = true;
				break;
			}
			printf("***%d\t���ռ۸�仯�ź�\n", GetCurrentThreadId());
			PriceChangeEvent.ResetEvent();
			//ȷ�ϼ۸�仯�Ƿ��뱾�������
			EnterCriticalSection(&PriceChangeCriticalSection);
			if ( strcmp(price_change_symbol, orderReq.symbol) != 0 || price_change_direction != orderReq.direction ){
				printf("***%d\t�۸�仯�뱾�����޹أ������ȴ�\n", GetCurrentThreadId());
				continue;
			}
			LeaveCriticalSection(&PriceChangeCriticalSection);
			
			//����
			char errorMsg[200];
			trader.GetOrderByOrderNo(orderReq.orderNo, info);
			
			oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = CANCEL_SEND, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = info.tradeVol; oneMsg.cancel_vol = info.canceledVol; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			nSuccess = trader.CancelOrder(orderReq.orderNo, errorMsg);
			//ȷ�ϳ����ɹ�
			trader.GetOrderByOrderNo(orderReq.orderNo, info);
			oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = CANCEL_RETURN, oneMsg.cmd_return = nSuccess, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = info.tradeVol; oneMsg.cancel_vol = info.canceledVol; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			while (info.orderStatus < 8){
				Sleep(100);
				trader.GetOrderByOrderNo(orderReq.orderNo, info);
			}
			printf("***%d\t������ = %d, �����ɹ���\n", GetCurrentThreadId(), orderReq.orderNo);
			
			oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = UPDATE_ORDER, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = info.tradeVol; oneMsg.cancel_vol = info.canceledVol; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			//���¼۸�
			orderReq.orderPrice = orderReq.direction == 0 ? global_symbol_price[orderReq.symbol].first : global_symbol_price[orderReq.symbol].second;
			printf("***%d\t�����۸�仯�����¼۸� = %3.7f\n", GetCurrentThreadId(), orderReq.orderPrice);

			//�µ�
			int riskNum;
		
			oneMsg.server_no = -1; oneMsg.cmd_status = INSERT_SEND; oneMsg.server_status = -1; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = 0; oneMsg.cancel_vol = 0; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			int nSuccess = trader.OrderInsert(orderReq, riskNum);
			
			oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = INSERT_RETURN; oneMsg.cmd_return = nSuccess, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = 0; oneMsg.cancel_vol = 0; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			if (nSuccess < 0){
				printf("***%d\t�µ�ʧ�ܣ�\n", GetCurrentThreadId());
				printf("***%s\n", trader.GetLastOrderInsertError());
				return -1;
			}
			//ȷ���µ��ɹ�
			trader.GetOrderByOrderNo(orderReq.orderNo, info);
			while ( info.orderStatus < 3 ){
				Sleep(100);
				trader.GetOrderByOrderNo(orderReq.orderNo, info);
			}

			oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = UPDATE_ORDER, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = info.tradeVol; oneMsg.cancel_vol = info.canceledVol; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			if (info.orderStatus == 3){
				printf("***%d\t�µ�ʧ�ܣ�\n", GetCurrentThreadId());
				printf("***%s\n", trader.GetLastOrderInsertError());
				return -1;
			}
			printf("***%d\t�µ��ɹ����µ��۸� = %3.7f\n", GetCurrentThreadId(), orderReq.orderPrice);
			global_order_NO.push_back(orderReq.orderNo);
			global_volumn[orderReq.orderNo] = orderReq.orderVol;
		}


	}
	if (!getTrade){
		printf("***%d\t������ʱ...\n", GetCurrentThreadId());
	}
	printf("***%d\tTrader Thread End...\n", GetCurrentThreadId());
	return 0;
}
#if 0
DWORD WINAPI TraderThread( void *para )
{
	printf("***Start Trader Thread...\n");
	FILE *stream = NULL;
	stream = fopen( LOG_PATH.c_str(), "w" );

	if (stream == NULL){
		getchar();
	}
	//fprintf( stream, "ʱ��\t���ر��\t�������\t״̬\t����\t�ɽ���\tί����\t������\t�۸�\n" );
	FILE *action_stream;
	action_stream = fopen( ACTION_PATH.c_str(), "r" );
	int no = 0;
	int action_no;
	while (fscanf( action_stream, "%d", &no ) != EOF){
		action_no = no;
	}
	action_no++;
	fclose(action_stream);

	//ǿ��ת������
	CIDMP_ORDER_INFO info;
	TraderThreadParameter *thread_para = (TraderThreadParameter *)para;
	CIDMPTradeApi *pTrader = thread_para->pTrader;
	CIDMP_ORDER_REQ *pOrder = thread_para->orderReq;
	pOrder->orderNo = -1;
	int maxSeconds = thread_para->maxSecs;
	int nTime = thread_para->nTime;
	double finalRatio = thread_para->finalRatio;
	double minGap = thread_para->minGap;
	//���ö����۸�
	if (pOrder->direction == 0){
		pOrder->orderPrice = global_symbol_price[pOrder->symbol].first;
	}else{
		pOrder->orderPrice = global_symbol_price[pOrder->symbol].second;
	}

	int riskNum = 0;
	
	printf("ί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);

	//�ύ����ί��
	OrderInsert(pTrader, pOrder, &info, stream, action_no);
	global_order_NO.push_back(pOrder->orderNo);
	clock_t start_time = clock();
	while ( clock() - start_time < maxSeconds * 1000 ){
		//�ȴ��źŴ���
		printf("***Wait Signal...\n");
		DWORD nSuccess = WaitForSingleObject(pOrder->direction == 0 ? global_event[pOrder->symbol].first : global_event[pOrder->symbol].second, 3000);
		if (nSuccess == WAIT_TIMEOUT){
			//������¼۸���Ϣ
			printf("***Heart Beat...%3.7f\n", pOrder->direction == 0 ? global_symbol_price[pOrder->symbol].first : global_symbol_price[pOrder->symbol].second);
		}else{
			OrderCancel(pTrader, pOrder, &info, stream, action_no);
			pOrder->orderVol = global_volumn[pOrder->orderNo];
			if (pOrder->orderVol == 0){
				break;
			}
			if (pOrder->direction == 0){
				pOrder->orderPrice = global_symbol_price[pOrder->symbol].first;
			}else{
				pOrder->orderPrice = global_symbol_price[pOrder->symbol].second;
			}
			
			OrderInsert(pTrader, pOrder, &info, stream, action_no);
			global_order_NO.push_back(pOrder->orderNo);
		}
	}
	return 0;
}
#endif
DWORD WINAPI TraderCore( LPVOID pParam )
{
	FILE *stream = NULL;
	stream = fopen( LOG_PATH.c_str(), "w" );

	if (stream == NULL){
		getchar();
	}
	//fprintf( stream, "ʱ��\t���ر��\t�������\t״̬\t����\t�ɽ���\tί����\t������\t�۸�\n" );
	FILE *action_stream;
	action_stream = fopen( ACTION_PATH.c_str(), "r" );
	int no = 0;
	int action_no;
	while (fscanf( action_stream, "%d", &no ) != EOF){
		action_no = no;
	}
	action_no++;
	fclose(action_stream);
	//ǿ��ת������
	TraderThreadParameter *thread_para = (TraderThreadParameter *)pParam;
	CIDMPTradeApi *pTrader = thread_para->pTrader;
	CIDMP_ORDER_REQ *pOrder = thread_para->orderReq;
	pOrder->orderNo = -1;
	int maxSeconds = thread_para->maxSecs;
	int nTime = thread_para->nTime;
	double finalRatio = thread_para->finalRatio;
	double minGap = thread_para->minGap;
	int orderStatus = thread_para->orderStatus;
	

	//��ʼ״̬����
	bool getTrade = false;
	bool orderInMarket = false;
	double priceInOrder = 0;
	double lastPrice = 0;
	int count_time = 0;
	clock_t start_time = clock();
	CIDMP_ORDER_INFO orderInfo;
	orderInfo.tradeVol = 0;
	orderInfo.canceledVol = 0;
	orderInfo.orderVol = 0;
	orderInfo.orderPrice = 0;
	orderInfo.orderStatus = -1;
	CIDMP_ORDER_INFO* pInfo;
	pInfo = &orderInfo;
	int riskNum = 0;
	int nSuccess = 0;
	CIDMP_TICK_QUOTATION_QUERY *realQuery = new CIDMP_TICK_QUOTATION_QUERY[1];
	CIDMP_TICK_QUOTATION_INFO *realInfo = new CIDMP_TICK_QUOTATION_INFO[1];
	realQuery->exchgcode = pOrder->exchgcode;
	strcpy( realQuery->symbol, pOrder->symbol );


	while (!getTrade){
		//ÿ3s���һ�����¼۸�
		clock_t cur_time = clock();
		if ( cur_time - start_time > count_time + 3000){
			printf("***Heart Beat...%d\n",cur_time - start_time);
			printf("***���¼۸�%6.3f\n", lastPrice);
			count_time = cur_time - start_time;
			printf("ί�к�\t״̬\t֤ȯ/��Լ����\t����\t��ƽ\t�۸�\t����\t������\t������\tʱ��\t����Ա\t������Ϣ\n");
			UpdateOrder(pTrader,pOrder,pInfo,stream,action_no);
			printf("%d\t%d\t%s\t%d\t%d\t%6.3f\t%d\t%d\t%d\t%s\t%s\t%s\n",
					pInfo->orderNo, pInfo->orderStatus, pInfo->symbol, pInfo->direction, pInfo->offsetFlagType,
					pInfo->orderPrice, pInfo->orderVol, pInfo->tradeVol, pInfo->canceledVol, pInfo->orderTime, pInfo->userId, pInfo->errorInfo);	
		}

		//TimeTest
		if ( cur_time - start_time >= 1000 * maxSeconds * ( 1 - finalRatio ) ){
			//�������ղ���
			printf("***Go To Final Step...\n");
			int result = GotoFinalStep( pTrader, pOrder, maxSeconds, nTime, finalRatio, minGap, stream, action_no );
			if ( result == 1 ){
				printf("***����ʱ...\n");
				thread_para->orderStatus = 1;
			}else if ( result == 0 ){
				thread_para->orderStatus = 2;
				printf("***���׳ɹ�...\n");
			}else{
				printf("***����ʧ��...\n");
				thread_para->orderStatus = -1;
			}
			break;
		}else{
			//��ȡ���¼۸�
			pTrader->GetTickQuotation( realQuery, realInfo, 1 );
			if ( pOrder->direction == 0 ){		//�����������Ϊ��
				lastPrice = realInfo[0].bp1;			//��ȡ������һ��			
			}else{
				lastPrice = realInfo[0].sp1;			//��ȡ������һ��
			}//if
			if (orderInMarket && pInfo->orderStatus != 7){	//������ж���
//				printf("OrderInMarket\n");
				if ( lastPrice != priceInOrder ){
					printf("ԭ�� = %6.3f, �ּ� = %6.3f\n", priceInOrder, lastPrice);
//					printf("PriceChange\n");
					OrderCancel( pTrader, pOrder, pInfo, stream, action_no );
					orderInMarket = false;
					getTrade = UpdateOrder( pTrader, pOrder, pInfo,stream, action_no );
					if (getTrade){
						continue;
					}
				}else{
//					printf("PriceUnchange\n");
				getTrade = UpdateOrder( pTrader, pOrder, pInfo,stream, action_no );
				continue;
				}//if
			}//if
			//InsertOrder
			//�������µĳɽ���Ϣ��ί�ж�����
			if (pOrder->orderPrice != lastPrice)
			{
				pOrder->orderPrice = lastPrice;
				printf("**���¶����۸����¼۸�=%6.3f\n", lastPrice);
			}
 			getTrade = UpdateOrder( pTrader, pOrder, pInfo, stream, action_no );
			printf("ί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);
			action_no++;
			nSuccess = OrderInsert( pTrader, pOrder, pInfo, stream, action_no );
//			nSuccess = pTrader->OrderInsert( *pOrder, riskNum );
			if ( nSuccess < 0 ){
				printf("%s\n", pTrader->GetLastOrderInsertError());
				switch ( nSuccess ){
				case -1:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡���δ��¼��\n");
					break;
				case -2:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡�����ѯ��ˮ��ʧ�ܡ�\n");
					break;
				case -3:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡������δͨ����\n");
					break;
				case -4:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡����µ�ʧ�ܡ�\n");
					break;
				case -5:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡���fundId��cellId�����ڻ�δ����Ȩ��\n");
					break;
				default:
					break;
				}
				thread_para->orderStatus = -1;
				break;
			}else{
				if (nSuccess == 0){
					thread_para->orderStatus = 1;
					break;
				}
		//		printf( "***������ί�гɹ���\n" );
				orderInMarket = true;
				priceInOrder = lastPrice;
			}//if
		}//if

	}//while
	if (getTrade){
		printf("***���׳ɹ���\n");
		thread_para->orderStatus = 2;
	}
	fclose( stream );
	return 1;
}

inline bool UpdateOrder( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE* stream, int action_no )
{
	//debug
	if (PROCED_DEBUG){
		printf("UpdataOrder\n");
	}
	

	int trade = pInfo->tradeVol;
	int cancel = pInfo->canceledVol;
	int sta = pInfo->orderStatus;
	if (pOrder->orderNo >= 0){
		pTrader->GetOrderByOrderNo( pOrder->orderNo, *pInfo );
	}else{
		return false;
	}
	if ( trade != pInfo->tradeVol || cancel != pInfo->canceledVol || pInfo->orderStatus != sta ){
		char* status = new char[10];
		GetCharStatus( pInfo->orderStatus, status );
		time_t rawtime;
		time(&rawtime);
		fprintf( stream, "ʱ�� = %s���ر�� = %d��������� = %d��ָ��״̬ = %s��������״̬ = %s���������� = %d����ƽ���� = %d������ = %d���ɽ��� = %d��ί���� = %d�������� = %d���۸�= %6.3f\n", 
			asctime(localtime(&rawtime)), action_no, pOrder->orderNo, "Update Order", status, pOrder->direction, pOrder->offsetFlagType,
			pInfo->orderVol, pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice );
		delete[] status;
	}
	if ( pInfo->orderStatus == 8 || pInfo->orderStatus == 5){
		pOrder->orderVol = pInfo->orderVol - pInfo->tradeVol;
//		PrintLogMessage( pTrader, pOrder, pInfo, stream, action_no, pInfo->orderStatus == 5 );
	}
	if ( pInfo->orderStatus == 6 ){
		//time_t rawtime;
	//	time(&rawtime);
		//fprintf(stream, "ʱ��:%s �����ɽ���\nί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", asctime(localtime(&rawtime)),pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);
//		PrintLogMessage( pTrader, pOrder, pInfo, stream, action_no, true );
		if ( pInfo->tradeVol > 0 ){
			FILE *trans_stream = NULL;
			trans_stream = fopen( TRANS_LOG_PATH.c_str(), "a" );
			time_t rawtime;
			time(&rawtime);
			global_trade_vol += pInfo->tradeVol;
			global_trade_mount += pInfo->tradeVol * pInfo->orderPrice;
			fprintf( trans_stream, "ʱ�� = %s���ر�� = %d���������� = %d���ɽ��� = %d���ɽ��۸� = %6.3f���ѳɽ����� = %6.3f\n", 
				asctime(localtime(&rawtime)), action_no, INI_VOL, pInfo->tradeVol, 
				pInfo->orderPrice, global_trade_mount/global_trade_vol );
			fclose(trans_stream);
		}
	}
	if (PROCED_DEBUG){
		printf("End UpdataOrder\n");
	}
	
	return pInfo->orderStatus == 6;
}

int OrderCancel( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE *stream, int action_no )
{
	//debug
	if (CANCEL_DEBUG){
		printf("OrderCancel\n");
	}
	if ( pInfo->tradeVol > 0 ){
		FILE *trans_stream = NULL;
		trans_stream = fopen( TRANS_LOG_PATH.c_str(), "a" );
		time_t rawtime;
		time(&rawtime);
		global_trade_vol += pInfo->tradeVol;
		global_trade_mount += pInfo->tradeVol * pInfo->orderPrice;
		fprintf( trans_stream, "ʱ�� = %s���ر�� = %d���ɽ��� = %d���ɽ��۸� = %6.3f���ѳɽ����� = %6.3f\n", asctime(localtime(&rawtime)), action_no, pInfo->tradeVol, 
			pInfo->orderPrice, global_trade_mount/global_trade_vol );
		fclose(trans_stream);
	}
	char errorMsg[200];
	bool cancelSuccess = false;
	while (!cancelSuccess){
		//PrintLogMessage( pTrader, pOrder, pInfo, stream, action_no, false );
		time_t rawtime;
		time(&rawtime);
		UpdateOrder( pTrader, pOrder, pInfo, stream, action_no );
		char* status = new char[10];
		GetCharStatus( pInfo->orderStatus, status );
		fprintf( stream, "ʱ�� = %s���ر�� = %d����������� = %d��ָ��״̬ = %s��������״̬ = %s���������� = %d����ƽ���� = %d������ = %d���ɽ��� = %d��ί���� = %d�������� = %d���۸�= %6.3f\n", 
			asctime(localtime(&rawtime)), action_no, pOrder->orderNo, "Cancel Send", status, pOrder->direction, pOrder->offsetFlagType,  
			pInfo->orderVol, pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice );
		int nSuccess = pTrader->CancelOrder( pOrder->orderNo, errorMsg );
		UpdateOrder( pTrader, pOrder, pInfo, stream, action_no );
		GetCharStatus( pInfo->orderStatus, status );
		time(&rawtime);
		fprintf( stream, "ʱ�� = %s���ر�� = %d����������� = %d��ָ��״̬ = %s��ָ��� = %d��������״̬ = %s���������� = %d����ƽ���� = %d������ = %d���ɽ��� = %d��ί���� = %d�������� = %d���۸�= %6.3f\n", 
			asctime(localtime(&rawtime)), action_no, pOrder->orderNo, "Cancel Return", nSuccess, status, pOrder->direction, pOrder->offsetFlagType, pInfo->orderVol, pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice );
		delete []status;
		if ( pInfo->orderStatus >= 7 ){
			printf("***�����ɹ���...\n");
			UpdateOrder( pTrader, pOrder, pInfo, stream, action_no );
		//	time_t rawtime;
		//	time(&rawtime);
		//	fprintf(stream, "ʱ��:%s ���������ɹ���\nί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", asctime(localtime(&rawtime)),pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);
		//	PrintLogMessage( pTrader, pOrder, pInfo, stream, action_no, false );
			cancelSuccess = true;
			return 0;
		}
		
		if ( pInfo->orderVol = pInfo->tradeVol + pInfo->canceledVol && pInfo->tradeVol > 0 ){
			cancelSuccess = true;
			printf("***����ʧ�ܣ�ʧ��ԭ��ί���ѳɽ�...\n");
			time(&rawtime);
			UpdateOrder( pTrader, pOrder, pInfo, stream, action_no );
			status = new char[10];
			GetCharStatus( pInfo->orderStatus, status );
			fprintf( stream, "ʱ�� = %s���ر�� = %d����������� = %d��ָ��״̬ = %s��������״̬ = %s���������� = %d����ƽ���� = %d������ = %d���ɽ��� = %d��ί���� = %d�������� = %d���۸�= %6.3f\n", 
				asctime(localtime(&rawtime)), action_no, pOrder->orderNo, "Cancel False", status, pOrder->direction, pOrder->offsetFlagType, pInfo->orderVol, pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice );
			delete[] status;
			return 1;
		}
		Sleep(50);
	}
	if (CANCEL_DEBUG){
		printf("End OrderCancel\n");
	}
}
int OrderInsert( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE* stream, int action_no )
{
	//debug
	if (INSERT_DEBUG){
		printf("OrderInsert\n");
	}
	FILE* action_stream;
	action_stream = fopen( ACTION_PATH.c_str(), "a" );
	fprintf( action_stream, "\n%d", action_no );
	fclose(action_stream);
	bool missionFailed = false;
	int riskNum = 0;
	//PrintLogMessage( pTrader, pOrder, pInfo, stream, action_no, true );
	time_t rawtime;
	time(&rawtime);
	char* status = new char[10];
	GetCharStatus( pInfo->orderStatus, status );
	fprintf( stream, "ʱ�� = %s���ر�� = %d����������� = %d��ָ��״̬ = %s��������״̬ = %s���������� = %d����ƽ���� = %d������ = %d���ɽ��� = %d��ί���� = %d�������� = %d���۸�= %6.3f\n", 
		asctime(localtime(&rawtime)), action_no, pOrder->orderNo, "Insert Send", status, pOrder->direction, pOrder->offsetFlagType, 
		pInfo->orderVol, pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice );
	int nSuccess = pTrader->OrderInsert( *pOrder, riskNum );
	time(&rawtime);
	UpdateOrder( pTrader, pOrder, pInfo, stream, action_no );
	GetCharStatus( pInfo->orderStatus, status );
	fprintf( stream, "ʱ�� = %s���ر�� = %d����������� = %d��ָ��״̬ = %s��ָ��� = %d��������״̬ = %s���������� = %d����ƽ���� = %d������ = %d���ɽ��� = %d��ί���� = %d�������� = %d���۸�= %6.3f\n", 
		asctime(localtime(&rawtime)), action_no, pOrder->orderNo, "Insert Send", nSuccess, status, pOrder->direction, pOrder->offsetFlagType, 
		pInfo->orderVol, pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice );
	delete []status;
//	while (true){
		if ( nSuccess < 0 ){
			printf("%s\n", pTrader->GetLastOrderInsertError());
			switch ( nSuccess ){
			case -1:
				printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡���δ��¼��\n");
				break;
			case -2:
				printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡�����ѯ��ˮ��ʧ�ܡ�\n");
				break;
			case -3:
				printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡������δͨ����\n");
				break;
			case -4:
				printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡����µ�ʧ�ܡ�\n");
				break;
			case -5:
				printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡���fundId��cellId�����ڻ�δ����Ȩ��\n");
				break;
			default:
				break;
			}
				printf("ί�к�\t״̬\t֤ȯ/��Լ����\t����\t��ƽ\t�۸�\t����\tʱ��\t����Ա\t������Ϣ\n");
				printf("%d\t%d\t%s\t%d\t%d\t%6.3f\t%d\t%s\t%s\t%s\n",
					pInfo->orderNo, pInfo->orderStatus, pInfo->symbol, pInfo->direction, pInfo->offsetFlagType,
					pInfo->orderPrice, pInfo->orderVol, pInfo->orderTime, pInfo->userId, pInfo->errorInfo);				
	//		continue;
			return nSuccess;
		}
		clock_t start_time = clock();
		while (true){
			UpdateOrder( pTrader, pOrder, pInfo,stream, action_no );
			if ( pInfo->orderStatus > 3 ){
				break;
			}
			if ( clock() - start_time > MAX_INSERT_SECS * 1000 ){
				missionFailed = true;
				break;
			}
		}
//	}
	if (missionFailed){
		printf("***�µ���ʱ��\n");
		return 0;
	}else{
		printf( "***������ί�гɹ������¼۸�=%6.3f\n", pOrder->orderPrice );
		//time_t rawtime;
		//time(&rawtime);
	//	PrintLogMessage( pTrader, pOrder, pInfo, stream, action_no, true );
		//fprintf(stream, "ʱ��:%s ����ί�гɹ���\nί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", asctime(localtime(&rawtime)),pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);
	}
	if (INSERT_DEBUG){
		printf("End OrderInsert\n");
	}
	return 1;
}

inline void GetCharStatus( int sta, char* data )
{
	//debug
	if (PROCED_DEBUG){
		printf("GetCharStatus\n");
	}

	switch (sta){
	case 1:
		strcpy( data, "δ��\0" );
//			data = "δ��\0";
		break;
	case 2:
		strcpy( data, "����\0" );	
		//data = "����\0";
		break;
	case  3:
		strcpy( data, "�ϵ�\0" );
//		data = "�ϵ�\0";
		break;
	case 4:
		strcpy( data, "�ѱ�\0" );
//		data = "�ѱ�\0";
		break;
	case  5:
		strcpy( data, "���ֳɽ�\0" );
//		data = "���ֳɽ�\0";
		break;
	case  6:
		strcpy( data, "ȫ���ɽ�\0" );
//		data = "ȫ���ɽ�\0";
		break;
	case  7:
		strcpy( data, "�ѱ�����\0" );
//		data = "�ѱ�����\0";
		break;
	case  8:
		strcpy( data, "��������\0" );	
//		data = "��������\0";
		break;
	case  9:
		strcpy( data, "��������\0" );
//		data = "��������\0";
		break;
	default:
		strcpy( data, "��ʼ״̬\0" );
		break;
	}
	if (PROCED_DEBUG){
		printf("End GetCharStatus\n");
	}
}
int GotoFinalStep( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, int maxSeconds, int nTime, double finalRatio, double minGap, FILE *stream, int action_no )
{
	clock_t start_time = clock();
	CIDMP_ORDER_INFO info;
	pTrader->GetOrderByOrderNo( pOrder->orderNo, info );
	bool getTrade = false;
	//�������г����е����򳷵�
	if ( info.orderVol > info.tradeVol + info.canceledVol ){
		OrderCancel( pTrader, pOrder, &info, stream, action_no );
	}
	while ( (clock() - start_time < maxSeconds * finalRatio * 1000) && !getTrade ){
		//���ö����۸�
		pOrder->orderPrice += minGap;
		for ( int i = 0; i < nTime; i++ ){
			//�µ�
			int nSuccess = OrderInsert( pTrader, pOrder, &info, stream, action_no );
			if ( nSuccess == 0){
				return 0;
			}else if ( nSuccess < 0 ){
				return -1;
			}

			//sleep
			//Sleep(100);
			while (true){
				UpdateOrder( pTrader, pOrder, &info, stream, action_no );
				if ( info.orderStatus >= 4 ){
					break;
				}
			}
			//����
			OrderCancel( pTrader, pOrder, &info, stream, action_no );
			getTrade = UpdateOrder( pTrader, pOrder, &info, stream, action_no );
			if (getTrade){
				break;
			}
		}
	}
	if (getTrade){
		return 1;
	}else{
		return 0;
	}

}

inline void PrintLogMessage( CIDMPTradeApi *pTader, CIDMP_ORDER_REQ *pOrder, CIDMP_ORDER_INFO *pInfo, FILE *stream, int actNo, bool insert )
{
	if (pOrder->orderNo < 0){
		return;
	}
	time_t rawtime;
	time(&rawtime);
	fprintf( stream, "%s\t%d\t\t%d\t\t%s\t%d\t%d\t%d\t%d\t%6.3f\n", asctime(localtime(&rawtime)), actNo, pOrder->orderNo, (insert?("�µ�"):("����")), pInfo->orderVol, 
		pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice);
}

DWORD WINAPI UpdateOrderVol( void *para )
{
	printf("***Start Update Order Volumn Thread...\n");
	//��½������
	char* serverIP = new char[16];
	strcpy(serverIP, SERVER_IP);
	int serverPort = SERVER_PORT;
	char* userID = new char[10];
	strcpy(userID, USER_NAME);
	char* userPW = new char[10];
	strcpy(userPW, USER_PASSWORD);
	CIDMPTradeApi trader;
	CIDMP_ORDER_INFO info;
	int nSuccess = trader.Connect(serverIP, serverPort, userID, userPW, 0);
	if (nSuccess != 1){
		printf("***Update Volumn Thread Login Failed!\n");
		getchar();
		return -1;
	}
	global_vol_ready = true;
	printf("***Update Order Volumn Thread Ready...\n");
	while (true)
	{
//		printf("***Check Order Volumn...\n");
//		printf("%d", global_order_NO.size());
		if (global_order_NO.size() == 0){
			continue;
		}
	//	printf("%d\t", global_order_NO.size());
		for ( list<int>::iterator it = global_order_NO.begin(); it != global_order_NO.end();  ){
			
			int order_no = *it;
			trader.GetOrderByOrderNo( order_no, info );
			if ( info.orderStatus == 3 || info.orderStatus > 5 ){
				//ɾ����orderNo��Ϣ
				global_order_NO.erase(it++);
				global_volumn.erase(order_no);
			}else{
				it++;
				global_volumn[order_no] = info.orderVol - info.tradeVol;
	//			printf("%d\t", global_volumn[order_no]);
			}
		}

		Sleep(UPDATE_WAIT_MILSECS);
	}
	printf("***Stop Update Order Volumn Thread...\n");
	return 0;
}

DWORD WINAPI UpdatePrice( void *para )
{
	printf("***Start Update Price Thread...\n");
	//��½������
	char* serverIP = new char[16];
	strcpy(serverIP, SERVER_IP);
	int serverPort = SERVER_PORT;
	char* userID = new char[10];
	strcpy(userID, USER_NAME);
	char* userPW = new char[10];
	strcpy(userPW, USER_PASSWORD);
	//CIDMP_ORDER_INFO info;
	CIDMPTradeApi trader;
	int nSuccess = trader.Connect(serverIP, serverPort, userID, userPW, 0);
	if (nSuccess != 1){
		printf("***Update Price Thread Login Failed!\n");
		getchar();
		return -1;
	}
	global_price_ready = true;
	printf("***Update Price Thread Ready...\n");
	CIDMP_TICK_QUOTATION_QUERY *query = new CIDMP_TICK_QUOTATION_QUERY[global_symbol.size()+1];
	CIDMP_TICK_QUOTATION_INFO *info = new CIDMP_TICK_QUOTATION_INFO[global_symbol.size()+1];
	for ( int i = 0; i < global_symbol.size(); i++ ){
		(query+i)->exchgcode = EXCHANGE;
		strcpy((query+i)->symbol, global_symbol[i].c_str());
	}

	//ѭ��
	while(true){
		//printf("***Check Price...\n");
		trader.GetTickQuotation(query, info, global_symbol.size());
		for ( int i = 0; i < global_symbol.size(); i++ ){
			//�������Ƿ��б仯
			if ( global_symbol_price[global_symbol[i]].first != info[i].bp1 ){
				printf("***%d\tBuy Price Changed, old price = %3.7f, new price = %3.7f\n", GetCurrentThreadId(), info[i].bp1, global_symbol_price[global_symbol[i]].first);
				global_symbol_price[global_symbol[i]].first = info[i].bp1;
				//global_event[global_symbol[i]].first.SetEvent();
				EnterCriticalSection(&PriceChangeCriticalSection);
				strcpy(price_change_symbol, global_symbol[i].c_str());
				price_change_direction = 0;
				LeaveCriticalSection(&PriceChangeCriticalSection);
				printf("***%d\t���ͼ۸�仯�ź�\n", GetCurrentThreadId());
				PriceChangeEvent.SetEvent();
			}
			//��������Ƿ��б仯
			if ( global_symbol_price[global_symbol[i]].second != info[i].sp1 ){
				printf("***%d\tSell Price Changed, old price = %3.7f, new price = %3.7f\n", GetCurrentThreadId(), info[i].sp1, global_symbol_price[global_symbol[i]].second);
				global_symbol_price[global_symbol[i]].second = info[i].sp1;
				//global_event[global_symbol[i]].second.SetEvent();
				EnterCriticalSection(&PriceChangeCriticalSection);
				strcpy(price_change_symbol, global_symbol[i].c_str());
				price_change_direction = 1;
				LeaveCriticalSection(&PriceChangeCriticalSection);
				printf("***%d\t���ͼ۸�仯�ź�\n", GetCurrentThreadId());
				PriceChangeEvent.SetEvent();
			}
			
		}
		Sleep(UPDATE_WAIT_MILSECS);
	}
	

	delete[] serverIP;
	delete[] userID;
	delete[] userPW;
	printf("***Stop Update Price Thread...\n");
	return 0;
}

DWORD WINAPI LogPrint( void *para )
{
	//��ȡ���ؽ��ױ��action_no
	FILE *action_stream;
	action_stream = fopen( ACTION_PATH.c_str(), "r" );
	int no = 0;
	int action_no;
	while (fscanf( action_stream, "%d", &no ) != EOF){
		action_no = no;
	}
	if (action_stream == NULL){
		printf("***[Log Print Thread]%d\t���ؽ��ױ�ż�¼�ļ���ȡʧ�ܣ�\n");
		getchar();
		return -1;
	}
	action_no++;
	fclose(action_stream);
	action_stream = fopen( ACTION_PATH.c_str(), "a");

	//��log�ļ���

	stream = fopen( LOG_PATH.c_str(), "w" );
	if (stream == NULL){
		printf("***[Log Print Thread]%d\tLog��¼�ļ���ʧ�ܣ�\n");
		getchar();
		return -1;
	}
	time_t rawtime;
	while (true){
		WaitForSingleObject(PrintLogEvent, INFINITE);
		PrintLogEvent.ResetEvent();
		if (logMsgQueue.empty()){
			printf("***[Log Print Thread]%d\tError!�������޴���ӡlog��Ϣ!\n");
			continue;
		}
		while(!logMsgQueue.empty()){
			LogMessage& oneMsg = logMsgQueue.front();
			//�����ǰһ��Log��Ϣ
			printf("print ��� = %d ��log��Ϣ\n", action_no);

			time(&rawtime);
			if (oneMsg.cmd_status == CANCEL_RETURN || oneMsg.cmd_status == INSERT_RETURN){
				fprintf(stream, "ʱ�� = %s���ر�� = %d,\t��������� = %d,\tָ��״̬ = %d,\tָ��� = %d,\t������״̬ = %d,\t�������� = %d,\t��ƽ���� = %d,\t�������� = %d,\t�ѳɽ����� = %d,\t�ѳ������� = %d,\t�۸� = %7.3f\n", 
					asctime(localtime(&rawtime)), action_no, oneMsg.server_no, oneMsg.cmd_status, oneMsg.cmd_return, oneMsg.server_status, oneMsg.direction, oneMsg.offset, oneMsg.order_vol, oneMsg.trade_vol, oneMsg.cancel_vol, oneMsg.price);
			}else{
				fprintf(stream, "ʱ�� = %s���ر�� = %d,\t��������� = %d,\tָ��״̬ = %d,\t������״̬ = %d,\t�������� = %d,\t��ƽ���� = %d,\t�������� = %d,\t�ѳɽ����� = %d,\t�ѳ������� = %d,\t�۸� = %7.3f\n", 
					asctime(localtime(&rawtime)), action_no, oneMsg.server_no, oneMsg.cmd_status, oneMsg.server_status, oneMsg.direction, oneMsg.offset, oneMsg.order_vol, oneMsg.trade_vol, oneMsg.cancel_vol, oneMsg.price);
			}
			action_no++;
			fprintf(action_stream, "%d\n", action_no);
			logMsgQueue.pop();
		}

	}
}
/*
int TraderCore( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTime, double finalRatio, double minGap )
{
	CIDMP_TICK_QUOTATION_QUERY *initQuery = new CIDMP_TICK_QUOTATION_QUERY[1];


	bool getTrade = false;
	bool orderInMarket = false;
	clock_t start_time = clock();
	double priceInOrder = 0;
	int count_time = 0;
	double lastPrice = 0;

	while ( !getTrade ){
		clock_t cur_time = clock();
		if ( clock() - start_time > count_time + 3000 ){
			printf("***Heart Beat...%d\n",clock() - start_time);
			printf("***���¼۸�%6.3f\n", lastPrice);
			count_time = clock() - start_time;
		}
		if ( cur_time - start_time >= 1000 * maxSeconds * (1 - finalRatio) ){
			//�������ղ���
			return GotoFinalStep( trader, orderReq, maxSeconds, nTime, finalRatio, minGap );
			break;
		}else{
			//��ȡ���¼۸�
			CIDMP_TICK_QUOTATION_QUERY *realQuery = new CIDMP_TICK_QUOTATION_QUERY[1];
			CIDMP_TICK_QUOTATION_INFO *realInfo = new CIDMP_TICK_QUOTATION_INFO[1];
			realQuery->exchgcode = EXCHANGE;
			strcpy( realQuery->symbol, SYMBOL );
			trader.GetTickQuotation( realQuery, realInfo, 1 );
			if ( orderReq.direction == 0 ){		//�����������Ϊ��
				lastPrice = realInfo[0].bp1 - 10;			//��ȡ������һ��			
			}else{
				lastPrice = realInfo[0].sp1;			//��ȡ������һ��
			}//if
			if ( orderInMarket ){
				if ( lastPrice != priceInOrder ){
					//Risk
					orderReq.orderPrice = lastPrice;
					int riskNum = 0;
	//				printf("ί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", orderReq.symbol, orderReq.direction, orderReq.offsetFlagType, orderReq.orderPrice, orderReq.orderVol);
					int riskSuccess = trader.RiskTest( orderReq, riskNum );
					switch (riskSuccess){
					case -1:
						printf("***����ʧ�ܣ�ʧ��ԭ�򡪡���δ��¼��\n");
						break;
					case -2:
						printf("***����ʧ�ܣ�ʧ��ԭ�򡪡�����ѯ��ˮ��ʧ�ܡ�\n");
						break;
					case -3:
						printf("***����ʧ�ܣ�ʧ��ԭ�򡪡������δͨ����\n");
						break;
					case -5:
						printf("***����ʧ�ܣ�ʧ��ԭ�򡪡���fundId��cellId�����ڻ�δ����Ȩ��\n");
						break;
					default:
						break;
					}//switch
					if ( riskNum > 0 ){
						printf("***���������Ŀ:%d\n",riskNum);
						return -1;
					}
										
					//OrderCancel����
					char errorMsg[200];
	//				PrintOrderMsg(orderReq);
					
					int cancelSuccess = trader.CancelOrder( orderReq.orderNo, errorMsg );//orderNo��batNo���� 
					if ( cancelSuccess < 0 ){	//����ʧ�ܵĲ��� todo
						printf("***����ʧ�ܣ�\n");
						return -1;
					}else{
						printf("***�����ɹ���\n");
						orderInMarket = false;
						priceInOrder = 0.0;
					}
				}else{
					//��ȡ���½�����ϢGet Trade
					CIDMP_ORDER_INFO orderInfo;
					getTrade = GetRealTrade( trader, orderReq, orderInfo );
					continue;
				}//if
			}else{
				//Risk
				//�������¼۸�ί�ж�����Ϣ��
				orderReq.orderPrice = lastPrice;
				int riskNum = 0;
		//		printf("ί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", orderReq.symbol, orderReq.direction, orderReq.offsetFlagType, orderReq.orderPrice, orderReq.orderVol);
				int riskSuccess = trader.RiskTest( orderReq, riskNum );
				switch (riskSuccess){
				case -1:
					printf("***����ʧ�ܣ�ʧ��ԭ�򡪡���δ��¼��\n");
					break;
				case -2:
					printf("***����ʧ�ܣ�ʧ��ԭ�򡪡�����ѯ��ˮ��ʧ�ܡ�\n");
					break;
				case -3:
					printf("***����ʧ�ܣ�ʧ��ԭ�򡪡������δͨ����\n");
					break;
				case -5:
					printf("***����ʧ�ܣ�ʧ��ԭ�򡪡���fundId��cellId�����ڻ�δ����Ȩ��\n");
					break;
				default:
					break;
				}//switch
				if ( riskNum > 0 ){
					printf("***���������Ŀ:%d\n",riskNum);
					return -1;
				}
			}//if
			//OrderInsert�µ�
			//���¶�������
			CIDMP_ORDER_INFO orderInfo;
			getTrade = GetRealTrade( trader, orderReq, orderInfo );
			if ( orderInfo.orderStatus == 5 ){
				orderReq.orderVol -= orderInfo.tradeVol;
			}
			int riskNum;
			int orderSuccess = trader.OrderInsert( orderReq, riskNum );
			
			//PrintOrderMsg(orderReq);
			if ( orderSuccess < 0 ){

				printf("%s\n", trader.GetLastOrderInsertError());
				switch (orderSuccess ){
				case -1:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡���δ��¼��\n");
					break;
				case -2:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡�����ѯ��ˮ��ʧ�ܡ�\n");
					break;
				case -3:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡������δͨ����\n");
					break;
				case -4:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡����µ�ʧ�ܡ�\n");

					break;
				case -5:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡���fundId��cellId�����ڻ�δ����Ȩ��\n");
					break;
				default:
					break;
				}

				getchar();
				return -1;
			}else{
				printf( "***������ί�гɹ���\n" );
				PrintOrderMsg(orderReq);
				orderInMarket = true;
				priceInOrder = lastPrice;
			}
			//��ȡ���½�����ϢGet Trade
			getTrade = GetRealTrade( trader, orderReq, orderInfo );
		}//if
	}//while

	return 1;
}

inline void PrintOrderMsg( CIDMP_ORDER_REQ &orderReq )
{			
	printf("ί�к�\t�ʲ��˻�\t�ʲ���Ԫ\tͶ�����\t֤ȯ/��Լ����\t����\t��ƽ\t�۸�\t����\n");
	printf("%d\t%d\t%d\t%I64d\t%s\t%d\t%d\t%6.3f\t%d\n\n", orderReq.orderNo, orderReq.fundId, orderReq.cellId, orderReq.proflId,
		orderReq.symbol, orderReq.direction, orderReq.offsetFlagType, orderReq.orderPrice, orderReq.orderVol );	
}

inline bool GetRealTrade( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, CIDMP_ORDER_INFO &orderInfo )
{
//	printf("\n***��ѯί����Ϣ��\n");
	
	int checkSuccess = trader.GetOrder( orderReq.orderNo, orderInfo );
	if(checkSuccess != 1) {	    
		printf("��ѯί����Ϣʧ��!\n");
	}
//	printf("ί�к�\t״̬\t֤ȯ/��Լ����\t����\t��ƽ\t�۸�\t����\tʱ��\t����Ա\t������Ϣ\n");
//	printf("%d\t%d\t%s\t%d\t%d\t%6.3f\t%d\t%s\t%s\t%s\n",
//		orderInfo.orderNo, orderInfo.orderStatus, orderInfo.symbol, orderInfo.direction, orderInfo.offsetFlagType,
//		orderInfo.orderPrice, orderInfo.orderVol, orderInfo.orderTime, orderInfo.userId, orderInfo.errorInfo);	
	//���ֲ��ֳɽ���ȫ���ɽ� todo
//	printf("%6.3f\n", orderReq.orderPrice);
	return (orderInfo.orderStatus == 6);	//1δ��2����3�ϵ�4�ѱ�5����6ȫ��7�ѱ�����8����9�������� 

}

int GotoFinalStep( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTime, double finalRatio, double minGap )
{
	clock_t start_time = clock();
	int max_time = 1000 * maxSeconds * finalRatio;
	bool orderInMarket = true;

	while ( clock() - start_time <= max_time ){	//û�г�ʱ
		for ( int i  = 0; i < N_TIME; i++ ){
			//�۸�����
			//�Ƿ���Ҫ���۸����ޣ� todo
			orderReq.orderPrice += minGap * nTime;
			//�µ�
			CIDMP_ORDER_INFO orderInfo;
			GetRealTrade( trader, orderReq, orderInfo );
			if ( orderInfo.orderStatus == 5 ){
				orderReq.orderVol -= orderInfo.tradeVol;
			} 
			int riskNum;
			int orderSuccess = trader.OrderInsert( orderReq, riskNum );
	//		PrintOrderMsg(orderReq);
			if ( orderSuccess < 0 ){
				switch (orderSuccess ){
				case -1:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡���δ��¼��\n");
					break;
				case -2:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡�����ѯ��ˮ��ʧ�ܡ�\n");
					break;
				case -3:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡������δͨ����\n");
					break;
				case -4:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡����µ�ʧ�ܡ�\n");
					break;
				case -5:
					printf("***����ί��ʧ�ܣ�ʧ��ԭ�򡪡���fundId��cellId�����ڻ�δ����Ȩ��\n");
					break;
				default:
					break;
				}
				return -1;
			}else{
				printf( "***������ί�гɹ���\n" );
				orderInMarket = true;
				PrintOrderMsg(orderReq);
			}
			
			Sleep(1000);

			//����
			char errorMsg[200];
			int cancelSuccess = trader.CancelOrder( orderReq.orderNo, errorMsg );//orderNo��batNo���� 
			if ( cancelSuccess < 0 ){	//����ʧ�ܵĲ��� todo
				printf("***����ʧ�ܣ�\n");
				printf("***%s\n", errorMsg);
				CIDMP_ORDER_INFO orderInfo;
				if ( GetRealTrade( trader, orderReq, orderInfo ) ){
					printf("***���׳ɹ���\n");
					return 1;
				}else{
					if (orderInMarket){
							printf("ί�к�\t״̬\t֤ȯ/��Լ����\t����\t��ƽ\t�۸�\t����\tʱ��\t����Ա\t������Ϣ\n");
							printf("%d\t%d\t%s\t%d\t%d\t%6.3f\t%d\t%s\t%s\t%s\n",
								orderInfo.orderNo, orderInfo.orderStatus, orderInfo.symbol, orderInfo.direction, orderInfo.offsetFlagType,
								orderInfo.orderPrice, orderInfo.orderVol, orderInfo.orderTime, orderInfo.userId, orderInfo.errorInfo);	
						return -2;
					}
				}
			}else{
				printf("***�����ɹ���\n");
				orderInMarket = false;
			}
			if ( GetRealTrade( trader, orderReq, orderInfo ) ){
				printf("***���׳ɹ���\n");
				return 1;
			}
		}
	}
	return 0;
}
*/