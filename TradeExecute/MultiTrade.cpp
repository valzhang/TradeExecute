
#include <iostream>
#include <queue>
#include "stdafx.h"
//常量定义区
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
//const char SYMBOL[8] = "IF1409";	//股票或期货代码
const int INI_PRICE = 0;
const int INI_VOL = 4;
const int FUND_SIZE = 100;
const int HOLD_SIZE = 300;
const int ORDER_SIZE = 300;
const int N_TIME = 3;
const int MAX_SECONDS = 600;
const int N_TICKS = 1;
const double FINAL_RATIO = 0.05;
//const int MAX_INSERT_SECS = 5;	//下单最长等待时间
const string LOG_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\log.txt";
const string TRANS_LOG_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\transaction_log.txt";
const string ACTION_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\action_log.txt";
const string SYMBOL_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\symbol.txt";
const int WAIT_MILLSECS = 500;
const int MAX_THREAD_NUM = 1;

//TDF常量定义区
const char TDF_SERVER_IP[32] = "114.80.154.34";
const char TDF_SERVER_PORT[8] = "10051";
const char TDF_USERNAME[32] = "TD7100001";
const char TDF_PASSWORD[32] = "44053825";

//类型申明区


//缓冲区变量
boost::thread_group global_group;
//queue<TraderCommand> global_cmd_queue;
CMDQueueBuffer cmd_buffer(10);
PriceBuffer price_buffer(SYMBOL_PATH);
bool global_program_termination = false;
boost::mutex global_create_thread_mutex;
boost::mutex global_order_no_mutex;
boost::condition_variable_any global_create_thread_condition;
list<int> global_order_no;
hash_map<int, int>global_order_vol;
LogMsgBuffer global_log_buffer(LOG_PATH, ACTION_PATH, TRANS_LOG_PATH);
//vector<string> global_all_symbol;
//hash_map<string, pair<double, double> > global_all_symbol_price;

//函数申明区
int LogMsgTest(CIDMPTradeApi &trader);
void TradeThread(int id_no);
void ReceiveCommandThread();
void ManageThread();
//int ReadSymbal();
void UpdatePriceThread();
void UpdateOrderNoThread();
void TDFUpdatePriceThread();
void PrintLogThread();
void PrintTransLogThread();
bool GoToFinalStep(CIDMPTradeApi &trader, CIDMP_ORDER_REQ &order_req, int max_secs, double gap, int try_times, int id_no, double total_price, int total_vol);
void RecvSys(THANDLE hTdf, TDF_MSG* pSysMsg);
void RecvData(THANDLE hTdf, TDF_MSG* pDataMsg);
void PrintMarketData(TDF_MARKET_DATA* pMarket, int item_count);
void PrintFutureData(TDF_FUTURE_DATA* pData, int item_count);
char* intarr2str(char* szBuf, int nBufLen, int arr[], int n);
char* chararr2str(char* szBuf, int nBufLen, char arr[], int n);
double intarr2double(const unsigned int* data);

int main()
{
	CIDMPTradeApi trader;
	printf("[Main Thread]***主程序开始\n");
	if (LogMsgTest(trader) < 0)
	{
		return -1;
	}



	boost::thread t1(ManageThread);
	boost::thread t2(ReceiveCommandThread);
//	boost::thread t3(UpdatePriceThread);
	boost::thread t3(TDFUpdatePriceThread);
	boost::thread t4(UpdateOrderNoThread);
	boost::thread t5(PrintLogThread);
	boost::thread t6(PrintTransLogThread);

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
	printf("[Main Thread]***主程序结束\n");
	getchar();
	system("PAUSE");
	return 0;
}

int LogMsgTest(CIDMPTradeApi &trader)
{
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
		return -1;
	}
	printf("***登录成功！\n服务器IP：%s,\t服务器端口：%d,\t用户名：%s...\n", SERVER_IP, SERVER_PORT, USER_NAME);

	//获取资产账户列表
	printf("***获取资产账户列表……\n");
	CIDMP_FUND_ACCOUNT_INFO accountInfo[FUND_SIZE];
	int accountSize = trader.GetFundAccount( accountInfo, FUND_SIZE );
	if (accountSize < 0){
		printf("获取资产账户列表失败！\n");
		getchar();
		return -1;
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
		return -1;
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
		return -1;
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
		return -1;
	}

	printf("现货总资金\t: %6.3f\n现货可用\t: %6.3f\n期货总资金\t: %6.3f\n期货可用\t: %6.3f\n\n",
		fInfo.stkTotal,fInfo.stkAvailable, fInfo.futureTotal, fInfo.futureAvailable);


	printf("\n***获得资产账户[%d]的资产单元[%d]的资金信息...\n", FUND_ID, CELL_ID);
	CIDMP_FUND_INFO cInfo;
	nSuccess = trader.GetFund( FUND_ID, CELL_ID, cInfo);

	if(nSuccess != 1) {
		printf("查询资产单元的资金信息失败\n");
		getchar();
		return -1;
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
		return -1;
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
		return -1;
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
		return -1;
	}

	printf("[序号]\t代码\t交易所代号\t持仓类型\t可用数量\t总数量\t可平今仓\t可平昨仓\n");
	for(int i = 0; i < nSize && i < HOLD_SIZE; i ++) {
		printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\n",
			i+1, holdInfo2[i].symbol, holdInfo2[i].exchgcode, holdInfo2[i].holdType, holdInfo2[i].available, holdInfo2[i].total,
			holdInfo2[i].available_jr, holdInfo2[i].available_zr);
	}

	return 0;

}

void ReceiveCommandThread()
{
	int count = 0;
	printf("[Receive Command Thread]***线程开始\n");
	while(true){
		printf("[Receive Command Thread]***是否继续交易？(Y/y继续，其他终止程序)\n");
		char flag;
		//scanf("%c", &flag);
		cin >> flag;
		if ( flag != 'y' && flag != 'Y' ){
			global_program_termination = true;
			global_create_thread_condition.notify_all();
	//		printf("[Receive Command Thread]***线程结束\n");
			break;
		}
		printf("[Receive Command Thread]***请输出交易指令：\n");
		//debug
		getchar();
		int exchange, direction, offset, vol, pj, secs;
		char symbol[8];
//		scanf("%d%s%d%d%d%d%d", &exchange, &symbol, &direction, &offset, &vol, &pj, &secs);
		try
		{
			cout << "代码 = ";
			cin >> symbol;
			cout << "买卖方向(0 买; 1 卖) = ";
			cin >> direction;
			cout << "开平方向(0 开; 1 平) = ";
			cin >> offset;
			cout << "交易数量(手) = ";
			cin >> vol;
			cout << "是否平今(0 否; 1 是) = ";
			cin >> pj;
			cout << "交易时间(secs) = ";
			cin >> secs;
		}
		
		catch (CException* e)
		{
			cout << e->ReportError();
			continue;
		}
		TraderCommand cmd;
		cmd.fund_id = FUND_ID; cmd.cell_id = CELL_ID; cmd.profl_id = PROFL_ID;
		//cmd.exchange_code = 6; strcpy(cmd.symbol, "IF1409"); cmd.direction = 0; cmd.offset_flag_type = 0; cmd.order_volumn = 1+count; cmd.pj = 0; cmd.max_seconds = 600;
		cmd.exchange_code = 6; strcpy(cmd.symbol, symbol); cmd.direction = direction; cmd.offset_flag_type = offset; cmd.order_volumn = vol; cmd.pj = pj; cmd.max_seconds = secs;
	//	++count;
//		global_cmd_queue.push(cmd);
		cmd_buffer.PutCommand(cmd);
	}
	printf("[Receive Command Thread]***线程结束\n");
}

void ManageThread()
{
	int count = 0;
	printf("[Manage Thread]***线程开始\n");
	//开启Max个线程
	for ( int i = 0; i < MAX_THREAD_NUM; i++ ){
		global_group.create_thread(boost::bind(TradeThread, count));
		++count;
	}
	
	printf("[Manage Thread]***初始线程池建设完成\n");
	while(true){
		//等待通知	todo
		{
			boost::mutex::scoped_lock lock(global_create_thread_mutex);
			global_create_thread_condition.wait(global_create_thread_mutex);
		}
		if (global_program_termination){
			//while(global_group.size() > 0){
				cmd_buffer.ReleaseCommand();
				global_log_buffer.Release();
				printf("[Manage Thread]***释放cmdbuffer线程\n");
				//boost::this_thread::sleep(boost::posix_time::seconds(1));
			//}
			break;
		}
		//开启一个线程
		global_group.create_thread(bind(TradeThread, count));
		++count;
	}
	global_group.join_all();
	printf("[Manage Thread]***线程结束\n");
}

void TradeThread(int id_no)
{
	printf("[Trader Command Thread %d]***线程开始...时间 = %s\n", id_no, boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()).c_str() );

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
	boost::posix_time::ptime start_time = boost::posix_time::second_clock::local_time();
	printf("[Trader Command Thread %d]***准备登陆...时间 = %s\n", id_no, boost::posix_time::to_iso_string(start_time).c_str() );
	int nSuccess = trader.Connect(serverIP, serverPort, userID, userPW, 0);
	if (nSuccess != 1){
		printf("[Trader Command Thread %d]***登陆失败\n", id_no);
		getchar();
		return;
	}
	printf("[Trader Command Thread %d]***登陆成功,所用时间 = %s\n", id_no, boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time() - start_time).c_str());
	TraderCommand cmd;
	if (global_program_termination){
		printf("[Trader Command Thread %d]***线程结束\n", id_no);
		return;
	}
	cmd_buffer.GetCommand(&cmd);
	if (global_program_termination){
		printf("[Trader Command Thread %d]***线程结束\n", id_no);
		return;
	}
	double total_price = 0;
	int total_vol = 0;

	//通知新建线程
	
	global_create_thread_condition.notify_one();

	printf("[Trader Thread %d]***装配订单\n", id_no);
	//装配订单
	CIDMP_ORDER_REQ order_req;
	CIDMP_ORDER_INFO order_info;
	order_req.fundId = cmd.fund_id;
	order_req.cellId = cmd.cell_id;
	order_req.proflId = cmd.profl_id;
	order_req.exchgcode = cmd.exchange_code;
	strcpy( order_req.symbol, cmd.symbol );
	order_req.direction = cmd.direction;
	order_req.offsetFlagType = cmd.offset_flag_type;
	order_req.orderVol = cmd.order_volumn;
	order_req.pj = cmd.pj;
	
	printf("[Trader Thread %d]***获取最新价格\n", id_no);
	//获取最新价格
	order_req.orderPrice = price_buffer.GetLastPrice(order_req.symbol, order_req.direction);

	int risk_num = 0;
	//提交订单
	printf("[Trader Thread %d]***提交订单\n", id_no);
	global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), -1, "Order Insert", 1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);
	nSuccess = trader.OrderInsert(order_req, risk_num);
	global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Insert Return", nSuccess, 1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);
	if (nSuccess < 0){
		printf("[Trader Thread %d]***下单失败！\n", id_no);
		printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
		printf("[Trader Thread %d]***下单价格 = %7.3f\t下单数量 = %d\n", id_no, order_req.orderPrice, order_req.orderVol);
		return;
	}
	//确认下单成功
	trader.GetOrderByOrderNo(order_req.orderNo, order_info);	
	while (order_info.orderStatus < 3){
		boost::this_thread::sleep(boost::posix_time::millisec(100));
		trader.GetOrderByOrderNo(order_req.orderNo, order_info);
	}
	global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
	if (order_info.orderStatus == 3){
		printf("[Trader Thread %d]***下单失败！\n", id_no);
		printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
		printf("[Trader Thread %d]***下单价格 = %7.3f\t下单数量 = %d\n", id_no, order_req.orderPrice, order_req.orderVol);

		return;
	}
	if (order_info.orderStatus > 6){
		printf("[Trader Thread %d]***服务器自动撤单！\n", id_no);
		printf("[Trader Thread %d]***订单状态 = %d\n", id_no, order_info.orderStatus);
				
		return;
	}
	printf("[Trader Thread %d]***下单成功！下单价格 = %7.3f\n", id_no, order_req.orderPrice);
	global_order_no_mutex.lock();
	global_order_no.push_back(order_req.orderNo);
	global_order_vol[order_req.orderNo] = order_req.orderVol;
	global_order_no_mutex.unlock();

	bool get_trade = false;
	//开始交易循环
	boost::posix_time::ptime begin_time = boost::posix_time::microsec_clock::local_time();
	while ((boost::posix_time::microsec_clock::local_time() - begin_time).total_milliseconds() <= MAX_SECONDS*(1-FINAL_RATIO)*1000){
		double new_price = price_buffer.WaitPriceChange(order_req.symbol, order_req.direction);
		if (new_price == 0){//超时
			//检查是否成交
			if (order_info.orderStatus == 6){
				printf("[Trader Thread %d]***订单成交！\n", id_no);
				get_trade = true;
				break;
			}else{
				continue;
			}
		}
		printf("***获取最新价格成功%7.3f\n", new_price);
		if (global_order_vol[order_req.orderNo] == 0){
			trader.GetOrderByOrderNo(order_req.orderNo, order_info);
			if (order_info.orderStatus == 6){
				printf("[Trader Thread %d]***订单成交！\n", id_no);
			get_trade = true;				
			}else{
				if (order_info.orderStatus == 7){
					printf("[Trader Thread %d]***订单失败！服务器自动撤单！程序返回...\n", id_no);
				}
				printf("[Trader Thread %d]***订单失败！\n", id_no);
				printf("[Trader Thread %d]***订单状态 = %d！\n", id_no, order_info.orderStatus);
			}
			break;
		}
		//撤单
		printf("***撤单\n");
		//更新数量
		if (order_req.orderVol != global_order_vol[order_req.orderNo]){
			total_vol += order_req.orderVol - global_order_vol[order_req.orderNo];
			total_price += order_req.orderPrice * (total_vol =order_req.orderVol - global_order_vol[order_req.orderNo]);			
			global_log_buffer.PutTransMsg(boost::posix_time::second_clock::local_time(), order_req.orderVol, order_req.orderVol - global_order_vol[order_req.orderNo], order_req.orderPrice, total_price/(double)total_vol );
			order_req.orderVol = global_order_vol[order_req.orderNo];	
		}
		char errorMsg[200];
		//trader.GetOrderByOrderNo(order_req.orderNo, order_info);
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Cancel", order_info.orderStatus, order_req.direction, order_req.offsetFlagType, order_info.orderVol, order_info.tradeVol, order_info.canceledVol, order_info.orderPrice);
		nSuccess = trader.CancelOrder(order_req.orderNo, errorMsg);
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Cancel Return", nSuccess, order_info.orderStatus, order_req.direction, order_req.offsetFlagType, order_info.orderVol, order_info.tradeVol, order_info.canceledVol, order_info.orderPrice);
		trader.GetOrderByOrderNo(order_req.orderNo, order_info);

		while (order_info.orderStatus < 6){
			boost::this_thread::sleep(boost::posix_time::millisec(100));
			printf("***撤单中...\n");
			trader.GetOrderByOrderNo(order_req.orderNo, order_info);
		}
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Update Order", order_info.orderStatus, order_req.direction, order_req.offsetFlagType, order_info.orderVol, order_info.tradeVol, order_info.canceledVol, order_info.orderPrice);
		printf("%d\n", order_info.orderStatus);
		if (order_info.orderStatus == 6){
			get_trade = true;
			printf("[Trader Thread %d]***订单成交！\n", id_no);
			break;
		}
		printf("[Trader Thread %d]***撤单成功！\n", id_no);

		//更新价格
		order_req.orderPrice = new_price;
		
		
		//下单
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), -1, "Order Insert", -1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);
		nSuccess = trader.OrderInsert(order_req, risk_num);
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Insert Return", nSuccess, 1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);

		if (nSuccess < 0){
			printf("[Trader Thread %d]***下单失败！\n", id_no);
			printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
			printf("[Trader Thread %d]***下单价格 = %7.3f\t下单数量 = %d\n", id_no, order_req.orderPrice, order_req.orderVol);
			return;
		}
		//确认下单成功
		trader.GetOrderByOrderNo(order_req.orderNo, order_info);
		while (order_info.orderStatus < 3){
			boost::this_thread::sleep(boost::posix_time::millisec(100));
			trader.GetOrderByOrderNo(order_req.orderNo, order_info);
		}
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
		if (order_info.orderStatus == 3){
			printf("[Trader Thread %d]***下单失败！\n", id_no);
			printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
			printf("[Trader Thread %d]***下单价格 = %7.3f\t下单数量 = %d\n", id_no, order_req.orderPrice, order_req.orderVol);
			return;
		}
		if (order_info.orderStatus > 6){
			printf("[Trader Thread %d]***服务器自动撤单！\n", id_no);
			printf("[Trader Thread %d]***订单状态 = %d\n", id_no, order_info.orderStatus);

			return;
		}
		printf("[Trader Thread %d]***下单成功！下单价格 = %7.3f\n", id_no, order_req.orderPrice);
		global_order_no_mutex.lock();
		global_order_no.push_back(order_req.orderNo);
		global_order_vol[order_req.orderNo] = order_req.orderVol;
		global_order_no_mutex.unlock();
		
	}

	if (!get_trade){
		//如果有单，撤单
		trader.GetOrderByOrderNo(order_req.orderNo, order_info);
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
		if (order_info.orderVol > order_info.tradeVol + order_info.canceledVol){
			printf("***撤单\n");
			char errorMsg[200];
			//trader.GetOrderByOrderNo(order_req.orderNo, order_info);
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Cancel", order_info.orderStatus, order_req.direction, order_req.offsetFlagType, order_info.orderVol, order_info.tradeVol, order_info.canceledVol, order_info.orderPrice);
			nSuccess = trader.CancelOrder(order_req.orderNo, errorMsg);
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Cancel Return", nSuccess, order_info.orderStatus, order_req.direction, order_req.offsetFlagType, order_info.orderVol, order_info.tradeVol, order_info.canceledVol, order_info.orderPrice);
			trader.GetOrderByOrderNo(order_req.orderNo, order_info);

			while (order_info.orderStatus < 6){
				boost::this_thread::sleep(boost::posix_time::millisec(100));
				trader.GetOrderByOrderNo(order_req.orderNo, order_info);
			}
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
			printf("[Trader Thread %d]***撤单成功！\n", id_no);
		}
		//计算minGap
		double minGap;
		if (order_req.exchgcode < 3){
			CIDMP_STOCK_INFO *info = new CIDMP_STOCK_INFO[6000];
			int stkinfo2 = trader.GetSysStockInfo(false,info,6000);
			for (int i = 0; i < stkinfo2; i++){
				if ( strcmp( info[i].symbol, order_req.symbol ) == 0 ){
					minGap = info[i].priceUnit;
					delete []info;
					break;
				}
			}
		}else{
			CIDMP_FUTRUE_INFO *ftinfo = new CIDMP_FUTRUE_INFO[3000];
			int ftinfoSize = trader.GetSysFutrueInfo(false,ftinfo,3000);
			for (int i = 0; i < ftinfoSize; i++){
				if ( strcmp( ftinfo[i].symbol, order_req.symbol ) == 0 ){
					minGap = ftinfo[i].priceUnit;
					delete []ftinfo;
					break;
				}
			}
		}
		if (order_req.direction == 1){
			minGap = 0 - minGap;
		}
		get_trade = GoToFinalStep(trader, order_req, MAX_SECONDS * FINAL_RATIO, minGap, N_TIME, id_no, total_price, total_vol);
		if (!get_trade){
			printf("[Trader Thread %d]***订单超时\n", id_no);
		}
	}else{
		total_vol += order_req.orderVol;
		total_price += order_req.orderPrice * order_req.orderVol;
		global_log_buffer.PutTransMsg(boost::posix_time::second_clock::local_time(), order_req.orderVol, order_req.orderVol, order_req.orderPrice, total_price/(double)total_vol);
		printf("增加一条成交log信息\n");
	}


	//boost::this_thread::sleep(boost::posix_time::seconds(10));

	printf("[Trader Command Thread %d]***线程结束\n", id_no);
}


#if 0
int ReadSymbal()
{
	ifstream in;
	in.open(SYMBOL_PATH);
	if (!in){
		return -1;
	}
	string tmp;
	while (getline(in, tmp)){
		if (tmp.length() > 3){
			global_all_symbol.push_back(tmp);
			global_all_symbol_price[tmp] = make_pair(0, 0);
		}
	}
}
#endif

void TDFUpdatePriceThread()
{
	TDF_OPEN_SETTING pSettings = {0};
	strcpy(pSettings.szIp, TDF_SERVER_IP);
	strcpy(pSettings.szPort, TDF_SERVER_PORT);
	strcpy(pSettings.szUser, TDF_USERNAME);
	strcpy(pSettings.szPwd, TDF_PASSWORD);
	pSettings.pfnSysMsgNotify = RecvSys;
	pSettings.pfnMsgHandler = RecvData;
	pSettings.szMarkets = "CF";
	string symbols;
	for ( int i = 0; i < price_buffer.all_symbol.size(); i++ ){
		symbols.append(price_buffer.all_symbol[i]);
		symbols.append(".CF;");
	}

	pSettings.szSubScriptions = symbols.c_str();
	
	TDF_ERR errorMsg;

	//设置心跳间隔时间 = 5s
	TDF_SetEnv(TDF_ENVIRON_HEART_BEAT_INTERVAL, 5);
	//设置3次未收到心跳信息认为掉线
	TDF_SetEnv(TDF_ENVIRON_MISSED_BEART_COUNT, 3);
	//设置每一个数据包接收超时时间 = 10s
	TDF_SetEnv(TDF_ENVIRON_OPEN_TIME_OUT, 10);

	THANDLE g_hTDF = TDF_Open(&pSettings, &errorMsg);
	while (true){
		boost::this_thread::sleep(boost::posix_time::seconds(1));
		if (global_program_termination){
			TDF_Close(g_hTDF);
			g_hTDF = NULL;
			break;
		}
	}
	printf("[TDF Update Price Thread]***线程结束\n");
}
void UpdatePriceThread()
{
	printf("[Update Price Thread]***线程开始...\n");
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
		return ;
	}

	delete[] serverIP;
	delete[] userID;
	delete[] userPW;
	printf("[Update Price Thread]***登陆成功！\n");
	CIDMP_TICK_QUOTATION_QUERY *query = new CIDMP_TICK_QUOTATION_QUERY[price_buffer.all_symbol.size()+1];
	CIDMP_TICK_QUOTATION_INFO *info = new CIDMP_TICK_QUOTATION_INFO[price_buffer.all_symbol.size()+1];
	for ( int i = 0; i < price_buffer.all_symbol.size(); i++ ){
		(query+i)->exchgcode = EXCHANGE;
		strcpy((query+i)->symbol, price_buffer.all_symbol[i].c_str());
	}

	while(true){
		if (global_program_termination){
			printf("[Update Price Thread]***退出循环！\n");
			break;
		}
		trader.GetTickQuotation(query, info, price_buffer.all_symbol.size());
		for ( int i = 0; i < price_buffer.all_symbol.size(); i++ ){
			price_buffer.UpdatePrice(info[i].bp1, price_buffer.all_symbol[i], 0);
			price_buffer.UpdatePrice(info[i].sp1, price_buffer.all_symbol[i], 1);

#if 0
			//检查买价是否有变化
			if (global_all_symbol_price[global_all_symbol[i]].first != info[i].bp1){
				printf("[Update Price Thread]***%s买价变化，原价 = %7.3f，新价 = %7.3f\n", global_all_symbol[i].c_str(), global_all_symbol_price[global_all_symbol[i]].first, info[i].bp1);
				global_all_symbol_price[global_all_symbol[i]].first = info[i].bp1;
				//发送价格变化信号
			}
			//检查卖价是否有变化
			if (global_all_symbol_price[global_all_symbol[i]].second != info[i].sp1){
				printf("[Update Price Thread]***%s卖价变化，原价 = %7.3f，新价 = %7.3f\n", global_all_symbol[i].c_str(), global_all_symbol_price[global_all_symbol[i]].second, info[i].sp1);
				global_all_symbol_price[global_all_symbol[i]].second = info[i].sp1;
				//发送价格变化信号
			}
#endif
		}
	}

	printf("[Update Price Thread]***线程结束\n");
}

void UpdateOrderNoThread()
{
	printf("[Update Order No Thread]***线程开始...\n");
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
		printf("[Update Order No Thread]***Update Volumn Thread Login Failed!\n");
		getchar();
		return ;
	}
	printf("[Update Order No Thread]***登陆成功!\n");

	delete []serverIP;
	delete []userID;
	delete []userPW;
	while (true){
		if (global_program_termination){
			printf("[Update Order No Thread]***退出循环!\n");
			break;
		}
		if (global_order_no.size() == 0){
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			continue;
		}
	//	printf("[Update Order No Thread]***更新订单信息\n");
		for ( list<int>::iterator it = global_order_no.begin(); it != global_order_no.end(); ){
			int order_no = *it;
			trader.GetOrderByOrderNo( order_no, info );
			if ( info.orderStatus == 3 || info.orderStatus > 5){
				global_order_no_mutex.lock();
				global_order_no.erase(it++);
				global_order_vol.erase(order_no);
				global_order_no_mutex.unlock();
			}else{
				it++;
				global_order_vol[order_no] = info.orderVol - info.tradeVol;
				cout << global_order_vol[order_no] << endl;
			}
		}
		boost::this_thread::sleep(boost::posix_time::millisec(500));
	}


	printf("[Update Order No Thread]***线程结束\n");
}

bool GoToFinalStep(CIDMPTradeApi &trader, CIDMP_ORDER_REQ &order_req, int max_secs, double gap, int try_times, int id_no, double total_price, int total_vol)
{
	CIDMP_ORDER_INFO order_info;
	bool get_trade =false;
	order_req.orderPrice = price_buffer.GetLastPrice(order_req.symbol, order_req.direction);
	boost::posix_time::ptime begin_time = boost::posix_time::microsec_clock::local_time();
	while ( (boost::posix_time::microsec_clock::local_time() - begin_time).total_milliseconds() <= max_secs * 1000 && !get_trade ){
		//设置订单价格
		order_req.orderPrice += gap;
		for ( int i = 0; i < try_times; i++ ){
			//下单
			int risk_num;
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), -1, "Order Insert", -1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);
			int nSuccess = trader.OrderInsert(order_req, risk_num);
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Insert Return", nSuccess, 1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);
			if (nSuccess < 0){
				printf("[Trader Thread %d]***下单失败！\n", id_no);
				printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
				printf("[Trader Thread %d]***下单价格 = %7.3f\t下单数量 = %d\n", id_no, order_req.orderPrice, order_req.orderVol);
				return false;
			}
			//确认下单成功
			trader.GetOrderByOrderNo(order_req.orderNo, order_info);
			while (order_info.orderStatus < 3){
				boost::this_thread::sleep(boost::posix_time::millisec(100));
				trader.GetOrderByOrderNo(order_req.orderNo, order_info);
			}
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
			if (order_info.orderStatus == 3){
				printf("[Trader Thread %d]***下单失败！\n", id_no);
				printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
				printf("[Trader Thread %d]***下单价格 = %7.3f\t下单数量 = %d\n", id_no, order_req.orderPrice, order_req.orderVol);
				return false;
			}
			printf("[Trader Thread %d]***下单成功！下单价格 = %7.3f\n", id_no, order_req.orderPrice);
			//撤单
			printf("***撤单\n");
			char errorMsg[200];
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Cancel", order_info.orderStatus, order_req.direction, order_req.offsetFlagType, order_info.orderVol, order_info.tradeVol, order_info.canceledVol, order_info.orderPrice);
			nSuccess = trader.CancelOrder(order_req.orderNo, errorMsg);
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Cancel Return", nSuccess, order_info.orderStatus, order_req.direction, order_req.offsetFlagType, order_info.orderVol, order_info.tradeVol, order_info.canceledVol, order_info.orderPrice);

			while (order_info.orderStatus < 6){
				printf("***撤单中...\n");
				boost::this_thread::sleep(boost::posix_time::millisec(100));
				trader.GetOrderByOrderNo(order_req.orderNo, order_info);
			}
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
			printf("[Trader Thread %d]***撤单成功！\n", id_no);
			if (order_info.tradeVol > 0){
				total_price += order_info.tradeVol * order_req.orderPrice;
				total_vol += order_info.tradeVol;
				global_log_buffer.PutTransMsg(boost::posix_time::second_clock::local_time(), order_req.orderVol, order_info.tradeVol, order_req.orderPrice, total_price/(double)total_vol);
				order_req.orderVol -= order_info.tradeVol;
			}
			//检查是否交易成功
			if (order_info.orderVol == order_info.tradeVol){
				printf("[Trader Thread %d]***交易成功！成交价格 = %7.3f\n", id_no, order_req.orderPrice);
				global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
				return true;
			}
		}
	}
	return false;
}


void PrintMarketData(TDF_MARKET_DATA* pMarket, int item_count)
{

	printf("-------- Market, Count:%d --------\n", item_count);

	char szBuf1[512];
	char szBuf2[512];
	char szBuf3[512];
	char szBuf4[512];
	char szBufSmall[64];
	for (int i = 0; i < item_count; i++)
	{
		const TDF_MARKET_DATA& marketData = pMarket[i];
		printf("万得代码 szWindCode: %s\n", marketData.szWindCode);
		printf("原始代码 szCode: %s\n", marketData.szCode);
		printf("业务发生日(自然日) nActionDay: %d\n", marketData.nActionDay);
		printf("交易日 nTradingDay: %d\n", marketData.nTradingDay);
		printf("时间(HHMMSSmmm) nTime: %d\n", marketData.nTime);
		printf("状态 nStatus: %d(%c)\n", marketData.nStatus, SAFE_CHAR(marketData.nStatus));
		printf("前收盘价 nPreClose: %d\n", marketData.nPreClose);
		printf("开盘价 nOpen: %d\n", marketData.nOpen);
		printf("最高价 nHigh: %d\n", marketData.nHigh);
		printf("最低价 nLow: %d\n", marketData.nLow);
		printf("最新价 nMatch: %d\n", marketData.nMatch);
		printf("申卖价 nAskPrice: %s \n", intarr2str(szBuf1, sizeof(szBuf1), (int*)marketData.nAskPrice, ELEM_COUNT(marketData.nAskPrice)));

		printf("申卖量 nAskVol: %s \n", intarr2str(szBuf2, sizeof(szBuf2), (int*)marketData.nAskVol, ELEM_COUNT(marketData.nAskVol)));

		printf("申买价 nBidPrice: %s \n", intarr2str(szBuf3, sizeof(szBuf3), (int*)marketData.nBidPrice, ELEM_COUNT(marketData.nBidPrice)));

		printf("申买量 nBidVol: %s \n", intarr2str(szBuf4, sizeof(szBuf4), (int*)marketData.nBidVol, ELEM_COUNT(marketData.nBidVol)));

		printf("成交笔数 nNumTrades: %d\n", marketData.nNumTrades);

		printf("成交总量 iVolume: %I64d\n", marketData.iVolume);
		printf("成交总金额 iTurnover: %I64d\n", marketData.iTurnover);
		printf("委托买入总量 nTotalBidVol: %I64d\n", marketData.nTotalBidVol);
		printf("委托卖出总量 nTotalAskVol: %I64d\n", marketData.nTotalAskVol);

		printf("加权平均委买价格 nWeightedAvgBidPrice: %u\n", marketData.nWeightedAvgBidPrice);
		printf("加权平均委卖价格 nWeightedAvgAskPrice: %u\n", marketData.nWeightedAvgAskPrice);

		printf("IOPV净值估值 nIOPV: %d\n",  marketData.nIOPV);
		printf("到期收益率 nYieldToMaturity: %d\n", marketData.nYieldToMaturity);
		printf("涨停价 nHighLimited: %d\n", marketData.nHighLimited);
		printf("跌停价 nLowLimited: %d\n", marketData.nLowLimited);
		printf("证券信息前缀 chPrefix: %s\n", chararr2str(szBufSmall, sizeof(szBufSmall), (char*)marketData.chPrefix, ELEM_COUNT(marketData.chPrefix)));
		printf("市盈率1 nSyl1: %d\n", marketData.nSyl1);
		printf("市盈率2 nSyl2: %d\n", marketData.nSyl2);
		printf("升跌2（对比上一笔） nSD2: %d\n", marketData.nSD2);
		if (item_count > 1)
		{
			printf("\n");
		}
	}

	printf("\n");

}

void PrintFutureData(TDF_FUTURE_DATA* pFuture, int nItems)
{
	boost::posix_time::ptime time_point = boost::posix_time::microsec_clock::local_time();
//	printf("-------- Future, Count:%d --------\n", nItems);
//	char szBuf1[256];
//	char szBuf2[256];
//	char szBuf3[256];
//	char szBuf4[256];

	TDF_OPTION_CODE codeInfo;
	
	for (int i=0; i<nItems; i++)
	{
		const TDF_FUTURE_DATA& futureData = pFuture[i];
		long server_secs = futureData.nTime;
		boost::posix_time::time_duration t = time_point.time_of_day();
	//	long local_secs = t.fractional_seconds() / 1000 + t.seconds() * 1000 + t.minutes() * 100000 + t.hours() * 10000000;
	//	cout << local_secs - server_secs << endl;
		price_buffer.UpdatePrice(intarr2double(futureData.nBidPrice), futureData.szCode, 0);
		//cout << intarr2double(futureData.nBidPrice) <<endl;
		price_buffer.UpdatePrice(intarr2double(futureData.nAskPrice), futureData.szCode, 1);
#if 0
		printf("万得代码 szWindCode: %s\n", futureData.szWindCode);


		printf("原始代码 szCode: %s\n", futureData.szCode);
		printf("业务发生日(自然日) nActionDay: %d\n", futureData.nActionDay);
		printf("交易日 nTradingDay: %d\n", futureData.nTradingDay);
		printf("时间(HHMMSSmmm) nTime: %d\n", futureData.nTime);
		cout << time_point << endl;
		printf("状态 nStatus: %d(%c)\n", futureData.nStatus, SAFE_CHAR(futureData.nStatus));

		printf("昨持仓 iPreOpenInterest: %I64d\n", futureData.iPreOpenInterest);
		printf("昨收盘价 nPreClose: %d\n", futureData.nPreClose);
		printf("昨结算 nPreSettlePrice: %d\n", futureData.nPreSettlePrice);
		printf("开盘价 nOpen: %d\n", futureData.nOpen);
		printf("最高价 nHigh: %d\n", futureData.nHigh);
		printf("最低价 nLow: %d\n", futureData.nLow);
		printf("最新价 nMatch: %d\n", futureData.nMatch);
		printf("成交总量 iVolume: %I64d\n", futureData.iVolume);
		printf("成交总金额 iTurnover: %I64d\n", futureData.iTurnover);
		printf("持仓总量 iOpenInterest: %I64d\n", futureData.iOpenInterest);
		printf("今收盘 nClose: %u\n", futureData.nClose);
		printf("今结算 nSettlePrice: %u\n", futureData.nSettlePrice);
		printf("涨停价 nHighLimited: %u\n", futureData.nHighLimited);
		printf("跌停价 nLowLimited: %u\n", futureData.nLowLimited);
		printf("昨虚实度 nPreDelta: %d\n", futureData.nPreDelta);
		printf("今虚实度 nCurrDelta: %d\n", futureData.nCurrDelta);

		printf("申卖价 nAskPrice: %s\n", intarr2str(szBuf1, sizeof(szBuf1), (int*)futureData.nAskPrice, ELEM_COUNT(futureData.nAskPrice)));
		printf("申卖量 nAskVol: %s\n", intarr2str(szBuf2, sizeof(szBuf2),(int*)futureData.nAskVol, ELEM_COUNT(futureData.nAskVol)));
		printf("申买价 nBidPrice: %s\n", intarr2str(szBuf3, sizeof(szBuf3),(int*)futureData.nBidPrice, ELEM_COUNT(futureData.nBidPrice)));
		printf("申买量 nBidVol: %s\n", intarr2str(szBuf4, sizeof(szBuf4),(int*)futureData.nBidVol, ELEM_COUNT(futureData.nBidVol)));

		if (nItems>1)
		{
			printf("\n");
		}

#endif
	}

//	printf("\n");
}
char* intarr2str(char* szBuf, int nBufLen, int arr[], int n)
{
	int nOffset = 0;
	for (int i=0; i<n; i++)
	{
		nOffset += _snprintf(szBuf+nOffset, nBufLen-nOffset, "%d ", arr[i]);
	}
	return szBuf;
}

void RecvSys(THANDLE hTdf, TDF_MSG* pSysMsg)
{
	{
		if (!pSysMsg ||! hTdf)
		{
			return;
		}

		switch (pSysMsg->nDataType)
		{
		case MSG_SYS_DISCONNECT_NETWORK:
			{
				printf("网络断开\n");
			}
			break;
		case MSG_SYS_CONNECT_RESULT:
			{
				TDF_CONNECT_RESULT* pConnResult = (TDF_CONNECT_RESULT*)pSysMsg->pData;
				if (pConnResult && pConnResult->nConnResult)
				{
					printf("连接 %s:%s user:%s, password:%s 成功!\n", pConnResult->szIp, pConnResult->szPort, pConnResult->szUser, pConnResult->szPwd);
				}
				else
				{
					printf("连接 %s:%s user:%s, password:%s 失败!\n", pConnResult->szIp, pConnResult->szPort, pConnResult->szUser, pConnResult->szPwd);
				}
			}
			break;
		case MSG_SYS_LOGIN_RESULT:
			{
				TDF_LOGIN_RESULT* pLoginResult = (TDF_LOGIN_RESULT*)pSysMsg->pData;
				if (pLoginResult && pLoginResult->nLoginResult)
				{
					printf("登陆成功！info:%s, nMarkets:%d\n", pLoginResult->szInfo, pLoginResult->nMarkets);
					for (int i=0; i<pLoginResult->nMarkets; i++)
					{
						printf("market:%s, dyn_date:%d\n", pLoginResult->szMarket[i], pLoginResult->nDynDate[i]);
					}
				}
				else
				{
					printf("登陆失败，原因:%s\n", pLoginResult->szInfo);
				}
			}
			break;
		case MSG_SYS_CODETABLE_RESULT:
			{
				TDF_CODE_RESULT* pCodeResult = (TDF_CODE_RESULT*)pSysMsg->pData;
				if (pCodeResult )
				{
					printf("接收到代码表：info:%s, 市场个数:%d\n", pCodeResult->szInfo, pCodeResult->nMarkets);
					for (int i=0; i<pCodeResult->nMarkets; i++)
					{
						printf("市场:%s, 代码表项数:%d, 代码表日期:%d\n", pCodeResult->szMarket[i], pCodeResult->nCodeCount[i], pCodeResult->nCodeDate[i]);
						//		if (g_nOutputCodeTable)
						{
							//获取代码表 
							TDF_CODE* pCodeTable; 
							unsigned int nItems;
							TDF_GetCodeTable(hTdf, pCodeResult->szMarket[i], &pCodeTable, &nItems);
							for (int i=0; i<nItems; i++)
							{
								TDF_CODE& code = pCodeTable[i];
								printf("windcode:%s, code:%s, market:%s, name:%s, nType:0x%x\n",code.szWindCode, code.szCode, code.szMarket, code.szCNName, code.nType);
							}
							TDF_FreeArr(pCodeTable);
						}
					}
				}
			}
			break;
		case MSG_SYS_QUOTATIONDATE_CHANGE:
			{
				TDF_QUOTATIONDATE_CHANGE* pChange = (TDF_QUOTATIONDATE_CHANGE*)pSysMsg->pData;
				if (pChange)
				{
					printf("收到行情日期变更通知，即将自动重连！交易所：%s, 原日期:%d, 新日期：%d\n", pChange->szMarket, pChange->nOldDate, pChange->nNewDate);
				}
			}
			break;
		case MSG_SYS_MARKET_CLOSE:
			{
				TDF_MARKET_CLOSE* pCloseInfo = (TDF_MARKET_CLOSE*)pSysMsg->pData;
				if (pCloseInfo)
				{
					printf("闭市消息:market:%s, time:%d, info:%s\n", pCloseInfo->szMarket, pCloseInfo->nTime, pCloseInfo->chInfo);
				}
			}
			break;
		case MSG_SYS_HEART_BEAT:
			{
				printf("收到心跳消息\n");
			}
			break;
		default:
			//		assert(0);
			break;
		}
	}

}

char* chararr2str(char* szBuf, int nBufLen, char arr[], int n)
{
	int nOffset = 0;
	for (int i=0; i<n; i++)
	{
		nOffset += _snprintf(szBuf+nOffset, nBufLen-nOffset, "%d(%c) ", arr[i], SAFE_CHAR(arr[i]));
	}
	return szBuf;
}

void RecvData(THANDLE hTdf, TDF_MSG* pDataMsg)
{	

	if (!pDataMsg || !hTdf)
	{
		return;
	}

	unsigned int item_count = pDataMsg->pAppHead->nItemCount;
	switch (pDataMsg->nDataType)
	{
	case MSG_DATA_MARKET:
		cout << "行情快照数据\n";
		PrintMarketData((TDF_MARKET_DATA*)pDataMsg->pData, item_count);
		break;
	case MSG_DATA_INDEX:
		cout << "指数行情快照数据\n";
		break;
	case MSG_DATA_FUTURE:
		PrintFutureData((TDF_FUTURE_DATA*)pDataMsg->pData, item_count);
		//cout << "期货行情快照数据\n";
		break;
	case MSG_DATA_TRANSACTION:
		cout << "逐笔成交\n";
		break;
	case MSG_DATA_ORDERQUEUE:
		cout << "委托队列\n";
		break;
	case MSG_DATA_ORDER:
		cout << "逐笔委托\n";
		break;
	default:
		break;
	}
}

double intarr2double(const unsigned int* data)
{
	char szBuf3[256];
	return atof(intarr2str(szBuf3, sizeof(szBuf3), (int*)data, ELEM_COUNT(data)))/10000;
}

void PrintLogThread()
{
	while(true){
		if (global_program_termination){
			break;
		}
		global_log_buffer.PrintMsg();
		boost::this_thread::sleep(boost::posix_time::millisec(500));
	}
	printf("[Print Log Thread]***线程结束\n");
}

void PrintTransLogThread()
{
	while(true){
		if (global_program_termination){
			break;
		}
		global_log_buffer.PrintTransMsg();
		boost::this_thread::sleep((boost::posix_time::millisec(500)));
	}
	printf("[Print Transaction Log Thread]***线程结束\n");
}