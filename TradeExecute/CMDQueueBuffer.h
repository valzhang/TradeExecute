#ifndef CMD_QUEUE_BUFFER_H
#define CMD_QUEUE_BUFFER_H

#include "stdafx.h"

class CMDQueueBuffer{
private:
	boost::mutex mu;
	boost::condition_variable_any cond_put;
	boost::condition_variable_any cond_get;
	std::queue<TraderCommand> cmd_queue;
	int unread, capacity;
	bool termin;
	bool is_full()
	{
		return unread == capacity;
	}
	bool is_empty()
	{
		return unread == 0;
	}

public:
	CMDQueueBuffer(size_t n):unread(0),capacity(n){
		termin = false;
	}

	void PutCommand(TraderCommand& cmd){
		{
			boost::mutex::scoped_lock lock(mu);
			while (is_full()){
				{
					//boost::mutex::scoped_lock lock(io_mu);
					printf("***full waiting...\n");
				}
				cond_put.wait(mu);
			}
			cmd_queue.push(cmd);
			++unread;
			printf("***未处理命令数 = %d\n", unread);
		}
		cond_get.notify_one();
	}

	void GetCommand(TraderCommand* pCmd){
		{
			boost::mutex::scoped_lock lock(mu);
			while(is_empty()){
				{
					//boost::mutex::scoped_lock lock(io_mu);
					printf("***登陆完成，等待命令...\n");
				}
				cond_get.wait(mu);
				if (termin){
					return;
				}
			}
			--unread;
			*pCmd = cmd_queue.front();
			cmd_queue.pop();
			printf("***取命令成功！剩余未处理命令 = %d\n", unread);
		}
		cond_put.notify_one();
	}

	void ReleaseCommand(){
		termin = true;
		cond_get.notify_all();
	}
};

#endif