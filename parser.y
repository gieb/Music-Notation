%{
#include <cstdio>
#include <iostream>
using namespace std;

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;
extern "C" char *yytext;
extern int line;
 
void yyerror(const char *s);
%}

%union {
	int num;
	char*str;
}

%token ALIAS VERSE ENDING REPEAT

%token KEY SCALE

%token METER TEMPO

%token FLAT SHARP NATURAL PAUSE PITCH  DOT BAR PHRASE REPEAT_BEG REPEAT_END LINE_BEG LINE_END

%token COLON COMMA

%token <num> NUMBER
%token <str> STRING

%%

file:
	alias line { cout << "Plik  "<< yytext << endl; }
	;

alias:
	alias ALIAS STRING chord { cout << "Alias  "<< yytext << endl; }
	| ALIAS STRING chord { cout << "Alias  "<< yytext << endl; }
	;

line:
	LINE_BEG line_inside LINE_END { cout << "Linia "<< yytext << endl; }
	;

line_inside:
	line_info verse string { cout << "Wnętrze linii "<< yytext << endl; }
	| line_info verse { cout << "Wnętrze linii "<< yytext << endl; }
	;

line_info:
	KEY SCALE { cout << "Klucz i skala "<< yytext << endl; }
	| KEY { cout << "Klucz "<< yytext << endl; }
	;

verse:
	verse verse_inside
	| verse_inside { cout << "Część "<< yytext << endl; }
	;

verse_inside:
	verse_declaration path { cout << "Wnętrze części "<< yytext << endl; }
	;

verse_declaration:
	VERSE STRING { cout << "Deklaracja części "<< yytext << endl; }
	;

string:
	string STRING
	| STRING { cout << "Napis "<< yytext << endl; }
	;

path:
	path path_inside
	| path_inside { cout << "Ścieżka "<< yytext << endl; }
	;

path_inside:
	phrase { cout << "Wnętrze ścieżki "<< yytext << endl; }
	| repeat { cout << "Wnętrze ścieżki "<< yytext << endl; }
	;

phrase:
	PHRASE bar_group PHRASE { cout << "Fraza "<< yytext << endl; }
	;

repeat:
	repeat_inside repeat_number ending_bar { cout << "Fraza powtarzana z taktem końcowym "<< yytext << endl; }
	| repeat_inside repeat_number { cout << "Fraza powtarzana "<< yytext << endl; }
	;

repeat_inside:
	REPEAT_BEG bar_group ending_bar REPEAT_END { cout << "Wnętrze frazy "<< yytext << endl; }
	| REPEAT_BEG bar_group REPEAT_END { cout << "Wnętrze frazy "<< yytext << endl; }
	;

repeat_number:
	REPEAT NUMBER { cout << "Powtarzanie "<< yytext << endl; }
	;

bar_group:
	bar bar_group
	| bar { cout << "Takty "<< yytext << endl; }
	;

ending_bar:
	ending bar { cout << "Takt końcowy "<< yytext << endl; }
	;

ending:
	ending COMMA NUMBER
	| ENDING NUMBER { cout << "Zakończenie "<< yytext << endl; }
	;

bar:
	bar_inside BAR { cout << "Takt "<< yytext << endl; }
	| bar_inside { cout << "Takt "<< yytext << endl; }
	;

bar_inside:
	bar_info sound_group { cout << "\tWnętrze taktu "<< yytext << endl; }
	| sound_group { cout << "\tWnętrze taktu "<< yytext << endl; }
	;

bar_info:
	METER TEMPO NUMBER { cout << "\t\tMetrum i tempo "<< yytext << endl; }
	;

sound_group:
	sound_group COMMA sound
	| sound { cout << "\t\tDźwięki "<< yytext << endl; }
	;

sound:
	note COLON NUMBER DOT { cout << "\t\tDźwięk "<< yytext << endl; }
	| note COLON NUMBER { cout << "\t\tDźwięk "<< yytext << endl; }
	;

note:
	tone { cout << "\t\tNuta "<< yytext << endl; }
	| chord { cout << "\t\tAkord "<< yytext << endl; }
	| STRING { cout << "\t\tMakro "<< yytext << endl; }
	| PAUSE { cout << "\t\tPauza "<< yytext << endl; }
	;

chord:
	tone COMMA chord
	| tone COMMA tone { cout << "\t\t\tChwyt "<< yytext << endl; }
	;

tone:
	symbol PITCH { cout << "\t\t\t\tWysokość "<< yytext << endl; }
	| PITCH { cout << "\t\t\t\tWysokość "<< yytext << endl; }
	;

symbol:
	FLAT { cout << "\t\t\t\tBemol "<< yytext << endl; }
	| SHARP { cout << "\t\t\t\tKrzyżyk "<< yytext << endl; }
	| NATURAL { cout << "\t\t\t\tKasownik "<< yytext << endl; }
	;

%%

int main(int argc, char* args[]) {
	FILE *f = fopen(args[1], "r");
	if (!f) {
		cerr << "fopen error " << endl ;
		return -1;
	}
	
	yyin = f;
	
	do {
		yyparse();
	} while (!feof(yyin));
	
}

void yyerror(const char *s) {
	cout << "line  " << line << ": " << yytext << ",  " << s << endl;
	exit(-1);
}
