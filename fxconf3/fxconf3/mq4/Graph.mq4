
void ShowControlPanel(int x, int y)
{
	Edit(DoubleToStr(BaseSellLot, 2), "EdtSellLot", x, y, 80, 32, clrLightPink, true);
	lotnames[OP_SELL] = "EdtSellLot";

	Edit(DoubleToStr(BaseBuyLot, 2), "EdtBuyLot", x + 85, y, 80, 32, clrLightBlue, true);
	lotnames[OP_BUY] = "EdtBuyLot";

	Button("<", "BtnMinus", x, y + 37, 35, 32, clrGray);
	Edit(DoubleToStr(fmin(BaseBuyLot, BaseSellLot), 2), "EdtLot", x + 40, y + 37, 85, 32, clrLightGray);
	Button(">", "BtnPlus", x + 130, y + 37, 35, 32, clrGray);
	
	Button("Sell", "BtnSell", x, y + 74, 80, 32, clrRed);
	Button("Buy", "BtnBuy", x + 85, y + 74, 80, 32, clrBlue);
	
	Button("Close", "BtnClsS", x, y + 111, 80, 32, clrRed);
	Button("Close", "BtnClsB", x + 85, y + 111, 80, 32, clrBlue);
	
	show_cpanel = true;
}

void DelControlPanel()
{
	ObjectDelete("EdtSellLot");
	ObjectDelete("EdtBuyLot");
	ObjectDelete("EdtLot");
	ObjectDelete("BtnMinus");
	ObjectDelete("BtnPlus");
	ObjectDelete("BtnSell");
	ObjectDelete("BtnBuy");
	ObjectDelete("BtnClsS");
	ObjectDelete("BtnClsB");
	show_cpanel = false;
}

void Edit(string text, string name, int x, int y, int width, int height, color col, bool read_only = false)
{
	ObjectCreate(0, name, OBJ_EDIT, 0, 0, 0);
	ObjectSetInteger(0, name, OBJPROP_XDISTANCE, x);
	ObjectSetInteger(0, name, OBJPROP_YDISTANCE, y);
	ObjectSetInteger(0, name, OBJPROP_XSIZE, width);
	ObjectSetInteger(0, name, OBJPROP_YSIZE, height);
	ObjectSetInteger(0, name, OBJPROP_ALIGN, ALIGN_CENTER);
	ObjectSetInteger(0, name, OBJPROP_CORNER, CORNER_LEFT_UPPER);
	ObjectSetString(0, name, OBJPROP_TEXT, text);
	ObjectSetInteger(0, name, OBJPROP_READONLY, read_only);
	ObjectSetString(0, name, OBJPROP_FONT, "Lucida Console");
	ObjectSetInteger(0, name, OBJPROP_FONTSIZE, 10);
	ObjectSetInteger(0, name, OBJPROP_COLOR, clrBlack);
	ObjectSetInteger(0, name, OBJPROP_BGCOLOR, col);
	ObjectSetInteger(0, name, OBJPROP_BORDER_COLOR, clrWhite);
	ObjectSetInteger(0, name, OBJPROP_BACK, false);
	ObjectSetInteger(0, name, OBJPROP_SELECTABLE, false);
	ObjectSetInteger(0, name, OBJPROP_SELECTED, false);
	ObjectSetInteger(0, name, OBJPROP_HIDDEN, false);
	ObjectSetInteger(0, name, OBJPROP_ZORDER, 1);
}
void Button(string caption, string name, int x, int y, int width, int height, color col)
{
	ObjectCreate(0, name, OBJ_BUTTON, 0, 0, 0);
	ObjectSetInteger(0, name, OBJPROP_XDISTANCE, x);
	ObjectSetInteger(0, name, OBJPROP_YDISTANCE, y);
	ObjectSetInteger(0, name, OBJPROP_XSIZE, width);
	ObjectSetInteger(0, name, OBJPROP_YSIZE, height);
	ObjectSetInteger(0, name, OBJPROP_CORNER, CORNER_LEFT_UPPER);
	ObjectSetString(0, name, OBJPROP_TEXT, caption);
	ObjectSetString(0, name, OBJPROP_FONT, "Lucida Console");
	ObjectSetInteger(0, name, OBJPROP_FONTSIZE, 10);
	ObjectSetInteger(0, name, OBJPROP_COLOR, clrWhite); //цвет текста
	ObjectSetInteger(0, name, OBJPROP_BGCOLOR, col);
	ObjectSetInteger(0, name, OBJPROP_BORDER_COLOR, clrWhite);
	ObjectSetInteger(0, name, OBJPROP_BACK, false);
	ObjectSetInteger(0, name, OBJPROP_SELECTABLE, false);
	ObjectSetInteger(0, name, OBJPROP_SELECTED, false);
	ObjectSetInteger(0, name, OBJPROP_HIDDEN, false);
	ObjectSetInteger(0, name, OBJPROP_ZORDER, 1);
}