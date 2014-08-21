#include "TraderCommand.h"
#include <boost/thread/thread.hpp>
 #include <boost/date_time/posix_time/posix_time.hpp>
 #include <boost/date_time/gregorian/gregorian.hpp>
//#include <libs/thread/src/win32/thread.cpp>
//#include <libs/thread/src/win32/tss_dll.cpp>
//#include <libs/thread/src/win32/tss_pe.cpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include "IDMPTradeApi.h"
#include <queue>
#include "CMDQueueBuffer.h"
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <hash_map>
#include <fstream>
#include <stdexcept>
#include <afx.h>
#include "PriceBuffer.h"
#include "LogMsgBuffer.h"
#include "TDFAPI.h"
#include <boost/assert.hpp>
#define SAFE_CHAR(ch) ((ch) ? (ch) : ' ')
#define ELEM_COUNT(arr) (sizeof(arr)/sizeof(arr[0]))

#ifndef MY_LOG_MSG
#define MY_LOG_MSG
typedef struct MyLogMsg{
	boost::posix_time::ptime log_time;
	int server_no;
	char cmd_status[16];
	int cmd_rtn;
	int server_status;
	int direction;
	int offset_flag;
	int order_vol;
	int trade_vol;
	int cancel_vol;
	double price;
} MyLogMsg;
#endif

#ifndef TRANS_LOG_MSG
#define TRANS_LOG_MSG
typedef struct TransLogMsg{
	boost::posix_time::ptime trans_time;
	//int local_no;
	int order_vol;
	int trade_vol;
	double trade_price;
	double mean_price;
} TransLogMsg;
#endif