%{
#include <string>
#include <vector>

using namespace std;

#define YY_DECL extern "C" int yylex()
int line = 1;

#include "parser.tab.h"
%}

%%

[ \t]				;
\n				{ line++; }

alias				{ return   ALIAS; }
verse				{ return   VERSE; }
ending				{ return   ENDING; }
repeat				{ return   REPEAT; }

K:[GF]				{
					yylval.str = new string(yytext);
					return   KEY;
				}
S:[ABCDEFG](dur|mol)		{
					yylval.str = new string(yytext);
					return   SCALE;
				}

M:[1-8]\/[1248]			{
					yylval.str = new string(yytext);
					return   METER;
				}
T:				{ return   TEMPO; }

\-				{ return   FLAT; }
\+				{ return   SHARP; }
~				{ return   NATURAL; }
_				{ return   PAUSE; }
[abcdefg][012345]		{
					yylval.str = new string(yytext);
					return   PITCH;
				}
\.				{ return   DOT; }
\|				{ return   BAR; }
\|\|				{ return   PHRASE; }
\|:				{ return   REPEAT_BEG; }
:\|				{ return   REPEAT_END; }
\{				{ return   LINE_BEG; }
\}				{ return   LINE_END; }

:				{ return   COLON; }
,				{ return   COMMA; }

[0-9]+				{
					yylval.str = new string(yytext);
					return   NUMBER;
				}
[_a-zA-Z]+[_\-a-zA-Z0-9]*	{
					yylval.str = new string(yytext);
					return   STRING;
				}

%%

