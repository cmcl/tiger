/*
 * Translate module for translating abstract syntax tree
 * to IR form.
 * Created by Craig McL on 4/5/2012
 */
#include "translate.h"
#include "frame.h"

struct Tr_level_ {
	Tr_level parent;
	Temp_label name;
	F_frame frame;
	Tr_accessList formals;
};
static struct Tr_level_ outer = {NULL, NULL, NULL, NULL};

struct Tr_access_ {
	Tr_level level;
	F_access access;
};

struct Tr_accessList_ {
	Tr_access head;
	Tr_accessList tail;
};

static Tr_accessList makeAccessList(Tr_level level, U_boolList formals);

static Tr_accessList makeAccessList(Tr_level level, U_boolList formals)
{
	U_boolList fmls;
	Tr_accessList headList = tailList = NULL;
	for (fmls = formals; fmls; fmls = fmls->tail) {
		Tr_access access = Tr_allocLocal(level, fmls->head);
		if (headList) {
			tailList->tail = Tr_AccessList(access, NULL);
			tailList = tailList->tail;
		} else {
			headList = Tr_AccessList(access, NULL);
			tailList = headList;
		}
	}
	return headList;
}


Tr_level Tr_outermost(void)
{
	return &outer;
}

/*
 * Adds an extra formal parameter onto the list passed to
 * the frame constructor. This parameter represents the static link
 */
Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals)
{
	Tr_level level = checked_malloc(sizeof(*level));
	level->parent = parent;
	level->name = name;
	level->frame = newFrame(label, U_BoolList(TRUE, formals));
	level->formals = makeAccessList(level, formals);
	return level;
}

Tr_access Tr_allocLocal(Tr_level level, bool escape)
{
	Tr_access local = checked_malloc(sizeof(*tra));
	local->level = level;
	local->access = F_allocLocal(level->frame, escape);
	return local;
}

Tr_accessList Tr_formals(Tr_level level)
{
	return level->formals;
}

Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail)
{
	Tr_accessList list = checked_malloc(sizeof(*list));
	list->head = head;
	list->tail = tail;
	return list;
}
