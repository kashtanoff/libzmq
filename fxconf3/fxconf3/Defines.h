#pragma once

#define DEBUG 1
#define CHECK_ACCESS   0 // �������� �� ������� �������� ������������
#define LOCAL          1 // ���������� ������ �������� ������������, ��� ���������
#define PROFILE        0 // ��������������
#define TRACE_STRATEGY 0 // ����������� ��������� � �������

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

#define PERIOD_M1      1     // 1 ������
#define PERIOD_M5      5     // 5 �����
#define PERIOD_M15     15    // 15 �����
#define PERIOD_M30     30    // 30 �����
#define PERIOD_H1      60    // 1 ���
#define PERIOD_H4      240   // 4 ����
#define PERIOD_D1      1440  // 1 ����
#define PERIOD_W1      10080 // 1 ������
#define PERIOD_MN1     43200 // 1 �����

#define NUM_ORDERS   100
#define MAX_BARS     100
#define PI           3.14159265358979323846

#define QUOTEME(s)       #s
#define INCLUDE_FILE(f)  QUOTEME(f)

#define STRAT_PATH  fxc/strat/DefaultStrategy.cpp
#define STRAT_CLASS fxc::strategy::DefaultStrategy