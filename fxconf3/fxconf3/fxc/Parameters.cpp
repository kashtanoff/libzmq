#pragma once


#include "fxc.h"
#include "debug/Debug.h"
#include "../Property.h"
#include "TerminalInfo.cpp"
#include <vector>
#include <string>

namespace fxc {
	//ѕараметры советника
	class Parameters 
	{
		public:
			int			inputStopNew[2];		//ќстановить открытие новой сетки
			int			inputStop[2];		    //ќстановить торговлю дл€ типа
			double		inputBaseLot[2];
			int			inputFirstFree[2];

			int			inputStep;
			int			inputFirstTakeProfit;
			int			inputTakeProfit;
			int			inputStopLoss;
			int			inputMaxGridLevel;		// ћаксимальный уровень сетки
			double		inputMaxLot;			//70 максимальный лот
			double		inputPipsMultiplier;		//67 множитель прибыли
			int			inputAveragingLevel;
			int			inputAverageAll;
			int			inputCloseMode;
			int			inputFreeLvl;
			int			inputTimeFrame;
			int			inputPeriod1;
			double		inputDeviation;
			int			inputMinDev;
			int			inputRollBack;
			int			inputPeriod2;
			int			inputMagic;
			std::string	inputCommentText;
			int			inputSlippage;
			int			inputAutoMM;
			int			inputMMEquity;

			double		deltaStep;
			double		deltaFirstTP;
			double		deltaTP;
			double		deltaSL;
			double		deltaRollback;
			double		deltaMinDev;

			Parameters(CPropertyList* registrator) {
				registrator->Register("StopNewBuy",      &inputStopNew[0]);
				registrator->Register("StopBuy",         &inputStop[0]);
				registrator->Register("BaseBuyLot",      &inputBaseLot[0]);
				registrator->Register("FirstBuyFree",    &inputFirstFree[0]);
				registrator->Register("StopNewSell",     &inputStopNew[1]);
				registrator->Register("StopSell",        &inputStop[1]);
				registrator->Register("BaseSellLot",     &inputBaseLot[1]);
				registrator->Register("FirstSellFree",   &inputFirstFree[1]);

				registrator->Register("Step",            &inputStep);
				registrator->Register("FirstTakeProfit", &inputFirstTakeProfit);
				registrator->Register("TakeProfit",      &inputTakeProfit);
				registrator->Register("StopLoss",        &inputStopLoss);
				registrator->Register("MaxGridLevel",    &inputMaxGridLevel);
				registrator->Register("MaxLot",          &inputMaxLot);
				registrator->Register("PipsMultiplier",  &inputPipsMultiplier);
				registrator->Register("AveragingLevel",  &inputAveragingLevel);
				registrator->Register("AverageAll",      &inputAverageAll);
				registrator->Register("CloseMode",       &inputCloseMode);
				registrator->Register("FreeLvl",         &inputFreeLvl);
				registrator->Register("TimeFrame",       &inputTimeFrame);
				registrator->Register("Period1",         &inputPeriod1);
				registrator->Register("Deviation",       &inputDeviation);
				registrator->Register("MinDev",          &inputMinDev);
				registrator->Register("RollBack",        &inputRollBack);
				registrator->Register("Period2",         &inputPeriod2);
				registrator->Register("Magic",           &inputMagic);
				registrator->Register("AutoMM",          &inputAutoMM);
				registrator->Register("MMEquity",        &inputMMEquity);
			}

			void paramsDeltaCalc(double oldPoint) {
				deltaStep		= inputStep				* oldPoint;
				deltaFirstTP	= inputFirstTakeProfit	* oldPoint;
				deltaTP			= inputTakeProfit		* oldPoint;
				deltaSL			= inputStopLoss			* oldPoint;
				deltaRollback	= inputRollBack			* oldPoint;
				deltaMinDev		= inputMinDev			* oldPoint;
			}

	};

}