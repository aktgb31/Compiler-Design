%{
    #include<stdlib.h>
    #include<stdio.h>
    FILE *targetFile=NULL;
    #include "exprTree.h"
    #include "exprTree.c"
    
    
    int yylex(void);
    
%}

%union{
    struct tnode *node;
}

%type <node> expr NUM program END
%token NUM PLUS MINUS MUL DIV END
%left PLUS MINUS
%left MUL DIV

%%

program: expr END { 
    $$=$2; 
    evaluate($1,targetFile); 
    exit(0);
}
;

expr: expr PLUS expr {$$=makeOperatorNode('+',$1,$3);}
    | expr MINUS expr {$$=makeOperatorNode('-',$1,$3);}
    | expr MUL expr {$$=makeOperatorNode('*',$1,$3);}
    | expr DIV expr {$$=makeOperatorNode('/',$1,$3);}
    | '(' expr ')' {$$=$2;}
    | NUM {$$=$1;}
    ;

%%

int yyerror (char const *s){
    printf("yyerror %s",s);
}

int main(void){
    targetFile=fopen("B190343CS.xsm","w");
    initializeCodeGenerator();
    yyparse();
    fclose(targetFile);
    return 0;
}