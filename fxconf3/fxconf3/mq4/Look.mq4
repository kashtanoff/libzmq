class Look
{
public:
	int x, y;
	int dx, dy;

	void Look(int font_size, color text_color, int pos_x, int pos_y, string font_name = "Consolas")
	{
		fontSize  = font_size;
		fontName  = font_name;
		textColor = text_color;
		x         = pos_x;
		y         = pos_y;
		SetSymbolSize();
		Init();
	}

	void Init()
	{
		rows = 0;
		cols = 0;
		dx   = cols*letterW + 10;
		dy   = rows*letterH + 10;
		MakeRect(x, y, dx, dy, clrDarkBlue, clrBlue, "textbox");
	}

	void Set(string name, string label)
	{
		if (GetIndex(name) > -1) {
			Change(name, label);
			return;
		}

		names[rows]  = name;
		labels[rows] = label;
		MakeLabel(
			name, 
			x + 5, 
			y + 5 + rows*letterH, 
			label, 
			fontSize, 
			textColor, 
			fontName
		);
		rows++;
		ResizeBox(label);
	}

	void Set(string name, string label, int value)
	{
		if (GetIndex(name) > -1) {
			Change(name, value);
			return;
		}

		names[rows]  = name;
		labels[rows] = label;
		digits[rows] = 0;
		MakeLabel(
			name, 
			x + 5, 
			y + 5 + rows*letterH, 
			label + sdts(value, 0), 
			fontSize, 
			textColor, 
			fontName
		);
		rows++;
		ResizeBox(label);
	}

	void Set(string name, string label, double value, int _digits = 2)
	{
		if (GetIndex(name) > -1) {
			Change(name, value);
			return;
		}

		names[rows]  = name;
		labels[rows] = label;
		digits[rows] = _digits;
		MakeLabel(
			name, 
			x + 5, 
			y + 5 + rows*letterH, 
			label + sdts(value, _digits), 
			fontSize, 
			textColor, 
			fontName
		);
		rows++;
		ResizeBox(label);
	}

	void Change(string name, string label)
	{
		ChangeLabel(name, label);
		ResizeBox(label);
	}

	void Change(string name, int value)
	{
		int index = GetIndex(name);
		if (index<0)
			return;

		string s = labels[index] + sdts(value, 0);
		ChangeLabel(name, s);
		ResizeBox(s);
	}

	void Change(string name, double value)
	{
		int index = GetIndex(name);
		if (index<0)
			return;

		string s = labels[index] + sdts(value, digits[index]);
		ChangeLabel(name, s);
		ResizeBox(s);
	}

	void DrawHistory()
	{
		int ord_total     = OrdersHistoryTotal();
		datetime cur_date = TimeCurrent();
		int seconds       = 86400 * DrawHistoryDays;

		datetime open_date;
		datetime close_date;
		double open_price, close_price;
		int ticket;
		int _type;

		for (int i = ord_total; i >= 0; i--)
		{
			if (!OrderSelect(i, SELECT_BY_POS, MODE_HISTORY))
				continue;

			close_date = OrderCloseTime();
			_type      = OrderType();

			if (cur_date - close_date <= seconds && _type <= OP_SELL && OrderSymbol() == symbolName)
			{
				open_date   = OrderOpenTime();
				open_price  = OrderOpenPrice();
				close_price = OrderClosePrice();
				ticket      = OrderTicket();
				DrawOrder(ticket, _type, open_date, open_price, close_date, close_price);
			}
		}
	}
	void DrawOrder(int _ticket, int _type, datetime _open_date, double _open_price, datetime _close_date, double _close_price)
	{
		if (_close_price == 0)
			return;

		string open_name  = DoubleToStr(_ticket, 0) + " o";
		string close_name = DoubleToStr(_ticket, 0) + " c";
		string line_name  = DoubleToStr(_ticket, 0) + " l";

		// создаём открывающую стрелку
		ObjectCreate(open_name, OBJ_ARROW, 0, _open_date, _open_price); // открывающая стрелка Бай-ордера
		ObjectSet(open_name, OBJPROP_ARROWCODE, 1);                     // код стрелки 232 
		ObjectSet(open_name, OBJPROP_COLOR, colors[_type]);             // цвет стрелки
		
		// создаём закрывающую стрелку
		ObjectCreate(close_name, OBJ_ARROW, 0, _close_date, _close_price); // закрывающая стрелка Бай-ордера
		ObjectSet(close_name, OBJPROP_ARROWCODE, 3);                       // код стрелки 231
		ObjectSet(close_name, OBJPROP_COLOR, colors[_type]);               // цвет стрелки
		
		// создаём линии
		ObjectCreate(line_name, OBJ_TREND, 0, _open_date, _open_price, _close_date, _close_price);
		ObjectSet(line_name, OBJPROP_RAY, false);           // запрещаем рисовать луч
		ObjectSet(line_name, OBJPROP_WIDTH, 0);             // устанавливаем толщину линии
		ObjectSet(line_name, OBJPROP_STYLE, 0);             // устанавливаем тип линии (отрезки)
		ObjectSet(line_name, OBJPROP_STYLE, STYLE_DOT);     // устанавливаем тип штриховки (пунктирная линия)
		ObjectSet(line_name, OBJPROP_COLOR, colors[_type]); // устанавливаем цвет(синий/красный)
	}
   void DelOldDraws()
	{
		string _name;
		int _type;
	
		for (int i = 0; i<ObjectsTotal(); i++)
		{
			_name = ObjectName(i);
			_type = ObjectType(_name);
			
			if (_type != OBJ_ARROW && _type != OBJ_TREND)
				continue;
			
			if (ObjectGetInteger(0, _name, OBJPROP_TIME) < (TimeCurrent() - 86400 * DrawHistoryDays))
				ObjectDelete(_name);
		}
	}

private:
	int    cols, rows;
	int    letterW, letterH;
	string names[20];
	string labels[20];
	int    digits[16];
	int    spaces;
	int    fontSize;
	string fontName;
	color  textColor;

	void SetSymbolSize()
	{
		kdpi = c_getdpi() / 72.0;
		TextSetFont(fontName, fontSize);
		TextGetSize("-", letterW, letterH);
		letterH = (int) round(letterH * kdpi);
		letterW = (int) round(letterW * kdpi);
	}

	string sdts(double value, int d)
	{
		int len = StringLen(DoubleToStr(value, 0));
		spaces = fmax(spaces, len + 1);
		string prefix;
		StringInit(prefix, spaces - len, ' ');
		return (prefix + DoubleToStr(value, d));
	}

	int GetIndex(string name)
	{
		for (int i = 0; i<rows; i++)
			if (names[i] == name)
				return (i);

		return (-1);
	}

	void MakeRect(int px, int py, int w, int h, color fill, color border, string name = "box")
	{
		ObjectCreate(name, OBJ_RECTANGLE_LABEL, 0, 0, 0);
		ObjectSetInteger(0, name, OBJPROP_XDISTANCE, px);
		ObjectSetInteger(0, name, OBJPROP_YDISTANCE, py);
		ObjectSetInteger(0, name, OBJPROP_XSIZE, w);
		ObjectSetInteger(0, name, OBJPROP_YSIZE, h);
		ObjectSetInteger(0, name, OBJPROP_BGCOLOR, fill);
		ObjectSetInteger(0, name, OBJPROP_BORDER_TYPE, BORDER_FLAT);
		ObjectSetInteger(0, name, OBJPROP_CORNER, CORNER_LEFT_UPPER);
		ObjectSetInteger(0, name, OBJPROP_COLOR, border);
		ObjectSetInteger(0, name, OBJPROP_STYLE, STYLE_SOLID);
		ObjectSetInteger(0, name, OBJPROP_WIDTH, 1);
		ObjectSetInteger(0, name, OBJPROP_SELECTABLE, false);
		ObjectSetInteger(0, name, OBJPROP_SELECTED, false);
		ObjectSetInteger(0, name, OBJPROP_HIDDEN, true);
		ObjectSet(name, OBJPROP_ZORDER, 1);
	}

	void MakeLabel(string name, int px, int py, string text, int font_size = 10, color text_color = clrRed, string font_name = "Consolas")
	{
		ObjectCreate(name, OBJ_LABEL, 0, 0, 0);
		ObjectSetInteger(0, name, OBJPROP_XDISTANCE, px);
		ObjectSetInteger(0, name, OBJPROP_YDISTANCE, py);
		ObjectSetInteger(0, name, OBJPROP_CORNER, CORNER_LEFT_UPPER);
		ObjectSetString(0, name, OBJPROP_TEXT, text);
		ObjectSetString(0, name, OBJPROP_FONT, font_name);
		ObjectSetInteger(0, name, OBJPROP_FONTSIZE, font_size);
		ObjectSetInteger(0, name, OBJPROP_ANCHOR, ANCHOR_LEFT_UPPER);
		ObjectSetInteger(0, name, OBJPROP_COLOR, text_color);
		ObjectSetInteger(0, name, OBJPROP_BACK, false);
		ObjectSetInteger(0, name, OBJPROP_SELECTABLE, false);
		ObjectSetInteger(0, name, OBJPROP_SELECTED, false);
		ObjectSetInteger(0, name, OBJPROP_HIDDEN, false);
		ObjectSetInteger(0, name, OBJPROP_XSIZE, 10);
		WindowRedraw();
	}

	void ChangeLabel(string name, string text)
	{
		ObjectSetString(0, name, OBJPROP_TEXT, text);
	}

	void ResizeBox(string text)
	{
		cols = fmax(cols, StringLen(text));
		dx   = cols*letterW + 10;
		dy   = rows*letterH + 10;
		ObjectSetInteger(0, "textbox", OBJPROP_XSIZE, dx);
		ObjectSetInteger(0, "textbox", OBJPROP_YSIZE, dy);
		WindowRedraw();
	}
};