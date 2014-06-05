#ifndef CIDMP_DATATYPE_H__
#define CIDMP_DATATYPE_H__

//定义i64为一个64位的整型数据
#ifdef WIN32
#define i64 long long
#else
#define i64 long
#endif


//资产账户信息
typedef struct {
	int id;//编号
	char name[200];//名称
} CIDMP_FUND_ACCOUNT_INFO;


//投资单元信息
typedef struct {
	int id;//编号
	char name[200];//名称
} CIDMP_FUND_CELL_INFO;

//投资组合信息
typedef struct {
	i64 id;//编号
	char name[200];//名称
} CIDMP_FUND_PROFL_INFO;


//投资单元或者投资账户的资金信息
typedef struct {
	double futureAvailable;//期货交易可用资金
	double futureTotal;//期货总资金
	double stkAvailable;//现货交易可用
	double stkTotal;//现货总资金
}CIDMP_FUND_INFO;

//投资单元或者投资账户的期货资产报表信息
typedef struct {
	double futureAvailable;//期货交易可用资金
	double futureTotal;//期货资金余额
	double futureMargin;//占用保证金
	double beginEquity;//期初权益
	double endEquity;//期末权益
	double closeProfit;//平仓盈亏
	double positionProfit;//持仓盈亏
	double fee;//总手续费
	double risk;//风险度
}CIDMP_FUTURE_REPORT;

//版本信息
typedef struct {
	char dllVersion[32];//DLL version
	char dbVersion[32];//数据库 version
	char servVersion[32];//中间层version
}CIDMP_VERSION_INFO;

//投资单元或者投资账户的持仓信息
typedef struct {
	char symbol[8];//股票代码或者期货合约代码
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	int holdType;//持仓类型 0 多仓 1 空仓
	int available;//交易可用数量
	int total;//总持仓数量
	double closeProfit;//平仓盈亏
	double positionProfit;//持仓盈亏
	double fee;//总手续费
	double cost1;//当前证券总成本(平均成本)
	double cost2;//当前证券总成本(先进先出算法)
	double etfin;//ETF 申购
	double etfout;//ETF 赎回
	double etfuseamt;//ETF申购使用当日买入数量
	int boughtAmt;//当日买入/开仓成交量
	int soldAmt;//当日卖出/平仓成交量

	int available_jr;//可平今仓数量
	int available_zr;//可平昨仓数量
}CIDMP_HOLD_INFO;


//订单信息
typedef struct {
	int orderNo;//投资管理系统订单号
	int batNo;//订单流水号
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	char symbol[8];//股票代码或者期货合约代码
	int direction;//买卖方向 0 买 1 卖
	int offsetFlagType;//开平方向 0 开 1 平 （证券可忽略此字段）
	double orderPrice;//订单价格
	int orderVol;//订单数量
	int tradeVol;//成交数量
	int canceledVol;//撤单数量
	int orderStatus;//1未报2待报3废单4已报5部成6全成7已报撤单8部成撤单9待撤
	char userId[15];//下单用户id
	char orderTime[20];//下单时间，格式为yyyy-MM-ddTHH:mm:ss,例:2012-02-14T13:43:51
	char errorInfo[100];//错误信息描述
	int pj;/// 是否平今 0:否 1:是

}CIDMP_ORDER_INFO;


//订单请求
typedef struct {
	int orderNo;//订单号，该号不需要填充，如果订单成功，系统会自动填充
	int fundId;//资产账户编码
	int cellId;//资产单元编码
	i64 proflId;//资产组合编码
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	char symbol[8];//股票代码或者期货合约代码
	int direction;//买卖方向 0 买 1 卖
	int offsetFlagType;//开平方向 0 开 1 平 （证券可忽略此字段）
	double orderPrice;//订单价格
	int orderVol;//订单数量
	int pj;/// 是否平今 0:否 1:是
}CIDMP_ORDER_REQ;

//订单请求
typedef struct {
	int orderNo;//订单号，该号不需要填充，如果订单成功，系统会自动填充
	int fundId;//资产账户编码
	int cellId;//资产单元编码
	i64 proflId;//资产组合编码
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	char symbol[8];//股票代码或者期货合约代码
	int direction;//买卖方向 0 买 1 卖
	int offsetFlagType;//开平方向 0 开 1 平 （证券可忽略此字段）
	double orderPrice;//订单价格
	int orderVol;//订单数量
	char errorInfo[512]; // 错误信息
}CIDMP_ORDER_HOLDMOVEREQ;



//订单请求(批量)
typedef struct {
	int orderNo;//订单号，该号不需要填充，如果订单成功，系统会自动填充
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	char symbol[8];//股票代码或者期货合约代码
	int direction;//买卖方向 0 买 1 卖
	int offsetFlagType;//开平方向 0 开 1 平 （证券可忽略此字段）
	double orderPrice;//订单价格
	int orderVol;//订单数量
	char errorInfo[512]; // 错误信息
	char reqId[8]; // 下单请求ID，由用户指定该属性值，仅用于用户识别该笔订单,系统不对其做任何处理
	int pj;/// 是否平今 0:否 1:是
}CIDMP_BAT_ORDER_REQ;


//成交信息
typedef struct {
	int orderNo;//投资管理系统订单号
	__int64 tradeNo;//投资管理系统的成交回报编号	
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	char symbol[8];//股票代码或者期货合约代码
	int direction;//买卖方向 0 买 1 卖
	int offsetFlagType;//开平方向 0 开 1 平 （证券可忽略此字段）
	double tradePrice;//成交价格
	int tradeVol;//成交数量
	double tradeFund;//成交金额
	char userId[15];//下单用户id
	char systemNo[20];//交易合同号
	char tradeTime[20];//成交时间，格式为yyyy-MM-ddTHH:mm:ss,例:2012-02-14T13:43:51
	int pj;/// 是否平今 0:否 1:是

}CIDMP_TRADE_INFO;

//风控信息
typedef struct {
	char symbol[8];//股票代码或者期货合约代码
	int riskId;//风控编号
	int direction;//买卖方向 0 买 1 卖
	int offsetFlagType;//开平方向 0 开 1 平 （证券可忽略此字段）
	char explain[500];//风控说明
	double molecular;//分子
	double denominator;//分母
	char compdire[20];//比较方向

	char riskValue[30];//结算结果
	char waringOperate[20];//触警操作
	char warning[20];//触警阀值
	int orderNum;//委托数量
	double orderPrice;//委托价格

}CIDMP_RISK_INFO;


//----------------------------------------行情相关

//实时行情信息
typedef struct {
	char symbol[8];//股票代码或者期货合约代码
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	char name[12];//股票名称或者期货合约名称
	char time[20];             //时间
	double zrsp;             //昨日收盘
	double jrkp;             //今日开盘
	double zgj;              //最高价
	double zdj;              //最低价
	double zxj;              //最新价
	double cjje;             //成交金额
	double cjsl;             //成交数量
	double zrjs;             //昨日结算
	double jrjs;             //今日结算
	double zcc;              //总持仓



	char tpbj;             //停盘标记 Y 正常 N 停牌
	double sp1;              //卖价1
	double sp2;              //卖价2
	double sp3;              //卖价3
	double sp4;              //卖价4
	double sp5;              //卖价5
	double sl1;              //卖量1
	double sl2;              //卖量2
	double sl3;              //卖量3
	double sl4;              //卖量4
	double sl5;              //卖量5
	double bp1;              //买价1
	double bp2;              //买价2
	double bp3;              //买价3
	double bp4;              //买价4
	double bp5;              //买价5
	double bl1;              //买量1
	double bl2;              //买量2
	double bl3;              //买量3
	double bl4;              //买量4
	double bl5;              //买量5


	double iopv;     // iopv 

	/// <summary>
	/// 涨停价
	/// </summary>
	double ztj;
	/// <summary>
	///跌停价  
	/// </summary>
	double dtj;
	/// <summary>
	/// 今日收盘
	/// </summary>
	double jrsp;

}CIDMP_TICK_QUOTATION_INFO;



//分钟行情信息
typedef struct {
	char symbol[8];//股票代码或者期货合约代码
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	char name[12];//股票名称或者期货合约名称
	char time[20];             //时间
	double zrsp;             //昨日收盘
	
	double zxj;              //最新价
	double cjje;             //成交金额
	double cjsl;             //成交数量

}CIDMP_MIN_QUOTATION_INFO;


//实时行情查询信息
typedef struct  {
	char symbol[8];//股票代码或者期货合约代码
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	
}CIDMP_TICK_QUOTATION_QUERY;


//系统其它信息------------------------------------------

//自动化交易组合篮子信息
typedef struct {
	char symbol[8];//股票代码或者期货合约代码
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	int basketVol;//篮子数量
}CIDMP_SYS_BASKET;

//股票基本信息
typedef struct 
{
	 char symbol[8];//股票代码或者期货合约代码
	 char stkname[12];//股票名称
	 int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	 double upPrice;//涨停价格
	 double lowPrice;//跌停价格
	 char currency[4];//币别 RMB，USD HKD
	 double priceUnit;//最小价格变动
	 double upDealNum;//最大下单数量
	 double lowDealNum;//最小下单数量

}CIDMP_STOCK_INFO;


//期货合约基本信息
typedef struct 
{
	 char symbol[8];//股票代码或者期货合约代码
	 int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	 double upPrice;//涨停价格
	 double lowPrice;//跌停价格
	 double moneyRate;//保证金比例
	 double multiplier;//合约乘数
	 char mktdate1[20];//上市日
	 char mktdate2[20];//到期日
	 double priceUnit;//最小价格变动
	 double upDealNum;//最大下单数量
	 double lowDealNum;//最小下单数量
}CIDMP_FUTRUE_INFO;


//系统估值信息
typedef struct {
	char ffdate[20];// 日期
    char a0[100];//科目号
	char a1[100];//科目名称
	double a2;//数量
	double a3;//价格
	double a4;//成本
	char a5[20];//
	double a6;//
	double a7;//市值
	char a8[20];//
	double a9;//
	char a10[50];//
	char a11[20];//
	char fundid[6];//基金账户
}CIDMP_SYS_VALUATION;

//保证金基本信息
typedef struct 
{
	char date[20];   // 日期
	char holdacc[100];//持仓账户
	int fundid;   //资产账户
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	char symbol[8];//股票代码或者期货合约代码
	char stkname[12];//股票名称
	int direction;//买卖方向 0 买 1 卖
	int amt;//持仓数量
	double settlementprice;//今結算价
	double lsettlementprice;//昨結算价
	double margin;//保证金
	double sprofit;//浮动盈亏
}CIDMP_MARGIN_INFO;


//费用基本信息
typedef struct  
{

	char date[20];   // 日期
	int fundid;   //资产账户
	int exchgcode;//交易编号 1 上海股票交所 2 深圳股票交易所 3 上海商品期货交易所 4 郑州商品期货交易所 5 大连商品期货交易所 6 中国金融期货交易所
	char symbol[8];//股票代码或者期货合约代码
	char stkname[12];//股票名称
	int direction;//买卖方向 0 买 1 卖
	int cjamt;//成交数量
	double cjprice;//成交价格
	double fee;//手续费

}CIDMP_FEE_INFO;

//出入金基本信息
typedef struct
{
	char date[20];   // 日期 
	int fundid;   //资产账户
	int direction;//出入金方向 0 出金 1 入金
	double cashincashout;//出入金金额

}CIDMP_CASHINOUT_INFO;


typedef struct
{
	char exchgcode[10];		// 交易所
	char stkcode[12];		// 合约
	char stkcodename[50];	// 合约名称
	char dealdata[12];		// 交割日
	char lasttrday[12];		// 最后可开仓日期
	char pcday[12];			// 强平日期(交割日前一交易日)
}CIDMP_INDEX_LIST;
#endif