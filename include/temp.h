#ifndef TIGER_TEMP_H_
#define TIGER_TEMP_H_
/*
 * temp.h 
 *
 */
#include <stdio.h>
#include "symbol.h"
#include "table.h"

// Table type mapping labels to information
typedef struct TAB_table_ *TL_table;

typedef struct Temp_temp_ *Temp_temp;
Temp_temp Temp_newtemp(void);

typedef struct Temp_tempList_ *Temp_tempList;
struct Temp_tempList_ { Temp_temp head; Temp_tempList tail;};
Temp_tempList Temp_TempList(Temp_temp h, Temp_tempList t);
Temp_tempList Temp_TempList_join(Temp_tempList first, Temp_tempList second);
Temp_tempList TL(Temp_temp t, Temp_tempList l);

typedef S_symbol Temp_label;
Temp_label Temp_newlabel(void);
Temp_label Temp_namedlabel(string name);
string Temp_labelstring(Temp_label s);

TL_table TL_empty(void);
void TL_enter(TL_table t, Temp_label label, void *v);
void *TL_look(TL_table t, Temp_label label); 

typedef struct Temp_labelList_ *Temp_labelList;
struct Temp_labelList_ { Temp_label head; Temp_labelList tail;};
Temp_labelList Temp_LabelList(Temp_label h, Temp_labelList t);

typedef struct Temp_map_ *Temp_map;
Temp_map Temp_empty(void);
Temp_map Temp_layerMap(Temp_map over, Temp_map under);
void Temp_enter(Temp_map m, Temp_temp t, string s);
string Temp_look(Temp_map m, Temp_temp t);
void Temp_dumpMap(FILE *out, Temp_map m);

Temp_map Temp_name(void);

#endif /* TIGER_TEMP_H_ */
