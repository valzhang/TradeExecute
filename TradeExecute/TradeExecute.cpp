#include "IDMPTradeApi.h"
#include <iomanip>
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char SERVER_IP[16] = "196.168.0.163";
const int SERVER_PORT = 8080;
const char USER_NAME[20] = "Admin";
const char USER_PASSWORD[20] = "1";

const int FUND_ID = 100;	//�ʲ��˻�����
const int CELL_ID = 1001;	//�ʲ���Ԫ����
const i64 PROFL_ID = 111;	//�ʲ���ϱ���
const int DIRECTION = 0;	//���׷���
const int EXCHANGE = 6;	//���������
const int OFFSET = 0;		//��ƽ����
const int PJ = 0;			//�Ƿ�ƽ��
const char SYMBOL[8] = "IF1409";	//��Ʊ���ڻ�����
const int INI_PRICE = 0;
const int INI_VOL = 1;

const int FUND_SIZE = 100;
const int HOLD_SIZE = 300;
const int ORDER_SIZE = 300;
const int N_TIME = 3;
const int MAX_SECONDS = 600;
const int N_TICKS = 1;
const double FINAL_RATIO = 0.05;
const int MAX_INSERT_SECS = 5;	//�µ���ȴ�ʱ��
const string LOG_PATH = "C:\\Users\\Val\\Documents\\GitHub\\TradeExecute\\log.txt";

typedef struct TraderThreadParameter{
	CIDMPTradeApi* pTrader;
	CIDMP_ORDER_REQ* orderReq;
	int maxSecs;
	int nTicks;
	double finalRatio;
	double minGap;
	int orderStatus;
} TraderThreadParameter;

DWORD WINAPI TraderCore( void *para );

//��ӡ��ί��������Ϣ
inline void PrintOrderMsg( CIDMP_ORDER_REQ &orderReq );
int OrderCancel( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE *stream );
inline bool UpdateOrder( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE* stream );
int OrderInsert( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE *stream );
//��ȡ���½�����Ϣ
//inline bool GetRealTrade( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, CIDMP_ORDER_INFO &orderInfo );

int GotoFinalStep( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, int maxSeconds, int nTicks, double finalRatio, double minGap, FILE *stream );
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


	double minGap = 0.0;
	if ( orderReq.exchgcode < 3 ){
		CIDMP_STOCK_INFO *info = new CIDMP_STOCK_INFO[6000];
		int stkinfo2 = trader.GetSysStockInfo(false,info,6000);
		for (int i = 0; i < stkinfo2; i++){
			if ( strcmp( info[i].symbol, orderReq.symbol ) == 0 ){
				minGap = info[i].priceUnit;
				break;
			}
		}
		delete []info;
	}else{
		CIDMP_FUTRUE_INFO *ftinfo = new CIDMP_FUTRUE_INFO[3000];
		int ftinfoSize = trader.GetSysFutrueInfo(false,ftinfo,3000);
		for (int i = 0; i < ftinfoSize; i++){
			if ( strcmp( ftinfo[i].symbol, orderReq.symbol ) == 0 ){
				minGap = ftinfo[i].priceUnit;
				break;
			}
		}
		delete []ftinfo;
	}
	if (orderReq.direction == 1){
		minGap = 0 - minGap;
	}

	TraderThreadParameter thread_para;
	thread_para.pTrader = &trader;
	thread_para.orderReq = &orderReq;
	//*(thread_para.pTrader) = trader;
	//*(thread_para.orderReq) = orderReq;
	thread_para.maxSecs = MAX_SECONDS;
	thread_para.nTicks = N_TICKS;
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
	CloseHandle(TraderCore);

/*	if (nSuccess < 0){
		printf("***����ʧ�ܣ�ʧ��ԭ�򣺷�ؼ��ʧ�ܣ�\n");
	}else if (nSuccess == 0){
		printf("***����ʧ�ܣ�ʧ��ԭ�򣺽��׳�ʱ��\n");
	}else{
		printf("***���׳ɹ���\n");
	}
	CIDMP_ORDER_INFO orderInfo;*/
//	GetRealTrade( trader, orderReq, orderInfo );
	getchar();
	return 0;
}

DWORD WINAPI TraderCore( LPVOID pParam )
{
	FILE *stream = NULL;
	stream = fopen( LOG_PATH.c_str(), "w" );
	if (stream == NULL){
		getchar();
	}
	//ǿ��ת������
	TraderThreadParameter *thread_para = (TraderThreadParameter *)pParam;
	CIDMPTradeApi *pTrader = thread_para->pTrader;
	CIDMP_ORDER_REQ *pOrder = thread_para->orderReq;
	int maxSeconds = thread_para->maxSecs;
	int nTicks = thread_para->nTicks;
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
			printf("***���¼۸�%f\n", lastPrice);
			count_time = cur_time - start_time;
		}

		//TimeTest
		if ( cur_time - start_time >= 1000 * maxSeconds * ( 1 - finalRatio ) ){
			//�������ղ���
			printf("***Go To Final Step...\n");
			int result = GotoFinalStep( pTrader, pOrder, maxSeconds, nTicks, finalRatio, minGap, stream );
			if ( result == 1 ){
				printf("***����ʱ...\n");
			}else if ( result == 0 ){
				printf("***���׳ɹ�...\n");
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
			if (orderInMarket){	//������ж���
				if ( lastPrice != priceInOrder ){
					OrderCancel( pTrader, pOrder, pInfo, stream );
					getTrade = UpdateOrder( pTrader, pOrder, pInfo,stream );
					if (getTrade){
						continue;
					}
				}else{
				getTrade = UpdateOrder( pTrader, pOrder, pInfo,stream );
				continue;
				}//if
			}else{
				//�������¼۸񵽶���ί����Ϣ��
				pOrder->orderPrice = lastPrice;
				printf("**���¶����۸����¼۸�=%f\n", lastPrice);
			}//if
			//InsertOrder
			//�������µĳɽ���Ϣ��ί�ж�����
			getTrade = UpdateOrder( pTrader, pOrder, pInfo, stream );
			printf("ί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);
			nSuccess = OrderInsert( pTrader, pOrder, pInfo, stream );
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
				orderStatus = -1;
				break;
			}else{
				if (nSuccess == 0){
					orderStatus = 1;
					break;
				}
				printf( "***������ί�гɹ���\n" );
				orderInMarket = true;
				priceInOrder = lastPrice;
			}//if
		}//if

	}//while
	if (getTrade){
		printf("***���׳ɹ���\n");
		orderStatus = 2;
	}
	fclose( stream );
	return 1;
}

inline bool UpdateOrder( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE* stream )
{
	if (pOrder->orderNo >= 0){
		pTrader->GetOrderByOrderNo( pOrder->orderNo, *pInfo );
	}
	if ( pInfo->orderStatus == 8 || pInfo->orderStatus == 5){
		pOrder->orderVol = pInfo->orderVol - pInfo->tradeVol;
	}
	if ( pInfo->orderStatus == 6 ){
		time_t rawtime;
		time(&rawtime);
		fprintf(stream, "ʱ��:%s �����ɽ���\nί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", asctime(localtime((&rawtime))),pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);

	}
	return pInfo->orderStatus == 6;
}

int OrderCancel( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE *stream )
{
	char errorMsg[200];
	bool cancelSuccess = false;
	while (!cancelSuccess){
		int nSuccess = pTrader->CancelOrder( pOrder->orderNo, errorMsg );
		UpdateOrder( pTrader, pOrder, pInfo, stream );
		if ( nSuccess >0 ){
			printf("***�����ɹ���...\n");
			time_t rawtime;
			time(&rawtime);
			fprintf(stream, "ʱ��:%s ���������ɹ���\nί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", asctime(localtime((&rawtime))),pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);

			cancelSuccess = true;
			return 0;
		}
		if ( pInfo->orderVol = pInfo->tradeVol + pInfo->canceledVol ){
			cancelSuccess = true;
			printf("***����ʧ�ܣ�ʧ��ԭ��ί���ѳɽ�...\n");
			return 1;
		}
	}

}
int OrderInsert( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, CIDMP_ORDER_INFO* pInfo, FILE* stream )
{
	bool missionFailed = false;
	int riskNum = 0;
	int nSuccess = pTrader->OrderInsert( *pOrder, riskNum );
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
			UpdateOrder( pTrader, pOrder, pInfo,stream );
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
		printf( "***������ί�гɹ������¼۸�=%f\n", pOrder->orderPrice );
		time_t rawtime;
		time(&rawtime);

		fprintf(stream, "ʱ��:%s ����ί�гɹ���\nί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", asctime(localtime((&rawtime))),pOrder->symbol, pOrder->direction, pOrder->offsetFlagType, pOrder->orderPrice, pOrder->orderVol);
	}
	return 1;
}
int GotoFinalStep( CIDMPTradeApi* pTrader, CIDMP_ORDER_REQ* pOrder, int maxSeconds, int nTicks, double finalRatio, double minGap, FILE *stream )
{
	clock_t start_time = clock();
	CIDMP_ORDER_INFO info;
	pTrader->GetOrderByOrderNo( pOrder->orderNo, info );
	bool getTrade = false;
	//�������г����е����򳷵�
	if ( info.orderVol > info.tradeVol + info.canceledVol ){
		OrderCancel( pTrader, pOrder, &info, stream );
	}
	while ( (clock() - start_time < maxSeconds * finalRatio * 1000) && !getTrade ){
		//���ö����۸�
		pOrder->orderPrice += minGap;
		for ( int i = 0; i < nTicks; i++ ){
			//�µ�
			if (OrderInsert( pTrader, pOrder, &info, stream ) == 0){
				return 0;
			}

			//sleep
			//Sleep(100);
			while (true){
				UpdateOrder( pTrader, pOrder, &info, stream );
				if ( info.orderStatus == 4 ){
					break;
				}
			}
			//����
			OrderCancel( pTrader, pOrder, &info, stream );
			getTrade = UpdateOrder( pTrader, pOrder, &info, stream );
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
/*
int TraderCore( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTicks, double finalRatio, double minGap )
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
			printf("***���¼۸�%f\n", lastPrice);
			count_time = clock() - start_time;
		}
		if ( cur_time - start_time >= 1000 * maxSeconds * (1 - finalRatio) ){
			//�������ղ���
			return GotoFinalStep( trader, orderReq, maxSeconds, nTicks, finalRatio, minGap );
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
//	printf("%f\n", orderReq.orderPrice);
	return (orderInfo.orderStatus == 6);	//1δ��2����3�ϵ�4�ѱ�5����6ȫ��7�ѱ�����8����9�������� 

}

int GotoFinalStep( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTicks, double finalRatio, double minGap )
{
	clock_t start_time = clock();
	int max_time = 1000 * maxSeconds * finalRatio;
	bool orderInMarket = true;

	while ( clock() - start_time <= max_time ){	//û�г�ʱ
		for ( int i  = 0; i < N_TIME; i++ ){
			//�۸�����
			//�Ƿ���Ҫ���۸����ޣ� todo
			orderReq.orderPrice += minGap * nTicks;
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