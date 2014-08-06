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

const int FUND_ID = 100;	//资产账户编码
const int CELL_ID = 1001;	//资产单元编码
const i64 PROFL_ID = 10011;	//资产组合编码
const int DIRECTION = 0;	//交易方向
const int EXCHANGE = 6;	//交易所编号
const int OFFSET = 1;		//开平方向
const int PJ = 0;			//是否平今
const char SYMBOL[8] = "IF1409";	//股票或期货代码
const int INI_PRICE = 0;
const int INI_VOL = 4;

const int FUND_SIZE = 100;
const int HOLD_SIZE = 300;
const int ORDER_SIZE = 300;
const int N_TIME = 3;
const int MAX_SECONDS = 600;
const int N_TICKS = 1;
const double FINAL_RATIO = 0.05;
const int MAX_INSERT_SECS = 5;	//下单最长等待时间
const string LOG_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\log.txt";
const string TRANS_LOG_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\transaction_log.txt";
const string ACTION_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\action_log.txt";
const string SYMBOL_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\symbol.txt";
//const bool DEBUG = false;
const bool PROCED_DEBUG = false;
const bool INSERT_DEBUG = true;
const bool CANCEL_DEBUG = true;
const int UPDATE_WAIT_MILSECS = 100;		//行情更新间隔毫秒数
FILE *stream = NULL;
//全局变量
int global_trade_vol = 0;		//累计已交易量
double global_trade_mount = 0;		//累计已交易金额
int global_order_vol = 0;		//订单中还未成交量
double global_last_price = 0.0;		//最新价
int global_last_order_no = 0;
typedef hash_map <string, pair<double, double> > HashMapPriceData;
HashMapPriceData global_symbol_price;		//最新的价格
vector<string> global_symbol;				//所有Symbol
list<int> global_order_NO;				//所有的orderNo
typedef hash_map <int, int> HashMapVolData;
HashMapVolData global_volumn;
CRITICAL_SECTION global_cs;				//进程锁
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
CRITICAL_SECTION CommandCriticalSection;	//指令进程锁
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
//打印出委托请求信息
inline void PrintOrderMsg( CIDMP_ORDER_REQ &orderReq );
int OrderCancel( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE *stream, int action_no );
inline bool UpdateOrder( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE* stream, int action_no );
int OrderInsert( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE *stream, int action_no );
inline void PrintLogMessage( CIDMPTradeApi *pTader, CIDMP_ORDER_REQ *pOrder, CIDMP_ORDER_INFO *pInfo, FILE *stream, int actNo, bool insert );
//获取最新交易信息
//inline bool GetRealTrade( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, CIDMP_ORDER_INFO &orderInfo );
inline void GetCharStatus( int sta, char* data );
int GotoFinalStep( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, int maxSeconds, int nTime, double finalRatio, double minGap, FILE *stream, int action_no );
int main()
{

	CIDMPTradeApi trader;
	printf("是否登录交易服务器？(Y/y登录，其他返回)\n");
	//登录
	printf("***正在登录： %s, %d, %s...\n", SERVER_IP, SERVER_PORT, USER_NAME);
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
		printf("登录失败！\n");
		printf("%d\n", logSuccess);
		getchar();
		return 0;
	}
	printf("***登录成功！\n服务器IP：%s,\t服务器端口：%d,\t用户名：%s...\n", SERVER_IP, SERVER_PORT, USER_NAME);

	//获取资产账户列表
	printf("***获取资产账户列表……\n");
	CIDMP_FUND_ACCOUNT_INFO accountInfo[FUND_SIZE];
	int accountSize = trader.GetFundAccount( accountInfo, FUND_SIZE );
	if (accountSize < 0){
		printf("获取资产账户列表失败！\n");
		getchar();
		return 0;
	}
	printf("[序号]\t代码\t名称\n");
	for(int i = 0;i < accountSize; i++)
	{
		printf("%d\t%d\t%s\n", i+1, accountInfo[i].id, accountInfo[i].name);
	}

	//获得指定资产账户的资产单元列表
	printf("\n***获得资产账户[%d]的资产单元列表...\n", FUND_ID);
	CIDMP_FUND_CELL_INFO CellInfo[FUND_SIZE];	
	int cellSize = trader.GetFundCell( FUND_ID, CellInfo, FUND_SIZE);
	if(cellSize == -1) {
		printf("获取资产单元列表失败\n");
		getchar();
		return 0;
	}	 
	printf("[序号]\t代码\t名称\n");
	for(int i = 0;i < cellSize; i++)
	{
		printf("%d\t%d\t%s\n", i+1, CellInfo[i].id, CellInfo[i].name);
	}

	//获得资产单元的投资组合列表
	printf("\n***获得资产账户[%d]的资产单元[%d]的投资组合列表...\n", FUND_ID, CELL_ID);
	CIDMP_FUND_PROFL_INFO ProfInfo[FUND_SIZE];	
	int proSize = trader.GetFundProfl( FUND_ID, CELL_ID, ProfInfo, FUND_SIZE);
	if(proSize == -1) {
		printf("获取投资组合列表失败\n");
		getchar();	
		return 0;
	}	
	printf("[序号]\t代码\t名称\n");
	for(int i = 0; i < proSize; i++)
	{
		printf("%d\t%I64d\t%s\n", i+1, ProfInfo[i].id, ProfInfo[i].name);		
	}
	
	//查询资金信息
	printf("\n***获得资产账户[%d]的资金信息...\n", FUND_ID, CELL_ID);
	CIDMP_FUND_INFO fInfo;
	int nSuccess = trader.GetFund( FUND_ID, fInfo);
	if(nSuccess != 1) {
		printf("查询资产账户的资金信息失败\n");
		printf("%d\n", nSuccess);
		getchar();
		return 0;
	}

	printf("现货总资金\t: %6.3f\n现货可用\t: %6.3f\n期货总资金\t: %6.3f\n期货可用\t: %6.3f\n\n",
		fInfo.stkTotal,fInfo.stkAvailable, fInfo.futureTotal, fInfo.futureAvailable);

		
	printf("\n***获得资产账户[%d]的资产单元[%d]的资金信息...\n", FUND_ID, CELL_ID);
	CIDMP_FUND_INFO cInfo;
	nSuccess = trader.GetFund( FUND_ID, CELL_ID, cInfo);
	
	if(nSuccess != 1) {
		printf("查询资产单元的资金信息失败\n");
		getchar();
		return 0;
	}
	printf("现货总资金\t: %6.3f\n现货可用\t: %6.3f\n期货总资金\t: %6.3f\n期货可用\t: %6.3f\n\n",
		cInfo.stkTotal, cInfo.stkAvailable, cInfo.futureTotal, cInfo.futureAvailable);
	
	//查询持仓信息
	printf("\n***获得资产账户[%d]的持仓信息...\n", FUND_ID);
	CIDMP_HOLD_INFO holdInfo[HOLD_SIZE];
	int nSize = trader.GetHold( FUND_ID, 0, "", 2, holdInfo, HOLD_SIZE );
	if(nSize < 0) {
		printf("查询资产账户持仓信息失败\n");
		getchar();
		return 0;
	}
	printf("[序号]\t代码\t交易所代号\t持仓类型\t可用数量\t总数量\t可平今仓\t可平昨仓\n");
	for(int i = 0; i < nSize && i < HOLD_SIZE; i ++) {
		printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\n",
			i+1, holdInfo[i].symbol, holdInfo[i].exchgcode, holdInfo[i].holdType, holdInfo[i].available, holdInfo[i].total, 
			holdInfo[i].available_jr, holdInfo[i].available_zr);
	}

	//查询资产单元持仓信息
	printf("\n***获得资产账户[%d]的资产单元[%d]的持仓信息...\n", FUND_ID, CELL_ID);
	CIDMP_HOLD_INFO holdInfo1[HOLD_SIZE];
	nSize = trader.GetHold( FUND_ID, CELL_ID, 0, "", 2, holdInfo1, HOLD_SIZE );

	if(nSize < 0) {
		printf("查询资产单元持仓信息失败\n");
		getchar();
		return 0;
	}
	printf("[序号]\t代码\t交易所代号\t持仓类型\t可用数量\t总数量\t可平今仓\t可平昨仓\n");
	for(int i = 0; i < nSize && i < HOLD_SIZE; i ++) {
		printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\n",
			i+1, holdInfo1[i].symbol, holdInfo1[i].exchgcode, holdInfo1[i].holdType, holdInfo1[i].available,holdInfo1[i].total, 
			holdInfo1[i].available_jr, holdInfo1[i].available_zr);
	}
	
	//查询投资组合持仓信息
	printf("\n***获得资产账户[%d]的资产单元[%d]的投资组合[%I64d]的持仓信息...\n", FUND_ID, CELL_ID, PROFL_ID);
	CIDMP_HOLD_INFO holdInfo2[HOLD_SIZE];
	nSize = trader.GetHold( FUND_ID, CELL_ID, PROFL_ID, 0, "", 2, holdInfo2, HOLD_SIZE );

	if(nSize < 0) {
		printf("查询投资组合持仓信息失败\n");
		getchar();
		return 0;
	}
	
	printf("[序号]\t代码\t交易所代号\t持仓类型\t可用数量\t总数量\t可平今仓\t可平昨仓\n");
	for(int i = 0; i < nSize && i < HOLD_SIZE; i ++) {
		printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\n",
			i+1, holdInfo2[i].symbol, holdInfo2[i].exchgcode, holdInfo2[i].holdType, holdInfo2[i].available, holdInfo2[i].total,
			holdInfo2[i].available_jr, holdInfo2[i].available_zr);
	}
#if 0
	//创建订单对象
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
	//预处理过程
	//读取SymbolList
	printf("***读取Symbol List...\n");
	ifstream in;
	in.open(SYMBOL_PATH);
	if (!in){
		printf("***Symbol List 读取失败！\n");
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

	//初始化临界区
	InitializeCriticalSection(&CommandCriticalSection);
	InitializeCriticalSection(&PriceChangeCriticalSection);
	InitializeCriticalSection(&global_cs);
	global_price_ready = false;
	global_vol_ready = false;
	global_trade_ready = false;
	HANDLE priceHandler = CreateThread( NULL, 0, UpdatePrice, NULL, 0, NULL );
	HANDLE volumeHandler = CreateThread( NULL, 0, UpdateOrderVol, NULL, 0, NULL );
	HANDLE logPrintHandler = CreateThread( NULL, 0, LogPrint, NULL, 0, NULL );
	printf("***等待其他进程准备就绪...\n");
	while(!global_vol_ready || !global_price_ready){
		Sleep(500);
	}
//	printf("***所有进程准备就绪！\n");

	 TradeCommandEvent.ResetEvent();
		
		//开始启动第一个交易进程
		HANDLE traderHandler = CreateThread(NULL, 0, TraderThread, NULL, 0, NULL);
		
		//等待交易进程准备就绪信息
		//printf("***Wait For Trade Ready Signal...\n");
		printf("***等待交易线程准备就绪信号\n");
		TradeReadyEvent.ResetEvent();
		WaitForSingleObject(TradeReadyEvent, INFINITE);
		//printf("***Get Trade Ready Signal...\n");
		printf("***接收交易线程准备就绪信号\n");
		TradeReadyEvent.ResetEvent();
	//	printf("***Trade Thread Get Ready!\n");
		while(true){
			printf("***输入Y/y继续下一个交易指令，输入其他结束程序运行...\n");
			char flag;
			scanf("%c", &flag);
			if ( flag != 'Y' && flag != 'y' ){
				break;
			}
			printf("***请输出交易指令：(交易所代码 代码 买卖方向 开平方向 交易数量 是否平今 交易时间/secs)\n");
			getchar();

			//debug
			//	scanf( "%d%s%d%d%d%d%d", &exchange, &symbol, &direction, &offset, &volumn, &pj, &trade_secs );
			EnterCriticalSection(&CommandCriticalSection);
			trader_command.fund_id = FUND_ID; trader_command.cell_id = CELL_ID; trader_command.profl_id = PROFL_ID;
			trader_command.exchange_code = 6; strcpy(trader_command.symbol, "IF1409"); trader_command.direction = 0; trader_command.offset_flag_type = 0; trader_command.order_volumn = 1; trader_command.pj = 0; trader_command.max_seconds = 600;
			LeaveCriticalSection(&CommandCriticalSection);
			//发送交易指令
			printf("***发送交易指令信号\n");
			TradeCommandEvent.SetEvent();
			CreateThread(NULL, 0, TraderThread, NULL, 0, NULL);
			printf("***等待交易线程准备就绪信号\n");
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
	printf("是否开始下单？(确认请输入Y/y回车，输入其他返回\n)");
	char start_order = 0;
	scanf("%c", &start_order);
	if ( start_order != 'Y' && start_order != 'y' ){
		return 0;
	}
	//风险试算
	printf( "\n***风险试算： 资产账户[%d],资产单元[%d],投资组合[%I64d]\n",FUND_ID,CELL_ID,PROFL_ID );
	nSuccess = trader.RiskTest( orderReq, riskNum );
	printf("委托信息:合约=%s,买卖=%d,开平=%d,价格=%6.3f,数量=%d\n", orderReq.symbol, orderReq.direction, orderReq.offsetFlagType, orderReq.orderPrice,
		orderReq.orderVol);
	printf("接口返回值:%d\n",nSuccess);
	printf("触发风控数目:%d\n",riskNum);

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
		printf("***交易失败！未知错误！\n");
		break;;
	case 1:
		printf("***交易失败！下单超时！\n");
		break;
	case 2:
		printf("***交易成功！\n");
		break;
	default:
		break;
	}
	//CloseHandle(TraderCore);

/*	if (nSuccess < 0){
		printf("***交易失败，失败原因：风控检查失败！\n");
	}else if (nSuccess == 0){
		printf("***交易失败，失败原因：交易超时！\n");
	}else{
		printf("***交易成功！\n");
	}
	CIDMP_ORDER_INFO orderInfo;*/
//	GetRealTrade( trader, orderReq, orderInfo );
//	printf("***任意输入结束本次下单……\n");
#endif 
	system("PAUSE");
	return 0;
}
DWORD WINAPI TraderThread( void *para )
{
	printf("***%d\tTrader Thread Begin...\n", GetCurrentThreadId());
	//登陆服务器
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
	printf("***%d\t发送线程准备就绪信号...\n", GetCurrentThreadId());
	TradeReadyEvent.SetEvent();
	
	//等待交易信号
	printf("***%d\t等待交易指令信号\n", GetCurrentThreadId());
	WaitForSingleObject(TradeCommandEvent, INFINITE);
	printf("***%d\t接收交易指令信号\n", GetCurrentThreadId());
	TradeCommandEvent.ResetEvent();
	
	//进入临界区，获取交易指令
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

	//获取最新交易价格
	orderReq.orderPrice = orderReq.direction == 0 ? global_symbol_price[orderReq.symbol].first : global_symbol_price[orderReq.symbol].second;
	printf("***%d\t开始交易...\n", GetCurrentThreadId());
	//下单
	int riskNum;

	LogMessage oneMsg;
	oneMsg.server_no = -1; oneMsg.cmd_status = INSERT_SEND; oneMsg.server_status = -1; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = 0; oneMsg.cancel_vol = 0; oneMsg.price = orderReq.orderPrice;
	logMsgQueue.push(oneMsg);
	//通知print进程打印
	PrintLogEvent.SetEvent();
	
	nSuccess = trader.OrderInsert(orderReq, riskNum);

	trader.GetOrderByOrderNo(orderReq.orderNo, info);
	oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = INSERT_RETURN; oneMsg.cmd_return = nSuccess, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = 0; oneMsg.cancel_vol = 0; oneMsg.price = orderReq.orderPrice;
	logMsgQueue.push(oneMsg);
	PrintLogEvent.SetEvent();

	if (nSuccess < 0){
		printf("***%d\t下单失败！\n", GetCurrentThreadId());
		printf("***%s\n", trader.GetLastOrderInsertError());
		return -1;
	}
	//确认下单成功
	trader.GetOrderByOrderNo(orderReq.orderNo, info);
	while ( info.orderStatus < 3 ){
		Sleep(100);
		trader.GetOrderByOrderNo(orderReq.orderNo, info);
	}
	
	oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = UPDATE_ORDER, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = info.tradeVol; oneMsg.cancel_vol = info.canceledVol; oneMsg.price = orderReq.orderPrice;
	logMsgQueue.push(oneMsg);
	PrintLogEvent.SetEvent();

	if (info.orderStatus == 3){
		printf("***%d\t下单失败！\n", GetCurrentThreadId());
		printf("***%s\n", trader.GetLastOrderInsertError());
		return -1;
	}
	printf("***%d\t下单成功！下单价格 = %3.7f\n", GetCurrentThreadId(), orderReq.orderPrice);
	global_order_NO.push_back(orderReq.orderNo);
	global_volumn[orderReq.orderNo] = orderReq.orderVol;

	//开始交易循环
//	CEvent& priceEvent = orderReq.direction == 0 ? global_event[orderReq.symbol].first : global_event[orderReq.symbol].second;
	clock_t start_time = clock();
	bool getTrade = false;
	PriceChangeEvent.ResetEvent();
	while (clock() - start_time <= maxSecs * 1000){
		if (WaitForSingleObject(PriceChangeEvent, 3000) == WAIT_TIMEOUT ){
			printf("Heart Beat...%d\n", clock());
			//trader.GetOrderByOrderNo(orderReq.orderNo, info);
			if (global_volumn[orderReq.orderNo] == 0){
				printf("***%d\t订单成交!\n", GetCurrentThreadId());
				getTrade = true;
				break;
			}
			continue;
		}else{
			if (global_volumn[orderReq.orderNo] == 0){
				printf("***%d\t订单成交!\n", GetCurrentThreadId());
				getTrade = true;
				break;
			}
			printf("***%d\t接收价格变化信号\n", GetCurrentThreadId());
			PriceChangeEvent.ResetEvent();
			//确认价格变化是否与本订单相关
			EnterCriticalSection(&PriceChangeCriticalSection);
			if ( strcmp(price_change_symbol, orderReq.symbol) != 0 || price_change_direction != orderReq.direction ){
				printf("***%d\t价格变化与本订单无关，继续等待\n", GetCurrentThreadId());
				continue;
			}
			LeaveCriticalSection(&PriceChangeCriticalSection);
			
			//撤单
			char errorMsg[200];
			trader.GetOrderByOrderNo(orderReq.orderNo, info);
			
			oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = CANCEL_SEND, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = info.tradeVol; oneMsg.cancel_vol = info.canceledVol; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			nSuccess = trader.CancelOrder(orderReq.orderNo, errorMsg);
			//确认撤单成功
			trader.GetOrderByOrderNo(orderReq.orderNo, info);
			oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = CANCEL_RETURN, oneMsg.cmd_return = nSuccess, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = info.tradeVol; oneMsg.cancel_vol = info.canceledVol; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			while (info.orderStatus < 8){
				Sleep(100);
				trader.GetOrderByOrderNo(orderReq.orderNo, info);
			}
			printf("***%d\t订单号 = %d, 撤单成功！\n", GetCurrentThreadId(), orderReq.orderNo);
			
			oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = UPDATE_ORDER, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = info.tradeVol; oneMsg.cancel_vol = info.canceledVol; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			//更新价格
			orderReq.orderPrice = orderReq.direction == 0 ? global_symbol_price[orderReq.symbol].first : global_symbol_price[orderReq.symbol].second;
			printf("***%d\t订单价格变化，最新价格 = %3.7f\n", GetCurrentThreadId(), orderReq.orderPrice);

			//下单
			int riskNum;
		
			oneMsg.server_no = -1; oneMsg.cmd_status = INSERT_SEND; oneMsg.server_status = -1; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = 0; oneMsg.cancel_vol = 0; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			int nSuccess = trader.OrderInsert(orderReq, riskNum);
			
			oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = INSERT_RETURN; oneMsg.cmd_return = nSuccess, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = 0; oneMsg.cancel_vol = 0; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			if (nSuccess < 0){
				printf("***%d\t下单失败！\n", GetCurrentThreadId());
				printf("***%s\n", trader.GetLastOrderInsertError());
				return -1;
			}
			//确认下单成功
			trader.GetOrderByOrderNo(orderReq.orderNo, info);
			while ( info.orderStatus < 3 ){
				Sleep(100);
				trader.GetOrderByOrderNo(orderReq.orderNo, info);
			}

			oneMsg.server_no = orderReq.orderNo; oneMsg.cmd_status = UPDATE_ORDER, oneMsg.server_status = info.orderStatus; oneMsg.direction = orderReq.direction; oneMsg.offset = orderReq.offsetFlagType; oneMsg.order_vol = orderReq.orderVol; oneMsg.trade_vol = info.tradeVol; oneMsg.cancel_vol = info.canceledVol; oneMsg.price = orderReq.orderPrice;
			logMsgQueue.push(oneMsg);
			PrintLogEvent.SetEvent();

			if (info.orderStatus == 3){
				printf("***%d\t下单失败！\n", GetCurrentThreadId());
				printf("***%s\n", trader.GetLastOrderInsertError());
				return -1;
			}
			printf("***%d\t下单成功！下单价格 = %3.7f\n", GetCurrentThreadId(), orderReq.orderPrice);
			global_order_NO.push_back(orderReq.orderNo);
			global_volumn[orderReq.orderNo] = orderReq.orderVol;
		}


	}
	if (!getTrade){
		printf("***%d\t订单超时...\n", GetCurrentThreadId());
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
	//fprintf( stream, "时间\t本地编号\t订单编号\t状态\t总量\t成交量\t委托量\t撤单量\t价格\n" );
	FILE *action_stream;
	action_stream = fopen( ACTION_PATH.c_str(), "r" );
	int no = 0;
	int action_no;
	while (fscanf( action_stream, "%d", &no ) != EOF){
		action_no = no;
	}
	action_no++;
	fclose(action_stream);

	//强制转换参数
	CIDMP_ORDER_INFO info;
	TraderThreadParameter *thread_para = (TraderThreadParameter *)para;
	CIDMPTradeApi *pTrader = thread_para->pTrader;
	CIDMP_ORDER_REQ *pOrder = thread_para->orderReq;
	pOrder->orderNo = -1;
	int maxSeconds = thread_para->maxSecs;
	int nTime = thread_para->nTime;
	double finalRatio = thread_para->finalRatio;
	double minGap = thread_para->minGap;
	//设置订单价格
	if (pOrder->direction == 0){
		pOrder->orderPrice = global_symbol_price[pOrder->symbol].first;
	}else{
		pOrder->orderPrice = global_symbol_price[pOrder->symbol].second;
	}

	int riskNum = 0;
	
	printf("委托信息:合约=%s,买卖=%d,开平=%d,价格=%6.3f,数量=%d\n", pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);

	//提交订单委托
	OrderInsert(pTrader, pOrder, &info, stream, action_no);
	global_order_NO.push_back(pOrder->orderNo);
	clock_t start_time = clock();
	while ( clock() - start_time < maxSeconds * 1000 ){
		//等待信号触发
		printf("***Wait Signal...\n");
		DWORD nSuccess = WaitForSingleObject(pOrder->direction == 0 ? global_event[pOrder->symbol].first : global_event[pOrder->symbol].second, 3000);
		if (nSuccess == WAIT_TIMEOUT){
			//输出最新价格信息
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
	//fprintf( stream, "时间\t本地编号\t订单编号\t状态\t总量\t成交量\t委托量\t撤单量\t价格\n" );
	FILE *action_stream;
	action_stream = fopen( ACTION_PATH.c_str(), "r" );
	int no = 0;
	int action_no;
	while (fscanf( action_stream, "%d", &no ) != EOF){
		action_no = no;
	}
	action_no++;
	fclose(action_stream);
	//强制转换参数
	TraderThreadParameter *thread_para = (TraderThreadParameter *)pParam;
	CIDMPTradeApi *pTrader = thread_para->pTrader;
	CIDMP_ORDER_REQ *pOrder = thread_para->orderReq;
	pOrder->orderNo = -1;
	int maxSeconds = thread_para->maxSecs;
	int nTime = thread_para->nTime;
	double finalRatio = thread_para->finalRatio;
	double minGap = thread_para->minGap;
	int orderStatus = thread_para->orderStatus;
	

	//初始状态变量
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
		//每3s输出一次最新价格
		clock_t cur_time = clock();
		if ( cur_time - start_time > count_time + 3000){
			printf("***Heart Beat...%d\n",cur_time - start_time);
			printf("***最新价格：%6.3f\n", lastPrice);
			count_time = cur_time - start_time;
			printf("委托号\t状态\t证券/合约代码\t买卖\t开平\t价格\t数量\t交易量\t撤单量\t时间\t交易员\t错误信息\n");
			UpdateOrder(pTrader,pOrder,pInfo,stream,action_no);
			printf("%d\t%d\t%s\t%d\t%d\t%6.3f\t%d\t%d\t%d\t%s\t%s\t%s\n",
					pInfo->orderNo, pInfo->orderStatus, pInfo->symbol, pInfo->direction, pInfo->offsetFlagType,
					pInfo->orderPrice, pInfo->orderVol, pInfo->tradeVol, pInfo->canceledVol, pInfo->orderTime, pInfo->userId, pInfo->errorInfo);	
		}

		//TimeTest
		if ( cur_time - start_time >= 1000 * maxSeconds * ( 1 - finalRatio ) ){
			//进入最终步骤
			printf("***Go To Final Step...\n");
			int result = GotoFinalStep( pTrader, pOrder, maxSeconds, nTime, finalRatio, minGap, stream, action_no );
			if ( result == 1 ){
				printf("***程序超时...\n");
				thread_para->orderStatus = 1;
			}else if ( result == 0 ){
				thread_para->orderStatus = 2;
				printf("***交易成功...\n");
			}else{
				printf("***交易失败...\n");
				thread_para->orderStatus = -1;
			}
			break;
		}else{
			//获取最新价格
			pTrader->GetTickQuotation( realQuery, realInfo, 1 );
			if ( pOrder->direction == 0 ){		//如果订单方向为买
				lastPrice = realInfo[0].bp1;			//获取最新买一价			
			}else{
				lastPrice = realInfo[0].sp1;			//获取最新卖一价
			}//if
			if (orderInMarket && pInfo->orderStatus != 7){	//如果已有订单
//				printf("OrderInMarket\n");
				if ( lastPrice != priceInOrder ){
					printf("原价 = %6.3f, 现价 = %6.3f\n", priceInOrder, lastPrice);
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
			//更新最新的成交信息与委托订单量
			if (pOrder->orderPrice != lastPrice)
			{
				pOrder->orderPrice = lastPrice;
				printf("**更新订单价格，最新价格=%6.3f\n", lastPrice);
			}
 			getTrade = UpdateOrder( pTrader, pOrder, pInfo, stream, action_no );
			printf("委托信息:合约=%s,买卖=%d,开平=%d,价格=%6.3f,数量=%d\n", pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);
			action_no++;
			nSuccess = OrderInsert( pTrader, pOrder, pInfo, stream, action_no );
//			nSuccess = pTrader->OrderInsert( *pOrder, riskNum );
			if ( nSuccess < 0 ){
				printf("%s\n", pTrader->GetLastOrderInsertError());
				switch ( nSuccess ){
				case -1:
					printf("***订单委托失败！失败原因——【未登录】\n");
					break;
				case -2:
					printf("***订单委托失败！失败原因——【查询流水号失败】\n");
					break;
				case -3:
					printf("***订单委托失败！失败原因——【风控未通过】\n");
					break;
				case -4:
					printf("***订单委托失败！失败原因——【下单失败】\n");
					break;
				case -5:
					printf("***订单委托失败！失败原因——【fundId或cellId不存在或未被授权】\n");
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
		//		printf( "***订单已委托成功！\n" );
				orderInMarket = true;
				priceInOrder = lastPrice;
			}//if
		}//if

	}//while
	if (getTrade){
		printf("***交易成功！\n");
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
		fprintf( stream, "时间 = %s本地编号 = %d服务器编号 = %d，指令状态 = %s，服务器状态 = %s，买卖方向 = %d，开平方向 = %d，总量 = %d，成交量 = %d，委托量 = %d，撤单量 = %d，价格= %6.3f\n", 
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
		//fprintf(stream, "时间:%s 订单成交！\n委托信息:合约=%s,买卖=%d,开平=%d,价格=%6.3f,数量=%d\n", asctime(localtime(&rawtime)),pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);
//		PrintLogMessage( pTrader, pOrder, pInfo, stream, action_no, true );
		if ( pInfo->tradeVol > 0 ){
			FILE *trans_stream = NULL;
			trans_stream = fopen( TRANS_LOG_PATH.c_str(), "a" );
			time_t rawtime;
			time(&rawtime);
			global_trade_vol += pInfo->tradeVol;
			global_trade_mount += pInfo->tradeVol * pInfo->orderPrice;
			fprintf( trans_stream, "时间 = %s本地编号 = %d，订单总量 = %d，成交量 = %d，成交价格 = %6.3f，已成交均价 = %6.3f\n", 
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
		fprintf( trans_stream, "时间 = %s本地编号 = %d，成交量 = %d，成交价格 = %6.3f，已成交均价 = %6.3f\n", asctime(localtime(&rawtime)), action_no, pInfo->tradeVol, 
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
		fprintf( stream, "时间 = %s本地编号 = %d，服务器编号 = %d，指令状态 = %s，服务器状态 = %s，买卖方向 = %d，开平方向 = %d，总量 = %d，成交量 = %d，委托量 = %d，撤单量 = %d，价格= %6.3f\n", 
			asctime(localtime(&rawtime)), action_no, pOrder->orderNo, "Cancel Send", status, pOrder->direction, pOrder->offsetFlagType,  
			pInfo->orderVol, pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice );
		int nSuccess = pTrader->CancelOrder( pOrder->orderNo, errorMsg );
		UpdateOrder( pTrader, pOrder, pInfo, stream, action_no );
		GetCharStatus( pInfo->orderStatus, status );
		time(&rawtime);
		fprintf( stream, "时间 = %s本地编号 = %d，服务器编号 = %d，指令状态 = %s，指令返回 = %d，服务器状态 = %s，买卖方向 = %d，开平方向 = %d，总量 = %d，成交量 = %d，委托量 = %d，撤单量 = %d，价格= %6.3f\n", 
			asctime(localtime(&rawtime)), action_no, pOrder->orderNo, "Cancel Return", nSuccess, status, pOrder->direction, pOrder->offsetFlagType, pInfo->orderVol, pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice );
		delete []status;
		if ( pInfo->orderStatus >= 7 ){
			printf("***撤单成功！...\n");
			UpdateOrder( pTrader, pOrder, pInfo, stream, action_no );
		//	time_t rawtime;
		//	time(&rawtime);
		//	fprintf(stream, "时间:%s 订单撤销成功！\n委托信息:合约=%s,买卖=%d,开平=%d,价格=%6.3f,数量=%d\n", asctime(localtime(&rawtime)),pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);
		//	PrintLogMessage( pTrader, pOrder, pInfo, stream, action_no, false );
			cancelSuccess = true;
			return 0;
		}
		
		if ( pInfo->orderVol = pInfo->tradeVol + pInfo->canceledVol && pInfo->tradeVol > 0 ){
			cancelSuccess = true;
			printf("***撤单失败！失败原因，委托已成交...\n");
			time(&rawtime);
			UpdateOrder( pTrader, pOrder, pInfo, stream, action_no );
			status = new char[10];
			GetCharStatus( pInfo->orderStatus, status );
			fprintf( stream, "时间 = %s本地编号 = %d，服务器编号 = %d，指令状态 = %s，服务器状态 = %s，买卖方向 = %d，开平方向 = %d，总量 = %d，成交量 = %d，委托量 = %d，撤单量 = %d，价格= %6.3f\n", 
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
	fprintf( stream, "时间 = %s本地编号 = %d，服务器编号 = %d，指令状态 = %s，服务器状态 = %s，买卖方向 = %d，开平方向 = %d，总量 = %d，成交量 = %d，委托量 = %d，撤单量 = %d，价格= %6.3f\n", 
		asctime(localtime(&rawtime)), action_no, pOrder->orderNo, "Insert Send", status, pOrder->direction, pOrder->offsetFlagType, 
		pInfo->orderVol, pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice );
	int nSuccess = pTrader->OrderInsert( *pOrder, riskNum );
	time(&rawtime);
	UpdateOrder( pTrader, pOrder, pInfo, stream, action_no );
	GetCharStatus( pInfo->orderStatus, status );
	fprintf( stream, "时间 = %s本地编号 = %d，服务器编号 = %d，指令状态 = %s，指令返回 = %d，服务器状态 = %s，买卖方向 = %d，开平方向 = %d，总量 = %d，成交量 = %d，委托量 = %d，撤单量 = %d，价格= %6.3f\n", 
		asctime(localtime(&rawtime)), action_no, pOrder->orderNo, "Insert Send", nSuccess, status, pOrder->direction, pOrder->offsetFlagType, 
		pInfo->orderVol, pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice );
	delete []status;
//	while (true){
		if ( nSuccess < 0 ){
			printf("%s\n", pTrader->GetLastOrderInsertError());
			switch ( nSuccess ){
			case -1:
				printf("***订单委托失败！失败原因——【未登录】\n");
				break;
			case -2:
				printf("***订单委托失败！失败原因——【查询流水号失败】\n");
				break;
			case -3:
				printf("***订单委托失败！失败原因——【风控未通过】\n");
				break;
			case -4:
				printf("***订单委托失败！失败原因——【下单失败】\n");
				break;
			case -5:
				printf("***订单委托失败！失败原因——【fundId或cellId不存在或未被授权】\n");
				break;
			default:
				break;
			}
				printf("委托号\t状态\t证券/合约代码\t买卖\t开平\t价格\t数量\t时间\t交易员\t错误信息\n");
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
		printf("***下单超时！\n");
		return 0;
	}else{
		printf( "***订单已委托成功！最新价格=%6.3f\n", pOrder->orderPrice );
		//time_t rawtime;
		//time(&rawtime);
	//	PrintLogMessage( pTrader, pOrder, pInfo, stream, action_no, true );
		//fprintf(stream, "时间:%s 订单委托成功！\n委托信息:合约=%s,买卖=%d,开平=%d,价格=%6.3f,数量=%d\n", asctime(localtime(&rawtime)),pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);
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
		strcpy( data, "未报\0" );
//			data = "未报\0";
		break;
	case 2:
		strcpy( data, "待报\0" );	
		//data = "待报\0";
		break;
	case  3:
		strcpy( data, "废单\0" );
//		data = "废单\0";
		break;
	case 4:
		strcpy( data, "已报\0" );
//		data = "已报\0";
		break;
	case  5:
		strcpy( data, "部分成交\0" );
//		data = "部分成交\0";
		break;
	case  6:
		strcpy( data, "全部成交\0" );
//		data = "全部成交\0";
		break;
	case  7:
		strcpy( data, "已报撤单\0" );
//		data = "已报撤单\0";
		break;
	case  8:
		strcpy( data, "撤单部成\0" );	
//		data = "撤单部成\0";
		break;
	case  9:
		strcpy( data, "撤单待撤\0" );
//		data = "撤单待撤\0";
		break;
	default:
		strcpy( data, "初始状态\0" );
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
	//若现在市场上有单，则撤单
	if ( info.orderVol > info.tradeVol + info.canceledVol ){
		OrderCancel( pTrader, pOrder, &info, stream, action_no );
	}
	while ( (clock() - start_time < maxSeconds * finalRatio * 1000) && !getTrade ){
		//设置订单价格
		pOrder->orderPrice += minGap;
		for ( int i = 0; i < nTime; i++ ){
			//下单
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
			//撤单
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
	fprintf( stream, "%s\t%d\t\t%d\t\t%s\t%d\t%d\t%d\t%d\t%6.3f\n", asctime(localtime(&rawtime)), actNo, pOrder->orderNo, (insert?("下单"):("撤单")), pInfo->orderVol, 
		pInfo->tradeVol, (pInfo->orderVol - pInfo->tradeVol - pInfo->canceledVol), pInfo->canceledVol, pInfo->orderPrice);
}

DWORD WINAPI UpdateOrderVol( void *para )
{
	printf("***Start Update Order Volumn Thread...\n");
	//登陆服务器
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
				//删除此orderNo信息
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
	//登陆服务器
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

	//循环
	while(true){
		//printf("***Check Price...\n");
		trader.GetTickQuotation(query, info, global_symbol.size());
		for ( int i = 0; i < global_symbol.size(); i++ ){
			//检查买价是否有变化
			if ( global_symbol_price[global_symbol[i]].first != info[i].bp1 ){
				printf("***%d\tBuy Price Changed, old price = %3.7f, new price = %3.7f\n", GetCurrentThreadId(), info[i].bp1, global_symbol_price[global_symbol[i]].first);
				global_symbol_price[global_symbol[i]].first = info[i].bp1;
				//global_event[global_symbol[i]].first.SetEvent();
				EnterCriticalSection(&PriceChangeCriticalSection);
				strcpy(price_change_symbol, global_symbol[i].c_str());
				price_change_direction = 0;
				LeaveCriticalSection(&PriceChangeCriticalSection);
				printf("***%d\t发送价格变化信号\n", GetCurrentThreadId());
				PriceChangeEvent.SetEvent();
			}
			//检查卖价是否有变化
			if ( global_symbol_price[global_symbol[i]].second != info[i].sp1 ){
				printf("***%d\tSell Price Changed, old price = %3.7f, new price = %3.7f\n", GetCurrentThreadId(), info[i].sp1, global_symbol_price[global_symbol[i]].second);
				global_symbol_price[global_symbol[i]].second = info[i].sp1;
				//global_event[global_symbol[i]].second.SetEvent();
				EnterCriticalSection(&PriceChangeCriticalSection);
				strcpy(price_change_symbol, global_symbol[i].c_str());
				price_change_direction = 1;
				LeaveCriticalSection(&PriceChangeCriticalSection);
				printf("***%d\t发送价格变化信号\n", GetCurrentThreadId());
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
	//读取本地交易编号action_no
	FILE *action_stream;
	action_stream = fopen( ACTION_PATH.c_str(), "r" );
	int no = 0;
	int action_no;
	while (fscanf( action_stream, "%d", &no ) != EOF){
		action_no = no;
	}
	if (action_stream == NULL){
		printf("***[Log Print Thread]%d\t本地交易编号记录文件读取失败！\n");
		getchar();
		return -1;
	}
	action_no++;
	fclose(action_stream);
	action_stream = fopen( ACTION_PATH.c_str(), "a");

	//打开log文件流

	stream = fopen( LOG_PATH.c_str(), "w" );
	if (stream == NULL){
		printf("***[Log Print Thread]%d\tLog记录文件打开失败！\n");
		getchar();
		return -1;
	}
	time_t rawtime;
	while (true){
		WaitForSingleObject(PrintLogEvent, INFINITE);
		PrintLogEvent.ResetEvent();
		if (logMsgQueue.empty()){
			printf("***[Log Print Thread]%d\tError!队列中无待打印log信息!\n");
			continue;
		}
		while(!logMsgQueue.empty()){
			LogMessage& oneMsg = logMsgQueue.front();
			//输出最前一条Log信息
			printf("print 编号 = %d 的log信息\n", action_no);

			time(&rawtime);
			if (oneMsg.cmd_status == CANCEL_RETURN || oneMsg.cmd_status == INSERT_RETURN){
				fprintf(stream, "时间 = %s本地编号 = %d,\t服务器编号 = %d,\t指令状态 = %d,\t指令返回 = %d,\t服务器状态 = %d,\t买卖方向 = %d,\t开平方向 = %d,\t订单总量 = %d,\t已成交数量 = %d,\t已撤单数量 = %d,\t价格 = %7.3f\n", 
					asctime(localtime(&rawtime)), action_no, oneMsg.server_no, oneMsg.cmd_status, oneMsg.cmd_return, oneMsg.server_status, oneMsg.direction, oneMsg.offset, oneMsg.order_vol, oneMsg.trade_vol, oneMsg.cancel_vol, oneMsg.price);
			}else{
				fprintf(stream, "时间 = %s本地编号 = %d,\t服务器编号 = %d,\t指令状态 = %d,\t服务器状态 = %d,\t买卖方向 = %d,\t开平方向 = %d,\t订单总量 = %d,\t已成交数量 = %d,\t已撤单数量 = %d,\t价格 = %7.3f\n", 
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
			printf("***最新价格：%6.3f\n", lastPrice);
			count_time = clock() - start_time;
		}
		if ( cur_time - start_time >= 1000 * maxSeconds * (1 - finalRatio) ){
			//进入最终步骤
			return GotoFinalStep( trader, orderReq, maxSeconds, nTime, finalRatio, minGap );
			break;
		}else{
			//获取最新价格
			CIDMP_TICK_QUOTATION_QUERY *realQuery = new CIDMP_TICK_QUOTATION_QUERY[1];
			CIDMP_TICK_QUOTATION_INFO *realInfo = new CIDMP_TICK_QUOTATION_INFO[1];
			realQuery->exchgcode = EXCHANGE;
			strcpy( realQuery->symbol, SYMBOL );
			trader.GetTickQuotation( realQuery, realInfo, 1 );
			if ( orderReq.direction == 0 ){		//如果订单方向为买
				lastPrice = realInfo[0].bp1 - 10;			//获取最新买一价			
			}else{
				lastPrice = realInfo[0].sp1;			//获取最新卖一价
			}//if
			if ( orderInMarket ){
				if ( lastPrice != priceInOrder ){
					//Risk
					orderReq.orderPrice = lastPrice;
					int riskNum = 0;
	//				printf("委托信息:合约=%s,买卖=%d,开平=%d,价格=%6.3f,数量=%d\n", orderReq.symbol, orderReq.direction, orderReq.offsetFlagType, orderReq.orderPrice, orderReq.orderVol);
					int riskSuccess = trader.RiskTest( orderReq, riskNum );
					switch (riskSuccess){
					case -1:
						printf("***试算失败！失败原因——【未登录】\n");
						break;
					case -2:
						printf("***试算失败！失败原因——【查询流水号失败】\n");
						break;
					case -3:
						printf("***试算失败！失败原因——【风控未通过】\n");
						break;
					case -5:
						printf("***试算失败！失败原因——【fundId或cellId不存在或未被授权】\n");
						break;
					default:
						break;
					}//switch
					if ( riskNum > 0 ){
						printf("***触发风控数目:%d\n",riskNum);
						return -1;
					}
										
					//OrderCancel撤单
					char errorMsg[200];
	//				PrintOrderMsg(orderReq);
					
					int cancelSuccess = trader.CancelOrder( orderReq.orderNo, errorMsg );//orderNo与batNo区别 
					if ( cancelSuccess < 0 ){	//撤单失败的操作 todo
						printf("***撤单失败！\n");
						return -1;
					}else{
						printf("***撤单成功！\n");
						orderInMarket = false;
						priceInOrder = 0.0;
					}
				}else{
					//获取最新交易信息Get Trade
					CIDMP_ORDER_INFO orderInfo;
					getTrade = GetRealTrade( trader, orderReq, orderInfo );
					continue;
				}//if
			}else{
				//Risk
				//更新最新价格到委托订单信息中
				orderReq.orderPrice = lastPrice;
				int riskNum = 0;
		//		printf("委托信息:合约=%s,买卖=%d,开平=%d,价格=%6.3f,数量=%d\n", orderReq.symbol, orderReq.direction, orderReq.offsetFlagType, orderReq.orderPrice, orderReq.orderVol);
				int riskSuccess = trader.RiskTest( orderReq, riskNum );
				switch (riskSuccess){
				case -1:
					printf("***试算失败！失败原因——【未登录】\n");
					break;
				case -2:
					printf("***试算失败！失败原因——【查询流水号失败】\n");
					break;
				case -3:
					printf("***试算失败！失败原因——【风控未通过】\n");
					break;
				case -5:
					printf("***试算失败！失败原因——【fundId或cellId不存在或未被授权】\n");
					break;
				default:
					break;
				}//switch
				if ( riskNum > 0 ){
					printf("***触发风控数目:%d\n",riskNum);
					return -1;
				}
			}//if
			//OrderInsert下单
			//更新订单数量
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
					printf("***订单委托失败！失败原因——【未登录】\n");
					break;
				case -2:
					printf("***订单委托失败！失败原因——【查询流水号失败】\n");
					break;
				case -3:
					printf("***订单委托失败！失败原因——【风控未通过】\n");
					break;
				case -4:
					printf("***订单委托失败！失败原因——【下单失败】\n");

					break;
				case -5:
					printf("***订单委托失败！失败原因——【fundId或cellId不存在或未被授权】\n");
					break;
				default:
					break;
				}

				getchar();
				return -1;
			}else{
				printf( "***订单已委托成功！\n" );
				PrintOrderMsg(orderReq);
				orderInMarket = true;
				priceInOrder = lastPrice;
			}
			//获取最新交易信息Get Trade
			getTrade = GetRealTrade( trader, orderReq, orderInfo );
		}//if
	}//while

	return 1;
}

inline void PrintOrderMsg( CIDMP_ORDER_REQ &orderReq )
{			
	printf("委托号\t资产账户\t资产单元\t投资组合\t证券/合约代码\t买卖\t开平\t价格\t数量\n");
	printf("%d\t%d\t%d\t%I64d\t%s\t%d\t%d\t%6.3f\t%d\n\n", orderReq.orderNo, orderReq.fundId, orderReq.cellId, orderReq.proflId,
		orderReq.symbol, orderReq.direction, orderReq.offsetFlagType, orderReq.orderPrice, orderReq.orderVol );	
}

inline bool GetRealTrade( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, CIDMP_ORDER_INFO &orderInfo )
{
//	printf("\n***查询委托信息：\n");
	
	int checkSuccess = trader.GetOrder( orderReq.orderNo, orderInfo );
	if(checkSuccess != 1) {	    
		printf("查询委托信息失败!\n");
	}
//	printf("委托号\t状态\t证券/合约代码\t买卖\t开平\t价格\t数量\t时间\t交易员\t错误信息\n");
//	printf("%d\t%d\t%s\t%d\t%d\t%6.3f\t%d\t%s\t%s\t%s\n",
//		orderInfo.orderNo, orderInfo.orderStatus, orderInfo.symbol, orderInfo.direction, orderInfo.offsetFlagType,
//		orderInfo.orderPrice, orderInfo.orderVol, orderInfo.orderTime, orderInfo.userId, orderInfo.errorInfo);	
	//区分部分成交与全部成交 todo
//	printf("%6.3f\n", orderReq.orderPrice);
	return (orderInfo.orderStatus == 6);	//1未报2待报3废单4已报5部成6全成7已报撤单8部成9撤单待撤 

}

int GotoFinalStep( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTime, double finalRatio, double minGap )
{
	clock_t start_time = clock();
	int max_time = 1000 * maxSeconds * finalRatio;
	bool orderInMarket = true;

	while ( clock() - start_time <= max_time ){	//没有超时
		for ( int i  = 0; i < N_TIME; i++ ){
			//价格增加
			//是否需要检查价格上限？ todo
			orderReq.orderPrice += minGap * nTime;
			//下单
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
					printf("***订单委托失败！失败原因——【未登录】\n");
					break;
				case -2:
					printf("***订单委托失败！失败原因——【查询流水号失败】\n");
					break;
				case -3:
					printf("***订单委托失败！失败原因——【风控未通过】\n");
					break;
				case -4:
					printf("***订单委托失败！失败原因——【下单失败】\n");
					break;
				case -5:
					printf("***订单委托失败！失败原因——【fundId或cellId不存在或未被授权】\n");
					break;
				default:
					break;
				}
				return -1;
			}else{
				printf( "***订单已委托成功！\n" );
				orderInMarket = true;
				PrintOrderMsg(orderReq);
			}
			
			Sleep(1000);

			//撤单
			char errorMsg[200];
			int cancelSuccess = trader.CancelOrder( orderReq.orderNo, errorMsg );//orderNo与batNo区别 
			if ( cancelSuccess < 0 ){	//撤单失败的操作 todo
				printf("***撤单失败！\n");
				printf("***%s\n", errorMsg);
				CIDMP_ORDER_INFO orderInfo;
				if ( GetRealTrade( trader, orderReq, orderInfo ) ){
					printf("***交易成功！\n");
					return 1;
				}else{
					if (orderInMarket){
							printf("委托号\t状态\t证券/合约代码\t买卖\t开平\t价格\t数量\t时间\t交易员\t错误信息\n");
							printf("%d\t%d\t%s\t%d\t%d\t%6.3f\t%d\t%s\t%s\t%s\n",
								orderInfo.orderNo, orderInfo.orderStatus, orderInfo.symbol, orderInfo.direction, orderInfo.offsetFlagType,
								orderInfo.orderPrice, orderInfo.orderVol, orderInfo.orderTime, orderInfo.userId, orderInfo.errorInfo);	
						return -2;
					}
				}
			}else{
				printf("***撤单成功！\n");
				orderInMarket = false;
			}
			if ( GetRealTrade( trader, orderReq, orderInfo ) ){
				printf("***交易成功！\n");
				return 1;
			}
		}
	}
	return 0;
}
*/