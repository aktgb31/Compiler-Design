all: parser lexer generator execute clean

parser: parser.y
	yacc -d parser.y

lexer: lexer.l y.tab.h
	lex lexer.l

generator: y.tab.c lex.yy.c 
	gcc y.tab.c lex.yy.c -o generator.out

execute: generator
	./generator.out

clean:
	rm -f *.o *.out *.tab.c *.tab.h *.yy.c *.yy.h 
