#pragma once

#define DEBUG 1
#define CHECK_ACCESS   0 // Включена ли сетевая проверка пользователя
#define LOCAL          1 // Определяет сервер проверки пользователя, как локальный
#define PROFILE        0 // Профилирование
#define TRACE_STRATEGY 0 // Логирование стратегии в консоль

#define OP_BUY       0
#define OP_SELL      1
#define OP_BUYLIMIT  2
#define OP_SELLLIMIT 3
#define OP_BUYSTOP   4
#define OP_SELLSTOP  5

#define JOB_EXIT   0
#define JOB_CREATE 1
#define JOB_MODIFY 2
#define JOB_DELETE 3
#define JOB_CLOSE  4

#define NUM_ORDERS   100
#define MAX_BARS     100
#define PI           3.14159265358979323846

#define QUOTEME(s)       #s
#define INCLUDE_FILE(f)  QUOTEME(f)

#define STRAT_PATH  fxc/strat/DefaultStrategy.cpp
#define STRAT_CLASS fxc::strategy::DefaultStrategy