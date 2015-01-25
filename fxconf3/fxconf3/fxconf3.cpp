#pragma once
#ifndef FXCONF3_CPP
#define FXCONF3_CPP

// fxconf.cpp: определяет экспортированные функции для приложения DLL.

#include "Defines.h"
#include "fxc/fxc.h"
#include "fxc/Simbiot.cpp"

using namespace fxc;

Simbiot* pool[POOL_SIZE];

#pragma region Интерфейс с MQL

//Ищет свободную ячейку в пуле и создает новый экземпляр симбиота
_DLLAPI int __stdcall c_init(char* symbol)    
{
	mutex.lock();
	if (first_run)
	{
		memset(pool, 0, POOL_SIZE);
		first_run = false;
		console   = false;

		if (DEBUG == 1)
		{
			/*RedirectIOToConsole();
			console = true;
			msg << "Debug logging activated\r\n" << msg_box;*/
			
			if(AllocConsole())                     // Создаем консоль, у процесса не более одной.
            {
				//SetConsoleTitle("Debug Console");
                // Связываем буферы консоли с предопределенными файловыми описателями.
                freopen("conin$","r",stdin);
                freopen("CONOUT$","w",stdout);
                freopen("conout$","w",stderr);
				console = true;
				msg << "Debug logging activated\r\n" << msg_box;
            }
			else
				MessageBoxA(NULL, "console dont alloc", "fxconf3.dll ", MB_OK);
		}
	}

	for (int h=0; h<POOL_SIZE; h++)
	{
		if (pool[h] == nullptr)
		{
			//bp = true;
			//msg << "Pool position: " << h << msg_box;
			pool[h] = new Simbiot(symbol);
			mutex.unlock();
			return(h);
		}
	}

	bp = true;
	msg << "Pool position not found" << msg_box;
	mutex.unlock();
	return(-1);
}

//Освобождает память и индекс в пуле
_DLLAPI void __stdcall c_deinit(int h)   
{
	mutex.lock();
	delete pool[h];
	pool[h] = nullptr;
	for (int h=0; h<POOL_SIZE; h++)
	{
		if (pool[h] != nullptr)
		{
			mutex.unlock();
			return;
		}
	}
	//FreeConsole();
	mutex.unlock();
}

//Выполняет этап алгоритма, возвращает индекс необходимого действия
_DLLAPI int __stdcall c_getjob(int h)
{
	mutex.lock();
	curH = h;
	//curSymbol = pool[curH]->symbol;
	int res = pool[h]->getjob();
	mutex.unlock();
	return(res);
}

//Выдает текущее разрешение экрана (для правильного масштабирования графики)
_DLLAPI int __stdcall c_getdpi()
{
	__try
	{
		HDC hDC = ::GetDC(NULL); 
		int nDPI = ::GetDeviceCaps(hDC, LOGPIXELSX); 
		ReleaseDC(NULL, hDC);
		return(nDPI);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bp = true;
		msg << "c_getdpi ERROR: " << GetExceptionCode() << msg_box;
	};
	return(1);
}

//Передает значение константе
_DLLAPI void __stdcall c_setint(int h, int index, int value)
{
	mutex.lock();
	curH = h;
	__try
	{
		switch(index)
		{
			case 2:		pool[h]->digits = value;				//2 количество десятичных знаков для инструмента
			case 9:		pool[h]->is_optimization = value;		//9 флаг оптимизации
			case 10:	pool[h]->is_visual = value;				//10 флаг визуального режима
			case 11:	pool[h]->is_testing = value;			//11 флаг тестирования
			case 50:	pool[h]->_stop_new[0] = value;			//50, 51 Остановить открытие новой сетки
			case 51:	pool[h]->_stop_new[1] = value;			//50, 51 Остановить открытие новой сетки
			case 52:	pool[h]->_stop_avr[0] = value;			//52, 53 Остановить открытие новой ступени
			case 53:	pool[h]->_stop_avr[1] = value;			//52, 53 Остановить открытие новой ступени
			case 54:	pool[h]->_max_grid_lvl = value;			//54 Максимальный уровень сетки
			case 59:	pool[h]->_forward_lvl = value;			//59 с какого уровня выставлять форвардные сделки
			case 64:	pool[h]->_av_lvl = value;				//64 ступень усреднения
			case 66:	pool[h]->_op_av_lvl = value;			//66 уровень начала противоположного усреднения
			case 68:	pool[h]->_safe_copy = value;			//68 вести расчет прибыли с базового лота а не с начального
			case 73:	pool[h]->_trend_lvl = value;			//73
			case 76:	pool[h]->_repeat_lvl = value;			//76
			case 79:	pool[h]->_period = value;				//79
			case 82:	pool[h]->_attemts = value;				//82
			case 83:	pool[h]->_auto_mm = value;				//83
			case 84:	pool[h]->_mm_equ = value;				//84
			case 88:	pool[h]->_first_free = value; break;	//88
			case 89:	pool[h]->_new_bar = value; break;		//89
			case 90:	pool[h]->_free_lvl = value; break;		//90
			case 92:	pool[h]->_periodf2 = value; break;		//92
			case 93:	pool[h]->_periodf3 = value; break;		//93
			case 94:	pool[h]->_buf_len = value; break;		//94
			case 97:	pool[h]->_opp_close = value; break;     //97 OppositeCLose;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bp = true;
		msg << "c_setint ERROR: " << GetExceptionCode() << msg_box;
	};
	mutex.unlock();
}

_DLLAPI void __stdcall c_setdouble(int h, int index, double value)
{
	mutex.lock();
	curH = h;
	__try
	{
		switch(index)
		{
			case 1:		pool[h]->point = value; break;			    //1 значение минимального шага цены
			case 4:		pool[h]->lot_step = value; break;			//4 минимальный шаг приращения лота
			case 5:		pool[h]->lot_min = value; break;			//5 минимальный лот
			case 6:		pool[h]->lot_max = value; break;			//6 максимальный лот
			case 7:		pool[h]->min_sl_tp = value; break;			//7 минимальное расстояние до стоплосса или тейкпрофита
			case 8:		pool[h]->freeze = value; break;				//8 расстояние заморозки ордеров
			case 55:	pool[h]->_step = value; break;				//55 базовый шаг, минимальный шаг (для трейлинг степа)
			//case 56:	pool[h]->_step_mult = value; break;			//56 множитель шага
			case 57:	pool[h]->_takeprofit = value; break;		//57 базовый тейкпрофит, минимальный (для трейлинг стопа)
			//case 58:	pool[h]->_tp_mult = value; break;			//58 множитель тейкпрофита
			//case 60:	pool[h]->_tr_stop = value; break;			//60 величина трейлинг стопа
			//case 61:	pool[h]->_tr_stop_mult = value; break;		//61 множитель трейлинг стопа
			//case 62:	pool[h]->_tr_step = value; break;			//62 величина трейлинга шага
			//case 63:	pool[h]->_tr_step_mult = value; break;		//63 множитель трейлинга шага
			case 65:	pool[h]->_av_lot = value; break;			//65 лот с которого начинается уменьшаться ступень усреднения
			case 67:	pool[h]->_pips_mult = value; break;			//67 множитель прибыли
			case 69:	pool[h]->_sell_lot = value; break;			//69 начальный лот на продажу
			case 97:	pool[h]->_buy_lot = value; break;			//97 начальный лот на покупку
			case 70:	pool[h]->_maxlot = value; break;			//70 максимальный лот
			case 71:	pool[h]->_lot_hadge_mult = value; break;	//71 процент хэджирования
			case 72:	pool[h]->_regres_mult = value; break;		//72 процент затухания
			case 74:	pool[h]->_trend_lot_mult = value; break;	//74
			case 75:	pool[h]->_trend_progress = value; break;	//75
			case 77:	pool[h]->_repeat_lot_mult = value; break;	//77
			case 78:	pool[h]->_repeat_progress = value; break;	//78
			//case 79:	pool[h]->_period = value; break;			//79
			case 80:	pool[h]->_deviation = value; break;			//80
			case 81:	pool[h]->_stoploss = value; break;			//81 стоплосс
			case 85:	pool[h]->_basket_hadge_mult = value; break; //85 хэдж множитель корзины
			case 86:	pool[h]->_forward_step_mult = value; break; //86 множитель шага при форварде
			case 87:	pool[h]->_delta = value; break;		//87
			case 91:	pool[h]->_multf = value; break;		//91
			case 95:	pool[h]->_rollback = value; break;			//95
			case 96:	pool[h]->_weighthadge = value; break;		//96
			case 555:	pool[h]->PostInit(); break;
			default:	msg << "set double index not found: " << index << msg_box;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bp = true;
		msg << "c_setdouble ERROR: " << GetExceptionCode() << msg_box;
	};
	mutex.unlock();
}

//Устанавливает связь между переменными MQL и DLL
_DLLAPI void __stdcall c_setvar(int h, int index, void* pointer)
{
	mutex.lock();
	curH = h;
	__try
	{
		switch(index)
		{
			case 102:	pool[h]->open_dd		= (double*)pointer; break;
			case 103:	pool[h]->total_lots		= (double*)pointer; break;
			case 104:	pool[h]->max_lvl		= (int*)pointer; break;
			case 105:	pool[h]->max_dd			= (double*)pointer; break;
			case 106:	pool[h]->indicator		= (double*)pointer; break;
			case 107:   pool[h]->count_p        = (int*)pointer; break;
			case 110:	pool[h]->o_ticket		= (int*)pointer; break;
			case 111:	pool[h]->o_type			= (int*)pointer; break;
			case 112:	pool[h]->o_lots			= (double*)pointer; break;
			case 113:	pool[h]->o_openprice	= (double*)pointer; break;
			case 114:	pool[h]->o_slprice		= (double*)pointer; break;
			case 115:	pool[h]->o_tpprice		= (double*)pointer; break;
			case 116:	pool[h]->indicator2		= (double*)pointer; break;
			case 200:	pool[h]->intret			= (int*)pointer; break;
			//case 117:	pool[h]->prev_indicator = (double*)pointer; break;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bp = true;
		msg << "c_setvar ERROR: " << GetExceptionCode() << msg_box;
	};
	mutex.unlock();
}

//Инициализация цикла обновления ордеров, поскольку цикл получаеться рваным (каждая итерация вызывается из MQL), 
//нельзя пользоватья результатами в MQL программе до завершения цикла
_DLLAPI void __stdcall c_refresh_init(int h, double ask, double bid, double equity)   
{
	mutex.lock();
	curH = h;
	pool[h]->refresh_init(ask, bid, equity);
	mutex.unlock();
}

//Добавляет новый ордер в цикле скана ордеров, в будущем возвращает код изменения
_DLLAPI int __stdcall c_refresh_order(int h, int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit=0)
{
	mutex.lock();
	curH = h;
	int res = pool[h]->refresh_order(_ticket, _type, _lots, _openprice, _tp, _sl, _profit);
	mutex.unlock();
	return(res);
	
}

//Нормализация лота для ручных операций
_DLLAPI double __stdcall c_norm_lot(int h, double _lots)
{
	//mutex.lock();
	curH = h;
	double res = pool[h]->normlot(_lots);
	//mutex.unlock();
	return(res);
}

_DLLAPI int __stdcall c_get_closed(int h)
{
	//mutex.lock();
	curH = h;
	int res = pool[h]->getclosed();
	//mutex.unlock();
	return(res);
}

_DLLAPI void __stdcall c_refresh_prices(int h, double *_closes, double *_highs, double *_lows, int _bars)
{
	//mutex.lock();
	curH = h;
	pool[h]->refresh_prices(_closes, _highs, _lows, _bars);
	//mutex.unlock();
}

_DLLAPI void __stdcall fcostransform(double a[], int tnn, int inversefct)
{
    int j;
    int n2;
    double sum;
    double y1;
    double y2;
    double theta;
    double wi;
    double wpi;
    double wr;
    double wpr;
    double wtemp;
    double twr;
    double twi;
    double twpr;
    double twpi;
    double twtemp;
    double ttheta;
    int i;
    int i1;
    int i2;
    int i3;
    int i4;
    double c1;
    double c2;
    double h1r;
    double h1i;
    double h2r;
    double h2i;
    double wrs;
    double wis;
    int nn;
    int ii;
    int jj;
    int n;
    int mmax;
    int m;
    int istep;
    int isign;
    double tempr;
    double tempi;

    if( tnn==1 )
    {
        y1 = a[0];
        y2 = a[1];
        a[0] = 0.5*(y1+y2);
        a[1] = 0.5*(y1-y2);
        if( inversefct )
        {
            a[0] += a[0];
            a[1] += a[1];
        }
        return;
    }
    wi = 0;
    wr = 1;
    theta = PI/tnn;
    wtemp = sin(theta*0.5);
    wpr = -2.0*wtemp*wtemp;
    wpi = sin(theta);
    sum = 0.5*(a[0]-a[tnn]);
    a[0] = 0.5*(a[0]+a[tnn]);
    n2 = tnn+2;
    for(j = 2; j <= tnn/2; j++)
    {
        wtemp = wr;
        wr = wtemp*wpr-wi*wpi+wtemp;
        wi = wi*wpr+wtemp*wpi+wi;
        y1 = 0.5*(a[j-1]+a[n2-j-1]);
        y2 = a[j-1]-a[n2-j-1];
        a[j-1] = y1-wi*y2;
        a[n2-j-1] = y1+wi*y2;
        sum = sum+wr*y2;
    }
    ttheta = 2.0*PI/tnn;
    c1 = 0.5;
    c2 = -0.5;
    isign = 1;
    n = tnn;
    nn = tnn/2;
    j = 1;
    for(ii = 1; ii <= nn; ii++)
    {
        i = 2*ii-1;
        if( j>i )
        {
            tempr = a[j-1];
            tempi = a[j];
            a[j-1] = a[i-1];
            a[j] = a[i];
            a[i-1] = tempr;
            a[i] = tempi;
        }
        m = n/2;
        while(m>=2&&j>m)
        {
            j = j-m;
            m = m/2;
        }
        j = j+m;
    }
    mmax = 2;
    while(n>mmax)
    {
        istep = 2*mmax;
        theta = 2.0*PI/(isign*mmax);
        wpr = -2.0*pow(sin(0.5*theta),2);
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for(ii = 1; ii <= mmax/2; ii++)
        {
            m = 2*ii-1;
            for(jj = 0; jj <= (n-m)/istep; jj++)
            {
                i = m+jj*istep;
                j = i+mmax;
                tempr = wr*a[j-1]-wi*a[j];
                tempi = wr*a[j]+wi*a[j-1];
                a[j-1] = a[i-1]-tempr;
                a[j] = a[i]-tempi;
                a[i-1] = a[i-1]+tempr;
                a[i] = a[i]+tempi;
            }
            wtemp = wr;
            wr = wr*wpr-wi*wpi+wr;
            wi = wi*wpr+wtemp*wpi+wi;
        }
        mmax = istep;
    }
    twpr = -2.0*pow(sin(0.5*ttheta),2);
    twpi = sin(ttheta);
    twr = 1.0+twpr;
    twi = twpi;
    for(i = 2; i <= tnn/4+1; i++)
    {
        i1 = i+i-2;
        i2 = i1+1;
        i3 = tnn+1-i2;
        i4 = i3+1;
        wrs = twr;
        wis = twi;
        h1r = c1*(a[i1]+a[i3]);
        h1i = c1*(a[i2]-a[i4]);
        h2r = -c2*(a[i2]+a[i4]);
        h2i = c2*(a[i1]-a[i3]);
        a[i1] = h1r+wrs*h2r-wis*h2i;
        a[i2] = h1i+wrs*h2i+wis*h2r;
        a[i3] = h1r-wrs*h2r+wis*h2i;
        a[i4] = -h1i+wrs*h2i+wis*h2r;
        twtemp = twr;
        twr = twr*twpr-twi*twpi+twr;
        twi = twi*twpr+twtemp*twpi+twi;
    }
    h1r = a[0];
    a[0] = h1r+a[1];
    a[1] = h1r-a[1];
    a[tnn] = a[1];
    a[1] = sum;
    j = 4;
    while(j<=tnn)
    {
        sum = sum+a[j-1];
        a[j-1] = sum;
        j = j+2;
    }
    if( inversefct )
    {
        for(j = 0; j <= tnn; j++)
        {
            a[j] = a[j]*2/tnn;
        }
    }
    return;
}

#pragma endregion

#endif