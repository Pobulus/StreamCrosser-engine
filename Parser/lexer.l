%{
#include "Parser.hpp"
#include "Scanner.hpp"
#include "../includes/json.hpp"
#include "../includes/httplib.h"
#include <sstream>
#define YY_DECL int sc::Scanner::lex(sc::Parser::value_type *yylval, sc::Parser::location_type* yylocation )
#define ADV yylocation->columns(strlen(YYText()));
// workaround for bug in flex 2.6.4
#define yypanic(X) (void)(X)
%}

%option c++ interactive noyywrap noyylineno nodefault outfile="Scanner.cpp"

dseq            ([[:digit:]]+)
dseq_opt        ({dseq}?)
frac            (({dseq_opt}"."{dseq})|{dseq}".")
exp             ([eE][+-]?{dseq})
exp_opt         ({exp}?)

integer         ({dseq})
float           (({frac}{exp_opt})|({dseq}{exp}))
label           ([[:alpha:]_][[:alnum:]_]*)
string 	        ["]([^"\\\n]|\\(.|\n))*["] 
fltvar          ([[:lower:]]+)
seconds 	({integer}+"s")
miliseconds	({integer}+"ms")
minutes		({integer}+"min")
rowCount ({integer}+"r")

timestamp	({minutes}{seconds}?{miliseconds}?|{seconds}{miliseconds}?|{miliseconds})

%%

{integer}       ADV yylval->emplace<long long>(strtoll(YYText(), nullptr, 10)); return Parser::token::INT;
{float}         ADV yylval->emplace<double>(strtod(YYText(), nullptr)); return Parser::token::FLT;
{label}         ADV yylval->emplace<std::string>(std::string(YYText())); return Parser::token::LABEL;
{string}        ADV yylval->emplace<std::string>(std::string(YYText())); return Parser::token::STRING;
{rowCount}      ADV yylval->emplace<std::string>(std::string(YYText())); return Parser::token::ROW_COUNT;
{timestamp}     ADV yylval->emplace<std::string>(std::string(YYText())); return Parser::token::TIMESTAMP;
"<<"		    ADV return Parser::token::L_JOIN;
">>"		    ADV return Parser::token::R_JOIN;
"><"	    	ADV return Parser::token::I_JOIN;
"<>"		    ADV return Parser::token::F_JOIN;
"->"            ADV return Parser::token::ARROW;
">"     		ADV return Parser::token::GT;
"<"	        	ADV return Parser::token::LT;
">="	       	ADV return Parser::token::GE;
"<="	       	ADV return Parser::token::LE;
"=="	       	ADV return Parser::token::EQ;
"!="	       	ADV return Parser::token::NE;
"+"             ADV return Parser::token::PLUS;
"-"             ADV return Parser::token::MINUS;
"*"             ADV return Parser::token::MULTIPLY;
"//"            ADV return Parser::token::DOUBLE_SLASH;
"/+"            ADV return Parser::token::REMAINDER;
"/"             ADV return Parser::token::SLASH;
"%"             ADV return Parser::token::PERCENT;
"^"             ADV return Parser::token::EXPONENT;
"("             ADV return Parser::token::L_PARENTH;
")"             ADV return Parser::token::R_PARENTH;
"["             ADV return Parser::token::L_BRACKET;
"]"             ADV return Parser::token::R_BRACKET;
"{"             ADV return Parser::token::L_BRACE;
"}"             ADV return Parser::token::R_BRACE;
"."             ADV return Parser::token::DOT;
"!"             ADV return Parser::token::NOT;
","             ADV return Parser::token::COMMA;
"~"             ADV return Parser::token::LIKE;
";"             ADV return Parser::token::SEMI;
":"             ADV return Parser::token::AS;
"@"             ADV return Parser::token::AT;
"?"             ADV return Parser::token::QUESTION;
"&"             ADV return Parser::token::AND;
"|"             ADV return Parser::token::OR;
"$"             ADV return Parser::token::DOLLAR;
"#"             ADV return Parser::token::HASH;
"="             ADV return Parser::token::ASSIGN;
\n              yylocation->lines(1); //return Parser::token::EOL;
<<EOF>>         ADV return Parser::token::YYEOF;
.               ADV /* no action on unmatched input */

%%

int yyFlexLexer::yylex() {
    throw std::runtime_error("Invalid call to yyFlexLexer::yylex()");
}


int helpMessage()  {
    std::cout << 
"please provide query and optionally arguments,\n\
the query should be the last argument and must be quoted\n\n\
ARGUMENTS:\n\
-s\tEnables server mode with a web interface\n\
-h\tshows this help mesage\n" 
    << std::endl;
    return 0;
}
int runServer() {
    httplib::Server svr;
    if (!svr.set_mount_point("/", "./web")) {
        std::cerr << "couldn't open server dir" << std::endl;
        return 1;
    }
    
    std::stringstream scannerIn;
    sc::Scanner scanner{ scannerIn, std::cerr };
    sc::Parser parser{ &scanner };
    std::cout.precision(10);
    svr.Post("/query", [&parser, &scannerIn](const httplib::Request& req, httplib::Response& res) {
        scannerIn << req.body << std::endl;
        if(parser.parse() == 0) {
            res.set_content(IdManager::getInstance()->dump(), "text/plain");
            IdManager::getInstance()->clear();
	    }
	
    });
    std::string addres ="localhost"; 
    int port = 8081; 
    std::cout << "server listening on: http://" << addres << ":" << port << std::endl;
    svr.listen(addres,port);
    return 0;
}
int main(int argc, char *argv[]) {
    if(argc == 1) {return helpMessage();}
    for(int i = 1; i < argc; i++){
       if(!strcmp(argv[i], "-h") ) return helpMessage();  
       if(!strcmp(argv[i], "-s") ) return runServer();
    }
    std::stringstream scannerIn;
    sc::Scanner scanner{ scannerIn, std::cerr };
    sc::Parser parser{ &scanner };
    std::cout.precision(10);
    
	scannerIn << argv[argc-1] << ';';
	if(parser.parse() == 0) {
		std::cout << IdManager::getInstance()->dump() << std::endl;
		// IdManager::getInstance()->clear();
	}
	
    return 0;
    
}
