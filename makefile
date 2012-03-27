IDIR =./include
SDIR =./src
TESTDIR=./test
ODIR =./obj
BDIR =./bin

COMPILER=gcc
CFLAGS=-I$(IDIR) -Wall -g -c -o
OPTIONS=-o

LEXER_OBJECTS= driver.o lex.yy.o errormsg.o util.o
PARSER_OBJECTS= parsetest.o y.tab.o lex.yy.o errormsg.o util.o
PARSER_OBJS = $(patsubst %,$(ODIR)/%,$(PARSER_OBJECTS))
LEXER_OBJS = $(patsubst %,$(ODIR)/%,$(LEXER_OBJECTS))

PROG_NAME=$(BDIR)/parsertest
LEXER_PROG_NAME=$(BDIR)/lextest

all: $(PARSER_OBJS)
	$(COMPILER) $^ $(OPTIONS) $(PROG_NAME)
	
lextest: $(LEXER_OBJS)
	$(COMPILER) $^ $(OPTIONS) $@
	
$(ODIR)/%.o: $(SDIR)/%.c
	$(COMPILER) $(CFLAGS) $@ $<
	
$(ODIR)/%.o: $(TESTDIR)/%.c
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
	rm -f $(ODIR)/*.o $(PROG_NAME) $(LEXER_PROG_NAME)
