%{
#include <stdio.h>
#include <string>
#include "preprocessing/ASTNode.h"
#include "parser.h"
extern int yylineno;
void yyerror(const char* s, ...);
#define SAVE_TOKEN yylval.string = new std::string(yytext);
%}

%option noyywrap nodefault yylineno

digs (-)?[0-9]+
chars [a-zA-Z_]+


%%
"#"[^\n]*                      { /* # as comments */ }
[ \t\n\r]                      ; //ignores spaces

 /* operation */
"+"                            { return TPLUS;}
"-"                            { return TMINUS;}
"*"                            { return TFFTIMES;}
"/"                            { return TDIVIDE;}
"%"                            { return TMOD;  }
"&"			                   { return TAND; }
"|"			                   { return TOR; }
"^"			                   { return TXOR; }

 /* base type */
"uint"                         { SAVE_TOKEN; return TYUINT; }
"uint1"                        { SAVE_TOKEN; return TYUINT1; }
"uint4"                        { SAVE_TOKEN; return TYUINT4; }
"uint6"                        { SAVE_TOKEN; return TYUINT6; }
"uint8"                        { SAVE_TOKEN; return TYUINT8; }
"uint12"                       { SAVE_TOKEN; return TYUINT12; }
"uint16"                       { SAVE_TOKEN; return TYUINT16; }
"uint32"                       { SAVE_TOKEN; return TYUINT32; }
"uint64"                       { SAVE_TOKEN; return TYUINT64; }
"uint128"                      { SAVE_TOKEN; return TYUINT128; }
"uint256"                      { SAVE_TOKEN; return TYUINT256; }

 /* expression and statement token */
"("                            { return TLPAREN;} /* expression */
")"                            { return TRPAREN;}
"~"                            { return TNOT;}
"<"                            { return TLABRACKET; }
">"                            { return TRABRACKET; }
"View"                         { return TVIEW; }
"touint"                       { return TTOUINT; }
"<<<"                          { return TRSHIFTL; }
">>>"                          { return TRSHIFTR; }
"<<"                           { return TSHIFTL; }
">>"                           { return TSHIFTR; }
"["                            { return TLBRACKET; }
"]"                            { return TRBRACKET; }
","                            { return TCOMMA;}
";"                            { return TSEMICOLON; } /* statement */
"="                            { return TASSIGN;}
"for"			               { return TFOR; }
"from"                         { return TFROM; }
"to"                           { return TTO; }
"return"		               { return TRETURN; }
"{"                            { return TLBRACE; }
"}"                            { return TRBRACE; }

"r_fn"                         { return TRFN; }
"k_schd"                       { return TKEYSCHD; }
"s_fn"                         { return TSFN; }
"fn"                           { return TFN; } /* main function */
"sbox"                         { return TSBOX; }
"pbox"                         { return TPBOX; }
"pboxm"                        { return TPBOXM; }
"ffm"                          { return TFFM; }
"@cipher"                      { SAVE_TOKEN; return TCIPHER; }

{digs}                         { SAVE_TOKEN; return TINTEGER;}
[a-zA-Z_][a-zA-Z0-9_]*         { SAVE_TOKEN; return TIDENTIFIER;}
.                              { printf("Unknown token! %s\n", yytext);} /*complains about every unknown character*/

%%
void yyerror(const char* s, ...) {

}
