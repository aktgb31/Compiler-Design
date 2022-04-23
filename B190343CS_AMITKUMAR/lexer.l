%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "y.tab.h"
	#include "generator.h"

	int number;
    char *var;
%}

%%

"FUN" {return FUN;}
"read" {return READ;}
"write" {return WRITE;}
"return" {return RETURN;}
"if" {return IF;}
"else" {return ELSE;}
[A-Za-z][A-Za-z0-9]* {var=yytext;yylval.node=makeLeafVar(var);return VAR;}
[0-9]+	{number = atoi(yytext); yylval.node = makeLeafNum(number); return NUM;}
"+"	{return PLUS;}
"-"	{return MINUS;}
"*"	{return MUL;}
"=" {return EQUALS;}
"<=" {return LE;}
">=" {return GE;}
"==" {return EQ;}
"!=" {return NE;}
"<" {return LT;}
">" {return GT;}
";"   {return *yytext;}
"{"   {++localScope;return *yytext;}
"}"   {--localScope;return *yytext;}
" "     {}
[ \t]	{}
[()]	{return *yytext;}
[\n]	{}
.	{yyerror("unknown character\n");exit(1);}

%%

int yywrap(void) {
	return 1;
}