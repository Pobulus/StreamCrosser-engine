#pragma once
#include "IdManager.hpp"

namespace sc {

class Scanner : public yyFlexLexer {
public:
  Scanner(std::istream &arg_yyin, std::ostream &arg_yyout)
      : yyFlexLexer(arg_yyin, arg_yyout) {}
  Scanner(std::istream *arg_yyin = nullptr, std::ostream *arg_yyout = nullptr)
      : yyFlexLexer(arg_yyin, arg_yyout) {}
  int lex(Parser::value_type *yylval, sc::Parser::location_type *yylocation);
};

} // namespace sc
