#!/bin/bash

bison -dv parser.y | echo 'bison'
flex lexer.l | echo 'flex'
g++ parser.tab.c lex.yy.c -lfl -o parser | echo 'kompilacja'
cd interfejs
echo "budowanie interfejsu"
g++ ../lex.yy.c interfejs.cpp wxMidi/wxMidi.cpp wxMidi/wxMidiDatabase.cpp -o inter `pkg-config --cflags liblomse` `pkg-config --libs liblomse` -lstdc++  `wx-config --cppflags` `wx-config --libs` -lfl -L./wxMidi  -lportmidi -lporttime `wx2812-config --libs`  `wx2812-config --cflags` 
cp  inter  ../