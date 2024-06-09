// lexer.h
#ifndef LEXER_H
#define LEXER_H
#include "utils.h"
#include <fstream>
#include <sstream>
#include <regex>

void line_analize(std::string line);
void anylize(const char *filename);

#endif