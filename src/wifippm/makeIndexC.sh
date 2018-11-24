#!/bin/bash
echo "#include <pgmspace.h>" > index.html.c
echo "char index_html[] PROGMEM = R\"=====(" >> index.html.c
echo "$(cat index.html)" >> index.html.c
echo "      )=====\";" >> index.html.c

