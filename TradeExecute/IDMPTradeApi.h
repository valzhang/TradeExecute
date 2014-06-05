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
	 * ���ӽ���Ͷ�ʹ���ϵͳ
	 * @servIp ϵͳ�м��IP��ַ(�������֤ͨ����,IP��д����)
	 * @servPort ϵͳ�м��˿�(�������֤ͨ����,�˿�����д0)
	 * @userId �û���
	 * @password �û�����
	 * @netType ����������� 0(����������vpnֱ�ӽ��� 1 ��֤ͨ�������)
	 *
	 * @return �����Ƿ��¼�ɹ� 1 �ɹ� -1 ����ʧ�� -2 �û������ڻ����������
	 */
	int Connect(char *servIp,int servPort,char *userId,char* password,int netType = 0);

	/**
	 * �ǳ�Ͷ�ʹ���ϵͳ
	 */
	void DisConnect();

	/**
	 * ��ѯ�ʲ��˻�
	 * @info �ʲ��˻���Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,һ�������һ���û����ʲ��˻����ᳬ��10��
	 * @infoSize �ʲ��˻���Ϣ���鳤��(�û�����Ľṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻��ʲ��˻������������ѯʧ��  -1 δ��¼
	 */
	int GetFundAccount(CIDMP_FUND_ACCOUNT_INFO *info,int infoSize);

	/**
	 * ��ѯfundID �Ƿ����
	 * @fundId �ʲ�ID
	 *
	 * @return ���fundID ���ڷ���True�����򷵻�false
	 */
	bool CheckFund(int fundId);

	/**
	 * ��ѯCellID �Ƿ����
	 * @cellId �˻�ID
	 *
	 * @return ���cellID ���ڷ���True�����򷵻�false
	 */
	bool CheckCell(int fundId,int cellId);


	/**
	 * ��ѯ�ʲ���Ԫ
	 * @fundId �ʲ��˻�����
	 * @info �ʲ���Ԫ��Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,һ�������һ���û����ʲ��˻�����ĵ�Ԫ���ᳬ��20��
	 * @infoSize �ʲ���Ԫ��Ϣ���鳤��(�û�����Ľṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ظ��ʲ��˻��µ��ʲ���Ԫ�����������ѯʧ��  -1 δ��¼  -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int GetFundCell(int fundId,CIDMP_FUND_CELL_INFO *info,int infoSize);

	/**
	 * ��ѯ�ʲ����(Ͷ�����)
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ���Ԫ����
	 * @info �ʲ������Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,һ�������һ���û����ʲ���Ԫ�������ϲ��ᳬ��50��
	 * @infoSize �ʲ������Ϣ���鳤��(�û�����Ľṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ظ��ʲ��˻����ʲ���Ԫ�µ��ʲ���������������ѯʧ��  -1 δ��¼  -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int GetFundProfl(int fundId,int cellId,CIDMP_FUND_PROFL_INFO *info,int infoSize);



	/**
	 * ˢ���ʲ��˻����ʲ���Ԫ���ʲ���ϵ���Ϣ
	 *
	 * @retrun 1 ˢ�³ɹ� -1 δ��¼
	 */
	int ReFresh();

	/**
	 * ��ѯ�ʲ��˻��ʽ���Ϣ
	 * @fundId �ʲ��˻�����
	 * @fundInfo �ʽ���Ϣ
	 *
	 * @retrun 1 ��ѯ�ɹ� -1 δ��¼  -5 fundId�����ڻ�δ����Ȩ
	 */
	int GetFund(int fundId,CIDMP_FUND_INFO &fundInfo);

	 void string_replace(std::string& strBig, const std::string & strsrc, const std::string &strdst);

	/**
	 ��ѯ�м�㡢DLL �����ݿ�İ汾��Ϣ
	 * @retrun 1 ��ѯ�ɹ� -1 δ��¼
	 */
	int GetVersionInfo(CIDMP_VERSION_INFO &info);

	
	/**
	 * ��ѯ�ʲ���Ԫ�ʽ���Ϣ
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ���Ԫ����
	 * @fundInfo �ʽ���Ϣ
	 *
	 * @retrun 1 ��ѯ�ɹ� -1 δ��¼  -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int GetFund(int fundId,int cellId,CIDMP_FUND_INFO &fundInfo);


	/**
	 * ��ѯ�ʲ��˻��ڻ�����
	 * @fundId �ʲ��˻�����
	 * @fundInfo �ڻ�������Ϣ
	 *
	 * @retrun 1 ��ѯ�ɹ� -1 δ��¼  -5 fundId�����ڻ�δ����Ȩ
	 */
	int GetFutureRpt(int fundId,CIDMP_FUTURE_REPORT &fundInfo);

	
	/**
	 * ��ѯ�ʲ���Ԫ�ڻ�����
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ���Ԫ����
	 * @fundInfo �ڻ�������Ϣ
	 *
	 * @retrun 1 ��ѯ�ɹ� -1 δ��¼  -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int GetFutureRpt(int fundId,int cellId,CIDMP_FUTURE_REPORT &fundInfo);

	
	/**
	 * ��ѯ�ʲ��˻��ֲ�
	 * @fundId �ʲ��˻�����
	 * @exchgcode ��������� 0 ��ѯȫ��������  1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	 * @symbol ֤ȯ������Լ����,���������ַ������򷵻����д���ĳֲ�
	 * @holdType �ֲ����� 0 ��� 1 �ղ� 2 �ղ�+���
	 * @info �ֲ���Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,һ�����������3000���ռ��㹻��ѯ�����еĵ��ճֲ�
	 * @infoSize �ֲ���Ϣ���鳤��(�û�����Ľṹ�����)
	 * @flag �Ƿ���˵��ֲ�Ϊ0�ĳֲ� true ����   false ������
	 *
	 * @return �����ѯ�ɹ� �򷵻سֲ���Ϣ�����������ѯʧ��  -1 δ��¼  -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int GetHold(int fundId,int exchgcode,char *symbol,int holdType,CIDMP_HOLD_INFO *info,int infoSize,bool flag = false);

	/**
	 * ��ѯ�ʲ���Ԫ�ֲ�
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ���Ԫ����
	 * @exchgcode ��������� 0 ��ѯȫ��������  1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	 * @symbol ֤ȯ������Լ����,���������ַ������򷵻����д���ĳֲ�
	 * @holdType �ֲ����� 0 ��� 1 �ղ� 2 �ղ�+���
	 * @info �ֲ���Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,һ�����������3000���ռ��㹻��ѯ�����еĵ��ճֲ�
	 * @infoSize �ֲ���Ϣ���鳤��(�û�����Ľṹ�����)
	 * @flag �Ƿ���˵��ֲ�Ϊ0�ĳֲ� true ����   false ������
	 *
	 * @return �����ѯ�ɹ� �򷵻سֲ���Ϣ�����������ѯʧ��  -1 δ��¼  -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int GetHold(int fundId,int cellId,int exchgcode,char *symbol,int holdType,CIDMP_HOLD_INFO *info,int infoSize,bool flag = false);

	/**
	 * ��ѯ�ʲ���ϳֲ�
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ���Ԫ����
	 * @proflId �ʲ���ϱ���
	 * @exchgcode ��������� 0 ��ѯȫ��������  1 �Ϻ���Ʊ���� 2 ���ڹ�Ʊ������ 3 �Ϻ���Ʒ�ڻ������� 4 ֣����Ʒ�ڻ������� 5 ������Ʒ�ڻ������� 6 �й������ڻ�������
	 * @symbol ֤ȯ������Լ����,���������ַ������򷵻����д���ĳֲ�
	 * @holdType �ֲ����� 0 ��� 1 �ղ� 2 �ղ�+���
	 * @info �ֲ���Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,һ�����������3000���ռ��㹻��ѯ�����еĵ��ճֲ�
	 * @infoSize �ֲ���Ϣ���鳤��(�û�����Ľṹ�����)
	 * @flag �Ƿ���˵��ֲ�Ϊ0�ĳֲ� true ����   false ������
	 *
	 * @return �����ѯ�ɹ� �򷵻سֲ���Ϣ�����������ѯʧ��  -1 δ��¼  -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int GetHold(int fundId,int cellId,i64 proflId,int exchgcode,char *symbol,int holdType,CIDMP_HOLD_INFO *info,int infoSize,bool flag = false);

	/**
	 * ��ѯ�ʲ��˻�ί�ж�����Ϣ
	 * @fundId �ʲ��˻�����
	 * @info ������Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,���ڽ������Ĳ�ȷ���ԣ�������Ϣ���ܻ�ܶ࣬
			 һ�㲻�Ƽ�Ƶ�����ñ��ӿ�,�����ڿͻ�������ά��ί�ж�������,�Ի������Ϣ���и���ϸ���ȵĲ�ѯ��
			 �������ĳһ����ˮ�ŵĲ�ѯ���ٸ��»������ݡ�����ͻ��˸ոյ�½���߳��ֻ��汻�ƻ����������ͨ��
			 �˽ӿڽ��л��湹�����������¡�
	 * @infoSize ������Ϣ���鳤��(�û�����Ľṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ض���ί����Ϣ�����������ѯʧ��  -1 δ��¼  -5 fundId�����ڻ�δ����Ȩ
	 */
	int GetOrder(int fundId,CIDMP_ORDER_INFO *info,int infoSize);

	/**
	 * ��ѯ�ʲ���Ԫί�ж�����Ϣ
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ���Ԫ����
	 * @info ������Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,���ڽ������Ĳ�ȷ���ԣ�������Ϣ���ܻ�ܶ࣬
			 һ�㲻�Ƽ�Ƶ�����ñ��ӿ�,�����ڿͻ�������ά��ί�ж�������,�Ի������Ϣ���и���ϸ���ȵĲ�ѯ��
			 �������ĳһ����ˮ�ŵĲ�ѯ���ٸ��»������ݡ�����ͻ��˸ոյ�½���߳��ֻ��汻�ƻ����������ͨ��
			 �˽ӿڽ��л��湹�����������¡�
	 * @infoSize ������Ϣ���鳤��(�û�����Ľṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ض���ί����Ϣ�����������ѯʧ��  -1 δ��¼  -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int GetOrder(int fundId,int cellId,CIDMP_ORDER_INFO *info,int infoSize);

	/**
	 * ��ѯ�ʲ����ί�ж�����Ϣ
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ���Ԫ����
	 * @proflId �ʲ���ϱ���
	 * @info ������Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,���ڽ������Ĳ�ȷ���ԣ�������Ϣ���ܻ�ܶ࣬
			 һ�㲻�Ƽ�Ƶ�����ñ��ӿ�,�����ڿͻ�������ά��ί�ж�������,�Ի������Ϣ���и���ϸ���ȵĲ�ѯ��
			 �������ĳһ����ˮ�ŵĲ�ѯ���ٸ��»������ݡ�����ͻ��˸ոյ�½���߳��ֻ��汻�ƻ����������ͨ��
			 �˽ӿڽ��л��湹�����������¡�
	 * @infoSize ������Ϣ���鳤��(�û�����Ľṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ض���ί����Ϣ�����������ѯʧ��  -1 δ��¼  -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int GetOrder(int fundId,int cellId,i64 proflId,CIDMP_ORDER_INFO *info,int infoSize);



	/**
	 * ��ѯĳһ�ض�ί�еĳɽ���ϸ
	 * @orderNo ί�б��
	 * @info �ɽ���Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�.һ��ί�ж�Ӧ�ĳɽ����������ί�е�λ�ɽ�����
	 * @infoSize �ֲ���Ϣ���鳤��(�û�����Ľṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ض���ί����Ϣ�����������ѯʧ��  -1 δ��¼
	 */
	int GetTradeByOrderNo(int orderNo,CIDMP_TRADE_INFO *info,int infoSize);

	/**
	 * ��ѯ�ʲ��˻��ɽ���Ϣ
	 * @fundId �ʲ��˻�����
	 * @info �ɽ���Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,���ڽ������Ĳ�ȷ���ԣ�������Ϣ���ܻ�ܶ࣬
			 һ�㲻�Ƽ�Ƶ�����ñ��ӿ�,�����ڿͻ�������ά��ί�гɽ�����,�Ի������Ϣ���и���ϸ���ȵĲ�ѯ��
			 �������ĳһ����ˮ�ŵĲ�ѯ���ٸ��»������ݡ�����ͻ��˸ոյ�½���߳��ֻ��汻�ƻ����������ͨ��
			 �˽ӿڽ��л��湹�����������¡�
	 * @infoSize �ֲ���Ϣ���鳤��(�û�����Ľṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ض���ί����Ϣ�����������ѯʧ��  -1 δ��¼   -5 fundId�����ڻ�δ����Ȩ
	 */
	int GetTrade(int fundId,CIDMP_TRADE_INFO *info,int infoSize);

	/**
	 * ��ѯ�ʲ���Ԫ�ɽ���Ϣ
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ���Ԫ����
	 * @info �ɽ���Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,���ڽ������Ĳ�ȷ���ԣ�������Ϣ���ܻ�ܶ࣬
			 һ�㲻�Ƽ�Ƶ�����ñ��ӿ�,�����ڿͻ�������ά��ί�гɽ�����,�Ի������Ϣ���и���ϸ���ȵĲ�ѯ��
			 �������ĳһ����ˮ�ŵĲ�ѯ���ٸ��»������ݡ�����ͻ��˸ոյ�½���߳��ֻ��汻�ƻ����������ͨ��
			 �˽ӿڽ��л��湹�����������¡�
	 * @infoSize �ɽ���Ϣ���鳤��(�û�����Ľṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ض���ί����Ϣ�����������ѯʧ��  -1 δ��¼  -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int GetTrade(int fundId,int cellId,CIDMP_TRADE_INFO *info,int infoSize);

	/**
	 * ��ѯ�ʲ����ί�ж�����Ϣ
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ���Ԫ����
	 * @proflId �ʲ���ϱ���
	 * @info �ɽ���Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,���ڽ������Ĳ�ȷ���ԣ�������Ϣ���ܻ�ܶ࣬
			 һ�㲻�Ƽ�Ƶ�����ñ��ӿ�,�����ڿͻ�������ά��ί�гɽ�����,�Ի������Ϣ���и���ϸ���ȵĲ�ѯ��
			 �������ĳһ����ˮ�ŵĲ�ѯ���ٸ��»������ݡ�����ͻ��˸ոյ�½���߳��ֻ��汻�ƻ����������ͨ��
			 �˽ӿڽ��л��湹�����������¡�
	 * @infoSize �ɽ���Ϣ���鳤��(�û�����Ľṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ض���ί����Ϣ�����������ѯʧ��  -1 δ��¼ -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int GetTrade(int fundId,int cellId,i64 proflId,CIDMP_TRADE_INFO *info,int infoSize);



	/**
	 * ��ѯĳһ��������Ϣ
	 * @batNo ί������
	 * @info ������Ϣ
	 *
	 * @return �����ѯ�ɹ� �򷵻�1�������ѯʧ��  -1 δ��¼
	 */
	int GetOrder(int batNo,CIDMP_ORDER_INFO &info);


	/**
	 * ��ѯĳһ��������Ϣ
	 * @orderNo ί�б��
	 * @info ������Ϣ
	 *
	 * @return �����ѯ�ɹ� �򷵻�1�������ѯʧ��  -1 δ��¼
	 */
	int GetOrderByOrderNo(int orderNo,CIDMP_ORDER_INFO &info);


	///�������---------------------------------------------------------------

	/**
	 * �¶���
	 * @info ����������Ϣ�����µ��ɹ�����Ϣ�е���ˮ�Żᱻ��ֵ����ˮ�ſ���������ѯ������Ϣ�Լ�����
	 * @riskNum ������ص����� ���Ը���GetRisk ��ѯ������ķ����Ϣ
	 *
	 * @return ����ɹ� �򷵻�1������ʧ�� -1 δ��¼ -2 ��ѯ��ˮ��ʧ�� -3 ���δͨ�� -4 �µ�ʧ�� -5 cellId��fundId�����ڻ�δ����Ȩ 
	 */
	int OrderInsert(CIDMP_ORDER_REQ &info,int &riskNum);

	
	/**
	 * �����ֲ�
	 * @info ����������Ϣ�����µ��ɹ�����Ϣ�е���ˮ�Żᱻ��ֵ����ˮ�ſ���������ѯ������Ϣ�Լ�����
	 * @riskNum �����´ﻮ����Լ����
	 *
	 * @return ����ɹ� �򷵻�1������ʧ�� -1 δ��¼ -2 ��ѯ��ˮ��ʧ��  -5 cellId��fundId�����ڻ�δ����Ȩ 
	 */
	int HoldMove(CIDMP_ORDER_REQ *info,CIDMP_ORDER_REQ *outinfo,int num);
	/**
	 * ��������
	
	 * @info ����������Ϣ
	 * @riskNum ������ص����� ���Ը���GetRisk ��ѯ������ķ����Ϣ
	 *
	 * @return �������ɹ�ͨ�� �򷵻�1������ʧ�� -1 δ��¼ -2 ��ѯ��ˮ��ʧ�� -3 ���δͨ�� -5 cellId��fundId�����ڻ�δ����Ȩ
	 */
	int RiskTest(CIDMP_ORDER_REQ &info,int &riskNum);


	/**
	 * �¶���(����)
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ���Ԫ����
	 * @proflId �ʲ���ϱ���
	 * @info ����������Ϣ���飬���µ��ɹ�����Ϣ�е���ˮ�Żᱻ��ֵ����ˮ�ſ���������ѯ������Ϣ�Լ�����
	 * @num �����������
	 * @riskNum ������ص����� ���Ը���GetRisk ��ѯ������ķ����Ϣ
	 *
	 * @return ����ɹ� �򷵻�1������ʧ�� -1 δ��¼ -2 ��ѯ��ˮ��ʧ�� -3 ���δͨ�� -4 �µ�ʧ�� -5 cellid��fundId�����ڻ�δ����Ȩ -6 �µ�����ʧ��
	 */
	int OrderInsert(int fundId,int cellId,i64 proflId,CIDMP_BAT_ORDER_REQ *info,int num,int &riskNum);

	
	/**
	 * ��������(����)
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ���Ԫ����
	 * @proflId �ʲ���ϱ���
	 * @info ����������Ϣ����
	 * 
	 * @riskNum ������ص����� ���Ը���GetRisk ��ѯ������ķ����Ϣ
	 *
	 * @return �������ɹ�ͨ�� �򷵻�1������ʧ�� -1 δ��¼ -2 ��ѯ��ˮ��ʧ�� -3 ���δͨ�� -5 fundId��cellId�����ڻ�δ����Ȩ
	 */
	int RiskTest(int fundId,int cellId,i64 proflId,CIDMP_BAT_ORDER_REQ *info,int num,int &riskNum);


	


	/**
	 * ȡ������
	 * @batNo �������
	 * @errorMsg ����������Ϣ��һ������ 200�ֽڿռ伴��
	 *
	 * @return ����ɹ� �򷵻�1�������ѯʧ�� -1 δ��¼ -2 ����ʧ��(ϵͳ�����ոö����ĳ�������)
	 */
	int CancelOrder(int batNo,char *errorMsg);

	
	/**
	 * ȡ������(����)
	 * @batNo �������
	 * @errorMsg ����������Ϣ��һ������ 200�ֽڿռ伴��
	 *
	 * @return ����ɹ� �򷵻�1�������ѯʧ�� -1 δ��¼ -2 ����ʧ��(ϵͳ�����ոö����ĳ�������) -6 �µ�����ʧ��
	 */
	int CancelOrder(CIDMP_BAT_ORDER_REQ *info,int num,char *errorMsg);

	/**
	 * ��ȡ���һ�δ����ķ����Ϣ
	 *
	 * @index λ������ 
	 * 
	 * @return ����ɹ� ���ط����Ϣ�����ʧ�� ���� NULL
	 */
	CIDMP_RISK_INFO *GetRisk(int index);

	/**
	 * ��ȡ���һ���µ�ʧ�ܵ���Ϣ
	 * 
	 * @return ���س�����Ϣ�ַ���
	 */
	char *GetLastOrderInsertError();

	void split(std::string& s, std::string delim,std::vector< std::string >* ret);

	int RiskTest00(int fundId,int cellId,i64 proflId,CIDMP_BAT_ORDER_REQ *info,int num,int &riskNum,std::string &params,std::string &paramsRisk,int &sn);
	
	int RiskTest02(CIDMP_ORDER_REQ *info,int num,std::string &params);

	 int getIndex(int exchgcode,char *seccode);

	///�������---------------------------------------------------------------
	
	/**
	 * ��ȡʵʱ������Ϣ��������
	 * @query ��Ҫ��ѯ�Ĺ�Ʊ��Ϣ����,��Ҫ�û�����������㹻�Ŀռ�,����Ϊ�����Ʊ�����ڻ�������
	 * @info ������Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�
	 * @infoSize ������Ϣ���鳤��(�û����������ͷ��ؽṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ض���ί����Ϣ�����������ѯʧ��  -1 δ��¼ -2 ��ѯʧ��
	 */
	int GetTickQuotation(CIDMP_TICK_QUOTATION_QUERY *query,CIDMP_TICK_QUOTATION_INFO *info,int infoSize);

	/**
	 * ��ȡ����������Ϣ
	 * @exchgcode ����������
	 * @symbol ֤ȯ��������ڻ���Լ
	 * @start ��ʼʱ�� ��ʽΪyyMMddHHmmss ��:120406101100 ����2012��4��6��10��11��
	 * @end ����ʱ�� ��ʽΪyyMMddHHmmss ��:120406111100 ����2012��4��6��11��11��
	 * @info ������Ϣ���飬��Ҫ�û�����������㹻�Ŀռ䣬һ����һ������
	 * @infoSize ������Ϣ���鳤��(�û����������ͷ��ؽṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ض���ί����Ϣ�����������ѯʧ��  -1 δ��¼ -2 ��ѯʧ��
	 */
	int GetMinQuotation(int exchgcode,char *symbol,char *start,char *end,CIDMP_MIN_QUOTATION_INFO *info,int infoSize);
	
	///ϵͳ������������---------------------------------------------------------------
	
	/**
	 * ��ȡϵͳ�Զ�������ģ��-->���ģ�幦�ܵ�������Ϣ
	 * @basketId ���ӱ��
	 * @info ���Ӹ�ȯ��Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�
	 * @infoSize ������Ϣ���鳤��(�û����������ͷ��ؽṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ض������Ӹ�ȯ��Ϣ�����������ѯʧ��  -1 δ��¼ -2 ��ѯʧ��
	 */
	int GetSysBasket(char *basketId,CIDMP_SYS_BASKET *info,int infoSize);

	/**
	 * ��ѯ�����ֵ�����Ϣ
	 * @fundId �ʲ��˻�����
	 * @date ��ֵ����
	 * @info ��ֵ��Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�,һ�������ֵ�Ŀ�Ŀ��ֲָ�ȯ����Ķ�����أ�һ�㲻���ڳֲָ�ȯ������� + 200
	 * @infoSize �ʲ���Ԫ��Ϣ���鳤��(�û�����Ľṹ�����)
	 *
	 * @return �����ѯ�ɹ� �򷵻ظ��ʲ��˻��µ��ʲ���Ԫ�����������ѯʧ��  -1 δ��¼ -5 fundId �����ڻ�δ����Ȩ
	 */
	int GetFundValuation(int fundId,char *date,CIDMP_SYS_VALUATION *info,int infoSize);


	/**
	 * ��ȡϵͳ��Ʊ������Ϣ
	 * @isRefresh �Ƿ�ˢ��
	 * @info �洢��Ʊ��Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�
	 * @infoSize  ��Ʊ��Ϣ���鳤��(�û����������ͷ��ؽṹ�����)
	 * @return ��Ʊ��Ϣ�б�
	 */
	int GetSysStockInfo(bool isRefresh,CIDMP_STOCK_INFO *info,int infoSize);

    /**
	 * ��ȡϵͳ�ڻ���Լ������Ϣ
	 * @isRefresh �Ƿ�ˢ�Ƿ�ˢ��(�����������²���Ҫˢ�£�ϵͳ��¼֮��ͻὫ��Ϣ�����ڴ�)��
	 * @info �ڻ���Ϣ���飬��Ҫ�û�����������㹻�Ŀռ�
	 * @infoSize  �ڻ���Ϣ���鳤��(�û����������ͷ��ؽṹ�����)
	 * @return �ڻ���Ϣ�б�
	 */
	int  GetSysFutrueInfo(bool isRefresh,CIDMP_FUTRUE_INFO *info,int infoSize);

     /**
	 * ��ѯ�������ݱ�֤�𱨱� 
	 * @fundId �ʲ��˻����� 
	 * @cellId �ʲ��˻�����
	 * @symbol ��Ʊ��������ڻ���Լ���루����""����ʾ���й�Ʊ��
	 * @exchgcode ���ױ�ţ�0 ��ʾ���н�������
	 * @timeDate ��ѯ���ڣ�YYYYMMDD��
	 * @CIDMP_MARGIN_INFO *info �������ݱ�֤�𱨱�
	 * @infoSize  �������ݱ�֤�𱨱�(�û����������ͷ��ؽṹ�����)
	 * @return  �����ѯ�ɹ� �򷵻���Ϣ�б����� �������ѯʧ�ܣ����ظ���  -1 δ��¼-5 fundId��cellId �����ڻ�δ����Ȩ
	 */

	int  GetMargin(int fundId, int cellId,char *symbol, int exchgcode, char *timeDate,CIDMP_MARGIN_INFO *info,int infoSize);

    /**
	 * ��ѯ�������������ѱ��� 
	 * @fundId �ʲ��˻�����
	 * @cellId �ʲ��˻�����
	 * @symbol ��Ʊ��������ڻ���Լ���루����""����ʾ���й�Ʊ��
	 * @exchgcode ���ױ�ţ�0 ��ʾ���н�������
	 * @timeDate ��ѯ���ڣ�YYYYMMDD��
	 * @flag �Ƿ���Ҫ���ܣ�true ���ܣ�false ����Ҫ���ܣ�
	 * @CIDMP_FEE_INFO *info ��ѯ����������������
	 * @infoSize  ��ѯ����������������(�û����������ͷ��ؽṹ�����)
	 * @return  �����ѯ�ɹ� �򷵻���Ϣ�б����� �������ѯʧ�ܣ����ظ���  -1 δ��¼-5 fundId��cellId �����ڻ�δ����Ȩ
	 */

	int  GetFee(int fundId,int cellId, char *symbol, int exchgcode, char *timeDate,bool flag,CIDMP_FEE_INFO *info,int infoSize);

     /**
	 * ��ѯ�������ݳ�������  
	 * @fundId �ʲ��˻����� 
	 * @cellId �ʲ��˻����� 
	 * @timeDate ��ѯ���ڣ�YYYYMMDD��
	 * @CIDMP_CASHINOUT_INFO *info �������������ѻ��ܱ��� 
	 * @infoSize  ��ѯ�������ݳ���������(�û����������ͷ��ؽṹ�����)
	 * @return  �����ѯ�ɹ� �򷵻���Ϣ�б����� �������ѯʧ�ܣ����ظ���  -1 δ��¼-5 fundId��cellId �����ڻ�δ����Ȩ
	 */

	int GetCashInOut(int fundId, int cellId, char *timeDate,CIDMP_CASHINOUT_INFO *info,int infoSize);

};
#endif