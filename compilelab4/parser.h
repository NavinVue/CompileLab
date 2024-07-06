// parser.h 语法分析，语义处理+代码生成（不会使用bison）
#ifndef PARSER_H
#define PARSER_H
#include "utils.h"
#include <algorithm>

/**
 * 存储函数变量，类型，var
 */
typedef struct arg
{
    WordType argtype;
    word argword;
} arg;

/**
 * 储存函数的信息，包括局部变量，栈指针等
 */
typedef struct FuncInfo
{
    int fptr = -4;                   // 储存栈指针（局部变量的位置）
    int argnum;                      // 储存函数有几个参数，堆栈平衡需要
    int iflabel = 0;                 // 储存label的index，用于生成label(for if else sentence)
    int while_label = 0;             // for while sentence
    std::stack<std::string> opstack; // op stack of one func
    std::stack<var> numberstack;     // numberstack of one func (useless, actually)
    std::vector<var> vars;           // 储存变量，函数传入的参数也实际上被存在这里
    std::vector<arg> args;           // 函数的参数（函数调用的时候需要）
} FuncInfo;

/**
 * 储存函数的名字，以及指向它自身的信息的指针 等
 */
typedef struct Func
{
    FuncInfo *info;   // info table
    std::string name; // name
    WordType type;    // return type
} Func;

extern std::vector<Func *> funcs; // 储存所有函数

void parser(); // 总程序？遍历tokens，然后处理、生成汇编代码

int findVar(std::string name, Func *func, bool strict);                                   // 查找某个函数的变量，存在则直接返回栈指针，不存在则创建并返回指针
Func *findFunc(std::string name);                                                         // 查找某个函数，存在则返回指针，不存在则创建并返回指针
int findArg(std::string name, Func *func);                                                // 查找某个函数的参数，返回其在参数列表中的位置
int scanDefineVarNum(std::vector<word>::iterator start, std::vector<word>::iterator end); // 扫描定义变量个数
// void declareVar(Func* func, std::vector<word> words); //变量声明

void defineVars(Func *func, std::vector<word> words);                                                 // 变量定义（一句）
void defineVar(Func *func, std::vector<word> words);                                                  // 变量定义（一个）
void assignVar(Func *func, std::vector<word> words);                                                  // 变量赋值（一个变量）
void calcuExpression(Func *func, std::vector<word>::iterator start, std::vector<word>::iterator end); // 表达式计算
// void declareFunc(std::vector<word> words); //函数声明
void declareFunc(std::vector<word>::iterator start, std::vector<word>::iterator end, std::vector<arg> args); // 函数声明
void declareFunc(std::string fname, std::vector<arg> args, WordType ftype);
// void defineFunc(std::vector<word> words); //函数定义
void defineFunc(std::vector<word>::iterator start, std::vector<word>::iterator end, std::vector<arg> args); // 函数声明
// void parseFunc(std::vector<word>::iterator start, std::vector<word>::iterator end); //函数parse
void parseFunc(std::vector<word> words, Func *func, int while_label_index);                                 // 函数parse（一句一句）
std::vector<std::vector<word>> getArgs(std::vector<word>::iterator start, std::vector<word>::iterator end); // 在calcuExpression中调用，获取函数参数
int updataIter(std::vector<word>::iterator start, std::vector<word>::iterator end);                         // 更新iter，主要是配合上面的getarg使用的

// void callFunc(std::vector<word> words); //函数调用
void callFunc(std::vector<std::vector<word>> args, std::string fname, Func *fatherfunc);
void returnFunc(std::vector<word> words, Func *func); // return 语句
// std::vector<std::vector<word>> findComma(std::vector<word> words); //返回， 分割的内容（第一个逗号“，"前后的内容）
int findComma(std::vector<word> words); // 返回， 分割的内容（第一个逗
void parseWhile(std::vector<word> condition_words, std::vector<word> while_words, Func *func, int if_label_index, int while_label_index);
void continueFunc(int while_label_index);
void breakFunc(int while_label_index);
void parseFunc2(std::vector<word>::iterator start, std::vector<word>::iterator end, Func *func, int if_label_index, int while_label_index);
void parseWhile(std::vector<word> condition_words, std::vector<word> while_words, Func *func, int if_label_index, int while_label_index);
void parseIfElse(std::vector<word> condition_words, std::vector<word> if_words, std::vector<word> else_words, Func *func, int if_label_index, int while_label_index);
std::vector<std::vector<word>> findIfElse(std::vector<word>::iterator begin, std::vector<word>::iterator end);
std::vector<std::vector<word>> findIfElse(std::vector<word> words);
std::vector<std::vector<word>> findWhile(std::vector<word>::iterator begin, std::vector<word>::iterator end);
// void CallFunc(std::vector<word> words); //函数调用

// void CallPrint(); //内置print函数的调用

#endif // PARSER_H