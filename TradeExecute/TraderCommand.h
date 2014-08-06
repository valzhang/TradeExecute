#ifndef TRADER_COMMAND_H
#define TRADER_COMMAND_H
class TraderCommand{
public:
	int fund_id;
	int cell_id;
	int profl_id;
	int exchange_code;
	char symbol[8];
	int direction;
	int offset_flag_type;
	int order_volumn;
	int pj;
	int max_seconds;
};
#endif