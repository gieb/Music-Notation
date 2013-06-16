%{
#include <cstdio>
#include <iostream>
#include <string>
#include <typeinfo>

using namespace std;

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;
extern "C" char *yytext;
extern int line;
 
void yyerror(const char *s);

typedef struct alias_struct {
	string alias;
	string value;
} alias_struct;

alias_struct*aliases = NULL;

%}

%code requires {
	enum NOTE_TYPE {IS_TONE, IS_CHORD, IS_ALIAS, IS_PAUSE};
	
	typedef struct chord_struct{
		int n;
		string*tones;
	} chord_struct;
	
	typedef struct note_struct{
		NOTE_TYPE type;
		void*value;
	} note_struct;
	
	string getSound(note_struct* n, string length, bool dot);
	string getNote(string*n, string length, bool dot);
}

%union {
	string*str;
	chord_struct*chord;
	note_struct*note;
}

%token <str>
	ALIAS VERSE ENDING REPEAT
	KEY SCALE
	METER TEMPO
	FLAT SHARP NATURAL PAUSE PITCH DOT BAR PHRASE REPEAT_BEG REPEAT_END LINE_BEG LINE_END
	COLON COMMA
	NUMBER STRING

%type <str>
	bar_inside sound_group sound tone symbol

%type <chord>
	chord

%type <note>
	note

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
	bar_info sound_group { cout << *$2 << endl; }
	| sound_group { cout << *$1 << endl; }
	;

bar_info:
	METER TEMPO NUMBER { cout << "\t\tMetrum i tempo "<< yytext << endl; }
	;

sound_group:
	sound_group COMMA sound{
		string*temp = new string(*$1 + *$3);
		$$ = temp;
	}
	| sound {
		string*temp = new string(*$1);
		$$ = temp;
	}
	;

sound:
	note COLON NUMBER DOT {
		string*temp = new string(getSound($1, *$3, true));
		$$ = temp;
		
		delete $1;
		delete $3;
	}
	| note COLON NUMBER {
		string*temp = new string(getSound($1, *$3, false));
		$$ = temp;
		
		delete $1;
		delete $3;
	}
	;

note:
	tone {
		$$ = new note_struct;
		$$->type = IS_TONE;
		$$->value = (void*)$1;
	}
	| chord {
		$$ = new note_struct;
		$$->type = IS_CHORD;
		$$->value = (void*)$1;
	}
	| STRING {
		$$ = new note_struct;
		$$->type = IS_ALIAS;
		$$->value = (void*)$1;
	}
	| PAUSE {
		$$ = new note_struct;
		$$->type = IS_PAUSE;
		$$->value = NULL;
	}
	;

chord:
	tone COMMA chord {
		$$ = $3;
		
		string*temp = new string[$$->n + 1];
		for(int i = 0; i < $$->n; i++){
			temp[i] = $$->tones[i]; 
		}
		temp[$$->n++] = *$1;
		
		delete $1;
		delete[] $$->tones;
		
		$$->tones = temp;
	}
	| tone COMMA tone {
		$$ = new chord_struct;
		$$->n = 2;
		$$->tones = new string[$$->n];
		$$->tones[0] = *$1;
		$$->tones[1] = *$3;
		
		delete $1;
		delete $3;
	}
	;

tone:
	symbol PITCH {
		string*temp = new string(*$1 + *$2);
		$$ = temp;
		
		delete $1;
		delete $2;
	}
	| PITCH {
		string*temp = new string(*$1);
		$$ = temp;
		
		delete $1;
	}
	;

symbol:
	FLAT {
		$$ = new string("+");
	}
	| SHARP {
		$$ = new string("-");
	}
	| NATURAL {
		$$ = new string(" ");
	}
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

string getSound(note_struct*n, string length, bool dot){
	int i;
	string sound;
	string*temp;
	chord_struct*c;
	
	switch(n->type){
		case IS_TONE:
			temp = new string(getNote((string*)n->value, length, dot));
			sound = *temp;
			
			delete temp;
			break;
		case IS_CHORD:
			
			c = (chord_struct*)n->value;
			
			sound = "(chord ";
			
			for(i = 0; i < c->n; i++){
				sound += getNote(&c->tones[i], length, dot);
			}
			sound += ")";
			
			delete c;
			break;
		case IS_ALIAS:
			sound = "Alias";
			break;
		case IS_PAUSE:
			temp = new string(getNote(NULL, length, dot));
			sound = *temp;
			
			delete temp;
			break;
	}
	return sound;
}

string getNote(string*n, string length, bool dot){
	string temp;
	
	if(n){
		temp = "(n " + *n + " '" + length + ")";
	}else{
		temp = "(r '" + length + ")";
	}
	
	return temp;
}
