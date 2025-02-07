%{
#include <iostream>
#include <string>
#include <cmath>
#include <FlexLexer.h>
%}

%require "3.8"
%skeleton "glr2.cc"
// %glr-parser

%language "C++"
%defines "Parser.hpp"
%output "Parser.cpp"
%locations

%define api.parser.class {Parser}
%define api.namespace {sc}
%define api.value.type variant
%define parse.error detailed
%parse-param {Scanner* scanner}

%code requires
{
    namespace sc {
        class Scanner;
    } // namespace sc
} // %code requires

%code
{
    #include "Scanner.hpp"
    #include <map>
    #include <string>
    #define yylex(x, l) scanner->lex(x, l)
    IdManager*  idMan = IdManager::getInstance(); 
}

%token              EOL L_PARENTH R_PARENTH SEMI 
%token 		    L_BRACKET R_BRACKET
%token		    L_BRACE R_BRACE
%token <long long>  INT
%token <double>     FLT
%token <std::string> STRING LABEL TIMESTAMP ROW_COUNT

%nterm <size_t>	    source condition property window streamOut tableOut propList dimension valueList arithExp
%nterm <size_t>     trigger trCond function value dimList tableStruct compare like trComp trLike rawProp 


%nterm <std::string>join compOp logOp
%nonassoc           ASSIGN
%precedence        JOIN
%left               PLUS MINUS 
%left               DOLLAR AND OR HASH QUESTION
%left               MULTIPLY SLASH PERCENT DOUBLE_SLASH REMAINDER
%left               L_JOIN R_JOIN I_JOIN F_JOIN 
            
%left 		    LT GT EQ LE GE NE LIKE
%left		    COMMA DOT
%precedence         UMINUS
%precedence         AT 
%nterm <size_t>     joinCond

%right              EXPONENT 
%right              AS
%token              NOT
%right     ARROW
%%

queries	: %empty {}
	| query{}
	| queries query{ }
	;

query	: SEMI {}
	| source { }
        | error SEMI                 {@$; yyerrok; }
	;

source  : L_PARENTH source R_PARENTH {$$ = $2; }
	| LABEL { $$ = idMan->addSource($1, "slct"); }
	| source join source joinCond %prec PERCENT  { $$ = idMan->addNode($2, "join"); idMan->setNodeLR($$, $1, $3); if($4) idMan->addNodeInput($$, $4);}
        | source L_PARENTH I_JOIN R_PARENTH source   %prec SLASH  { $$ = idMan->addNode("Inner Join", "join"); idMan->setNodeLR($$, $1, $5);}
        | source L_BRACKET window R_BRACKET {$$ = idMan->addNode("Window", "buff"); idMan->setNodeLR($$, $1, $3); }
        | source L_BRACKET window trigger R_BRACKET { $$=idMan->addTrigger($1, $3, $4); }
	| source AS LABEL { $$ = idMan->addAlias($3, $1);}
        | source streamOut { $$=$1; idMan->addNodeInput($2, $1);}
        | source tableOut { $$=$1; idMan->prependNodeInput($2, $1);}
	| source PERCENT condition {  $$ = idMan->addNode("Filter", "filt"); idMan->setNodeLR($$, $3, $1);}
	;

joinCond: QUESTION condition {$$=$2;};

tableOut: HASH LABEL {$$ = idMan->addNode($2, "tabl");}
        | HASH LABEL tableStruct {$$ = idMan->addNode($2, "tabl"); idMan->addNodeInput($$, $3);}
        ;

tableStruct : L_BRACKET valueList R_BRACKET {$$=$2;}
            | L_BRACKET R_BRACKET {$$=idMan->addNode("Values", "list");}
            | L_BRACE dimList ARROW valueList R_BRACE {$$=idMan->addNode("CUBE", "cube"); idMan->setNodeLR($$, $2, $4);}
            ;


streamOut       : DOLLAR LABEL   {  $$=idMan->addNode($2, "strm"); }
                | DOLLAR LABEL L_BRACKET R_BRACKET {$$=idMan->addNode($2, "strm"); idMan->addNodeInput($$, idMan->addNode("Values", "list"));}

                | DOLLAR LABEL L_BRACKET valueList R_BRACKET { $$=idMan->addNode($2, "strm"); idMan->addNodeInput($$, $4);}
                ;

value   : property {$$ = $1; }
        | function {$$ = $1;}
        ;

valueList   : value {$$=idMan->addNode("Values", "list"); idMan->addNodeInput($$, $1); }
            | valueList COMMA value {idMan->addNodeInput($1, $3);$$=$1; }
            ;
dimList : dimension { $$=idMan->addNode("Dimensions", "list"); idMan->addNodeInput($$, $1); }
        | dimList COMMA dimension { idMan->addNodeInput($1, $3);$$=$1;}
        ;

dimension   : property {$$=$1; } 
            | L_BRACKET propList R_BRACKET {$$=$2;}
            ;

propList: property {$$=idMan->addNode("COMPLEX DIM","list"); idMan->addNodeInput($$, $1);}
        | propList COMMA property {idMan->addNodeInput($1, $3); $$=$1;}
        ;


function    : LABEL L_PARENTH value R_PARENTH { $$ = idMan->addNode($1+"()", "func"); idMan->addNodeInput($$, $3);}
            ;

window  : TIMESTAMP { $$ = idMan->addNode($1, "cons");}
        | ROW_COUNT {  $$ = idMan->addNode($1, "cons");}
        ;

trigger : AT trCond { $$ = $2; } 
        ;

trCond  : window { $$ = $1;}
        | L_PARENTH trCond R_PARENTH   {$$ = $2; }
        | trComp { $$ = $1;}
        | trLike { $$ = $1;}
        | trCond logOp trCond { $$ = idMan->addNode($2, "expr"); idMan->setNodeLR($$, $1, $3);}
        ;

trComp  : L_PARENTH trComp R_PARENTH {$$=$2;}
        | trComp compOp trComp {$$ = idMan->addNode($2, "expr"); idMan->setNodeLR($$, $1, $3);}
        | rawProp {$$=$1;}
        | FLT { $$ = idMan->addNode($1, "cons");}
        | INT { $$ = idMan->addNode($1, "cons");}
        | STRING { $$ = idMan->addNode($1, "cons"); }
        ;

trLike  : L_PARENTH trLike R_PARENTH {$$=$2;}
        | rawProp LIKE STRING {$$ = idMan->addNode("LIKE", "expr"); idMan->setNodeLR($$, $1, idMan->addNode($3, "cons"));}
        ;


logOp   : AND {$$ = "AND";}
        | OR {$$ = "OR";}
        ;

rawProp : LABEL {$$=idMan->addNode($1, "prop");}
        | rawProp DOT LABEL {$$=idMan->addProp($3, $1);}
        ;

property: source DOT LABEL { $$ = idMan->addProp($3, $1); }
        | property DOT LABEL { $$ = idMan->addProp($3, $1); }
        ;

compare : L_PARENTH compare R_PARENTH   {$$ = $2; }
        | compare compOp compare { $$ = idMan->addNode($2, "expr"); idMan->setNodeLR($$, $1, $3);}
        | STRING { $$ = idMan->addNode($1, "cons");}
        | arithExp {$$ = $1;}
        ;

like    : L_PARENTH like R_PARENTH {$$=$2;}
        | property LIKE STRING {$$ = idMan->addNode("LIKE", "expr"); idMan->setNodeLR($$, $1, idMan->addNode($3, "cons"));}
        ;

condition   : L_PARENTH condition R_PARENTH   {$$ = $2; }
            | condition logOp condition { $$ = idMan->addNode($2, "expr"); idMan->setNodeLR($$, $1, $3);}
            | NOT condition { $$ = idMan->addNode("NOT", "expr"); idMan->addNodeInput($$, $2);}
            | compare {$$ = $1;}
            | like {$$ = $1;}
            ;

compOp  : LT { $$ = "<";}
        | GT { $$ = ">";}
        | EQ { $$ = "==";}
        | LE { $$ = "<=";}
        | GE { $$ = ">=";}
        | NE { $$ = "!=";}
        ;

join	: I_JOIN { $$ = "Inner Join";} 
	| L_JOIN {$$ = "Left Join";} 
	| R_JOIN {$$ = "Right Join";}
	| F_JOIN { $$ = "Full Join";}
	;



arithExp:  value {$$ = $1; }
        | INT { $$ = idMan->addNode($1, "cons");}
        | FLT { $$ = idMan->addNode($1, "cons");}
        | MINUS arithExp %prec UMINUS   {$$ = idMan->addNode("NEG", "expr"); idMan->addNodeInput($$, $2);}
        | arithExp PLUS arithExp {$$ = idMan->addNode("+", "expr"); idMan->setNodeLR($$, $1, $3);}
        | arithExp MINUS arithExp {$$ = idMan->addNode("-", "expr"); idMan->setNodeLR($$, $1, $3);}
        | arithExp SLASH arithExp {$$ = idMan->addNode("/", "expr"); idMan->setNodeLR($$, $1, $3);}
        | arithExp DOUBLE_SLASH arithExp {$$ = idMan->addNode("//", "expr"); idMan->setNodeLR($$, $1, $3);}
        | arithExp REMAINDER arithExp {$$ = idMan->addNode("/+", "expr"); idMan->setNodeLR($$, $1, $3);}
        | arithExp MULTIPLY arithExp {$$ = idMan->addNode("*", "expr"); idMan->setNodeLR($$, $1, $3);}
        | arithExp EXPONENT arithExp {$$ = idMan->addNode("^", "expr"); idMan->setNodeLR($$, $1, $3);}
        | L_PARENTH arithExp R_PARENTH { $$ = $2; }
        ;

%%

void sc::Parser::error(const location_type& loc, const std::string& msg) {
    std::cerr << msg <<" @ " << loc << std::endl;
    std::stringstream temp;
    temp << loc;
    std::string stringified;
    temp >> stringified;
    idMan->reportError(stringified.substr(4) + "[L.C]:" + msg);
}
