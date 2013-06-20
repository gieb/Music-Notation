%{
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <typeinfo>

using namespace std;

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;
extern "C" char *yytext;
extern int line;
 
void yyerror(const char *s);

map<string, vector<string>* > aliases;
map<string, string* > verses;

%}

%code requires {
	enum NOTE_TYPE {IS_TONE, IS_CHORD, IS_ALIAS, IS_PAUSE};
	
	typedef struct note_struct{
		NOTE_TYPE type;
		void*value;
	} note_struct;
	
	string getSound(note_struct* n, string length, bool dot);
	string getNote(string*n, string length, bool dot);
}

%union {
	string*str;
	vector<string>*chord;
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
	file
	line line_inside line_info
	verse verse_inside verse_declaration string
	path path_inside
	phrase repeat repeat_number repeat_inside
	bar_group ending_bar ending bar bar_info bar_inside
	sound_group sound tone symbol

%type <chord>
	chord

%type <note>
	note

%%

file:
	alias line {
		cout << "(score" << *$2 << ")";
	}
	;

alias:
	alias ALIAS STRING chord {
		aliases[*$3]=$4;
		
		delete $3;
	}
	| ALIAS STRING chord {
		aliases[*$2]=$3;
		
		delete $2;
	}
	;

line:
	LINE_BEG line_inside LINE_END {
		$$ = new string("(instrument(musicData" + *$2 + "))");
	}
	;

line_inside:
	line_info verse string {
		$$ = new string(*$1 + *$2 + *$3);
		
		delete $1;
		delete $2;
		delete $3;
	}
	| line_info verse {
		$$ = new string(*$1 + *$2);
		
		delete $1;
		delete $2;
	}
	;

line_info:
	KEY SCALE {
		$$ = new string("(clef " + $1->substr(2,1) + ")(key " + $2->substr(2,1) + ")");
	}
	| KEY {
		$$ = new string("(clef " + $1->substr(2,1) + ")");
	}
	;

verse:
	verse verse_inside{
		$$ = new string(*$1 + *$2);
		
		delete $1;
		delete $2;
	}
	| verse_inside {
		$$ = $1;
	}
	;

verse_inside:
	verse_declaration path {
		$$ = verses[*$1] = $2;
	}
	;

verse_declaration:
	VERSE STRING {
		$$ = $2;
	}
	;

string:
	string STRING {
		$$ = new string(*$1 + *verses[*$2]);
		
		delete $1;
		delete $2;
	}
	| STRING {
		$$ = verses[*$1];
	}
	;

path:
	path path_inside {
		$$ = new string(*$1 + *$2);
		
		delete $1;
		delete $2;
	}
	| path_inside {
		$$ = $1;
	}
	;

path_inside:
	phrase {
		$$ = $1;
	}
	| repeat {
		$$ = $1;
	}
	;

phrase:
	PHRASE bar_group PHRASE {
		$$ = $2;
	}
	;

repeat:
	repeat_inside repeat_number ending_bar {
		$$ = new string(*$1 + *$2 + *$3);
		
		delete $1;
		delete $2;
		delete $3;
	}
	| repeat_inside repeat_number {
		$$ = new string(*$1 + *$2);
		
		delete $1;
		delete $2;
	}
	;

repeat_inside:
	REPEAT_BEG bar_group ending_bar REPEAT_END {
		$$ = new string("(barline |:)" + *$2 + *$3 + "(barline :|)");
		
		delete $2;
		delete $3;
	}
	| REPEAT_BEG bar_group REPEAT_END {
		$$ = new string("(barline |:)" + *$2 + "(barline :|)");
		
		delete $2;
	}
	;

repeat_number:
	REPEAT NUMBER {
		$$ = new string("(text \"Repeat: " + *$2 + "\")");
		
		delete $2;
	}
	;

bar_group:
	bar_group bar {
		$$ = new string(*$1 + *$2);
		
		delete $1;
		delete $2;
	}
	| bar {
		$$ = $1;
	}
	;

ending_bar:
	ending bar {
		$$ = new string(*$1 + *$2);
	}
	;

ending:
	ending COMMA NUMBER {
		$$ = new string($1->substr($1->size()-2,2) + ", " + *$3 + "\")");
	}
	| ENDING NUMBER {
		$$ = new string("(text \"Ending: " + *$2 + "\")");
	}
	;

bar:
	bar_inside BAR {
		$$ = new string(*$1 + "(barline |)");
		
		delete $1;
	}
	| bar_inside {
		$$ = $1;
	}
	;

bar_inside:
	bar_info sound_group {
		$$ = new string(*$1 + *$2);
		
		delete $1;
		delete $2;
	}
	| sound_group {
		$$ = $1;
	}
	;

bar_info:
	METER TEMPO NUMBER {
		string meter = "(time " + $1->substr(2,1) + " " + $1->substr(4,1) + ")";
		string tempo = "(text \"Tempo: " + *$3 + "\")";
		$$ = new string(meter + tempo);
	}
	;

sound_group:
	sound_group COMMA sound{
		$$ = new string(*$1 + *$3);
		
		delete $1;
		delete $3;
	}
	| sound {
		$$ = $1;
	}
	;

sound:
	note COLON NUMBER DOT {
		$$ = new string(getSound($1, *$3, true));
		
		delete $1;
		delete $3;
	}
	| note COLON NUMBER {
		$$ = new string(getSound($1, *$3, false));
		
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
		$$->push_back(*$1);
		
		delete $1;
	}
	| tone COMMA tone {
		$$ = new vector<string>();
		$$->push_back(*$1);
		$$->push_back(*$3);
		
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

/////////////////////////////MAIN/////////////////////////////

void parse(conts char* patch) {
	FILE *f = fopen(patch);
	if (!f) {
		cerr << "fopen error " << endl ;
		return -1;
	}
	
	yyin = f;
	
	do {
		yyparse();
	} while (!feof(yyin));
	
}

///////////////////////////FUNCTIONS///////////////////////////

void yyerror(const char *s) {
	cout << "line  " << line << ": " << yytext << ",  " << s << endl;
	exit(-1);
}

string getSound(note_struct*n, string length, bool dot){
	int i;
	string sound;
	string*temp;
	vector<string>*c;
	
	switch(n->type){
		case IS_TONE:
			temp = new string(getNote((string*)n->value, length, dot));
			sound = *temp;
			
			delete temp;
			break;
		case IS_CHORD:
			c = (vector<string>*)n->value;
			
			sound = "(chord ";
			
			for(i = 0; i < c->size(); i++){
				sound += getNote(&c->at(i), length, dot);
			}
			sound += ")";
			
			delete c;
			break;
		case IS_ALIAS:
			temp = (string*)n->value;
			c = aliases[*temp];
			
			sound = "(chord ";
			
			for(i = 0; i < c->size(); i++){
				sound += getNote(&c->at(i), length, dot);
			}
			sound += ")";
			
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
	if(n){
		return "(n " + *n + " '" + length + ")";
	}else{
		return "(r '" + length + ")";
	}
}
