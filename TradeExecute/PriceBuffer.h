#ifndef PRICE_BUFFER_H
#define PRICE_BUFFER_H
#include "stdafx.h"

class PriceBuffer{
private:
	boost::mutex mu;
	boost::condition_variable_any cond_price_change;
	boost::mutex tag_mu;

	int direct_change;
	string symbol_change;

	hash_map<string, pair<double, double> > all_symbol_price;
	hash_map<string, int> symbol_index;
		


public:
	vector<string> all_symbol;
	PriceBuffer(string file_name)
	{
		ifstream in;
		in.open(file_name);
		if (!in){
			cout << "Symbol List 读取失败\n";
			return;
		}
		string tmp;
		int count = 0;
		while (getline(in, tmp)){
			if (tmp.length() > 3){
				all_symbol.push_back(tmp);
				all_symbol_price[tmp] = make_pair(0, 0);
				symbol_index[tmp] = count++;
			}
		}
	}

	void UpdatePrice(double price, string symbol, int direction)
	{
		if (direction == 0){
			if (all_symbol_price[symbol].first != price){
				{
					tag_mu.lock();
					symbol_change = symbol;
					direct_change = direction;
					tag_mu.unlock();
				}
	//			printf("%s 买价变化，原价 = %7.3f，新价 = %7.3f\n", symbol.c_str(), all_symbol_price[symbol].first, price);
				all_symbol_price[symbol].first = price;				
				cond_price_change.notify_one();
				
			}
		}else{
			if (all_symbol_price[symbol].second != price){
				{
					tag_mu.lock();
					symbol_change = symbol;
					direct_change = direction;
					tag_mu.unlock();
				}				
		//		printf("%s 卖价变化，原价 = %7.3f，新价 = %7.3f\n", symbol.c_str(), all_symbol_price[symbol].second, price);
				all_symbol_price[symbol].second = price;
				cond_price_change.notify_all();
			}
		}
	}


	double WaitPriceChange(string symbol, int direction)
	{
		while(true){
			//等待锁触发
			boost::mutex::scoped_lock lock(mu);
			if (cond_price_change.timed_wait(lock, boost::get_system_time() + boost::posix_time::seconds(3))){
				boost::mutex::scoped_lock tag_lock(tag_mu);
				if (symbol_change.compare(symbol) == 0 && direct_change == direction){
					//如果目标合约，跳出
					printf("***目标合约，跳出\n");
					break;
				}else{
					printf("***无关合约，继续等待\n");
				}
			}else{//超时
				printf("***心跳信息...\n");
				return 0;
			}

		}
		return direction == 0 ? all_symbol_price[symbol].first : all_symbol_price[symbol].second;
#if 0
		if (direction == 0){
//			all_cond_buy_price_change[symbol].wait(mu);
			while(true){
				cond_price_change.wait(mu);
				tag_mu.lock();
				if (symbol_change.compare(symbol) == 0 && direct_change == direction){
					tag_mu.unlock();
					break;
				}else{
					tag_mu.unlock();
				}
			}			
			return all_symbol_price[symbol].first;
		}else{
//			all_cond_sell_price_change[symbol].wait(mu);
			while(true){
				cond_price_change.wait(mu);
				tag_mu.lock();
				if (symbol_change.compare(symbol) == 0 && direct_change == direction){
					tag_mu.unlock();
					break;
				}else{
					tag_mu.unlock();
				}
			}
			return all_symbol_price[symbol].second;
		}
#endif
	}

	double GetLastPrice(string symbol, int direction)
	{
		return direction == 0 ? all_symbol_price[symbol].first : all_symbol_price[symbol].second;
	}
};

#endif