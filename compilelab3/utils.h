/**
 * @author navinvue
 * @date 2024/06/05
 * @note utils.h
*/
// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stack>

enum WordType
{
    MAIN, // main
    RETURN,
    VAR,
    OP,
    ASSIGNMET,     //=
    FUNC,          // func
    NUMBER,        // number
    INT,           // int
    VOID,          // void
    LPARENTTHESIS, //(
    RPARENTTHESIS, // )
    LBRANCE,       //{
    RBRANCE,       // }
    SEMICOLON,     // ;
    COMMA,         //,
    PRINT,         // print_ln
    UNKNOWN,        // unknown
    MINUS, // -(取相反数)
    IF, //if
    ELSE, //else
    BREAK, //break
    CONTINUE, //continue
    WHILE, //while 
};

enum ErrorType{ // undo
    FINDARG, //find arg error

};

typedef struct Word
{
    WordType wtype;
    std::string name;
} word;

typedef struct Var
{
    int ptr;          // 栈指针
    std::string name; // 变量名
} var;

// functions
extern std::map<std::string, int> operatorMap;
extern std::vector<std::vector<word>> tokens; // 存储tokens，词法分析结果

extern std::vector<word> tokens1;
extern bool parser_needwarn;

void print_prefix();
void print_postfix();                                                                    // 打印x86汇编代码的前置部分
void display_tokens(std::vector<word> words);                                            // 打印tokens
void display_tokens(std::vector<word>::iterator start, std::vector<word>::iterator end); // 重载打印tokens，查错
void trans_wordtype(WordType wtype);                                                     // 打印wordtype（string）
void trans_op(std::string op);
// void anylizer(std::string str); // 词法分析
void parser();  // 语法分析

#endif // UTILS_H