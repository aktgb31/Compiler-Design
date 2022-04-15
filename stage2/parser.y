%{
    #include<stdlib.h>
    #include<stdio.h>
    #include "generator.h"
    #include "generator.c"
    
    
    int yylex(void);
    
%}

%union{
    struct tnode *node;
}

%type <node> expr NUM program ID Slist Stmt AssgStmt InputStmt OutputStmt
%token NUM PLUS MINUS MUL DIV EQUALS PBEGIN PEND ID READ WRITE
%left PLUS MINUS
%left MUL DIV

%%

program: PBEGIN Slist PEND { 
    $$=$2; 
    FILE *targetFile=fopen("B190343CS.xsm","w");
    initGenerator(targetFile);
    evaluate($2,targetFile); 
    codeExit(targetFile);
    fclose(targetFile);
    exit(0);
}
| PBEGIN PEND{ 
    FILE *targetFile=fopen("B190343CS.xsm","w");
    initGenerator(targetFile);
    codeExit(targetFile);
    fclose(targetFile);
    exit(0);}
;

Slist: Slist Stmt{$$=makeNonLeafNode('c',NTCONNECTOR,$1,$2);}
     | Stmt{$$=$1;}
     ;

Stmt : InputStmt | OutputStmt |AssgStmt;

InputStmt : READ '(' ID ')' ';' {$$=makeNonLeafNode('r',NTREAD,$3,NULL);}
         ;

OutputStmt : WRITE '(' expr ')' ';' {$$=makeNonLeafNode('w',NTWRITE,$3,NULL);}
          ;

AssgStmt : ID EQUALS expr ';' {$$=makeNonLeafNode('=',NTOPERATOR,$1,$3);}
         ;

expr: expr PLUS expr {$$=makeNonLeafNode('+',NTOPERATOR,$1,$3);}
    | expr MINUS expr {$$=makeNonLeafNode('-',NTOPERATOR,$1,$3);}
    | expr MUL expr {$$=makeNonLeafNode('*',NTOPERATOR,$1,$3);}
    | expr DIV expr {$$=makeNonLeafNode('/',NTOPERATOR,$1,$3);}
    | '(' expr ')' {$$=$2;}
    | NUM {$$=$1;}
    | ID {$$=$1;}
    ;

%%

int yyerror (char const *s){
    printf("yyerror %s",s);
}

int main(void){
    extern FILE *yyin;
    yyin=fopen("input.txt","r");
    yyparse();
    fclose(yyin);
    return 0;
}