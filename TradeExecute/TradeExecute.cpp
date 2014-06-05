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

int TraderCore( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTicks, double finalRatio );

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
	nSuccess = TraderCore( trader, orderReq, 600, 3, 0.05 );
}

int TraderCore( CIDMPTradeApi &trader, CIDMP_ORDER_REQ &orderReq, int maxSeconds, int nTicks, double finalRatio )
{
	bool getTrade = false;
	bool orderInMarket = false;
	clock_t start_time = clock();
	double priceInOrder = 0;
	while ( !getTrade ){
		clock_t cur_time = clock();
		if ( cur_time - start_time >= maxSeconds * (1 - finalRatio) ){
			//进入最终步骤
			FinalStep();
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
					//OrderCancel

				}else{
					//获取最新交易信息Get Trade
					continue;
				}//if
			}else{
				//Risk
			}//if
			//OrderInsert
			//获取最新交易信息Get Trade

		}//if
	}//while
}