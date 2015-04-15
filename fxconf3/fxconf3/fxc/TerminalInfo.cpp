#pragma once

#include "fxc.h"
#include "debug/Debug.h"
#include "../Property.h"
#include <string>
#include <array>

namespace fxc {
	class TerminalInfo : public CPropertyList 
	{
	public:
		std::string accountCompany;			//�������� ���������� ��������, � ������� ��������������� ������� ����
		std::string accountCurrency;		//������������ ������ �������� �����
		int			accountFreeMarginMode;	//�������� ��������� �������, ����������� ��� �������� ������� �� ������� �����
		int			accountLeverage;		//�������� ����� �������� �����
		std::string accountName;			//��� ������������ �������� �����
		std::string accountNumber;			//����� �����
		std::string accountServer;			//������������ ��������� �������
		int			accountStopoutLevel;	//�������� ������, �� �������� ������������ ��������� Stop Out
		int			accountStopoutMode;		//����� ������� ������ Stop Out
		int			accountTradeMode;		//��� ��������� ����� demo/contest/real
		int			accountLimitOrders;		//����������� ���������� ���������� ����������� ���������� ������� (0-����������� ���)
		std::string	symbolName;				//�������� �����������
		double		symbolPoint;			//������ ������ � ������ ���������
		int			symbolDigits;			//���������� ���� ����� ������� � ���� �����������. ��� �������� ����������� �������� � ���������������� ���������� Digits
		int			symbolStopLevel;		//���������� ���������� ������� ����-�����/����-������� � �������
		int			symbolLotSize;			//������ ��������� � ������� ������ �����������
		double		symbolTickValue;		//������ ������������ ��������� ���� ����������� � ������ ��������
		int			symbolTickSize;			//����������� ��� ��������� ���� ����������� � �������
		double		symbolSwapLong;			//������ ����� ��� ������� �� �������
		double		symbolSwapShort;		//������ ����� ��� ������� �� �������
		double		symbolMinLot;			//����������� ������ ����
		double		symbolLotStep;			//��� ��������� ������� ����
		double		symbolMaxLot;			//������������ ������ ����
		int			symbolSwapType;			//����� ���������� ������. 0 - � �������; 1 - � ������� ������ �����������; 2 - � ���������; 3 - � ������ ��������� �������
		int			symbolProfitCalcMode;	//������ ������� �������. 0 - Forex; 1 - CFD; 2 - Futures
		int			symbolMarginCalcMode;	//������ ������� ��������� �������. 0 - Forex; 1 - CFD; 2 - Futures; 3 - CFD �� �������
		double		symbolMarginInit;		//��������� ��������� ���������� ��� 1 ����
		double		symbolMarginMaintenance;//������ ��������� ������� ��� ��������� �������� ������� � ������� �� 1 ���
		double		symbolMarginHadged;		//�����, ��������� � ���������� ������� � ������� �� 1 ���
		double		symbolMarginRequired;	//������ ��������� �������, ����������� ��� �������� 1 ����
		int			symbolFreezeLevel;		//������� ��������� ������� � �������. ���� ���� ���������� ��������� � ��������, ������������ ������� ���������, �� ����� �� ����� ���� �������������, ������� ��� ������
		int			mqlTradeAllowed;		//���������� �� �������� ��� ������ ���������� ���������
		int			mqlSignalAllowed;		//���������� �� ������ � ��������� ������ ���������� ���������
		int			mqlDebug;				//������� ������ ���������� ��������� � ������ �������
		int			mqlProfiler;			//������� ������ ���������� ��������� � ������ �������������� ����
		int			mqlTester;				//������� ������ ���������� ��������� � �������
		int			mqlOptimization;		//������� ������ ���������� ��������� � �������� �����������
		int			mqlVisualMode;			//������� ������ ���������� ��������� � ���������� ������ ������������

		int			k_point = 1;				//����������� ��������� ������ � ����� �������
		double		deltaStopLevel;
		double		deltaFreezeLevel;
	
		TerminalInfo() {
			Register("accountCompany",			&accountCompany);			//�������� ���������� ��������, � ������� ��������������� ������� ����
			Register("accountCurrency",			&accountCurrency);			//������������ ������ �������� �����
			Register("accountFreeMarginMode",	&accountFreeMarginMode);	//�������� ��������� �������, ����������� ��� �������� ������� �� ������� �����
			Register("accountLeverage",			&accountLeverage);			//�������� ����� �������� �����
			Register("accountName",				&accountName);				//��� ������������ �������� �����
			Register("accountNumber",			&accountNumber);			//����� �����
			Register("accountServer",			&accountServer);			//������������ ��������� �������
			Register("accountStopoutLevel",		&accountStopoutLevel);		//�������� ������, �� �������� ������������ ��������� Stop Out
			Register("accountStopoutMode",		&accountStopoutMode);		//����� ������� ������ Stop Out
			Register("accountTradeMode",		&accountTradeMode);			//��� ��������� ����� demo/contest/real
			Register("accountLimitOrders",		&accountLimitOrders);		//����������� ���������� ���������� ����������� ���������� ������� (0-����������� ���)
			Register("symbolName",				&symbolName);				//�������� �����������
			Register("symbolPoint",				&symbolPoint);				//������ ������ � ������ ���������
			Register("symbolDigits",			&symbolDigits);				//���������� ���� ����� ������� � ���� �����������. ��� �������� ����������� �������� � ���������������� ���������� Digits
			Register("symbolStopLevel",			&symbolStopLevel);			//���������� ���������� ������� ����-�����/����-������� � �������
			Register("symbolLotSize",			&symbolLotSize);			//������ ��������� � ������� ������ �����������
			Register("symbolTickValue",			&symbolTickValue);			//������ ������������ ��������� ���� ����������� � ������ ��������
			Register("symbolTickSize",			&symbolTickSize);			//����������� ��� ��������� ���� ����������� � �������
			Register("symbolSwapLong",			&symbolSwapLong);			//������ ����� ��� ������� �� �������
			Register("symbolSwapShort",			&symbolSwapShort);			//������ ����� ��� ������� �� �������
			Register("symbolMinLot",			&symbolMinLot);				//����������� ������ ����
			Register("symbolLotStep",			&symbolLotStep);			//��� ��������� ������� ����
			Register("symbolMaxLot",			&symbolMaxLot);				//������������ ������ ����
			Register("symbolSwapType",			&symbolSwapType);			//����� ���������� ������. 0 - � �������; 1 - � ������� ������ �����������; 2 - � ���������; 3 - � ������ ��������� �������
			Register("symbolProfitCalcMode",	&symbolProfitCalcMode);		//������ ������� �������. 0 - Forex; 1 - CFD; 2 - Futures
			Register("symbolMarginCalcMode",	&symbolMarginCalcMode);		//������ ������� ��������� �������. 0 - Forex; 1 - CFD; 2 - Futures; 3 - CFD �� �������
			Register("symbolMarginInit",		&symbolMarginInit);			//��������� ��������� ���������� ��� 1 ����
			Register("symbolMarginMaintenance",	&symbolMarginMaintenance);	//������ ��������� ������� ��� ��������� �������� ������� � ������� �� 1 ���
			Register("symbolMarginHadged",		&symbolMarginHadged);		//�����, ��������� � ���������� ������� � ������� �� 1 ���
			Register("symbolMarginRequired",	&symbolMarginRequired);		//������ ��������� �������, ����������� ��� �������� 1 ����
			Register("symbolFreezeLevel",		&symbolFreezeLevel);		//������� ��������� ������� � �������. ���� ���� ���������� ��������� � ��������, ������������ ������� ���������, �� ����� �� ����� ���� �������������, ������� ��� ������
			Register("mqlTradeAllowed",			&mqlTradeAllowed);			//���������� �� �������� ��� ������ ���������� ���������
			Register("mqlSignalAllowed",		&mqlSignalAllowed);			//���������� �� ������ � ��������� ������ ���������� ���������
			Register("mqlDebug",				&mqlDebug);					//������� ������ ���������� ��������� � ������ �������
			Register("mqlProfiler",				&mqlProfiler);				//������� ������ ���������� ��������� � ������ �������������� ����
			Register("mqlTester",				&mqlTester);				//������� ������ ���������� ��������� � �������
			Register("mqlOptimization",			&mqlOptimization);			//������� ������ ���������� ��������� � �������� �����������
			Register("mqlVisualMode",			&mqlVisualMode);			//������� ������ ���������� ��������� � ���������� ������ ������������
		}

		void printRegisteredProps() {
			for (auto pair : PropertyList)
			{
				switch (pair.second.Type)
				{
				case PropBool:
					msg << pair.first << ": " << *(pair.second.Bool) << "\r\n" << msg_box;
					break;
				case PropInt:
					msg << pair.first << ": " << *(pair.second.Int) << "\r\n" << msg_box;
					break;
				case PropDouble:
					msg << pair.first << ": " << *(pair.second.Double) << "\r\n" << msg_box;
					break;
				case PropBoolPtr:
					if (pair.second.BoolPtr == nullptr) {
						msg << pair.first << ": " << "n/a [0x" << pair.second.BoolPtr << "]\r\n" << msg_box;
					}
					else {
						msg << pair.first << ": "
							<< **(pair.second.BoolPtr)
							<< " [0x" << *(pair.second.BoolPtr) << "]"
							<< "\r\n" << msg_box;
					}
					break;
				case PropIntPtr:
					if (pair.second.IntPtr == nullptr) {
						msg << pair.first << ": " << "n/a [0x" << pair.second.IntPtr << "]\r\n" << msg_box;
					}
					else {
						msg << pair.first << ": "
							<< **(pair.second.IntPtr)
							<< " [0x" << *(pair.second.IntPtr) << "]"
							<< "\r\n" << msg_box;
					}
					break;
				case PropDoublePtr:
					if (pair.second.DoublePtr == nullptr) {
						msg << pair.first << ": " << "n/a [0x" << pair.second.DoublePtr << "]\r\n" << msg_box;
					}
					else {
						msg << pair.first << ": "
							<< **(pair.second.DoublePtr)
							<< " [0x" << *(pair.second.DoublePtr) << "]"
							<< "\r\n" << msg_box;
					}
					break;
				}
			}
		}
		
		inline double pointRecalc(int value) {
			return value * k_point * symbolPoint;
		}
		
		inline void deltaCalc() {
			deltaStopLevel		= symbolStopLevel	* symbolPoint;
			deltaFreezeLevel	= symbolFreezeLevel	* symbolPoint;
		}
	};
}
