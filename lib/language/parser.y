
%{
	#include <stdio.h>
	#include <stdlib.h>
    #include "preprocessing/ASTNode.h"
	extern FILE *fp;
	extern FILE *yyin;
	extern int yylex();
	extern int yylineno;
	extern void yyerror(const char* s, ...);
	using namespace ASTNode;
        ASTNode::NBlock *programRoot; /* the root node of our program AST:: */
%}

%union {
	ASTNode::NStatement* stmt;
	ASTNode::NBlock* block;
	std::string* string;
    ASTNode::NIdentifier* ident;
	ASTNode::NArrayIndex* index;
	ASTNode::NExpression* expr;
	ASTNode::NInteger* integer;
	ASTNode::NVariableList* varlist;
	ASTNode::NVariableDeclaration* var_decl;
	ASTNode::NStatementList* stmtList;
	std::vector<std::shared_ptr<ASTNode::NExpression>>* exprvec;
	std::vector<std::shared_ptr<ASTNode::NInteger>>* intvec;
	std::vector<std::shared_ptr<std::vector<std::shared_ptr<ASTNode::NInteger>>>>* intvecvec;
	ASTNode::NFunctionDeclaration* fundecl;
	ASTNode::NRoundFunctionDeclaration* rfundecl;
    ASTNode::NKeyScheduleDeclaration* kschddecl;
	ASTNode::NSboxFunctionDeclaration* sfundecl;
    ASTNode::NCipherFunctionDeclaration* cfundecl;
    //ASTNode::NCipherNameDeclaration* ciphername;
}

%token <string> TIDENTIFIER TINTEGER
// operators
%token <token> TPLUS TMINUS TDIVIDE TFFTIMES TMOD TAND TOR TXOR
// base type
%token <string> TYUINT TYUINT1 TYUINT4 TYUINT6 TYUINT8 TYUINT12 TYUINT16 TYUINT32 TYUINT64 TYUINT128 TYUINT256
// expression token
%token <token> TLPAREN TRPAREN TNOT TLABRACKET TRABRACKET TVIEW TTOUINT TRSHIFTL TRSHIFTR TSHIFTL TSHIFTR TLBRACKET TRBRACKET TCOMMA
// statement token
%token <token> TSEMICOLON TASSIGN TFOR TFROM TTO TRETURN TLBRACE TRBRACE
%token <token> TRFN TKEYSCHD TSFN TFN TSBOX TPBOX TPBOXM TFFM TCIPHER

%type <ident> ident primary_typename array_typename1 array_typename2 typename pbox_base pbox_type
%type <index> array_index
%type <expr> expr assign view_array touint
%type <exprvec> call_args
%type <intvec> num_list1
%type <intvecvec> num_list2
%type <integer> numeric
//%type <range> array_range
%type <stmt> var_decl stmt for_stmt box_decl
%type <varlist> func_decl_args
%type <block> block stmts
%type <fundecl> func_decl
%type <rfundecl> r_func_decl
%type <kschddecl> k_schd_decl
%type <sfundecl> s_func_decl
%type <cfundecl> c_func_decl
//%type <ciphername> cipher_name

%left TPLUS TMINUS TFFTIMES TDIVIDE TMOD TAND TOR TXOR
%left TSHIFTL TSHIFTR TRSHIFTL TRSHIFTR
%nonassoc TNOT

%locations
%define parse.trace
%start program


%%
program
	: stmts {programRoot = $1;}
	;

stmts
	: stmt {$$ = new NBlock(); $$->getStmtList()->push_back(std::shared_ptr<NStatement>($1));}
	| stmts stmt { $1->getStmtList()->push_back(std::shared_ptr<NStatement>($2));}
	;

stmt
	: func_decl {$$ = $1;}
	| r_func_decl {$$ = $1;}
	| k_schd_decl {$$ = $1;}
	| s_func_decl {$$ = $1;}
	| c_func_decl {$$ = $1;}
	| expr TSEMICOLON {$$ = new NExpressionStatement(std::shared_ptr<NExpression>($1));}
	| TRETURN expr TSEMICOLON {$$ = new NReturnStatement(std::shared_ptr<NExpression>($2));}
	| for_stmt{$$ = $1;}
	| var_decl TSEMICOLON{$$ = $1;}
	| box_decl TSEMICOLON{$$ = $1;}
    | TCIPHER TIDENTIFIER {$$ = new NCipherNameDeclaration(*$2); delete $2;}
	;

block
	: TLBRACE stmts TRBRACE {$$ = $2;}
	| TLBRACE TRBRACE {$$ = new NBlock();}
	;

primary_typename
    : TYUINT1 {$$ = new NIdentifier(*$1); $$->isType = true;  delete $1;}
    | TYUINT4 {$$ = new NIdentifier(*$1); $$->isType = true;  delete $1;}
    | TYUINT6 {$$ = new NIdentifier(*$1); $$->isType = true;  delete $1;}
    | TYUINT8 {$$ = new NIdentifier(*$1); $$->isType = true;  delete $1;}
    | TYUINT12 {$$ = new NIdentifier(*$1); $$->isType = true;  delete $1;}
    | TYUINT16 {$$ = new NIdentifier(*$1); $$->isType = true;  delete $1;}
    | TYUINT32 {$$ = new NIdentifier(*$1); $$->isType = true;  delete $1;}
    | TYUINT64 {$$ = new NIdentifier(*$1); $$->isType = true;  delete $1;}
    | TYUINT128 {$$ = new NIdentifier(*$1); $$->isType = true;  delete $1;}
    | TYUINT256 {$$ = new NIdentifier(*$1); $$->isType = true;  delete $1;}
    ;

// 下面两个类型用于专门处理pbox的类型，因为pbox index格式的类型只能位uint
pbox_base
    : TYUINT {$$ = new NIdentifier(*$1); $$->isType = true;  delete $1;}
    ;

pbox_type
    : pbox_base TLBRACKET expr TRBRACKET {
       $1->isArray = true;
       $1->arraySize->push_back(std::shared_ptr<NExpression>($3));
       $$ = $1;
    }
    ;

// 这里不能递归实现了，只能用一维数组（sbox，pbox）和二维数组（pboxm，ffm）两种,
// 分别由array_typename1 和 array_typename2实现
array_typename1
    : primary_typename TLBRACKET expr TRBRACKET {
        $1->isArray = true;
        $1->arraySize->push_back(std::shared_ptr<NExpression>($3));
        $$ = $1;
    }
    ;

array_typename2
    : primary_typename TLBRACKET expr TRBRACKET TLBRACKET expr TRBRACKET {
        $1->isArray = true;
        $1->arraySize->push_back(std::shared_ptr<NExpression>($3));
        $1->arraySize->push_back(std::shared_ptr<NExpression>($6));
        $$ = $1;
    }
    ;

typename
    : primary_typename
    | array_typename1
    ;

var_decl
	: typename ident {$$ = new NVariableDeclaration(std::shared_ptr<NIdentifier>($1), std::shared_ptr<NIdentifier>($2), nullptr); }
	| typename ident TASSIGN expr { $$ = new NVariableDeclaration(std::shared_ptr<NIdentifier>($1), std::shared_ptr<NIdentifier>($2), std::shared_ptr<NExpression>($4));}
	| typename ident TASSIGN TLBRACE call_args TRBRACE {
		 $$ = new NArrayInitialization(std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($1), std::shared_ptr<NIdentifier>($2), nullptr), std::shared_ptr<NExpressionList>($5));
	}
	;

// 这里我把box的声明主体写死成为只能直接输入整数，而非表达式
box_decl
    : TSBOX array_typename1 ident TASSIGN TLBRACE num_list1 TRBRACE {
         $$ = new NSboxInitialization(std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($2), std::shared_ptr<NIdentifier>($3), nullptr), std::shared_ptr<NIntegerList>($6));
    }
    | TPBOX pbox_type ident TASSIGN TLBRACE num_list1 TRBRACE {
         $$ = new NPboxInitialization(std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($2), std::shared_ptr<NIdentifier>($3), nullptr), std::shared_ptr<NIntegerList>($6));
    }
    | TPBOXM array_typename2 ident TASSIGN TLBRACE num_list2 TRBRACE {
         $$ = new NPboxmInitialization(std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($2), std::shared_ptr<NIdentifier>($3), nullptr), std::shared_ptr<NIntegerListList>($6));
    }
    | TFFM array_typename2 ident TASSIGN TLBRACE num_list2 TRBRACE {
         $$ = new NFfmInitialization(std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($2), std::shared_ptr<NIdentifier>($3), nullptr), std::shared_ptr<NIntegerListList>($6));
    }
	;

func_decl
	: typename ident TLPAREN func_decl_args TRPAREN block {
		$$ = new NFunctionDeclaration(
			std::shared_ptr<NIdentifier>($1),
			std::shared_ptr<NIdentifier>($2),
			std::shared_ptr<NVariableList>($4),
			std::shared_ptr<NBlock>($6));
	}
	;

func_decl_args
	: {$$ = new NVariableList(); }
	| var_decl {$$ = new NVariableList(); $<var_decl>1->isParameter = true; $$->push_back(std::shared_ptr<NVariableDeclaration>($<var_decl>1));}
	| func_decl_args TCOMMA var_decl {$<var_decl>3->isParameter = true; $1->push_back(std::shared_ptr<NVariableDeclaration>($<var_decl>3));}
	;

r_func_decl
    : TRFN typename ident TLPAREN typename ident TCOMMA typename ident TCOMMA typename ident TRPAREN block {
        $<ident>6->isParameter = true;
        $<ident>9->isParameter = true;
        $<ident>12->isParameter = true;
        $$ = new NRoundFunctionDeclaration(
            std::shared_ptr<NIdentifier>($2),
            std::shared_ptr<NIdentifier>($3),
            std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($5), std::shared_ptr<NIdentifier>($6), nullptr),
            std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($8), std::shared_ptr<NIdentifier>($9), nullptr),
            std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($11), std::shared_ptr<NIdentifier>($12), nullptr),
            std::shared_ptr<NBlock>($14));
    }
    ;

k_schd_decl
    : TKEYSCHD typename ident TLPAREN typename ident TRPAREN block {
        $<ident>6->isParameter = true;
        $$ = new NKeyScheduleDeclaration(
            std::shared_ptr<NIdentifier>($2),
            std::shared_ptr<NIdentifier>($3),
            std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($5), std::shared_ptr<NIdentifier>($6), nullptr),
            std::shared_ptr<NBlock>($8));
    }
    ;

s_func_decl
    : TSFN typename ident TLPAREN typename ident TRPAREN block {
        $<ident>6->isParameter = true;
        $$ = new NSboxFunctionDeclaration(
            std::shared_ptr<NIdentifier>($2),
            std::shared_ptr<NIdentifier>($3),
            std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($5), std::shared_ptr<NIdentifier>($6), nullptr),
            std::shared_ptr<NBlock>($8));
    }
    ;

c_func_decl
    : TFN typename ident TLPAREN typename ident TCOMMA typename ident TRPAREN block {
        $<ident>6->isParameter = true;
        $<ident>9->isParameter = true;
        $$ = new NCipherFunctionDeclaration(
            std::shared_ptr<NIdentifier>($2),
            std::shared_ptr<NIdentifier>($3),
            std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($5), std::shared_ptr<NIdentifier>($6), nullptr),
            std::make_shared<NVariableDeclaration>(std::shared_ptr<NIdentifier>($8), std::shared_ptr<NIdentifier>($9), nullptr),
            std::shared_ptr<NBlock>($11));
    }
    ;

ident
	: TIDENTIFIER {$$ = new NIdentifier(*$1); delete $1;}
	;

numeric
	: TINTEGER {$$ = new NInteger(atol($1->c_str()));}
	;

expr
	: assign { $$ = $1;}
	| ident TLPAREN call_args TRPAREN {
		$$ = new NFunctionCall(std::shared_ptr<NIdentifier>($1), std::shared_ptr<NExpressionList>($3), yylineno);
	}
	| ident {$<ident>$ = $1;}
	| numeric {$$ = $1;}
	| expr TPLUS expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::ADD, std::shared_ptr<NExpression>($3));}
	| expr TMINUS expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::MINUS, std::shared_ptr<NExpression>($3));}
	| expr TFFTIMES expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::FFTIMES, std::shared_ptr<NExpression>($3));}
	| expr TDIVIDE expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::DIVIDE, std::shared_ptr<NExpression>($3));}
	| expr TSHIFTL expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::LSH, std::shared_ptr<NExpression>($3));}
	| expr TSHIFTR expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::RSH, std::shared_ptr<NExpression>($3));}
	| expr TRSHIFTL expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::RLSH, std::shared_ptr<NExpression>($3));}
	| expr TRSHIFTR expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::RRSH, std::shared_ptr<NExpression>($3));}
	| expr TMOD expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::MOD, std::shared_ptr<NExpression>($3));}
	| expr TAND expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::AND, std::shared_ptr<NExpression>($3));}
	| expr TXOR expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::XOR, std::shared_ptr<NExpression>($3));}
	| expr TOR expr {$$ = new NBinaryOperator(std::shared_ptr<NExpression>($1), Operator::OR, std::shared_ptr<NExpression>($3));}
	| TNOT expr {$$ = new NUnaryOperator(std::shared_ptr<NExpression>($2), Operator::NOT); }
	| TMINUS expr {$$ = new NUnaryOperator(std::shared_ptr<NExpression>($2), Operator::MINUS); }
	| TLPAREN expr TRPAREN {$$ = $2;}
	| array_index { $$ = $1; }
    | view_array { $$ = $1; }
    | touint { $$ = $1; }
    | ident TLABRACKET expr TRABRACKET {$$ = new NBoxOperation(std::shared_ptr<NIdentifier>($1), std::shared_ptr<NExpression>($3)); }
	;

array_index
	: ident TLBRACKET expr TRBRACKET {
		$$ = new NArrayIndex(std::shared_ptr<NIdentifier>($1));
		$$->dimons->push_back(std::shared_ptr<NExpression>($3));
	}
	| array_index TLBRACKET expr TRBRACKET {
		$1->dimons->push_back(std::shared_ptr<NExpression>($3));
		$$ = $1;
	}
	;

assign
	: ident TASSIGN expr {
		$$ = new NAssignment(std::shared_ptr<NIdentifier>($1), std::shared_ptr<NExpression>($3));
	}
	| array_index TASSIGN expr {
		$$ = new NArrayAssignment(std::shared_ptr<NArrayIndex>($1), std::shared_ptr<NExpression>($3));
	}
	| expr TASSIGN expr
	;

view_array
    : TVIEW TLPAREN ident TCOMMA expr TCOMMA expr TRPAREN {
        $$ = new NViewArray(
            std::shared_ptr<NIdentifier>($3),
            std::shared_ptr<NExpression>($5),
            std::shared_ptr<NExpression>($7));
    }
    ;

touint
    : TTOUINT TLPAREN call_args TRPAREN {$$ = new NToUint(std::shared_ptr<NExpressionList>($3));}
    | TTOUINT TLPAREN ident TRPAREN {$$ = new NToUint(std::shared_ptr<NIdentifier>($3));}
    ;

call_args
	: {$$ = new NExpressionList();}
	| expr {$$ = new NExpressionList(); $$->push_back(std::shared_ptr<NExpression>($1));}
	| call_args TCOMMA expr {$1->push_back(std::shared_ptr<NExpression>($3));}
	;

num_list1
    : {$$ = new NIntegerList;}
    | numeric {$$ = new NIntegerList(); $$->push_back(std::shared_ptr<NInteger>($1));}
    | num_list1 TCOMMA numeric {$1->push_back(std::shared_ptr<NInteger>($3));}
    ;

num_list2
    : {$$ = new NIntegerListList;}
	| TLBRACE num_list1 TRBRACE {$$ = new NIntegerListList(); $$->push_back(std::shared_ptr<NIntegerList>($2));}
	| num_list2 TCOMMA TLBRACE num_list1 TRBRACE {$1->push_back(std::shared_ptr<NIntegerList>($4));}
	;

for_stmt
	: TFOR TLPAREN ident TFROM numeric TTO numeric TRPAREN block {
		$$ = new NForStatement(std::shared_ptr<NIdentifier>($3), std::shared_ptr<NInteger>($5), std::shared_ptr<NInteger>($7), std::shared_ptr<NBlock>($9) );
	}
	;
%%
