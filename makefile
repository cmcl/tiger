IDIR =./include
SDIR =./src
TESTDIR=./test
ODIR =./obj
BDIR =./bin

COMPILER=gcc
CFLAGS=-I$(IDIR) -Wall -g -c -o
OPTIONS=-o

LEXER_OBJECTS= driver.o lex.yy.o errormsg.o util.o
PARSER_OBJECTS= parsertest.o y.tab.o lex.yy.o errormsg.o util.o symbol.o absyn.o table.o
PARSE_OBJECTS= parse.o y.tab.o lex.yy.o errormsg.o util.o symbol.o absyn.o table.o prabsyn.o
SEM_OBJECTS = semantest.o y.tab.o lex.yy.o errormsg.o util.o symbol.o absyn.o \
	table.o prabsyn.o env.o semant.o types.o temp.o translate.o frame.o tree.o assem.o
STR_OBJECTS = stringtest.o util.o
TREE_OBJECTS = treetest.o y.tab.o lex.yy.o errormsg.o util.o symbol.o absyn.o \
	table.o prabsyn.o env.o semant.o types.o temp.o translate.o frame.o tree.o \
	assem.o canon.o printtree.o parse.o escape.o
	
LEXER_OBJS = $(patsubst %,$(ODIR)/%,$(LEXER_OBJECTS))
PARSER_OBJS = $(patsubst %,$(ODIR)/%,$(PARSER_OBJECTS))
PARSE_OBJS = $(patsubst %,$(ODIR)/%,$(PARSE_OBJECTS))
SEM_OBJS = $(patsubst %,$(ODIR)/%,$(SEM_OBJECTS))
STR_OBJS  = $(patsubst %, $(ODIR)/%, $(STR_OBJECTS))
TREE_OBJS = $(patsubst %, $(ODIR)/%, $(TREE_OBJECTS))

LEXER_PROG_NAME=$(BDIR)/lextest
PARSER_PROG_NAME=$(BDIR)/parsertest
PARSE_PROG_NAME=$(BDIR)/parse
SEM_PROG_NAME=$(BDIR)/semantest
STR_PROG_NAME=$(BDIR)/stringtest
PROG_NAME=$(BDIR)/treetest

all: $(TREE_OBJS)
	$(COMPILER) $^ $(OPTIONS) $(PROG_NAME)
	
parse: $(PARSE_OBJS)
	$(COMPILER) $^ $(OPTIONS) $(PARSE_PROG_NAME)
	
stringtest: $(STR_OBJS)
	$(COMPILER) $^ $(OPTIONS) $(STR_PROG_NAME)
	
semantest: $(SEM_OBJS)
	$(COMPILER) $^ $(OPTIONS) $(SEM_PROG_NAME)

parsertest: $(PARSER_OBJS)
	$(COMPILER) $^ $(OPTIONS) $(PARSER_PROG_NAME)
	
lextest: $(LEXER_OBJS)
	$(COMPILER) $^ $(OPTIONS) $(LEXER_PROG_NAME)
	
$(ODIR)/%.o: $(SDIR)/%.c
	$(COMPILER) $(CFLAGS) $@ $<
	
$(ODIR)/%.o: $(TESTDIR)/%.c
	$(COMPILER) $(CFLAGS) $@ $<

$(ODIR)/frame.o: $(SDIR)/amd64frame.c
	$(COMPILER) $(CFLAGS) $@ $<

$(ODIR)/lex.yy.o: $(SDIR)/lex.yy.c
	$(COMPILER) $(CFLAGS) $@ $<

$(SDIR)/lex.yy.c: $(SDIR)/tiger.lex
	flex -o $(SDIR)/lex.yy.c $<

$(ODIR)/y.tab.o: $(SDIR)/y.tab.c
	$(COMPILER) $(CFLAGS) $@ $<

$(SDIR)/y.tab.c: $(SDIR)/tiger.grm
	bison -ydvo $@ $<


clean:
	rm -f $(ODIR)/*.o $(LEXER_PROG_NAME) $(PARSER_PROG_NAME) $(PARSE_PROG_NAME) \
		$(SEM_PROG_NAME) $(STR_PROG_NAME) $(PROG_NAME)
