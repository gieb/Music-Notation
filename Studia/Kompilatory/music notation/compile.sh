#!/bin/bash

bison -dv parser.y | echo 'bison'
flex lexer.l | echo 'flex'
g++ parser.tab.c lex.yy.c -lfl -o parser | echo 'kompilacja'
