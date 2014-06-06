#include "IDMPTradeApi.h"
#include <iomanip>
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char SERVER_IP[16] = "";
const int SERVER_PORT = 0;
const char USER_NAME[20] = "";
const char USER_PASSWORD[20] = "";

const int FUND_ID = 0;	//�ʲ��˻�����
const int CELL_ID = 0;	//�ʲ���Ԫ����
const i64 PROFL_ID = 0;	//�ʲ���ϱ���
const int DIRECTION = 0;	//���׷���
const int EXCHANGE = 0;	//���������
const int OFFSET = 0;		//��ƽ����
const int PJ = 0;			//�Ƿ�ƽ��
const char SYMBOL[8] = "AG1409";	//��Ʊ���ڻ�����
const int INI_PRICE = 0;
const int INI_VOL = 0;

const int FUND_SIZE = 100;
const int HOLD_SIZE = 300;
const int ORDER_SIZE = 300;
const int N_TIME = 3;

int TraderCore( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTicks, double finalRatio, double minGap );

//��ӡ��ί��������Ϣ
inline void PrintOrderMsg( CIDMP_ORDER_REQ &orderReq );

//��ȡ���½�����Ϣ
inline bool GetRealTrade( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq );

int GotoFinalStep( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTicks, double finalRatio, double minGap );
int main()
{

	CIDMPTradeApi trader;
	printf("�Ƿ��¼���׷�������(Y/y��¼����������)\n");
	//��¼
	printf("***���ڵ�¼�� %s, %d, %s...\n", SERVER_IP, SERVER_PORT, USER_NAME);
	int logSuccess = trader.Connect( SERVER_IP, SERVER_PORT, USER_NAME, USER_PASSWORD, 0 );
	if ( logSuccess != 1 ){
		printf("��¼ʧ�ܣ�\n");
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
	nSuccess = TraderCore( trader, orderReq, 600, 3, 0.05, minGap );
	if (nSuccess < 0){
		printf("***����ʧ�ܣ�ʧ��ԭ�򣺷�ؼ��ʧ�ܣ�\n");
	}else if (nSuccess == 0){
		printf("***����ʧ�ܣ�ʧ��ԭ�򣺽��׳�ʱ��\n");
	}else{
		printf("***���׳ɹ���\n");
	}
	GetRealTrade( trader, orderReq );
}

int TraderCore( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTicks, double finalRatio, double minGap )
{


	bool getTrade = false;
	bool orderInMarket = false;
	clock_t start_time = clock();
	double priceInOrder = 0;
	while ( !getTrade ){
		clock_t cur_time = clock();
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
			double lastPrice;
			if ( orderReq.direction == 0 ){		//�����������Ϊ��
				lastPrice = realInfo[0].bp1;			//��ȡ������һ��			
			}else{
				lastPrice = realInfo[0].sp1;			//��ȡ������һ��
			}//if
			if ( orderInMarket ){
				if ( lastPrice != priceInOrder ){
					//Risk
					orderReq.orderPrice = lastPrice;
					int riskNum = 0;
					printf("ί����Ϣ:��Լ=%s,����=%d,��ƽ=%d,�۸�=%6.3f,����=%d\n", 
						orderReq.symbol, orderReq.direction, orderReq.offsetFlagType, orderReq.orderPrice, orderReq.orderVol);
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
					PrintOrderMsg(orderReq);
					
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
					getTrade = GetRealTrade( trader, orderReq );
					continue;
				}//if
			}else{
				//Risk
			}//if
			//OrderInsert�µ�
			int riskNum;
			int orderSuccess = trader.OrderInsert( orderReq, riskNum );
			PrintOrderMsg(orderReq);
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
				priceInOrder = lastPrice;
			}
			//��ȡ���½�����ϢGet Trade
			getTrade = GetRealTrade( trader, orderReq );
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

inline bool GetRealTrade( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq )
{
	printf("\n***��ѯί����Ϣ��\n");
	CIDMP_ORDER_INFO orderInfo;
	int checkSuccess = trader.GetOrder( orderReq.orderNo, orderInfo );
	if(checkSuccess != 1) {	    
		printf("��ѯί����Ϣʧ��!\n");
	}
	printf("ί�к�\t״̬\t֤ȯ/��Լ����\t����\t��ƽ\t�۸�\t����\tʱ��\t����Ա\t������Ϣ\n");
	printf("%d\t%d\t%s\t%d\t%d\t%6.3f\t%d\t%s\t%s\t%s\n",
		orderInfo.orderNo, orderInfo.orderStatus, orderInfo.symbol, orderInfo.direction, orderInfo.offsetFlagType,
		orderInfo.orderPrice, orderInfo.orderVol, orderInfo.orderTime, orderInfo.userId, orderInfo.errorInfo);	
	//���ֲ��ֳɽ���ȫ���ɽ� todo
	return (orderInfo.orderStatus == 6);	//1δ��2����3�ϵ�4�ѱ�5����6ȫ��7�ѱ�����8����9�������� 

}

int GotoFinalStep( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTicks, double finalRatio, double minGap )
{
	clock_t start_time = clock();
	int max_time = 1000 * maxSeconds * finalRatio;

	while ( clock() - start_time <= max_time ){	//û�г�ʱ
		for ( int i  = 0; i < N_TIME; i++ ){
			//�۸�����
			//�Ƿ���Ҫ���۸����ޣ� todo
			orderReq.orderPrice += minGap * nTicks;
			//�µ�
			int riskNum;
			int orderSuccess = trader.OrderInsert( orderReq, riskNum );
			PrintOrderMsg(orderReq);
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
			}
			
			//����
			char errorMsg[200];
			int cancelSuccess = trader.CancelOrder( orderReq.orderNo, errorMsg );//orderNo��batNo���� 
			if ( cancelSuccess < 0 ){	//����ʧ�ܵĲ��� todo
				printf("***����ʧ�ܣ�\n");
				if ( GetRealTrade( trader, orderReq ) ){
					printf("***���׳ɹ���\n");
					return 1;
				}else{
					return -1;
				}
			}else{
				printf("***�����ɹ���\n");
			}
			if ( GetRealTrade( trader, orderReq ) ){
				printf("***���׳ɹ���\n");
				return 1;
			}
		}
	}
	return 0;
}