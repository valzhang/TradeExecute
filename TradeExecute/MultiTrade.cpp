
#include <iostream>
#include <queue>
#include "stdafx.h"
//����������
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
//const char SYMBOL[8] = "IF1409";	//��Ʊ���ڻ�����
const int INI_PRICE = 0;
const int INI_VOL = 4;
const int FUND_SIZE = 100;
const int HOLD_SIZE = 300;
const int ORDER_SIZE = 300;
const int N_TIME = 3;
const int MAX_SECONDS = 600;
const int N_TICKS = 1;
const double FINAL_RATIO = 0.05;
//const int MAX_INSERT_SECS = 5;	//�µ���ȴ�ʱ��
const string LOG_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\log.txt";
const string TRANS_LOG_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\transaction_log.txt";
const string ACTION_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\action_log.txt";
const string SYMBOL_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\symbol.txt";
const int WAIT_MILLSECS = 500;
const int MAX_THREAD_NUM = 1;

//TDF����������
const char TDF_SERVER_IP[32] = "114.80.154.34";
const char TDF_SERVER_PORT[8] = "10051";
const char TDF_USERNAME[32] = "TD7100001";
const char TDF_PASSWORD[32] = "44053825";

//����������


//����������
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

//����������
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
	printf("[Main Thread]***������ʼ\n");
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
	printf("[Main Thread]***���������\n");
	getchar();
	system("PAUSE");
	return 0;
}

int LogMsgTest(CIDMPTradeApi &trader)
{
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
		return -1;
	}
	printf("***��¼�ɹ���\n������IP��%s,\t�������˿ڣ�%d,\t�û�����%s...\n", SERVER_IP, SERVER_PORT, USER_NAME);

	//��ȡ�ʲ��˻��б�
	printf("***��ȡ�ʲ��˻��б���\n");
	CIDMP_FUND_ACCOUNT_INFO accountInfo[FUND_SIZE];
	int accountSize = trader.GetFundAccount( accountInfo, FUND_SIZE );
	if (accountSize < 0){
		printf("��ȡ�ʲ��˻��б�ʧ�ܣ�\n");
		getchar();
		return -1;
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
		return -1;
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
		return -1;
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
		return -1;
	}

	printf("�ֻ����ʽ�\t: %6.3f\n�ֻ�����\t: %6.3f\n�ڻ����ʽ�\t: %6.3f\n�ڻ�����\t: %6.3f\n\n",
		fInfo.stkTotal,fInfo.stkAvailable, fInfo.futureTotal, fInfo.futureAvailable);


	printf("\n***����ʲ��˻�[%d]���ʲ���Ԫ[%d]���ʽ���Ϣ...\n", FUND_ID, CELL_ID);
	CIDMP_FUND_INFO cInfo;
	nSuccess = trader.GetFund( FUND_ID, CELL_ID, cInfo);

	if(nSuccess != 1) {
		printf("��ѯ�ʲ���Ԫ���ʽ���Ϣʧ��\n");
		getchar();
		return -1;
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
		return -1;
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
		return -1;
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
		return -1;
	}

	printf("[���]\t����\t����������\t�ֲ�����\t��������\t������\t��ƽ���\t��ƽ���\n");
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
	printf("[Receive Command Thread]***�߳̿�ʼ\n");
	while(true){
		printf("[Receive Command Thread]***�Ƿ�������ף�(Y/y������������ֹ����)\n");
		char flag;
		//scanf("%c", &flag);
		cin >> flag;
		if ( flag != 'y' && flag != 'Y' ){
			global_program_termination = true;
			global_create_thread_condition.notify_all();
	//		printf("[Receive Command Thread]***�߳̽���\n");
			break;
		}
		printf("[Receive Command Thread]***���������ָ�\n");
		//debug
		getchar();
		int exchange, direction, offset, vol, pj, secs;
		char symbol[8];
//		scanf("%d%s%d%d%d%d%d", &exchange, &symbol, &direction, &offset, &vol, &pj, &secs);
		try
		{
			cout << "���� = ";
			cin >> symbol;
			cout << "��������(0 ��; 1 ��) = ";
			cin >> direction;
			cout << "��ƽ����(0 ��; 1 ƽ) = ";
			cin >> offset;
			cout << "��������(��) = ";
			cin >> vol;
			cout << "�Ƿ�ƽ��(0 ��; 1 ��) = ";
			cin >> pj;
			cout << "����ʱ��(secs) = ";
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
	printf("[Receive Command Thread]***�߳̽���\n");
}

void ManageThread()
{
	int count = 0;
	printf("[Manage Thread]***�߳̿�ʼ\n");
	//����Max���߳�
	for ( int i = 0; i < MAX_THREAD_NUM; i++ ){
		global_group.create_thread(boost::bind(TradeThread, count));
		++count;
	}
	
	printf("[Manage Thread]***��ʼ�̳߳ؽ������\n");
	while(true){
		//�ȴ�֪ͨ	todo
		{
			boost::mutex::scoped_lock lock(global_create_thread_mutex);
			global_create_thread_condition.wait(global_create_thread_mutex);
		}
		if (global_program_termination){
			//while(global_group.size() > 0){
				cmd_buffer.ReleaseCommand();
				global_log_buffer.Release();
				printf("[Manage Thread]***�ͷ�cmdbuffer�߳�\n");
				//boost::this_thread::sleep(boost::posix_time::seconds(1));
			//}
			break;
		}
		//����һ���߳�
		global_group.create_thread(bind(TradeThread, count));
		++count;
	}
	global_group.join_all();
	printf("[Manage Thread]***�߳̽���\n");
}

void TradeThread(int id_no)
{
	printf("[Trader Command Thread %d]***�߳̿�ʼ...ʱ�� = %s\n", id_no, boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()).c_str() );

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
	boost::posix_time::ptime start_time = boost::posix_time::second_clock::local_time();
	printf("[Trader Command Thread %d]***׼����½...ʱ�� = %s\n", id_no, boost::posix_time::to_iso_string(start_time).c_str() );
	int nSuccess = trader.Connect(serverIP, serverPort, userID, userPW, 0);
	if (nSuccess != 1){
		printf("[Trader Command Thread %d]***��½ʧ��\n", id_no);
		getchar();
		return;
	}
	printf("[Trader Command Thread %d]***��½�ɹ�,����ʱ�� = %s\n", id_no, boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time() - start_time).c_str());
	TraderCommand cmd;
	if (global_program_termination){
		printf("[Trader Command Thread %d]***�߳̽���\n", id_no);
		return;
	}
	cmd_buffer.GetCommand(&cmd);
	if (global_program_termination){
		printf("[Trader Command Thread %d]***�߳̽���\n", id_no);
		return;
	}
	double total_price = 0;
	int total_vol = 0;

	//֪ͨ�½��߳�
	
	global_create_thread_condition.notify_one();

	printf("[Trader Thread %d]***װ�䶩��\n", id_no);
	//װ�䶩��
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
	
	printf("[Trader Thread %d]***��ȡ���¼۸�\n", id_no);
	//��ȡ���¼۸�
	order_req.orderPrice = price_buffer.GetLastPrice(order_req.symbol, order_req.direction);

	int risk_num = 0;
	//�ύ����
	printf("[Trader Thread %d]***�ύ����\n", id_no);
	global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), -1, "Order Insert", 1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);
	nSuccess = trader.OrderInsert(order_req, risk_num);
	global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Insert Return", nSuccess, 1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);
	if (nSuccess < 0){
		printf("[Trader Thread %d]***�µ�ʧ�ܣ�\n", id_no);
		printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
		printf("[Trader Thread %d]***�µ��۸� = %7.3f\t�µ����� = %d\n", id_no, order_req.orderPrice, order_req.orderVol);
		return;
	}
	//ȷ���µ��ɹ�
	trader.GetOrderByOrderNo(order_req.orderNo, order_info);	
	while (order_info.orderStatus < 3){
		boost::this_thread::sleep(boost::posix_time::millisec(100));
		trader.GetOrderByOrderNo(order_req.orderNo, order_info);
	}
	global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
	if (order_info.orderStatus == 3){
		printf("[Trader Thread %d]***�µ�ʧ�ܣ�\n", id_no);
		printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
		printf("[Trader Thread %d]***�µ��۸� = %7.3f\t�µ����� = %d\n", id_no, order_req.orderPrice, order_req.orderVol);

		return;
	}
	if (order_info.orderStatus > 6){
		printf("[Trader Thread %d]***�������Զ�������\n", id_no);
		printf("[Trader Thread %d]***����״̬ = %d\n", id_no, order_info.orderStatus);
				
		return;
	}
	printf("[Trader Thread %d]***�µ��ɹ����µ��۸� = %7.3f\n", id_no, order_req.orderPrice);
	global_order_no_mutex.lock();
	global_order_no.push_back(order_req.orderNo);
	global_order_vol[order_req.orderNo] = order_req.orderVol;
	global_order_no_mutex.unlock();

	bool get_trade = false;
	//��ʼ����ѭ��
	boost::posix_time::ptime begin_time = boost::posix_time::microsec_clock::local_time();
	while ((boost::posix_time::microsec_clock::local_time() - begin_time).total_milliseconds() <= MAX_SECONDS*(1-FINAL_RATIO)*1000){
		double new_price = price_buffer.WaitPriceChange(order_req.symbol, order_req.direction);
		if (new_price == 0){//��ʱ
			//����Ƿ�ɽ�
			if (order_info.orderStatus == 6){
				printf("[Trader Thread %d]***�����ɽ���\n", id_no);
				get_trade = true;
				break;
			}else{
				continue;
			}
		}
		printf("***��ȡ���¼۸�ɹ�%7.3f\n", new_price);
		if (global_order_vol[order_req.orderNo] == 0){
			trader.GetOrderByOrderNo(order_req.orderNo, order_info);
			if (order_info.orderStatus == 6){
				printf("[Trader Thread %d]***�����ɽ���\n", id_no);
			get_trade = true;				
			}else{
				if (order_info.orderStatus == 7){
					printf("[Trader Thread %d]***����ʧ�ܣ��������Զ����������򷵻�...\n", id_no);
				}
				printf("[Trader Thread %d]***����ʧ�ܣ�\n", id_no);
				printf("[Trader Thread %d]***����״̬ = %d��\n", id_no, order_info.orderStatus);
			}
			break;
		}
		//����
		printf("***����\n");
		//��������
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
			printf("***������...\n");
			trader.GetOrderByOrderNo(order_req.orderNo, order_info);
		}
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Update Order", order_info.orderStatus, order_req.direction, order_req.offsetFlagType, order_info.orderVol, order_info.tradeVol, order_info.canceledVol, order_info.orderPrice);
		printf("%d\n", order_info.orderStatus);
		if (order_info.orderStatus == 6){
			get_trade = true;
			printf("[Trader Thread %d]***�����ɽ���\n", id_no);
			break;
		}
		printf("[Trader Thread %d]***�����ɹ���\n", id_no);

		//���¼۸�
		order_req.orderPrice = new_price;
		
		
		//�µ�
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), -1, "Order Insert", -1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);
		nSuccess = trader.OrderInsert(order_req, risk_num);
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Insert Return", nSuccess, 1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);

		if (nSuccess < 0){
			printf("[Trader Thread %d]***�µ�ʧ�ܣ�\n", id_no);
			printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
			printf("[Trader Thread %d]***�µ��۸� = %7.3f\t�µ����� = %d\n", id_no, order_req.orderPrice, order_req.orderVol);
			return;
		}
		//ȷ���µ��ɹ�
		trader.GetOrderByOrderNo(order_req.orderNo, order_info);
		while (order_info.orderStatus < 3){
			boost::this_thread::sleep(boost::posix_time::millisec(100));
			trader.GetOrderByOrderNo(order_req.orderNo, order_info);
		}
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
		if (order_info.orderStatus == 3){
			printf("[Trader Thread %d]***�µ�ʧ�ܣ�\n", id_no);
			printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
			printf("[Trader Thread %d]***�µ��۸� = %7.3f\t�µ����� = %d\n", id_no, order_req.orderPrice, order_req.orderVol);
			return;
		}
		if (order_info.orderStatus > 6){
			printf("[Trader Thread %d]***�������Զ�������\n", id_no);
			printf("[Trader Thread %d]***����״̬ = %d\n", id_no, order_info.orderStatus);

			return;
		}
		printf("[Trader Thread %d]***�µ��ɹ����µ��۸� = %7.3f\n", id_no, order_req.orderPrice);
		global_order_no_mutex.lock();
		global_order_no.push_back(order_req.orderNo);
		global_order_vol[order_req.orderNo] = order_req.orderVol;
		global_order_no_mutex.unlock();
		
	}

	if (!get_trade){
		//����е�������
		trader.GetOrderByOrderNo(order_req.orderNo, order_info);
		global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
		if (order_info.orderVol > order_info.tradeVol + order_info.canceledVol){
			printf("***����\n");
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
			printf("[Trader Thread %d]***�����ɹ���\n", id_no);
		}
		//����minGap
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
			printf("[Trader Thread %d]***������ʱ\n", id_no);
		}
	}else{
		total_vol += order_req.orderVol;
		total_price += order_req.orderPrice * order_req.orderVol;
		global_log_buffer.PutTransMsg(boost::posix_time::second_clock::local_time(), order_req.orderVol, order_req.orderVol, order_req.orderPrice, total_price/(double)total_vol);
		printf("����һ���ɽ�log��Ϣ\n");
	}


	//boost::this_thread::sleep(boost::posix_time::seconds(10));

	printf("[Trader Command Thread %d]***�߳̽���\n", id_no);
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

	//�����������ʱ�� = 5s
	TDF_SetEnv(TDF_ENVIRON_HEART_BEAT_INTERVAL, 5);
	//����3��δ�յ�������Ϣ��Ϊ����
	TDF_SetEnv(TDF_ENVIRON_MISSED_BEART_COUNT, 3);
	//����ÿһ�����ݰ����ճ�ʱʱ�� = 10s
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
	printf("[TDF Update Price Thread]***�߳̽���\n");
}
void UpdatePriceThread()
{
	printf("[Update Price Thread]***�߳̿�ʼ...\n");
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
		return ;
	}

	delete[] serverIP;
	delete[] userID;
	delete[] userPW;
	printf("[Update Price Thread]***��½�ɹ���\n");
	CIDMP_TICK_QUOTATION_QUERY *query = new CIDMP_TICK_QUOTATION_QUERY[price_buffer.all_symbol.size()+1];
	CIDMP_TICK_QUOTATION_INFO *info = new CIDMP_TICK_QUOTATION_INFO[price_buffer.all_symbol.size()+1];
	for ( int i = 0; i < price_buffer.all_symbol.size(); i++ ){
		(query+i)->exchgcode = EXCHANGE;
		strcpy((query+i)->symbol, price_buffer.all_symbol[i].c_str());
	}

	while(true){
		if (global_program_termination){
			printf("[Update Price Thread]***�˳�ѭ����\n");
			break;
		}
		trader.GetTickQuotation(query, info, price_buffer.all_symbol.size());
		for ( int i = 0; i < price_buffer.all_symbol.size(); i++ ){
			price_buffer.UpdatePrice(info[i].bp1, price_buffer.all_symbol[i], 0);
			price_buffer.UpdatePrice(info[i].sp1, price_buffer.all_symbol[i], 1);

#if 0
			//�������Ƿ��б仯
			if (global_all_symbol_price[global_all_symbol[i]].first != info[i].bp1){
				printf("[Update Price Thread]***%s��۱仯��ԭ�� = %7.3f���¼� = %7.3f\n", global_all_symbol[i].c_str(), global_all_symbol_price[global_all_symbol[i]].first, info[i].bp1);
				global_all_symbol_price[global_all_symbol[i]].first = info[i].bp1;
				//���ͼ۸�仯�ź�
			}
			//��������Ƿ��б仯
			if (global_all_symbol_price[global_all_symbol[i]].second != info[i].sp1){
				printf("[Update Price Thread]***%s���۱仯��ԭ�� = %7.3f���¼� = %7.3f\n", global_all_symbol[i].c_str(), global_all_symbol_price[global_all_symbol[i]].second, info[i].sp1);
				global_all_symbol_price[global_all_symbol[i]].second = info[i].sp1;
				//���ͼ۸�仯�ź�
			}
#endif
		}
	}

	printf("[Update Price Thread]***�߳̽���\n");
}

void UpdateOrderNoThread()
{
	printf("[Update Order No Thread]***�߳̿�ʼ...\n");
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
		printf("[Update Order No Thread]***Update Volumn Thread Login Failed!\n");
		getchar();
		return ;
	}
	printf("[Update Order No Thread]***��½�ɹ�!\n");

	delete []serverIP;
	delete []userID;
	delete []userPW;
	while (true){
		if (global_program_termination){
			printf("[Update Order No Thread]***�˳�ѭ��!\n");
			break;
		}
		if (global_order_no.size() == 0){
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			continue;
		}
	//	printf("[Update Order No Thread]***���¶�����Ϣ\n");
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


	printf("[Update Order No Thread]***�߳̽���\n");
}

bool GoToFinalStep(CIDMPTradeApi &trader, CIDMP_ORDER_REQ &order_req, int max_secs, double gap, int try_times, int id_no, double total_price, int total_vol)
{
	CIDMP_ORDER_INFO order_info;
	bool get_trade =false;
	order_req.orderPrice = price_buffer.GetLastPrice(order_req.symbol, order_req.direction);
	boost::posix_time::ptime begin_time = boost::posix_time::microsec_clock::local_time();
	while ( (boost::posix_time::microsec_clock::local_time() - begin_time).total_milliseconds() <= max_secs * 1000 && !get_trade ){
		//���ö����۸�
		order_req.orderPrice += gap;
		for ( int i = 0; i < try_times; i++ ){
			//�µ�
			int risk_num;
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), -1, "Order Insert", -1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);
			int nSuccess = trader.OrderInsert(order_req, risk_num);
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Insert Return", nSuccess, 1, order_req.direction, order_req.offsetFlagType, order_req.orderVol, 0, 0, order_req.orderPrice);
			if (nSuccess < 0){
				printf("[Trader Thread %d]***�µ�ʧ�ܣ�\n", id_no);
				printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
				printf("[Trader Thread %d]***�µ��۸� = %7.3f\t�µ����� = %d\n", id_no, order_req.orderPrice, order_req.orderVol);
				return false;
			}
			//ȷ���µ��ɹ�
			trader.GetOrderByOrderNo(order_req.orderNo, order_info);
			while (order_info.orderStatus < 3){
				boost::this_thread::sleep(boost::posix_time::millisec(100));
				trader.GetOrderByOrderNo(order_req.orderNo, order_info);
			}
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
			if (order_info.orderStatus == 3){
				printf("[Trader Thread %d]***�µ�ʧ�ܣ�\n", id_no);
				printf("[Trader Thread %d]***%s\n", id_no, trader.GetLastOrderInsertError());
				printf("[Trader Thread %d]***�µ��۸� = %7.3f\t�µ����� = %d\n", id_no, order_req.orderPrice, order_req.orderVol);
				return false;
			}
			printf("[Trader Thread %d]***�µ��ɹ����µ��۸� = %7.3f\n", id_no, order_req.orderPrice);
			//����
			printf("***����\n");
			char errorMsg[200];
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Cancel", order_info.orderStatus, order_req.direction, order_req.offsetFlagType, order_info.orderVol, order_info.tradeVol, order_info.canceledVol, order_info.orderPrice);
			nSuccess = trader.CancelOrder(order_req.orderNo, errorMsg);
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Cancel Return", nSuccess, order_info.orderStatus, order_req.direction, order_req.offsetFlagType, order_info.orderVol, order_info.tradeVol, order_info.canceledVol, order_info.orderPrice);

			while (order_info.orderStatus < 6){
				printf("***������...\n");
				boost::this_thread::sleep(boost::posix_time::millisec(100));
				trader.GetOrderByOrderNo(order_req.orderNo, order_info);
			}
			global_log_buffer.PutMsg(boost::posix_time::second_clock::local_time(), order_req.orderNo, "Order Update", order_info.orderStatus , order_req.direction, order_req.offsetFlagType, order_req.orderVol, order_info.tradeVol, order_info.canceledVol, order_req.orderPrice);
			printf("[Trader Thread %d]***�����ɹ���\n", id_no);
			if (order_info.tradeVol > 0){
				total_price += order_info.tradeVol * order_req.orderPrice;
				total_vol += order_info.tradeVol;
				global_log_buffer.PutTransMsg(boost::posix_time::second_clock::local_time(), order_req.orderVol, order_info.tradeVol, order_req.orderPrice, total_price/(double)total_vol);
				order_req.orderVol -= order_info.tradeVol;
			}
			//����Ƿ��׳ɹ�
			if (order_info.orderVol == order_info.tradeVol){
				printf("[Trader Thread %d]***���׳ɹ����ɽ��۸� = %7.3f\n", id_no, order_req.orderPrice);
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
		printf("��ô��� szWindCode: %s\n", marketData.szWindCode);
		printf("ԭʼ���� szCode: %s\n", marketData.szCode);
		printf("ҵ������(��Ȼ��) nActionDay: %d\n", marketData.nActionDay);
		printf("������ nTradingDay: %d\n", marketData.nTradingDay);
		printf("ʱ��(HHMMSSmmm) nTime: %d\n", marketData.nTime);
		printf("״̬ nStatus: %d(%c)\n", marketData.nStatus, SAFE_CHAR(marketData.nStatus));
		printf("ǰ���̼� nPreClose: %d\n", marketData.nPreClose);
		printf("���̼� nOpen: %d\n", marketData.nOpen);
		printf("��߼� nHigh: %d\n", marketData.nHigh);
		printf("��ͼ� nLow: %d\n", marketData.nLow);
		printf("���¼� nMatch: %d\n", marketData.nMatch);
		printf("������ nAskPrice: %s \n", intarr2str(szBuf1, sizeof(szBuf1), (int*)marketData.nAskPrice, ELEM_COUNT(marketData.nAskPrice)));

		printf("������ nAskVol: %s \n", intarr2str(szBuf2, sizeof(szBuf2), (int*)marketData.nAskVol, ELEM_COUNT(marketData.nAskVol)));

		printf("����� nBidPrice: %s \n", intarr2str(szBuf3, sizeof(szBuf3), (int*)marketData.nBidPrice, ELEM_COUNT(marketData.nBidPrice)));

		printf("������ nBidVol: %s \n", intarr2str(szBuf4, sizeof(szBuf4), (int*)marketData.nBidVol, ELEM_COUNT(marketData.nBidVol)));

		printf("�ɽ����� nNumTrades: %d\n", marketData.nNumTrades);

		printf("�ɽ����� iVolume: %I64d\n", marketData.iVolume);
		printf("�ɽ��ܽ�� iTurnover: %I64d\n", marketData.iTurnover);
		printf("ί���������� nTotalBidVol: %I64d\n", marketData.nTotalBidVol);
		printf("ί���������� nTotalAskVol: %I64d\n", marketData.nTotalAskVol);

		printf("��Ȩƽ��ί��۸� nWeightedAvgBidPrice: %u\n", marketData.nWeightedAvgBidPrice);
		printf("��Ȩƽ��ί���۸� nWeightedAvgAskPrice: %u\n", marketData.nWeightedAvgAskPrice);

		printf("IOPV��ֵ��ֵ nIOPV: %d\n",  marketData.nIOPV);
		printf("���������� nYieldToMaturity: %d\n", marketData.nYieldToMaturity);
		printf("��ͣ�� nHighLimited: %d\n", marketData.nHighLimited);
		printf("��ͣ�� nLowLimited: %d\n", marketData.nLowLimited);
		printf("֤ȯ��Ϣǰ׺ chPrefix: %s\n", chararr2str(szBufSmall, sizeof(szBufSmall), (char*)marketData.chPrefix, ELEM_COUNT(marketData.chPrefix)));
		printf("��ӯ��1 nSyl1: %d\n", marketData.nSyl1);
		printf("��ӯ��2 nSyl2: %d\n", marketData.nSyl2);
		printf("����2���Ա���һ�ʣ� nSD2: %d\n", marketData.nSD2);
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
		printf("��ô��� szWindCode: %s\n", futureData.szWindCode);


		printf("ԭʼ���� szCode: %s\n", futureData.szCode);
		printf("ҵ������(��Ȼ��) nActionDay: %d\n", futureData.nActionDay);
		printf("������ nTradingDay: %d\n", futureData.nTradingDay);
		printf("ʱ��(HHMMSSmmm) nTime: %d\n", futureData.nTime);
		cout << time_point << endl;
		printf("״̬ nStatus: %d(%c)\n", futureData.nStatus, SAFE_CHAR(futureData.nStatus));

		printf("��ֲ� iPreOpenInterest: %I64d\n", futureData.iPreOpenInterest);
		printf("�����̼� nPreClose: %d\n", futureData.nPreClose);
		printf("����� nPreSettlePrice: %d\n", futureData.nPreSettlePrice);
		printf("���̼� nOpen: %d\n", futureData.nOpen);
		printf("��߼� nHigh: %d\n", futureData.nHigh);
		printf("��ͼ� nLow: %d\n", futureData.nLow);
		printf("���¼� nMatch: %d\n", futureData.nMatch);
		printf("�ɽ����� iVolume: %I64d\n", futureData.iVolume);
		printf("�ɽ��ܽ�� iTurnover: %I64d\n", futureData.iTurnover);
		printf("�ֲ����� iOpenInterest: %I64d\n", futureData.iOpenInterest);
		printf("������ nClose: %u\n", futureData.nClose);
		printf("����� nSettlePrice: %u\n", futureData.nSettlePrice);
		printf("��ͣ�� nHighLimited: %u\n", futureData.nHighLimited);
		printf("��ͣ�� nLowLimited: %u\n", futureData.nLowLimited);
		printf("����ʵ�� nPreDelta: %d\n", futureData.nPreDelta);
		printf("����ʵ�� nCurrDelta: %d\n", futureData.nCurrDelta);

		printf("������ nAskPrice: %s\n", intarr2str(szBuf1, sizeof(szBuf1), (int*)futureData.nAskPrice, ELEM_COUNT(futureData.nAskPrice)));
		printf("������ nAskVol: %s\n", intarr2str(szBuf2, sizeof(szBuf2),(int*)futureData.nAskVol, ELEM_COUNT(futureData.nAskVol)));
		printf("����� nBidPrice: %s\n", intarr2str(szBuf3, sizeof(szBuf3),(int*)futureData.nBidPrice, ELEM_COUNT(futureData.nBidPrice)));
		printf("������ nBidVol: %s\n", intarr2str(szBuf4, sizeof(szBuf4),(int*)futureData.nBidVol, ELEM_COUNT(futureData.nBidVol)));

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
				printf("����Ͽ�\n");
			}
			break;
		case MSG_SYS_CONNECT_RESULT:
			{
				TDF_CONNECT_RESULT* pConnResult = (TDF_CONNECT_RESULT*)pSysMsg->pData;
				if (pConnResult && pConnResult->nConnResult)
				{
					printf("���� %s:%s user:%s, password:%s �ɹ�!\n", pConnResult->szIp, pConnResult->szPort, pConnResult->szUser, pConnResult->szPwd);
				}
				else
				{
					printf("���� %s:%s user:%s, password:%s ʧ��!\n", pConnResult->szIp, pConnResult->szPort, pConnResult->szUser, pConnResult->szPwd);
				}
			}
			break;
		case MSG_SYS_LOGIN_RESULT:
			{
				TDF_LOGIN_RESULT* pLoginResult = (TDF_LOGIN_RESULT*)pSysMsg->pData;
				if (pLoginResult && pLoginResult->nLoginResult)
				{
					printf("��½�ɹ���info:%s, nMarkets:%d\n", pLoginResult->szInfo, pLoginResult->nMarkets);
					for (int i=0; i<pLoginResult->nMarkets; i++)
					{
						printf("market:%s, dyn_date:%d\n", pLoginResult->szMarket[i], pLoginResult->nDynDate[i]);
					}
				}
				else
				{
					printf("��½ʧ�ܣ�ԭ��:%s\n", pLoginResult->szInfo);
				}
			}
			break;
		case MSG_SYS_CODETABLE_RESULT:
			{
				TDF_CODE_RESULT* pCodeResult = (TDF_CODE_RESULT*)pSysMsg->pData;
				if (pCodeResult )
				{
					printf("���յ������info:%s, �г�����:%d\n", pCodeResult->szInfo, pCodeResult->nMarkets);
					for (int i=0; i<pCodeResult->nMarkets; i++)
					{
						printf("�г�:%s, ���������:%d, ���������:%d\n", pCodeResult->szMarket[i], pCodeResult->nCodeCount[i], pCodeResult->nCodeDate[i]);
						//		if (g_nOutputCodeTable)
						{
							//��ȡ����� 
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
					printf("�յ��������ڱ��֪ͨ�������Զ���������������%s, ԭ����:%d, �����ڣ�%d\n", pChange->szMarket, pChange->nOldDate, pChange->nNewDate);
				}
			}
			break;
		case MSG_SYS_MARKET_CLOSE:
			{
				TDF_MARKET_CLOSE* pCloseInfo = (TDF_MARKET_CLOSE*)pSysMsg->pData;
				if (pCloseInfo)
				{
					printf("������Ϣ:market:%s, time:%d, info:%s\n", pCloseInfo->szMarket, pCloseInfo->nTime, pCloseInfo->chInfo);
				}
			}
			break;
		case MSG_SYS_HEART_BEAT:
			{
				printf("�յ�������Ϣ\n");
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
		cout << "�����������\n";
		PrintMarketData((TDF_MARKET_DATA*)pDataMsg->pData, item_count);
		break;
	case MSG_DATA_INDEX:
		cout << "ָ�������������\n";
		break;
	case MSG_DATA_FUTURE:
		PrintFutureData((TDF_FUTURE_DATA*)pDataMsg->pData, item_count);
		//cout << "�ڻ������������\n";
		break;
	case MSG_DATA_TRANSACTION:
		cout << "��ʳɽ�\n";
		break;
	case MSG_DATA_ORDERQUEUE:
		cout << "ί�ж���\n";
		break;
	case MSG_DATA_ORDER:
		cout << "���ί��\n";
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
	printf("[Print Log Thread]***�߳̽���\n");
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
	printf("[Print Transaction Log Thread]***�߳̽���\n");
}