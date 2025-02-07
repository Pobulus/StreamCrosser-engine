# GNU Makefile based on cpp-tutor/flex-bison-cpp

FLEX = flex
BISON = bison 
CPP = g++
CPPFLAGS =
PARSER = parser


AUTOGEN = Parser.hpp Parser.cpp Scanner.cpp

all: $(PARSER)


$(AUTOGEN): lexer.l grammar.y
	$(FLEX) lexer.l
	$(BISON) grammar.y 
# -Wcounterexamples



$(PARSER): $(AUTOGEN) Scanner.hpp
	$(CPP) $(CPPFLAGS) -o $(PARSER) -g Parser.cpp Scanner.cpp IdManager.cpp

clean:
	rm -f $(PARSER) $(AUTOGEN) location.hh
