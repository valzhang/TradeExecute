#ifndef CIDMP_DATATYPE_H__
#define CIDMP_DATATYPE_H__

//����i64Ϊһ��64λ����������
#ifdef WIN32
#define i64 long long
#else
#define i64 long
#endif


//�ʲ��˻���Ϣ
typedef struct {
	int id;//���
	char name[200];//����
} CIDMP_FUND_ACCOUNT_INFO;


//Ͷ�ʵ�Ԫ��Ϣ
typedef struct {
	int id;//���
	char name[200];//����
} CIDMP_FUND_CELL_INFO;

//Ͷ�������Ϣ
typedef struct {
	i64 id;//���
	char name[200];//����
} CIDMP_FUND_PROFL_INFO;


//Ͷ�ʵ�Ԫ����Ͷ���˻����ʽ���Ϣ
typedef struct {
	double futureAvailable;//�ڻ����׿����ʽ�
	double futureTotal;//�ڻ����ʽ�
	double stkAvailable;//�ֻ����׿���
	double stkTotal;//�ֻ����ʽ�
}CIDMP_FUND_INFO;

//Ͷ�ʵ�Ԫ����Ͷ���˻����ڻ��ʲ�������Ϣ
typedef struct {
	double futureAvailable;//�ڻ����׿����ʽ�
	double futureTotal;//�ڻ��ʽ����
	double futureMargin;//ռ�ñ�֤��
	double beginEquity;//�ڳ�Ȩ��
	double endEquity;//��ĩȨ��
	double closeProfit;//ƽ��ӯ��
	double positionProfit;//�ֲ�ӯ��
	double fee;//��������
	double risk;//���ն�
}CIDMP_FUTURE_REPORT;

//�汾��Ϣ
typedef struct {
	char dllVersion[32];//DLL version
	char dbVersion[32];//���ݿ� version
	char servVersion[32];//�м��version
}CIDMP_VERSION_INFO;

//Ͷ�ʵ�Ԫ����Ͷ���˻��ĳֲ���Ϣ
typedef struct {
	char symbol[8];//��Ʊ��������ڻ���Լ����
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	int holdType;//�ֲ����� 0 ��� 1 �ղ�
	int available;//���׿�������
	int total;//�ֲܳ�����
	double closeProfit;//ƽ��ӯ��
	double positionProfit;//�ֲ�ӯ��
	double fee;//��������
	double cost1;//��ǰ֤ȯ�ܳɱ�(ƽ���ɱ�)
	double cost2;//��ǰ֤ȯ�ܳɱ�(�Ƚ��ȳ��㷨)
	double etfin;//ETF �깺
	double etfout;//ETF ���
	double etfuseamt;//ETF�깺ʹ�õ�����������
	int boughtAmt;//��������/���ֳɽ���
	int soldAmt;//��������/ƽ�ֳɽ���

	int available_jr;//��ƽ�������
	int available_zr;//��ƽ�������
}CIDMP_HOLD_INFO;


//������Ϣ
typedef struct {
	int orderNo;//Ͷ�ʹ���ϵͳ������
	int batNo;//������ˮ��
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	char symbol[8];//��Ʊ��������ڻ���Լ����
	int direction;//�������� 0 �� 1 ��
	int offsetFlagType;//��ƽ���� 0 �� 1 ƽ ��֤ȯ�ɺ��Դ��ֶΣ�
	double orderPrice;//�����۸�
	int orderVol;//��������
	int tradeVol;//�ɽ�����
	int canceledVol;//��������
	int orderStatus;//1δ��2����3�ϵ�4�ѱ�5����6ȫ��7�ѱ�����8���ɳ���9����
	char userId[15];//�µ��û�id
	char orderTime[20];//�µ�ʱ�䣬��ʽΪyyyy-MM-ddTHH:mm:ss,��:2012-02-14T13:43:51
	char errorInfo[100];//������Ϣ����
	int pj;/// �Ƿ�ƽ�� 0:�� 1:��

}CIDMP_ORDER_INFO;


//��������
typedef struct {
	int orderNo;//�����ţ��úŲ���Ҫ��䣬��������ɹ���ϵͳ���Զ����
	int fundId;//�ʲ��˻�����
	int cellId;//�ʲ���Ԫ����
	i64 proflId;//�ʲ���ϱ���
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	char symbol[8];//��Ʊ��������ڻ���Լ����
	int direction;//�������� 0 �� 1 ��
	int offsetFlagType;//��ƽ���� 0 �� 1 ƽ ��֤ȯ�ɺ��Դ��ֶΣ�
	double orderPrice;//�����۸�
	int orderVol;//��������
	int pj;/// �Ƿ�ƽ�� 0:�� 1:��
}CIDMP_ORDER_REQ;

//��������
typedef struct {
	int orderNo;//�����ţ��úŲ���Ҫ��䣬��������ɹ���ϵͳ���Զ����
	int fundId;//�ʲ��˻�����
	int cellId;//�ʲ���Ԫ����
	i64 proflId;//�ʲ���ϱ���
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	char symbol[8];//��Ʊ��������ڻ���Լ����
	int direction;//�������� 0 �� 1 ��
	int offsetFlagType;//��ƽ���� 0 �� 1 ƽ ��֤ȯ�ɺ��Դ��ֶΣ�
	double orderPrice;//�����۸�
	int orderVol;//��������
	char errorInfo[512]; // ������Ϣ
}CIDMP_ORDER_HOLDMOVEREQ;



//��������(����)
typedef struct {
	int orderNo;//�����ţ��úŲ���Ҫ��䣬��������ɹ���ϵͳ���Զ����
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	char symbol[8];//��Ʊ��������ڻ���Լ����
	int direction;//�������� 0 �� 1 ��
	int offsetFlagType;//��ƽ���� 0 �� 1 ƽ ��֤ȯ�ɺ��Դ��ֶΣ�
	double orderPrice;//�����۸�
	int orderVol;//��������
	char errorInfo[512]; // ������Ϣ
	char reqId[8]; // �µ�����ID�����û�ָ��������ֵ���������û�ʶ��ñʶ���,ϵͳ���������κδ���
	int pj;/// �Ƿ�ƽ�� 0:�� 1:��
}CIDMP_BAT_ORDER_REQ;


//�ɽ���Ϣ
typedef struct {
	int orderNo;//Ͷ�ʹ���ϵͳ������
	__int64 tradeNo;//Ͷ�ʹ���ϵͳ�ĳɽ��ر����	
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	char symbol[8];//��Ʊ��������ڻ���Լ����
	int direction;//�������� 0 �� 1 ��
	int offsetFlagType;//��ƽ���� 0 �� 1 ƽ ��֤ȯ�ɺ��Դ��ֶΣ�
	double tradePrice;//�ɽ��۸�
	int tradeVol;//�ɽ�����
	double tradeFund;//�ɽ����
	char userId[15];//�µ��û�id
	char systemNo[20];//���׺�ͬ��
	char tradeTime[20];//�ɽ�ʱ�䣬��ʽΪyyyy-MM-ddTHH:mm:ss,��:2012-02-14T13:43:51
	int pj;/// �Ƿ�ƽ�� 0:�� 1:��

}CIDMP_TRADE_INFO;

//�����Ϣ
typedef struct {
	char symbol[8];//��Ʊ��������ڻ���Լ����
	int riskId;//��ر��
	int direction;//�������� 0 �� 1 ��
	int offsetFlagType;//��ƽ���� 0 �� 1 ƽ ��֤ȯ�ɺ��Դ��ֶΣ�
	char explain[500];//���˵��
	double molecular;//����
	double denominator;//��ĸ
	char compdire[20];//�ȽϷ���

	char riskValue[30];//������
	char waringOperate[20];//��������
	char warning[20];//������ֵ
	int orderNum;//ί������
	double orderPrice;//ί�м۸�

}CIDMP_RISK_INFO;


//----------------------------------------�������

//ʵʱ������Ϣ
typedef struct {
	char symbol[8];//��Ʊ��������ڻ���Լ����
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	char name[12];//��Ʊ���ƻ����ڻ���Լ����
	char time[20];             //ʱ��
	double zrsp;             //��������
	double jrkp;             //���տ���
	double zgj;              //��߼�
	double zdj;              //��ͼ�
	double zxj;              //���¼�
	double cjje;             //�ɽ����
	double cjsl;             //�ɽ�����
	double zrjs;             //���ս���
	double jrjs;             //���ս���
	double zcc;              //�ֲܳ�



	char tpbj;             //ͣ�̱�� Y ���� N ͣ��
	double sp1;              //����1
	double sp2;              //����2
	double sp3;              //����3
	double sp4;              //����4
	double sp5;              //����5
	double sl1;              //����1
	double sl2;              //����2
	double sl3;              //����3
	double sl4;              //����4
	double sl5;              //����5
	double bp1;              //���1
	double bp2;              //���2
	double bp3;              //���3
	double bp4;              //���4
	double bp5;              //���5
	double bl1;              //����1
	double bl2;              //����2
	double bl3;              //����3
	double bl4;              //����4
	double bl5;              //����5


	double iopv;     // iopv 

	/// <summary>
	/// ��ͣ��
	/// </summary>
	double ztj;
	/// <summary>
	///��ͣ��  
	/// </summary>
	double dtj;
	/// <summary>
	/// ��������
	/// </summary>
	double jrsp;

}CIDMP_TICK_QUOTATION_INFO;



//����������Ϣ
typedef struct {
	char symbol[8];//��Ʊ��������ڻ���Լ����
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	char name[12];//��Ʊ���ƻ����ڻ���Լ����
	char time[20];             //ʱ��
	double zrsp;             //��������
	
	double zxj;              //���¼�
	double cjje;             //�ɽ����
	double cjsl;             //�ɽ�����

}CIDMP_MIN_QUOTATION_INFO;


//ʵʱ�����ѯ��Ϣ
typedef struct  {
	char symbol[8];//��Ʊ��������ڻ���Լ����
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	
}CIDMP_TICK_QUOTATION_QUERY;


//ϵͳ������Ϣ------------------------------------------

//�Զ����������������Ϣ
typedef struct {
	char symbol[8];//��Ʊ��������ڻ���Լ����
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	int basketVol;//��������
}CIDMP_SYS_BASKET;

//��Ʊ������Ϣ
typedef struct 
{
	 char symbol[8];//��Ʊ��������ڻ���Լ����
	 char stkname[12];//��Ʊ����
	 int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	 double upPrice;//��ͣ�۸�
	 double lowPrice;//��ͣ�۸�
	 char currency[4];//�ұ� RMB��USD HKD
	 double priceUnit;//��С�۸�䶯
	 double upDealNum;//����µ�����
	 double lowDealNum;//��С�µ�����

}CIDMP_STOCK_INFO;


//�ڻ���Լ������Ϣ
typedef struct 
{
	 char symbol[8];//��Ʊ��������ڻ���Լ����
	 int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	 double upPrice;//��ͣ�۸�
	 double lowPrice;//��ͣ�۸�
	 double moneyRate;//��֤�����
	 double multiplier;//��Լ����
	 char mktdate1[20];//������
	 char mktdate2[20];//������
	 double priceUnit;//��С�۸�䶯
	 double upDealNum;//����µ�����
	 double lowDealNum;//��С�µ�����
}CIDMP_FUTRUE_INFO;


//ϵͳ��ֵ��Ϣ
typedef struct {
	char ffdate[20];// ����
    char a0[100];//��Ŀ��
	char a1[100];//��Ŀ����
	double a2;//����
	double a3;//�۸�
	double a4;//�ɱ�
	char a5[20];//
	double a6;//
	double a7;//��ֵ
	char a8[20];//
	double a9;//
	char a10[50];//
	char a11[20];//
	char fundid[6];//�����˻�
}CIDMP_SYS_VALUATION;

//��֤�������Ϣ
typedef struct 
{
	char date[20];   // ����
	char holdacc[100];//�ֲ��˻�
	int fundid;   //�ʲ��˻�
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	char symbol[8];//��Ʊ��������ڻ���Լ����
	char stkname[12];//��Ʊ����
	int direction;//�������� 0 �� 1 ��
	int amt;//�ֲ�����
	double settlementprice;//��Y���
	double lsettlementprice;//��Y���
	double margin;//��֤��
	double sprofit;//����ӯ��
}CIDMP_MARGIN_INFO;


//���û�����Ϣ
typedef struct  
{

	char date[20];   // ����
	int fundid;   //�ʲ��˻�
	int exchgcode;//���ױ�� 1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	char symbol[8];//��Ʊ��������ڻ���Լ����
	char stkname[12];//��Ʊ����
	int direction;//�������� 0 �� 1 ��
	int cjamt;//�ɽ�����
	double cjprice;//�ɽ��۸�
	double fee;//������

}CIDMP_FEE_INFO;

//����������Ϣ
typedef struct
{
	char date[20];   // ���� 
	int fundid;   //�ʲ��˻�
	int direction;//������� 0 ���� 1 ���
	double cashincashout;//�������

}CIDMP_CASHINOUT_INFO;


typedef struct
{
	char exchgcode[10];		// ������
	char stkcode[12];		// ��Լ
	char stkcodename[50];	// ��Լ����
	char dealdata[12];		// ������
	char lasttrday[12];		// ���ɿ�������
	char pcday[12];			// ǿƽ����(������ǰһ������)
}CIDMP_INDEX_LIST;
#endif