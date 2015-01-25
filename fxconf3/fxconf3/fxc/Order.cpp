#pragma once

namespace fxc {

	//Структура описывающая ордер, для хранения менеджером ордеров
	struct Order
	{
		int		ticket;
		int		type;
		int		magic;
		double	lots;
		double	openprice;
		double	tpprice;
		double	slprice;
		double	closeprice;
		double	profit;
		bool	checked;
	};

}