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
		kdpi      = c_getdpi() / 72.0;
		SetSymbolSize();
	}

	void Init()
	{
		rows = 0;
		cols = 0;
		dx   = 0;
		dy   = 0;
		padX = 5;
		padY = 5;
		headerFontSize = 12;
		MakeRect(x, y, dx, dy, clrDarkBlue, clrBlue, "textbox");

		string name = "logo";
		if (ObjectCreate(0, name, OBJ_BITMAP_LABEL, 0, 0, 0)) {
			ObjectSetInteger(0, name, OBJPROP_CORNER, CORNER_LEFT_UPPER);
			ObjectSetInteger(0, name, OBJPROP_XDISTANCE, x + padX + 2);
			ObjectSetInteger(0, name, OBJPROP_YDISTANCE, y + padY);
			ObjectSetString(0, name, OBJPROP_BMPFILE, 0, "::logo.bmp");
			logoW = ObjectGetInteger(0, name, OBJPROP_XSIZE);
			logoH = ObjectGetInteger(0, name, OBJPROP_YSIZE);
		}
		
		int ch1 = GetCharH(fontName, headerFontSize);
		int ch2 = ch1;
		int oy  = (logoH - ch1 - ch2) / 2;
		MakeLabel("header1", 
			x + padX + logoW,
			y + padY + oy,
			" Olsen&Cleverton", headerFontSize, textColor, fontName
		);
		MakeLabel("header2", 
			x + padX + logoW,
			y + padY + oy + ch1,
			"", headerFontSize, textColor, fontName
		);
		SetHeader("");

		SetSymbolSize();
		Set("line", "--------------------------");
	}

	void SetHeader(string label) {
		int length = fmax(StringLen(ObjectGetString(0, "header1", OBJPROP_TEXT)), StringLen(label));
		headerW    = logoW + length * GetCharW(fontName, headerFontSize);
		headerH    = logoH;

		ObjectSetString(0, "header2", OBJPROP_TEXT, label);
	}

	void Set(string name, string label) {
		if (GetIndex(name) > -1) {
			Change(name, label);
			return;
		}

		names[rows] = name;
		MakeLabel(name, 
			x + padX, 
			y + padY + headerH + rows * letterH, 
			label, 
			fontSize, textColor, fontName
		);
		rows++;

		cols = fmax(cols, StringLen(label));
		ResizeBox();
	}

	void Change(string name, string label) {
		ObjectSetString(0, name, OBJPROP_TEXT, label);
		
		cols = fmax(cols, StringLen(label));
		ResizeBox();
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
	int    fontSize;
	string fontName;
	color  textColor;

	int padX, padY;
	int logoW, logoH;
	int headerH, headerW;
	int headerFontSize;


	int GetIndex(string name) {
		for (int i = 0; i < rows; i++) {
			if (names[i] == name) {
				return (i);
			}
		}
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
		ObjectSetInteger(0, name, OBJPROP_XDISTANCE,  px);
		ObjectSetInteger(0, name, OBJPROP_YDISTANCE,  py);
		ObjectSetInteger(0, name, OBJPROP_CORNER,     CORNER_LEFT_UPPER);
		ObjectSetString (0, name, OBJPROP_TEXT,       text);
		ObjectSetString (0, name, OBJPROP_FONT,       font_name);
		ObjectSetInteger(0, name, OBJPROP_FONTSIZE,   font_size);
		ObjectSetInteger(0, name, OBJPROP_ANCHOR,     ANCHOR_LEFT_UPPER);
		ObjectSetInteger(0, name, OBJPROP_COLOR,      text_color);
		ObjectSetInteger(0, name, OBJPROP_BACK,       false);
		ObjectSetInteger(0, name, OBJPROP_SELECTABLE, false);
		ObjectSetInteger(0, name, OBJPROP_SELECTED,   false);
		ObjectSetInteger(0, name, OBJPROP_HIDDEN,     false);
		ObjectSetInteger(0, name, OBJPROP_XSIZE,      10);
		WindowRedraw();
	}

	void ResizeBox() {
		dx = padX * 2 + fmax(cols * letterW, headerW);
		dy = padY * 2 + rows * letterH + headerH;

		ObjectSetInteger(0, "textbox", OBJPROP_XSIZE, dx);
		ObjectSetInteger(0, "textbox", OBJPROP_YSIZE, dy);
		WindowRedraw();
	}

	void SetSymbolSize() {
		letterW = GetCharW(fontName, fontSize);
		letterH = GetCharH(fontName, fontSize);
		Print("-> ", fontSize, " char size: ", letterW, "x", letterH);
	}

	int GetCharW(string _fontName, int _fontSize) {
		int w, h;
		TextSetFont(_fontName, _fontSize);
		TextGetSize("-", w, h);
		Print("-> ", _fontSize, " char raw size: ", w, "x", h);
		return (int) round(w * kdpi);
	}

	int GetCharH(string _fontName, int _fontSize) {
		int w, h;
		TextSetFont(_fontName, _fontSize);
		TextGetSize("-", w, h);
		Print("-> ", _fontSize, " char raw size: ", w, "x", h);
		return (int) round(h * kdpi);
	}

};