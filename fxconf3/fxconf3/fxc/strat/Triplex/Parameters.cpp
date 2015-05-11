#pragma once


#include "../../fxc.h"
#include "../../debug/Debug.h"
#include "../../../Property.h"
#include "../../TerminalInfo.cpp"
#include <vector>
#include <string>

namespace fxc {
	//Параметры советника OC Triplex
	class Parameters 
	{
		public: 
			std::string inputSetName;
			int			inputStopNew[2];		//Остановить открытие новой сетки
			int			inputStop[2];		    //Остановить торговлю для типа
			double		inputBaseLot[2];
			int			inputFirstFree[2];

			int			inputStep[3];
			int			inputTakeProfit[3];
			int			inputMaxLevel[3];
			int			inputPeriod[3];
			double		inputDeviation[3];
			int			inputDevPeriod[3];
			int			inputTimeframe[3];

			int			inputStopLoss;
			double		inputMaxLot;			
			double		inputPipsMultiplier;	
			int			inputAveragingLevel;
			int			inputAverageAll;
			int			inputCloseMode;
			int			inputFreeLvl;
			int			inputMinDev;
			int			inputRollBack;
			int			inputMagic;
			std::string	inputCommentText;
			int			inputSlippage;
			int			inputAutoMM;
			int			inputMMEquity;
			int			inputTestMode;

			double		deltaStep[3];
			double		deltaTP[3];;
			double		deltaSL;
			double		deltaRollback;
			double		deltaMinDev;

			Parameters(CPropertyList* registrator) {
				registrator->Register("SetName",		 &inputSetName);
				registrator->Register("StopNewBuy",      &inputStopNew[0]);
				registrator->Register("StopBuy",         &inputStop[0]);
				registrator->Register("BaseBuyLot",      &inputBaseLot[0]);
				registrator->Register("FirstBuyFree",    &inputFirstFree[0]);
				registrator->Register("StopNewSell",     &inputStopNew[1]);
				registrator->Register("StopSell",        &inputStop[1]);
				registrator->Register("BaseSellLot",     &inputBaseLot[1]);
				registrator->Register("FirstSellFree",   &inputFirstFree[1]);

				registrator->Register("Step1",           &inputStep[0]);
				registrator->Register("TakeProfit1",     &inputTakeProfit[0]);
				registrator->Register("MaxLevel1",		 &inputMaxLevel[0]);
				registrator->Register("Period1",         &inputPeriod[0]);
				registrator->Register("Deviation1",      &inputDeviation[0]);
				registrator->Register("DevPeriod1",      &inputDevPeriod[0]);
				registrator->Register("Timeframe1",      &inputTimeframe[0]);

				registrator->Register("Step2",           &inputStep[1]);
				registrator->Register("TakeProfit2",     &inputTakeProfit[1]);
				registrator->Register("MaxLevel2",		 &inputMaxLevel[1]);
				registrator->Register("Period2",         &inputPeriod[1]);
				registrator->Register("Deviation2",      &inputDeviation[1]);
				registrator->Register("DevPeriod2",      &inputDevPeriod[1]);
				registrator->Register("Timeframe2",      &inputTimeframe[1]);

				registrator->Register("Step3",           &inputStep[2]);
				registrator->Register("TakeProfit3",     &inputTakeProfit[2]);
				registrator->Register("MaxLevel3",		 &inputMaxLevel[2]);
				registrator->Register("Period3",         &inputPeriod[2]);
				registrator->Register("Deviation3",      &inputDeviation[2]);
				registrator->Register("DevPeriod3",      &inputDevPeriod[2]);
				registrator->Register("Timeframe3",      &inputTimeframe[2]);

				registrator->Register("StopLoss",        &inputStopLoss);
				registrator->Register("MaxLot",          &inputMaxLot);
				registrator->Register("PipsMultiplier",  &inputPipsMultiplier);
				registrator->Register("AveragingLevel",  &inputAveragingLevel);
				registrator->Register("AverageAll",      &inputAverageAll);
				registrator->Register("CloseMode",       &inputCloseMode);
				registrator->Register("FreeLvl",         &inputFreeLvl);
				registrator->Register("MinDev",          &inputMinDev);
				registrator->Register("RollBack",        &inputRollBack);
				registrator->Register("Magic",           &inputMagic);
				registrator->Register("Comment",		 &inputCommentText);
				registrator->Register("AutoMM",          &inputAutoMM);
				registrator->Register("MMEquity",        &inputMMEquity);
				registrator->Register("TestMode",		 &inputTestMode);
			}

			void paramsDeltaCalc(double oldPoint) {
				deltaStep[0]	= inputStep[0]			* oldPoint;
				deltaTP[0]		= inputTakeProfit[0]	* oldPoint;
				deltaStep[1]	= inputStep[1]			* oldPoint;
				deltaTP[1]		= inputTakeProfit[1]	* oldPoint;
				deltaStep[2]	= inputStep[2]			* oldPoint;
				deltaTP[2]		= inputTakeProfit[2]	* oldPoint;
				deltaSL			= inputStopLoss			* oldPoint;
				deltaRollback	= inputRollBack			* oldPoint;
				deltaMinDev		= inputMinDev			* oldPoint;

			}

	};

}