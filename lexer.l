%option yylineno
%option noyywrap
%option nounput

%{
#include "operations.hpp"
#include "parser.hpp"

extern void yyerror(int line, int col, const char* s);
int yylex();
%}

NUMBER -?[0-9]+
PIDENTIFIER [_a-z]+

%%
VAR             { return(VAR); }
BEGIN           { return(BEGIN_); }
END             { return(END); }
ASSIGN          { return(ASSIGN); }
READ            { return(READ); }
WRITE           { return(WRITE); }
IF              { return(IF); }
THEN            { return(THEN); }
ELSE            { return(ELSE); }
ENDIF           { return(ENDIF); }
WHILE           { return(WHILE); }
DO              { return(DO); }
ENDWHILE        { return(ENDWHILE); }
REPEAT          { return(REPEAT); }
UNTIL           { return(UNTIL); }
FOR             { return(FOR); }
FROM            { return(FROM); }
TO              { return(TO); }
DOWNTO          { return(DOWNTO); }
ENDFOR          { return(ENDFOR); }
EQ              { return(EQ); }
NEQ             { return(NEQ); }
LE              { return(LE); }
GE              { return(GE); }
LEQ             { return(LEQ); }
GEQ             { return(GEQ); }
PLUS            { return(PLUS); }
MINUS           { return(MINUS); }
TIMES           { return(TIMES); }
DIV             { return(DIV); }
MOD             { return(MOD); }
{NUMBER}        { yylval.value = atoll(yytext);
                  return (num);         }
{PIDENTIFIER}   { yylval.name = (char*) strdup(yytext);
                  return (pidentifier); }
\(([^\)]|\n)*\)
[ \t\n]+
.               { return(yytext[0]);    }
%%
