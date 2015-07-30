#pragma once

#define DEBUG          1
#define CHECK_ACCESS   1 // ¬ключена ли сетева€ проверка пользовател€
#define LOCAL          0 // ќпредел€ет сервер проверки пользовател€, как локальный
#define LIB_VERSION    1.0.15
#define PARTNER_CODE   0

#define MAGIC_OC 0x7ED80000

#define OP_BUY       0
#define OP_SELL      1
#define OP_BUYLIMIT  2
#define OP_SELLLIMIT 3
#define OP_BUYSTOP   4
#define OP_SELLSTOP  5
//work statuses
#define STATUS_OK				0
#define STATUS_DANGER			1
#define STATUS_SOFT_BREAK		2
#define STATUS_HARD_BREAK		3
#define STATUS_EMERGENCY_BREAK	4
//Errors and status providers
#define PROVIDER_SERVER		0
#define PROVIDER_TERMINAL	1
#define PROVIDER_BROKER		2
#define PROVIDER_MQL		3
#define PROVIDER_DLL		4
#define PROVIDER_STRATEGY	5
#define PROVIDERS_COUNT		6

#define JOB_EXIT   0
#define JOB_CREATE 1
#define JOB_MODIFY 2
#define JOB_DELETE 3
#define JOB_CLOSE  4
#define JOB_PRINT_ORDER	5
#define JOB_PRINT_TEXT	6
#define JOB_DRAW_ORDER	7
#define JOB_SHOW_VALUE	8
#define JOB_MASG_BOX	9

#define SHOW_STR_VALUE		0
#define SHOW_INT_VALUE		1
#define SHOW_DOUBLE_VALUE	2

#define MM_NOMM			0
#define MM_BALANCE		1
#define MM_EQUITY		2

#define STANDART_CONTRACT	100000

#define PERIOD_M1      1     // 1 минута
#define PERIOD_M5      5     // 5 минут
#define PERIOD_M15     15    // 15 минут
#define PERIOD_M30     30    // 30 минут
#define PERIOD_H1      60    // 1 час
#define PERIOD_H4      240   // 4 часа
#define PERIOD_D1      1440  // 1 день
#define PERIOD_W1      10080 // 1 недел€
#define PERIOD_MN1     43200 // 1 мес€ц

#define PRICE_CLOSE		1
#define PRICE_OPEN		2
#define PRICE_HIGH		3
#define PRICE_LOW		4
#define PRICE_MEDIAN	5
#define PRICE_TYPICAL	6
#define PRICE_WEIGHTED	7

#define NUM_ORDERS   100
#define MAX_BARS     100
#define PI           3.14159265358979323846

#define QUOTEME(s)       #s
#define INCLUDE_FILE(f)  QUOTEME(f)

//Single
//#define STRAT_PATH  fxc/strat/Single/SingleStrategy.cpp
//#define STRAT_CLASS fxc::strategy::SingleStrategy
//Triplex
//#define STRAT_PATH  fxc/strat/Triplex/TriplexStrategy.cpp
//#define STRAT_CLASS fxc::strategy::TriplexStrategy
//Single test
#define STRAT_PATH  fxc/strat/Single/SingleStrategy.cpp
#define STRAT_CLASS fxc::strategy::SingleStrategy
