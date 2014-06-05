#ifndef CIDMPTradeApi_H__
#define CIDMPTradeApi_H__
#include "IDMPTradeDataType.h"
#include <iostream>
#include <vector>

using namespace std;



class __declspec(dllexport) CIDMPTradeApi
{
private:
	void *ds;
public:
	CIDMPTradeApi(void);
	~CIDMPTradeApi(void);
public:	
	

	

	/**
	 * 连接进入投资管理系统
	 * @servIp 系统中间层IP地址(如果是深证通网络,IP填写本机)
	 * @servPort 系统中间层端口(如果是深证通网络,端口请填写0)
	 * @userId 用户名
	 * @password 用户密码
	 * @netType 网络接入类型 0(局域网或者vpn直接接入 1 深证通网络接入)
	 *
	 * @return 返回是否登录成功 1 成功 -1 连接失败 -2 用户不存在或者密码错误
	 */
	int Connect(char *servIp,int servPort,char *userId,char* password,int netType = 0);

	/**
	 * 登出投资管理系统
	 */
	void DisConnect();

	/**
	 * 查询资产账户
	 * @info 资产账户信息数组，需要用户自行申请好足够的空间,一般情况，一个用户的资产账户不会超过10个
	 * @infoSize 资产账户信息数组长度(用户申请的结构体个数)
	 *
	 * @return 如果查询成功 则返回资产账户总数，否则查询失败  -1 未登录
	 */
	int GetFundAccount(CIDMP_FUND_ACCOUNT_INFO *info,int infoSize);

	/**
	 * 查询fundID 是否存在
	 * @fundId 资产ID
	 *
	 * @return 如果fundID 存在返回True，否则返回false
	 */
	bool CheckFund(int fundId);

	/**
	 * 查询CellID 是否存在
	 * @cellId 账户ID
	 *
	 * @return 如果cellID 存在返回True，否则返回false
	 */
	bool CheckCell(int fundId,int cellId);


	/**
	 * 查询资产单元
	 * @fundId 资产账户编码
	 * @info 资产单元信息数组，需要用户自行申请好足够的空间,一般情况，一个用户的资产账户下设的单元不会超过20个
	 * @infoSize 资产单元信息数组长度(用户申请的结构体个数)
	 *
	 * @return 如果查询成功 则返回该资产账户下的资产单元总数，否则查询失败  -1 未登录  -5 fundId或cellId不存在或未被授权
	 */
	int GetFundCell(int fundId,CIDMP_FUND_CELL_INFO *info,int infoSize);

	/**
	 * 查询资产组合(投资组合)
	 * @fundId 资产账户编码
	 * @cellId 资产单元编码
	 * @info 资产组合信息数组，需要用户自行申请好足够的空间,一般情况，一个用户的资产单元下设的组合不会超过50个
	 * @infoSize 资产组合信息数组长度(用户申请的结构体个数)
	 *
	 * @return 如果查询成功 则返回该资产账户和资产单元下的资产组合总数，否则查询失败  -1 未登录  -5 fundId或cellId不存在或未被授权
	 */
	int GetFundProfl(int fundId,int cellId,CIDMP_FUND_PROFL_INFO *info,int infoSize);



	/**
	 * 刷新资产账户、资产单元、资产组合的信息
	 *
	 * @retrun 1 刷新成功 -1 未登录
	 */
	int ReFresh();

	/**
	 * 查询资产账户资金信息
	 * @fundId 资产账户编码
	 * @fundInfo 资金信息
	 *
	 * @retrun 1 查询成功 -1 未登录  -5 fundId不存在或未被授权
	 */
	int GetFund(int fundId,CIDMP_FUND_INFO &fundInfo);

	 void string_replace(std::string& strBig, const std::string & strsrc, const std::string &strdst);

	/**
	 查询中间层、DLL 和数据库的版本信息
	 * @retrun 1 查询成功 -1 未登录
	 */
	int GetVersionInfo(CIDMP_VERSION_INFO &info);

	
	/**
	 * 查询资产单元资金信息
	 * @fundId 资产账户编码
	 * @cellId 资产单元编码
	 * @fundInfo 资金信息
	 *
	 * @retrun 1 查询成功 -1 未登录  -5 fundId或cellId不存在或未被授权
	 */
	int GetFund(int fundId,int cellId,CIDMP_FUND_INFO &fundInfo);


	/**
	 * 查询资产账户期货报表
	 * @fundId 资产账户编码
	 * @fundInfo 期货报表信息
	 *
	 * @retrun 1 查询成功 -1 未登录  -5 fundId不存在或未被授权
	 */
	int GetFutureRpt(int fundId,CIDMP_FUTURE_REPORT &fundInfo);

	
	/**
	 * 查询资产单元期货报表
	 * @fundId 资产账户编码
	 * @cellId 资产单元编码
	 * @fundInfo 期货报表信息
	 *
	 * @retrun 1 查询成功 -1 未登录  -5 fundId或cellId不存在或未被授权
	 */
	int GetFutureRpt(int fundId,int cellId,CIDMP_FUTURE_REPORT &fundInfo);

	
	/**
	 * 查询资产账户持仓
	 * @fundId 资产账户编码
	 * @exchgcode 交易所编号 0 查询全部交易所  1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	 * @symbol 证券代码或合约代码,如果输入空字符串，则返回所有代码的持仓
	 * @holdType 持仓类型 0 多仓 1 空仓 2 空仓+多仓
	 * @info 持仓信息数组，需要用户自行申请好足够的空间,一般情况，申请3000个空间足够查询到所有的当日持仓
	 * @infoSize 持仓信息数组长度(用户申请的结构体个数)
	 * @flag 是否过滤掉持仓为0的持仓 true 过滤   false 不过滤
	 *
	 * @return 如果查询成功 则返回持仓信息总数，否则查询失败  -1 未登录  -5 fundId或cellId不存在或未被授权
	 */
	int GetHold(int fundId,int exchgcode,char *symbol,int holdType,CIDMP_HOLD_INFO *info,int infoSize,bool flag = false);

	/**
	 * 查询资产单元持仓
	 * @fundId 资产账户编码
	 * @cellId 资产单元编码
	 * @exchgcode 交易所编号 0 查询全部交易所  1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	 * @symbol 证券代码或合约代码,如果输入空字符串，则返回所有代码的持仓
	 * @holdType 持仓类型 0 多仓 1 空仓 2 空仓+多仓
	 * @info 持仓信息数组，需要用户自行申请好足够的空间,一般情况，申请3000个空间足够查询到所有的当日持仓
	 * @infoSize 持仓信息数组长度(用户申请的结构体个数)
	 * @flag 是否过滤掉持仓为0的持仓 true 过滤   false 不过滤
	 *
	 * @return 如果查询成功 则返回持仓信息总数，否则查询失败  -1 未登录  -5 fundId或cellId不存在或未被授权
	 */
	int GetHold(int fundId,int cellId,int exchgcode,char *symbol,int holdType,CIDMP_HOLD_INFO *info,int infoSize,bool flag = false);

	/**
	 * 查询资产组合持仓
	 * @fundId 资产账户编码
	 * @cellId 资产单元编码
	 * @proflId 资产组合编码
	 * @exchgcode 交易所编号 0 查询全部交易所  1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	 * @symbol 证券代码或合约代码,如果输入空字符串，则返回所有代码的持仓
	 * @holdType 持仓类型 0 多仓 1 空仓 2 空仓+多仓
	 * @info 持仓信息数组，需要用户自行申请好足够的空间,一般情况，申请3000个空间足够查询到所有的当日持仓
	 * @infoSize 持仓信息数组长度(用户申请的结构体个数)
	 * @flag 是否过滤掉持仓为0的持仓 true 过滤   false 不过滤
	 *
	 * @return 如果查询成功 则返回持仓信息总数，否则查询失败  -1 未登录  -5 fundId或cellId不存在或未被授权
	 */
	int GetHold(int fundId,int cellId,i64 proflId,int exchgcode,char *symbol,int holdType,CIDMP_HOLD_INFO *info,int infoSize,bool flag = false);

	/**
	 * 查询资产账户委托订单信息
	 * @fundId 资产账户编码
	 * @info 订单信息数组，需要用户自行申请好足够的空间,由于交易量的不确定性，订单信息可能会很多，
			 一般不推荐频繁调用本接口,建议在客户端自行维护委托订单表缓存,对缓存的信息进行更加细粒度的查询，
			 比如针对某一个流水号的查询，再更新缓存数据。如果客户端刚刚登陆或者出现缓存被破坏的情况可以通过
			 此接口进行缓存构建或批量更新。
	 * @infoSize 订单信息数组长度(用户申请的结构体个数)
	 *
	 * @return 如果查询成功 则返回订单委托信息总数，否则查询失败  -1 未登录  -5 fundId不存在或未被授权
	 */
	int GetOrder(int fundId,CIDMP_ORDER_INFO *info,int infoSize);

	/**
	 * 查询资产单元委托订单信息
	 * @fundId 资产账户编码
	 * @cellId 资产单元编码
	 * @info 订单信息数组，需要用户自行申请好足够的空间,由于交易量的不确定性，订单信息可能会很多，
			 一般不推荐频繁调用本接口,建议在客户端自行维护委托订单表缓存,对缓存的信息进行更加细粒度的查询，
			 比如针对某一个流水号的查询，再更新缓存数据。如果客户端刚刚登陆或者出现缓存被破坏的情况可以通过
			 此接口进行缓存构建或批量更新。
	 * @infoSize 订单信息数组长度(用户申请的结构体个数)
	 *
	 * @return 如果查询成功 则返回订单委托信息总数，否则查询失败  -1 未登录  -5 fundId或cellId不存在或未被授权
	 */
	int GetOrder(int fundId,int cellId,CIDMP_ORDER_INFO *info,int infoSize);

	/**
	 * 查询资产组合委托订单信息
	 * @fundId 资产账户编码
	 * @cellId 资产单元编码
	 * @proflId 资产组合编码
	 * @info 订单信息数组，需要用户自行申请好足够的空间,由于交易量的不确定性，订单信息可能会很多，
			 一般不推荐频繁调用本接口,建议在客户端自行维护委托订单表缓存,对缓存的信息进行更加细粒度的查询，
			 比如针对某一个流水号的查询，再更新缓存数据。如果客户端刚刚登陆或者出现缓存被破坏的情况可以通过
			 此接口进行缓存构建或批量更新。
	 * @infoSize 订单信息数组长度(用户申请的结构体个数)
	 *
	 * @return 如果查询成功 则返回订单委托信息总数，否则查询失败  -1 未登录  -5 fundId或cellId不存在或未被授权
	 */
	int GetOrder(int fundId,int cellId,i64 proflId,CIDMP_ORDER_INFO *info,int infoSize);



	/**
	 * 查询某一特定委托的成交明细
	 * @orderNo 委托编号
	 * @info 成交信息数组，需要用户自行申请好足够的空间.一笔委托对应的成交数不会大于委托单位成交数量
	 * @infoSize 持仓信息数组长度(用户申请的结构体个数)
	 *
	 * @return 如果查询成功 则返回订单委托信息总数，否则查询失败  -1 未登录
	 */
	int GetTradeByOrderNo(int orderNo,CIDMP_TRADE_INFO *info,int infoSize);

	/**
	 * 查询资产账户成交信息
	 * @fundId 资产账户编码
	 * @info 成交信息数组，需要用户自行申请好足够的空间,由于交易量的不确定性，订单信息可能会很多，
			 一般不推荐频繁调用本接口,建议在客户端自行维护委托成交表缓存,对缓存的信息进行更加细粒度的查询，
			 比如针对某一个流水号的查询，再更新缓存数据。如果客户端刚刚登陆或者出现缓存被破坏的情况可以通过
			 此接口进行缓存构建或批量更新。
	 * @infoSize 持仓信息数组长度(用户申请的结构体个数)
	 *
	 * @return 如果查询成功 则返回订单委托信息总数，否则查询失败  -1 未登录   -5 fundId不存在或未被授权
	 */
	int GetTrade(int fundId,CIDMP_TRADE_INFO *info,int infoSize);

	/**
	 * 查询资产单元成交信息
	 * @fundId 资产账户编码
	 * @cellId 资产单元编码
	 * @info 成交信息数组，需要用户自行申请好足够的空间,由于交易量的不确定性，订单信息可能会很多，
			 一般不推荐频繁调用本接口,建议在客户端自行维护委托成交表缓存,对缓存的信息进行更加细粒度的查询，
			 比如针对某一个流水号的查询，再更新缓存数据。如果客户端刚刚登陆或者出现缓存被破坏的情况可以通过
			 此接口进行缓存构建或批量更新。
	 * @infoSize 成交信息数组长度(用户申请的结构体个数)
	 *
	 * @return 如果查询成功 则返回订单委托信息总数，否则查询失败  -1 未登录  -5 fundId或cellId不存在或未被授权
	 */
	int GetTrade(int fundId,int cellId,CIDMP_TRADE_INFO *info,int infoSize);

	/**
	 * 查询资产组合委托订单信息
	 * @fundId 资产账户编码
	 * @cellId 资产单元编码
	 * @proflId 资产组合编码
	 * @info 成交信息数组，需要用户自行申请好足够的空间,由于交易量的不确定性，订单信息可能会很多，
			 一般不推荐频繁调用本接口,建议在客户端自行维护委托成交表缓存,对缓存的信息进行更加细粒度的查询，
			 比如针对某一个流水号的查询，再更新缓存数据。如果客户端刚刚登陆或者出现缓存被破坏的情况可以通过
			 此接口进行缓存构建或批量更新。
	 * @infoSize 成交信息数组长度(用户申请的结构体个数)
	 *
	 * @return 如果查询成功 则返回订单委托信息总数，否则查询失败  -1 未登录 -5 fundId或cellId不存在或未被授权
	 */
	int GetTrade(int fundId,int cellId,i64 proflId,CIDMP_TRADE_INFO *info,int infoSize);



	/**
	 * 查询某一订单的信息
	 * @batNo 委托批号
	 * @info 订单信息
	 *
	 * @return 如果查询成功 则返回1，否则查询失败  -1 未登录
	 */
	int GetOrder(int batNo,CIDMP_ORDER_INFO &info);


	/**
	 * 查询某一订单的信息
	 * @orderNo 委托编号
	 * @info 订单信息
	 *
	 * @return 如果查询成功 则返回1，否则查询失败  -1 未登录
	 */
	int GetOrderByOrderNo(int orderNo,CIDMP_ORDER_INFO &info);


	///交易相关---------------------------------------------------------------

	/**
	 * 下订单
	 * @info 订单请求信息，当下单成功，信息中的流水号会被赋值，流水号可以用来查询订单信息以及撤单
	 * @riskNum 触发风控的数量 可以根据GetRisk 查询到具体的风控信息
	 *
	 * @return 如果成功 则返回1，否则失败 -1 未登录 -2 查询流水号失败 -3 风控未通过 -4 下单失败 -5 cellId或fundId不存在或未被授权 
	 */
	int OrderInsert(CIDMP_ORDER_REQ &info,int &riskNum);

	
	/**
	 * 划拨持仓
	 * @info 订单请求信息，当下单成功，信息中的流水号会被赋值，流水号可以用来查询订单信息以及撤单
	 * @riskNum 请求下达划拨合约个数
	 *
	 * @return 如果成功 则返回1，否则失败 -1 未登录 -2 查询流水号失败  -5 cellId或fundId不存在或未被授权 
	 */
	int HoldMove(CIDMP_ORDER_REQ *info,CIDMP_ORDER_REQ *outinfo,int num);
	/**
	 * 风险试算
	
	 * @info 订单请求信息
	 * @riskNum 触发风控的数量 可以根据GetRisk 查询到具体的风控信息
	 *
	 * @return 如果试算成功通过 则返回1，否则失败 -1 未登录 -2 查询流水号失败 -3 风控未通过 -5 cellId或fundId不存在或未被授权
	 */
	int RiskTest(CIDMP_ORDER_REQ &info,int &riskNum);


	/**
	 * 下订单(批量)
	 * @fundId 资产账户编码
	 * @cellId 资产单元编码
	 * @proflId 资产组合编码
	 * @info 订单请求信息数组，当下单成功，信息中的流水号会被赋值，流水号可以用来查询订单信息以及撤单
	 * @num 请求数组个数
	 * @riskNum 触发风控的数量 可以根据GetRisk 查询到具体的风控信息
	 *
	 * @return 如果成功 则返回1，否则失败 -1 未登录 -2 查询流水号失败 -3 风控未通过 -4 下单失败 -5 cellid或fundId不存在或未被授权 -6 下单部分失败
	 */
	int OrderInsert(int fundId,int cellId,i64 proflId,CIDMP_BAT_ORDER_REQ *info,int num,int &riskNum);

	
	/**
	 * 风险试算(批量)
	 * @fundId 资产账户编码
	 * @cellId 资产单元编码
	 * @proflId 资产组合编码
	 * @info 订单请求信息数组
	 * 
	 * @riskNum 触发风控的数量 可以根据GetRisk 查询到具体的风控信息
	 *
	 * @return 如果试算成功通过 则返回1，否则失败 -1 未登录 -2 查询流水号失败 -3 风控未通过 -5 fundId或cellId不存在或未被授权
	 */
	int RiskTest(int fundId,int cellId,i64 proflId,CIDMP_BAT_ORDER_REQ *info,int num,int &riskNum);


	


	/**
	 * 取消订单
	 * @batNo 订单序号
	 * @errorMsg 撤单错误信息，一般申请 200字节空间即可
	 *
	 * @return 如果成功 则返回1，否则查询失败 -1 未登录 -2 撤单失败(系统不接收该订单的撤消请求)
	 */
	int CancelOrder(int batNo,char *errorMsg);

	
	/**
	 * 取消订单(批量)
	 * @batNo 订单序号
	 * @errorMsg 撤单错误信息，一般申请 200字节空间即可
	 *
	 * @return 如果成功 则返回1，否则查询失败 -1 未登录 -2 撤单失败(系统不接收该订单的撤消请求) -6 下单部分失败
	 */
	int CancelOrder(CIDMP_BAT_ORDER_REQ *info,int num,char *errorMsg);

	/**
	 * 获取最后一次触发的风控信息
	 *
	 * @index 位置索引 
	 * 
	 * @return 如果成功 返回风控信息，如果失败 返回 NULL
	 */
	CIDMP_RISK_INFO *GetRisk(int index);

	/**
	 * 获取最后一次下单失败的信息
	 * 
	 * @return 返回出错信息字符串
	 */
	char *GetLastOrderInsertError();

	void split(std::string& s, std::string delim,std::vector< std::string >* ret);

	int RiskTest00(int fundId,int cellId,i64 proflId,CIDMP_BAT_ORDER_REQ *info,int num,int &riskNum,std::string &params,std::string &paramsRisk,int &sn);
	
	int RiskTest02(CIDMP_ORDER_REQ *info,int num,std::string &params);

	 int getIndex(int exchgcode,char *seccode);

	///行情相关---------------------------------------------------------------
	
	/**
	 * 获取实时行情信息（批量）
	 * @query 需要查询的股票信息数组,需要用户自行申请好足够的空间,可以为多个股票或者期货的篮子
	 * @info 行情信息数组，需要用户自行申请好足够的空间
	 * @infoSize 行情信息数组长度(用户申请的请求和返回结构体个数)
	 *
	 * @return 如果查询成功 则返回订单委托信息总数，否则查询失败  -1 未登录 -2 查询失败
	 */
	int GetTickQuotation(CIDMP_TICK_QUOTATION_QUERY *query,CIDMP_TICK_QUOTATION_INFO *info,int infoSize);

	/**
	 * 获取分钟行情信息
	 * @exchgcode 交易所代码
	 * @symbol 证券代码或者期货合约
	 * @start 起始时间 格式为yyMMddHHmmss 例:120406101100 代表2012年4月6日10点11分
	 * @end 结束时间 格式为yyMMddHHmmss 例:120406111100 代表2012年4月6日11点11分
	 * @info 行情信息数组，需要用户自行申请好足够的空间，一分钟一个行情
	 * @infoSize 行情信息数组长度(用户申请的请求和返回结构体个数)
	 *
	 * @return 如果查询成功 则返回订单委托信息总数，否则查询失败  -1 未登录 -2 查询失败
	 */
	int GetMinQuotation(int exchgcode,char *symbol,char *start,char *end,CIDMP_MIN_QUOTATION_INFO *info,int infoSize);
	
	///系统其它帮助方法---------------------------------------------------------------
	
	/**
	 * 获取系统自动化交易模块-->组合模板功能的篮子信息
	 * @basketId 篮子编号
	 * @info 篮子个券信息数组，需要用户自行申请好足够的空间
	 * @infoSize 篮子信息数组长度(用户申请的请求和返回结构体个数)
	 *
	 * @return 如果查询成功 则返回订单篮子个券信息总数，否则查询失败  -1 未登录 -2 查询失败
	 */
	int GetSysBasket(char *basketId,CIDMP_SYS_BASKET *info,int infoSize);

	/**
	 * 查询基金估值相关信息
	 * @fundId 资产账户编码
	 * @date 估值日期
	 * @info 估值信息数组，需要用户自行申请好足够的空间,一个基金估值的科目与持仓个券代码的多少相关，一般不大于持仓个券代码个数 + 200
	 * @infoSize 资产单元信息数组长度(用户申请的结构体个数)
	 *
	 * @return 如果查询成功 则返回该资产账户下的资产单元总数，否则查询失败  -1 未登录 -5 fundId 不存在或未被授权
	 */
	int GetFundValuation(int fundId,char *date,CIDMP_SYS_VALUATION *info,int infoSize);


	/**
	 * 获取系统股票基本信息
	 * @isRefresh 是否刷新
	 * @info 存储股票信息数组，需要用户自行申请好足够的空间
	 * @infoSize  股票信息数组长度(用户申请的请求和返回结构体个数)
	 * @return 股票信息列表
	 */
	int GetSysStockInfo(bool isRefresh,CIDMP_STOCK_INFO *info,int infoSize);

    /**
	 * 获取系统期货合约基本信息
	 * @isRefresh 是否刷是否刷新(绝大多数情况下不需要刷新，系统登录之后就会将信息载入内存)新
	 * @info 期货信息数组，需要用户自行申请好足够的空间
	 * @infoSize  期货信息数组长度(用户申请的请求和返回结构体个数)
	 * @return 期货信息列表
	 */
	int  GetSysFutrueInfo(bool isRefresh,CIDMP_FUTRUE_INFO *info,int infoSize);

     /**
	 * 查询清算数据保证金报表 
	 * @fundId 资产账户编码 
	 * @cellId 资产账户编码
	 * @symbol 股票代码或者期货合约代码（输入""，表示所有股票）
	 * @exchgcode 交易编号（0 表示所有交易所）
	 * @timeDate 查询日期（YYYYMMDD）
	 * @CIDMP_MARGIN_INFO *info 清算数据保证金报表
	 * @infoSize  清算数据保证金报表(用户申请的请求和返回结构体个数)
	 * @return  如果查询成功 则返回信息列表总数 ，否则查询失败，返回负数  -1 未登录-5 fundId或cellId 不存在或未被授权
	 */

	int  GetMargin(int fundId, int cellId,char *symbol, int exchgcode, char *timeDate,CIDMP_MARGIN_INFO *info,int infoSize);

    /**
	 * 查询清算数据手续费报表 
	 * @fundId 资产账户编码
	 * @cellId 资产账户编码
	 * @symbol 股票代码或者期货合约代码（输入""，表示所有股票）
	 * @exchgcode 交易编号（0 表示所有交易所）
	 * @timeDate 查询日期（YYYYMMDD）
	 * @flag 是否需要汇总（true 汇总，false 不需要汇总）
	 * @CIDMP_FEE_INFO *info 查询清算数据手续报表
	 * @infoSize  查询清算数据手续报表(用户申请的请求和返回结构体个数)
	 * @return  如果查询成功 则返回信息列表总数 ，否则查询失败，返回负数  -1 未登录-5 fundId或cellId 不存在或未被授权
	 */

	int  GetFee(int fundId,int cellId, char *symbol, int exchgcode, char *timeDate,bool flag,CIDMP_FEE_INFO *info,int infoSize);

     /**
	 * 查询清算数据出入金報表  
	 * @fundId 资产账户编码 
	 * @cellId 资产账户编码 
	 * @timeDate 查询日期（YYYYMMDD）
	 * @CIDMP_CASHINOUT_INFO *info 清算数据手续费汇总报表 
	 * @infoSize  查询清算数据出入金報表长度(用户申请的请求和返回结构体个数)
	 * @return  如果查询成功 则返回信息列表总数 ，否则查询失败，返回负数  -1 未登录-5 fundId或cellId 不存在或未被授权
	 */

	int GetCashInOut(int fundId, int cellId, char *timeDate,CIDMP_CASHINOUT_INFO *info,int infoSize);

};
#endif