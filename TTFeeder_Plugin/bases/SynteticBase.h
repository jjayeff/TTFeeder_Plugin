//+------------------------------------------------------------------+
//|                                        UniFeeder Syntetic Quotes |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#pragma once

#define MAX_TICKS 128
//---- Tick structure
struct TickInfo
  {
   time_t            time;                // tick time
   char              security[16];        // symbol name
   double            bid,ask;             // prices
  };
//---- operand types
enum { OPERAND_SYMBOL, OPERAND_VALUE };
//---- operand description
struct SynteticOperand
  {
   int               type;                // operand type
   char              symbol[16];          // operand symbol
   double            value;               // operand value
   double            bid,ask;             // current symbol prices
  };
//----  symbol description
struct SynteticSymbol
  {
   char              name[16];            // symbol name
   SynteticOperand   left;                // left operand
   SynteticOperand   right;               // right operand
   char              operation;           // operation (+,-,*,/)
  };
//+------------------------------------------------------------------+
//| Symbols base					     |
//+------------------------------------------------------------------+
class CSynteticBase
  {
private:
   SynteticSymbol   *m_syntetics;         // symbols array
   int               m_syntetics_total;   // symbols total 
   int               m_syntetics_max;     // max array
   TickInfo          m_ticks[MAX_TICKS];  // ticks array
   int               m_ticks_total;       // ticks total
   int               m_ticks_current;     // current ticks position

public:
                     CSynteticBase();
                    ~CSynteticBase();

   void              Load(void);
   void              AddQuotes(const FeedData *data);
   int               GetTicks(FeedData *data);

private:
   void              AddSynteticSymbol(const SynteticSymbol *sym);
   void              RecalculateSymbol(const SynteticSymbol *cs);
   inline double     CalculateOperation(char op,double left,double right) const;
  };
//+------------------------------------------------------------------+
