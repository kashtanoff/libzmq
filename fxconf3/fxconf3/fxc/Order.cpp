#pragma once

namespace fxc {

	//��������� ����������� �����, ��� �������� ���������� �������
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