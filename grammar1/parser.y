%{
    #include<stdlib.h>
    #include<stdio.h>
    #include<string.h>
    #include "generator.h"
    #include "generator.c"
    
    
    int yylex(void);
    
%}

%union{
    struct tnode *node;
}

%type <node> E program Stmts Stmt AsgStmt ReadStmt WriteStmt IfStmt IfElseStmt WhileStmt
%token NUM PLUS MINUS MUL EQUALS FUN VAR READ WRITE LE GE EQ NE DO WHILE IF ELSE 

%nonassoc EQUALS 
%nonassoc EQ NE
%nonassoc LE GE
%left PLUS MINUS
%left MUL

%%

program: FUN '(' ')' '{' Stmts '}'{ 
    FILE *intermediateFile=fopen("intermediateFile.xsm","w+");
    initGenerator(intermediateFile);
    evaluate($5,intermediateFile); 
    codeExit(intermediateFile);
    rewind(intermediateFile);

    FILE *targetFile=fopen("B190343CS.xsm","w");
    linker(intermediateFile,targetFile);
    fclose(intermediateFile);
    fclose(targetFile);

    exit(0);
}
;

Stmts: Stmts Stmt{$$=makeNonLeafNode('c',NTCONNECTOR,$1,$2);}
     | Stmt{$$=$1;}
     ;

Stmt : AsgStmt 
    | IfStmt 
    | IfElseStmt 
    | WhileStmt 
    | ReadStmt 
    | WriteStmt 
    ;

AsgStmt : VAR EQUALS E ';' {$$=makeNonLeafNode('=',NTOPERATOR,$<node>1,$3);}
         ;

IfStmt : IF '(' E ')' '{' Stmts '}' {$$=makeNonLeafNode('i',NTIF,$3,$6);}
        ;

IfElseStmt : IF '(' E ')' '{' Stmts '}' ELSE '{' Stmts '}' {
    struct tnode *Node=makeNonLeafNode('c',NTELSE,$6,$10);
    $$=makeNonLeafNode('i',NTIF,$3,Node);}
    ;

WhileStmt : DO '{' Stmts '}' WHILE '(' E ')' ';' {$$=makeNonLeafNode('w',NTWHILE,$3,$7);}
           ;

ReadStmt : READ '(' VAR ')' ';' {$$=makeNonLeafNode('r',NTREAD,$<node>3,NULL);}
         ;

WriteStmt : WRITE '(' E ')' ';' {$$=makeNonLeafNode('w',NTWRITE,$<node>3,NULL);}
          ;
        
E: E PLUS E {$$=makeNonLeafNode('+',NTOPERATOR,$1,$3);}
    | E MINUS E {$$=makeNonLeafNode('-',NTOPERATOR,$1,$3);}
    | E MUL E {$$=makeNonLeafNode('*',NTOPERATOR,$1,$3);}
    | E LE E {$$=makeNonLeafNode('<',NTOPERATOR,$1,$3);}
    | E GE E {$$=makeNonLeafNode('>',NTOPERATOR,$1,$3);}
    | E EQ E {$$=makeNonLeafNode('e',NTOPERATOR,$1,$3);}
    | E NE E {$$=makeNonLeafNode('n',NTOPERATOR,$1,$3);}
    | '(' E ')' {$$=$2;}
    | NUM {$$=$<node>1;}
    | VAR {$$=$<node>1;}
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