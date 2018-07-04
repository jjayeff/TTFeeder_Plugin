//+------------------------------------------------------------------+
//|                                        UniFeeder Syntetic Quotes |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "stdafx.h"

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
CSynteticBase::CSynteticBase() :m_syntetics(NULL), m_syntetics_total(0), m_syntetics_max(0), m_ticks_total(0), m_ticks_current(0)
{
	memset(m_ticks, 0, sizeof(m_ticks));
	//----
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
CSynteticBase::~CSynteticBase()
{
	if (m_syntetics != NULL) { free(m_syntetics); m_syntetics = NULL; }
	m_syntetics_total = m_syntetics_max = 0;
	m_ticks_total = m_ticks_current = 0;
	//----
}
//+------------------------------------------------------------------+
//| Base load							     |
//+------------------------------------------------------------------+
void CSynteticBase::Load()
{
	FILE             *fp;
	SynteticSymbol    sym;
	char              tmp[2050], *cp, *cp2;
	int               i;
	//---- free old data
	if (m_syntetics != NULL) { free(m_syntetics); m_syntetics = NULL; }
	m_syntetics_total = m_syntetics_max = 0;
	m_ticks_total = m_ticks_current = 0;
	//---- open file
	_snprintf(tmp, sizeof(tmp) - 1, "%s.dat", ExtProgramPath);
	if ((fp = fopen(tmp, "rt")) == NULL) return;
	//---- read string by string
	while (fgets(tmp, sizeof(tmp) - 1, fp) != 0)
	{
		//----skip comments
		if (tmp[0] == 0 || tmp[0] == ';')   continue;
		if ((cp = strstr(tmp, ";")) != NULL)  *cp = 0;
		if ((cp = strstr(tmp, "\n")) != NULL) *cp = 0;
		//---- clear struct
		memset(&sym, 0, sizeof(sym));
		//---- find symbol
		if ((cp = strstr(tmp, "=")) == NULL) continue;
		*cp = 0;
		COPY_STR(sym.name, tmp);
		//---- search left operand
		cp++;
		cp2 = sym.left.symbol;
		sym.left.type = OPERAND_VALUE; // value operand by default
									   //---- analyze
		i = 0;
		while (*cp != 0 && strchr("+-*/", *cp) == NULL && i<sizeof(sym.left.symbol))
		{
			//---- search operand type
			if (strchr("01234567890.", *cp) == NULL) sym.left.type = OPERAND_SYMBOL;
			//---- copy symbol
			*cp2++ = *cp++;
			i++;
		}
		*cp2 = 0;
		//---- specify value for value operand
		if (sym.left.type == OPERAND_VALUE)
		{
			sym.left.value = atof(sym.left.symbol);
			sym.left.symbol[0] = 0;
		}
		//---- check operation
		if (*cp == 0 || strchr("+-*/", *cp) == NULL)   continue;
		sym.operation = *cp;
		//---- search right operand
		cp++;
		cp2 = sym.right.symbol;
		sym.right.type = OPERAND_VALUE;
		//---- analyze
		i = 0;
		while (*cp != 0 && strchr("+-*/", *cp) == NULL && i<sizeof(sym.right.symbol))
		{
			//---- search operand type
			if (strchr("01234567890.", *cp) == NULL) sym.right.type = OPERAND_SYMBOL;
			//---- copy symbol
			*cp2++ = *cp++;
			i++;
		}
		*cp2 = 0;
		//---- specify value for value operand
		if (sym.right.type == OPERAND_VALUE)
		{
			sym.right.value = atof(sym.right.symbol);
			sym.right.symbol[0] = 0;
		}
		//---- skipzeor right operand for divide (in order to prevent divide by zero)
		if (sym.operation == '/' && sym.right.type == OPERAND_VALUE && sym.right.value == 0) continue;
		//---- add record
		AddSynteticSymbol(&sym);
	}
	//---- close file
	fclose(fp);
}
//+------------------------------------------------------------------+
//| Add symbol						             |
//| Will override exist symbol					     |
//+------------------------------------------------------------------+
void CSynteticBase::AddSynteticSymbol(const SynteticSymbol *sym)
{
	SynteticSymbol   *buf;
	int               i;
	//---- check params
	if (sym == NULL) return;
	//---- check symbol existannce
	if (m_syntetics != NULL && m_syntetics_total>0)
	{
		for (i = 0;i<m_syntetics_total;i++)
			if (strcmp(sym->name, m_syntetics[i].name) == 0) break;
		//---- override
		if (i<m_syntetics_total)
		{
			memcpy(&m_syntetics[i], sym, sizeof(m_syntetics[i]));
			return;
		}
	}
	//---- check free memory in array
	if (m_syntetics_total + 1>m_syntetics_max)
	{
		//---- allocate
		if ((buf = (SynteticSymbol*)malloc((m_syntetics_total + 32) * sizeof(SynteticSymbol))) == NULL) return;
		//---- copy data
		if (m_syntetics != NULL && m_syntetics_total>0)
		{
			memcpy(buf, m_syntetics, m_syntetics_total * sizeof(SynteticSymbol));
			free(m_syntetics);
		}
		//---- replace array
		m_syntetics = buf;
		m_syntetics_max = m_syntetics_total + 32;
	}
	//---- add record
	memcpy(&m_syntetics[m_syntetics_total++], sym, sizeof(SynteticSymbol));
}
//+------------------------------------------------------------------+
//| Quotes adding                                                    |
//+------------------------------------------------------------------+
void CSynteticBase::AddQuotes(const FeedData *data)
{
	int               i, changed;
	SynteticSymbol   *cs;
	//---- check params
	if (data == NULL || m_syntetics == NULL) return;
	//---- look through quotes
	for (int j = 0;j<data->ticks_count;j++)
	{
		FeedTick *quote = (FeedTick*)&data->ticks[j];
		//---- loogin for symbol in operands
		for (i = 0;i<m_syntetics_total;i++)
		{
			cs = &m_syntetics[i];
			changed = FALSE;
			//---- check left operand
			if (cs->left.type == OPERAND_SYMBOL && strcmp(cs->left.symbol, quote->symbol) == 0)
			{
				//---- set new prices
				cs->left.ask = quote->ask;
				cs->left.bid = quote->bid;
				//---- set change flag
				changed = TRUE;
			}
			//---- check right operand
			if (cs->right.type == OPERAND_SYMBOL && strcmp(cs->right.symbol, quote->symbol) == 0)
			{
				//---- set new prices
				cs->right.ask = quote->ask;
				cs->right.bid = quote->bid;
				//---- set change flag
				changed = TRUE;
			}
			//---- need recalculation?
			if (changed != FALSE) RecalculateSymbol(cs);
		}
	}
	//----
}
//+------------------------------------------------------------------+
//| Synthetic symbol recalculation                                   |
//+------------------------------------------------------------------+
void CSynteticBase::RecalculateSymbol(const SynteticSymbol *cs)
{
	double   ask, bid, lvalue, rvalue;
	//---- check params
	if (cs == NULL) return;
	//---- calculate ask
	if (cs->left.type == OPERAND_SYMBOL)
		lvalue = cs->left.ask;
	else  lvalue = cs->left.value;
	if (cs->right.type == OPERAND_SYMBOL)
		rvalue = cs->operation == '/' ? cs->right.bid : cs->right.ask;
	else  rvalue = cs->right.value;
	//---- calculate
	if (lvalue == 0 || rvalue == 0) return;
	ask = CalculateOperation(cs->operation, lvalue, rvalue);
	//---- calculate bid
	if (cs->left.type == OPERAND_SYMBOL)
		lvalue = cs->left.bid;
	else  lvalue = cs->left.value;
	if (cs->right.type == OPERAND_SYMBOL)
		rvalue = cs->operation == '/' ? cs->right.ask : cs->right.bid;
	else  rvalue = cs->right.value;
	//---- calculate
	if (lvalue == 0 || rvalue == 0) return;
	bid = CalculateOperation(cs->operation, lvalue, rvalue);
	//---- check new quotes
	if (bid == 0 || ask == 0 || bid>ask) return;
	//---- add new quote
	if (m_ticks_total<MAX_TICKS)
	{
		COPY_STR(m_ticks[m_ticks_total].security, cs->name);
		m_ticks[m_ticks_total].ask = ask;
		m_ticks[m_ticks_total].bid = bid;
		m_ticks_total++;
		//---- call for more complex formulas
		FeedData data = { 0 };
		COPY_STR(data.ticks->symbol, cs->name);
		data.ticks->ask = ask;
		data.ticks->bid = bid;
		data.ticks_count = 1;
		AddQuotes(&data);
	}
	//----
}
//+------------------------------------------------------------------+
//| Next tick request                                                |
//+------------------------------------------------------------------+
int CSynteticBase::GetTicks(FeedData *data)
{
	//---- check params
	if (data == NULL) return(FALSE);
	data->ticks_count = 0;
	//---- any tick?
	if (m_ticks_total>0)
	{
		if (m_ticks_current<m_ticks_total)
		{
			for (data->ticks_count = 0;data->ticks_count<32;)
			{
				TickInfo   *ti = &m_ticks[m_ticks_current];
				FeedTick   *ft = &data->ticks[data->ticks_count];
				//---- copy daya
				COPY_STR(ft->symbol, ti->security);
				ft->bid = ti->bid;
				ft->ask = ti->ask;
				ft->ctm = ti->time;
				//---- raise count
				data->ticks_count++;
				m_ticks_current++;
				if (m_ticks_current >= m_ticks_total) break;
			}
			return(TRUE);
		}
		else m_ticks_total = 0;
	}
	//---- no ticks
	m_ticks_current = 0; m_ticks_total = 0;
	return(FALSE);
}
//+------------------------------------------------------------------+
//| Operation processing                                             |
//+------------------------------------------------------------------+
double CSynteticBase::CalculateOperation(char op, double left, double right) const
{
	double res = 0;
	//---- check operation
	switch (op)
	{
	case '+': res = left + right;              break;
	case '-': res = left - right;              break;
	case '*': res = left * right;              break;
	case '/': if (right != 0) res = left / right; break;
	}
	//---- result
	return(res);
}
//+------------------------------------------------------------------+
