%{
    #include<stdlib.h>
    #include<stdio.h>
    #include<string.h>
    #include "generator.h"
    #include "generator.c"
    
    
    int yylex(void);
    int yyerror(char const*s);
%}

%union{
    struct tnode *node;
}

%type <node> E program Stmts Stmt AsgStmt ReadStmt WriteStmt IfStmt IfElseStmt RetStmt
%token NUM PLUS MINUS MUL EQUALS FUN VAR READ WRITE LE GE EQ NE IF ELSE RETURN LT GT

%right EQUALS 
%left EQ NE
%left LE GE LT GT
%left PLUS MINUS
%left MUL

%%

program: ReadStmt VAR EQUALS FUN '(' VAR ')' '{' Stmts '}' WriteStmt { 
    if(strcmp($<node>2->varName,"argc")!=0 && strcmp($<node>6->varName,"argc")!=0)
    {
        printf("Error: Invalid Syntax\n");
        exit(1);
    }
    struct tnode *funCall=makeNonLeafNode('f',NTFUNCALL,$<node>6,NULL);
    struct tnode *equal=makeNonLeafNode('=',NTOPERATOR,$<node>2,funCall);
    struct tnode *funDecl=makeNonLeafNode('f',NTFUNDECL,$9,NULL);
    
    struct tnode * connector1=makeNonLeafNode('c',NTCONNECTOR,equal,$11);
    struct tnode * connector2=makeNonLeafNode('c',NTCONNECTOR,funDecl,connector1);
    $$=makeNonLeafNode('c',NTCONNECTOR,$1,connector2);


    FILE *intermediateFile=fopen("intermediateFile.xsm","w+");
    initGenerator(intermediateFile);
    evaluate($$,intermediateFile); 
    codeExit(intermediateFile);
    rewind(intermediateFile);

    FILE *targetFile=fopen("B190343CS.xsm","w");
    linker(intermediateFile,targetFile);
    fclose(intermediateFile);
    fclose(targetFile);
}
;

Stmts: Stmts Stmt{$$=makeNonLeafNode('c',NTCONNECTOR,$1,$2);}
     | Stmt{$$=$1;}
     ;

Stmt : AsgStmt 
    | IfStmt 
    | IfElseStmt 
    | WriteStmt
    | RetStmt
    ;

AsgStmt : VAR EQUALS E ';' {$$=makeNonLeafNode('=',NTOPERATOR,$<node>1,$3);}
         ;

IfStmt : IF '(' E ')' '{' Stmts '}' {$$=makeNonLeafNode('i',NTIF,$3,$6);}
        ;

IfElseStmt : IF '(' E ')' '{' Stmts '}' ELSE '{' Stmts '}' {
    struct tnode *Node=makeNonLeafNode('c',NTELSE,$6,$10);
    $$=makeNonLeafNode('i',NTIF,$3,Node);}
    ;

RetStmt : RETURN E ';' {$$=makeNonLeafNode('r',NTRETURN,$2,NULL);}
        ;

ReadStmt : READ '(' VAR ')' ';' {
    if(strcmp($<node>3->varName,"argc")!=0)
    {
        printf("Error: Invalid Syntax\n");
        exit(1);
    }
    $$=makeNonLeafNode('r',NTREAD,$<node>3,NULL);
    }
         ;

WriteStmt : WRITE '(' VAR ')' ';' {
    if(strcmp($<node>3->varName,"argc")!=0)
    {
        printf("Error: Invalid Syntax\n");
        exit(1);
    }
    $$=makeNonLeafNode('w',NTWRITE,$<node>3,NULL);}
          ;
        
E: E PLUS E {$$=makeNonLeafNode('+',NTOPERATOR,$1,$3);}
    | E MINUS E {$$=makeNonLeafNode('-',NTOPERATOR,$1,$3);}
    | E MUL E {$$=makeNonLeafNode('*',NTOPERATOR,$1,$3);}
    | E LT E {$$=makeNonLeafNode('<',NTOPERATOR,$1,$3);}
    | E GT E {$$=makeNonLeafNode('>',NTOPERATOR,$1,$3);}
    | E LE E {$$=makeNonLeafNode('L',NTOPERATOR,$1,$3);}
    | E GE E {$$=makeNonLeafNode('G',NTOPERATOR,$1,$3);}
    | E EQ E {$$=makeNonLeafNode('e',NTOPERATOR,$1,$3);}
    | E NE E {$$=makeNonLeafNode('n',NTOPERATOR,$1,$3);}
    | '(' E ')' {$$=$2;}
    | NUM {$$=$<node>1;}
    | VAR {$$=$<node>1;}
    | FUN '(' E ')' {$$=makeNonLeafNode('f',NTFUNCALL,$<node>3,NULL);}
    ;

%%

int yyerror (char const *s){
    printf("yyerror %s\n",s);
}

int main(void){
    //extern FILE *yyin;
    //yyin=fopen("input.txt","r");
    yyparse();
    //fclose(yyin);
    return 0;
}