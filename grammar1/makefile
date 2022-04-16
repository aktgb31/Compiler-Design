all: parser lexer generator execute clean

debug: parser lexer dgenerator execute clean

parser: parser.y
	yacc -d parser.y

lexer: lexer.l y.tab.h
	lex lexer.l
generator: y.tab.c lex.yy.c 
	gcc y.tab.c lex.yy.c -o generator.out

dgenerator: y.tab.c lex.yy.c 
	gcc -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover y.tab.c lex.yy.c -o generator.out

execute: generator
	./generator.out

clean:
	rm -f *.o *.out *.tab.c *.tab.h *.yy.c *.yy.h intermediateFile.xsm
