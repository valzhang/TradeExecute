#ifndef LOG_MSG_BUFFER_H
#define LOG_MSG_BUFFER_H
#include "stdafx.h"

class LogMsgBuffer{
private:
	queue<MyLogMsg> m_log;
	int m_log_size;
	boost::mutex m_mu_log;
	boost::condition_variable_any m_log_cond;
	ofstream log_out;
	ofstream action_out;
	int m_local_no;
	bool term;

	queue<TransLogMsg> m_trans_log;
	int m_trans_size;
	boost::mutex m_mu_trans;
	boost::condition_variable_any m_trand_cond;
	ofstream trans_out;

public:
	LogMsgBuffer(string path, string no_path, string trans_path) : m_log_size(0){ 
		term = false;
		log_out.open(path); 
		BOOST_ASSERT(log_out);
		trans_out.open(trans_path, ios::app);
		BOOST_ASSERT(trans_out);
		ifstream in;
		in.open(no_path);
		BOOST_ASSERT(in);
		int no;
		while (in >> no){}
		m_local_no = no;
		in.close();
		action_out.open(no_path, ios::app);
		BOOST_ASSERT(action_out);
	};

	void PutMsg(boost::posix_time::ptime time, int server_no, char* cmd_status, int cmd_rtn, int server_status, int direction, int offset, int order_vol, int trade_vol, int cancel_vol, double price){
		MyLogMsg msg;
		msg.log_time = time;
		msg.server_no = server_no;
		strcpy(msg.cmd_status, cmd_status);
		msg.cmd_rtn = cmd_rtn;
		msg.server_status = server_status;
		msg.direction = direction;
		msg.offset_flag = offset;
		msg.order_vol = order_vol;
		msg.trade_vol = trade_vol;
		msg.cancel_vol = cancel_vol;
		msg.price = price;
		{
			boost::mutex::scoped_lock lock(m_mu_log);
			m_log.push(msg);
			cout << "����һ��log\n";
			++m_log_size;
		}
		m_log_cond.notify_one();
	}
	void PutMsg(boost::posix_time::ptime time, int server_no, char* cmd_status, int server_status, int direction, int offset, int order_vol, int trade_vol, int cancel_vol, double price){
		MyLogMsg msg;
		msg.log_time = time;
		msg.server_no = server_no;
		strcpy(msg.cmd_status, cmd_status);
		msg.cmd_rtn = 999;
		msg.server_status = server_status;
		msg.direction = direction;
		msg.offset_flag = offset;
		msg.order_vol = order_vol;
		msg.trade_vol = trade_vol;
		msg.cancel_vol = cancel_vol;
		msg.price = price;
		{
			boost::mutex::scoped_lock lock(m_mu_log);
			m_log.push(msg);
			cout << "����һ��log\n";
			++m_log_size;
		}
		m_log_cond.notify_one();
	}
	void PrintMsg(){
		{
			boost::mutex::scoped_lock lock(m_mu_log);
			while(m_log_size == 0){
				m_log_cond.wait(m_mu_log);
				if (term){
					printf("Waii term suc\n");
					return;
				}
			}
			--m_log_size;
			cout << "��ӡһ��log\n";
			MyLogMsg msg = m_log.front();
			log_out << "ʱ�� = " << msg.log_time << '\t'
				<< "���ر�� = " << m_local_no << '\t'
				<< "��������� = " << msg.server_no << '\t'
				<< "ָ��״̬ = " << msg.cmd_status << '\t'
				<< "ָ��� = ";
			switch (msg.cmd_rtn){
			case 1:
				log_out << "��������";
				break;
			case 999:
				log_out << "�޷���";
				break;
			default:
				log_out << "���󷵻�";
				break;
			}
			log_out << '\t'
				<< "������״̬ = ";
			switch (msg.server_status){
			case 1:
				log_out << "δ��";
				break;;
			case 2:
				log_out << "����";
				break;
			case 3:
				log_out << "�ϵ�";
				break;
			case 4:
				log_out << "�ѱ�";
				break;
			case 5:
				log_out << "���ֳɽ�";
				break;
			case 6:
				log_out << "ȫ���ɽ�";
				break;
			case 7:
				log_out << "�ѱ�����";
				break;
			case 8:
				log_out << "���ֳ���";
				break;
			case 9:
				log_out << "��������";
				break;
			default:
				log_out << "δ֪״̬";
				break;
			}
			log_out << '\t'
				<< "�������� = " << (msg.direction == 0 ? "��" : "��") << '\t'
				<< "��ƽ���� = " << (msg.offset_flag == 0 ? "��" : "ƽ") << '\t'
				<< "�������� = " << msg.order_vol << '\t'
				<< "�ɽ����� = " << msg.trade_vol << '\t'
				<< "�������� = " << msg.cancel_vol << '\t'
				<< "�����۸� = " << msg.price << '\n';
			
			m_log.pop();
			++m_local_no;
			action_out << '\n' << m_local_no;
		}
	}
	void Release(){
		term = true;
		printf("***Buffer Thread Release\n");
		m_log_cond.notify_one();
		m_trand_cond.notify_one();
	}

	void PutTransMsg(boost::posix_time::ptime time,/* int local_no,*/ int order_vol, int trade_vol, double trade_price, double mean_price){
		TransLogMsg msg;
		msg.trans_time = time;
	//	msg.local_no = local_no;
		msg.order_vol = order_vol;
		msg.trade_vol = trade_vol;
		msg.trade_price = trade_price;
		msg.mean_price = mean_price;
		{
			boost::mutex::scoped_lock lock(m_mu_trans);
			m_trans_log.push(msg);
			++m_trans_size;
		}
		m_trand_cond.notify_one();
	}
	void PrintTransMsg(){
		{
			boost::mutex::scoped_lock lock(m_mu_trans);
			while(m_trans_size == 0){
				m_trand_cond.wait(m_mu_trans);
				if (term){
					return;
				}
				--m_trans_size;
				TransLogMsg msg = m_trans_log.front();
				trans_out << "ʱ�� = " << msg.trans_time << '\t'
			//		<< "���ر�� = " << msg.local_no << '\t'
					<< "�������� = " << msg.order_vol << '\t'
					<< "�ɽ����� = " << msg.trade_vol << '\t'
					<< "�ɽ��۸� = " << msg.trade_price << '\t'
					<< "�ɽ����� = " << msg.mean_price << '\n';
				m_trans_log.pop();
				cout << "��ӡһ���ɽ���Ϣ\n";
			}
		}
	}
};

#endif