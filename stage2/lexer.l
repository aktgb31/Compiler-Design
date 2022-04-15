%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "y.tab.h"
	#include "generator.h"

	int number;
    char var;
%}

%%

"BEGIN" {return PBEGIN;}
"END" {return PEND;}
"READ" {return READ;}
"WRITE" {return WRITE;}
[a-z] {var=*yytext;yylval.node=makeLeafVar(var);return ID;}
[0-9]+	{number = atoi(yytext); yylval.node = makeLeafNum(number); return NUM;}
"+"	{return PLUS;}
"-"	{return MINUS;}
"*"	{return MUL;}
"/"	{return DIV;}
"=" {return EQUALS;}
";"     {return *yytext;}
" "     {}
[ \t]	{}
[()]	{return *yytext;}
[\n]	{}
.	{yyerror("unknown character\n");exit(1);}

%%

int yywrap(void) {
	return 1;
}