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
		std::string accountCompany;			//название брокерской компании, в которой зарегистрирован текущий счет
		std::string accountCurrency;		//наименование валюты текущего счета
		int			accountFreeMarginMode;	//значение свободных средств, разрешенных для открытия ордеров на текущем счете
		int			accountLeverage;		//значение плеча текущего счета
		std::string accountName;			//имя пользователя текущего счета
		std::string accountNumber;			//Номер счета
		std::string accountServer;			//наименование активного сервера
		int			accountStopoutLevel;	//значение уровня, по которому определяется состояние Stop Out
		int			accountStopoutMode;		//режим расчета уровня Stop Out
		int			accountTradeMode;		//Тип торгового счета demo/contest/real
		int			accountLimitOrders;		//Максимально допустимое количество действующих отложенных ордеров (0-ограничений нет)
		std::string	symbolName;				//Название инструмента
		double		symbolPoint;			//Размер пункта в валюте котировки
		int			symbolDigits;			//Количество цифр после запятой в цене инструмента. Для текущего инструмента хранится в предопределенной переменной Digits
		int			symbolStopLevel;		//Минимально допустимый уровень стоп-лосса/тейк-профита в пунктах
		int			symbolLotSize;			//Размер контракта в базовой валюте инструмента
		double		symbolTickValue;		//Размер минимального изменения цены инструмента в валюте депозита
		int			symbolTickSize;			//Минимальный шаг изменения цены инструмента в пунктах
		double		symbolSwapLong;			//Размер свопа для ордеров на покупку
		double		symbolSwapShort;		//Размер свопа для ордеров на продажу
		double		symbolMinLot;			//Минимальный размер лота
		double		symbolLotStep;			//Шаг изменения размера лота
		double		symbolMaxLot;			//Максимальный размер лота
		int			symbolSwapType;			//Метод вычисления свопов. 0 - в пунктах; 1 - в базовой валюте инструмента; 2 - в процентах; 3 - в валюте залоговых средств
		int			symbolProfitCalcMode;	//Способ расчета прибыли. 0 - Forex; 1 - CFD; 2 - Futures
		int			symbolMarginCalcMode;	//Способ расчета залоговых средств. 0 - Forex; 1 - CFD; 2 - Futures; 3 - CFD на индексы
		double		symbolMarginInit;		//Начальные залоговые требования для 1 лота
		double		symbolMarginMaintenance;//Размер залоговых средств для поддержки открытых ордеров в расчете на 1 лот
		double		symbolMarginHadged;		//Маржа, взимаемая с перекрытых ордеров в расчете на 1 лот
		double		symbolMarginRequired;	//Размер свободных средств, необходимых для открытия 1 лота
		int			symbolFreezeLevel;		//Уровень заморозки ордеров в пунктах. Если цена исполнения находится в пределах, определяемых уровнем заморозки, то ордер не может быть модифицирован, отменен или закрыт
		int			mqlTradeAllowed;		//Разрешение на торговлю для данной запущенной программы
		int			mqlSignalAllowed;		//Разрешение на работу с сигналами данной запущенной программы
		int			mqlDebug;				//Признак работы запущенной программы в режиме отладки
		int			mqlProfiler;			//Признак работы запущенной программы в режиме профилирования кода
		int			mqlTester;				//Признак работы запущенной программы в тестере
		int			mqlOptimization;		//Признак работы запущенной программы в процессе оптимизации
		int			mqlVisualMode;			//Признак работы запущенной программы в визуальном режиме тестирования

		int			k_point = 1;				//Коэффициент пересчета старых и новых пунктов
		double		deltaStopLevel;
		double		deltaFreezeLevel;
	
		TerminalInfo() {
			Register("accountCompany",			&accountCompany);			//название брокерской компании, в которой зарегистрирован текущий счет
			Register("accountCurrency",			&accountCurrency);			//наименование валюты текущего счета
			Register("accountFreeMarginMode",	&accountFreeMarginMode);	//значение свободных средств, разрешенных для открытия ордеров на текущем счете
			Register("accountLeverage",			&accountLeverage);			//значение плеча текущего счета
			Register("accountName",				&accountName);				//имя пользователя текущего счета
			Register("accountNumber",			&accountNumber);			//Номер счета
			Register("accountServer",			&accountServer);			//наименование активного сервера
			Register("accountStopoutLevel",		&accountStopoutLevel);		//значение уровня, по которому определяется состояние Stop Out
			Register("accountStopoutMode",		&accountStopoutMode);		//режим расчета уровня Stop Out
			Register("accountTradeMode",		&accountTradeMode);			//Тип торгового счета demo/contest/real
			Register("accountLimitOrders",		&accountLimitOrders);		//Максимально допустимое количество действующих отложенных ордеров (0-ограничений нет)
			Register("symbolName",				&symbolName);				//Название инструмента
			Register("symbolPoint",				&symbolPoint);				//Размер пункта в валюте котировки
			Register("symbolDigits",			&symbolDigits);				//Количество цифр после запятой в цене инструмента. Для текущего инструмента хранится в предопределенной переменной Digits
			Register("symbolStopLevel",			&symbolStopLevel);			//Минимально допустимый уровень стоп-лосса/тейк-профита в пунктах
			Register("symbolLotSize",			&symbolLotSize);			//Размер контракта в базовой валюте инструмента
			Register("symbolTickValue",			&symbolTickValue);			//Размер минимального изменения цены инструмента в валюте депозита
			Register("symbolTickSize",			&symbolTickSize);			//Минимальный шаг изменения цены инструмента в пунктах
			Register("symbolSwapLong",			&symbolSwapLong);			//Размер свопа для ордеров на покупку
			Register("symbolSwapShort",			&symbolSwapShort);			//Размер свопа для ордеров на продажу
			Register("symbolMinLot",			&symbolMinLot);				//Минимальный размер лота
			Register("symbolLotStep",			&symbolLotStep);			//Шаг изменения размера лота
			Register("symbolMaxLot",			&symbolMaxLot);				//Максимальный размер лота
			Register("symbolSwapType",			&symbolSwapType);			//Метод вычисления свопов. 0 - в пунктах; 1 - в базовой валюте инструмента; 2 - в процентах; 3 - в валюте залоговых средств
			Register("symbolProfitCalcMode",	&symbolProfitCalcMode);		//Способ расчета прибыли. 0 - Forex; 1 - CFD; 2 - Futures
			Register("symbolMarginCalcMode",	&symbolMarginCalcMode);		//Способ расчета залоговых средств. 0 - Forex; 1 - CFD; 2 - Futures; 3 - CFD на индексы
			Register("symbolMarginInit",		&symbolMarginInit);			//Начальные залоговые требования для 1 лота
			Register("symbolMarginMaintenance",	&symbolMarginMaintenance);	//Размер залоговых средств для поддержки открытых ордеров в расчете на 1 лот
			Register("symbolMarginHadged",		&symbolMarginHadged);		//Маржа, взимаемая с перекрытых ордеров в расчете на 1 лот
			Register("symbolMarginRequired",	&symbolMarginRequired);		//Размер свободных средств, необходимых для открытия 1 лота
			Register("symbolFreezeLevel",		&symbolFreezeLevel);		//Уровень заморозки ордеров в пунктах. Если цена исполнения находится в пределах, определяемых уровнем заморозки, то ордер не может быть модифицирован, отменен или закрыт
			Register("mqlTradeAllowed",			&mqlTradeAllowed);			//Разрешение на торговлю для данной запущенной программы
			Register("mqlSignalAllowed",		&mqlSignalAllowed);			//Разрешение на работу с сигналами данной запущенной программы
			Register("mqlDebug",				&mqlDebug);					//Признак работы запущенной программы в режиме отладки
			Register("mqlProfiler",				&mqlProfiler);				//Признак работы запущенной программы в режиме профилирования кода
			Register("mqlTester",				&mqlTester);				//Признак работы запущенной программы в тестере
			Register("mqlOptimization",			&mqlOptimization);			//Признак работы запущенной программы в процессе оптимизации
			Register("mqlVisualMode",			&mqlVisualMode);			//Признак работы запущенной программы в визуальном режиме тестирования
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
