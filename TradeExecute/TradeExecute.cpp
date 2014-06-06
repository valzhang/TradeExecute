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

const int FUND_ID = 0;	//资产账户编码
const int CELL_ID = 0;	//资产单元编码
const i64 PROFL_ID = 0;	//资产组合编码
const int DIRECTION = 0;	//交易方向
const int EXCHANGE = 0;	//交易所编号
const int OFFSET = 0;		//开平方向
const int PJ = 0;			//是否平今
const char SYMBOL[8] = "AG1409";	//股票或期货代码
const int INI_PRICE = 0;
const int INI_VOL = 0;

const int FUND_SIZE = 100;
const int HOLD_SIZE = 300;
const int ORDER_SIZE = 300;
const int N_TIME = 3;

int TraderCore( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTicks, double finalRatio, double minGap );

//打印出委托请求信息
inline void PrintOrderMsg( CIDMP_ORDER_REQ &orderReq );

//获取最新交易信息
inline bool GetRealTrade( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq );

int GotoFinalStep( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTicks, double finalRatio, double minGap );
int main()
{

	CIDMPTradeApi trader;
	printf("是否登录交易服务器？(Y/y登录，其他返回)\n");
	//登录
	printf("***正在登录： %s, %d, %s...\n", SERVER_IP, SERVER_PORT, USER_NAME);
	int logSuccess = trader.Connect( SERVER_IP, SERVER_PORT, USER_NAME, USER_PASSWORD, 0 );
	if ( logSuccess != 1 ){
		printf("登录失败！\n");
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
		printf("***交易失败，失败原因：风控检查失败！\n");
	}else if (nSuccess == 0){
		printf("***交易失败，失败原因：交易超时！\n");
	}else{
		printf("***交易成功！\n");
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
			//进入最终步骤
			return GotoFinalStep( trader, orderReq, maxSeconds, nTicks, finalRatio, minGap );
			break;
		}else{
			//获取最新价格
			CIDMP_TICK_QUOTATION_QUERY *realQuery = new CIDMP_TICK_QUOTATION_QUERY[1];
			CIDMP_TICK_QUOTATION_INFO *realInfo = new CIDMP_TICK_QUOTATION_INFO[1];
			realQuery->exchgcode = EXCHANGE;
			strcpy( realQuery->symbol, SYMBOL );
			trader.GetTickQuotation( realQuery, realInfo, 1 );
			double lastPrice;
			if ( orderReq.direction == 0 ){		//如果订单方向为买
				lastPrice = realInfo[0].bp1;			//获取最新买一价			
			}else{
				lastPrice = realInfo[0].sp1;			//获取最新卖一价
			}//if
			if ( orderInMarket ){
				if ( lastPrice != priceInOrder ){
					//Risk
					orderReq.orderPrice = lastPrice;
					int riskNum = 0;
					printf("委托信息:合约=%s,买卖=%d,开平=%d,价格=%6.3f,数量=%d\n", 
						orderReq.symbol, orderReq.direction, orderReq.offsetFlagType, orderReq.orderPrice, orderReq.orderVol);
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
					PrintOrderMsg(orderReq);
					
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
					getTrade = GetRealTrade( trader, orderReq );
					continue;
				}//if
			}else{
				//Risk
			}//if
			//OrderInsert下单
			int riskNum;
			int orderSuccess = trader.OrderInsert( orderReq, riskNum );
			PrintOrderMsg(orderReq);
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
				priceInOrder = lastPrice;
			}
			//获取最新交易信息Get Trade
			getTrade = GetRealTrade( trader, orderReq );
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

inline bool GetRealTrade( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq )
{
	printf("\n***查询委托信息：\n");
	CIDMP_ORDER_INFO orderInfo;
	int checkSuccess = trader.GetOrder( orderReq.orderNo, orderInfo );
	if(checkSuccess != 1) {	    
		printf("查询委托信息失败!\n");
	}
	printf("委托号\t状态\t证券/合约代码\t买卖\t开平\t价格\t数量\t时间\t交易员\t错误信息\n");
	printf("%d\t%d\t%s\t%d\t%d\t%6.3f\t%d\t%s\t%s\t%s\n",
		orderInfo.orderNo, orderInfo.orderStatus, orderInfo.symbol, orderInfo.direction, orderInfo.offsetFlagType,
		orderInfo.orderPrice, orderInfo.orderVol, orderInfo.orderTime, orderInfo.userId, orderInfo.errorInfo);	
	//区分部分成交与全部成交 todo
	return (orderInfo.orderStatus == 6);	//1未报2待报3废单4已报5部成6全成7已报撤单8部成9撤单待撤 

}

int GotoFinalStep( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTicks, double finalRatio, double minGap )
{
	clock_t start_time = clock();
	int max_time = 1000 * maxSeconds * finalRatio;

	while ( clock() - start_time <= max_time ){	//没有超时
		for ( int i  = 0; i < N_TIME; i++ ){
			//价格增加
			//是否需要检查价格上限？ todo
			orderReq.orderPrice += minGap * nTicks;
			//下单
			int riskNum;
			int orderSuccess = trader.OrderInsert( orderReq, riskNum );
			PrintOrderMsg(orderReq);
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
			}
			
			//撤单
			char errorMsg[200];
			int cancelSuccess = trader.CancelOrder( orderReq.orderNo, errorMsg );//orderNo与batNo区别 
			if ( cancelSuccess < 0 ){	//撤单失败的操作 todo
				printf("***撤单失败！\n");
				if ( GetRealTrade( trader, orderReq ) ){
					printf("***交易成功！\n");
					return 1;
				}else{
					return -1;
				}
			}else{
				printf("***撤单成功！\n");
			}
			if ( GetRealTrade( trader, orderReq ) ){
				printf("***交易成功！\n");
				return 1;
			}
		}
	}
	return 0;
}