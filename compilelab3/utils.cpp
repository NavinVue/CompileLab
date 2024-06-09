#include "utils.h"

std::map<std::string, int> operatorMap = {
    {"&&", 2},
    {"||", 1},
    {"&", 5},
    {"^", 4},
    {"|", 3},
    {"!=", 6},
    {"==", 6},
    {">=", 7},
    {"<=", 7},
    {">", 7},
    {"<", 7},
    {"+", 8},
    {"-", 8},
    {"–", 8},
    {"*", 9},
    {"/", 9},
    {"%", 9},
    {"!", 10},
    {"~", 10},
    {"minus", 10},
    {"(", 0},
    {")", 11}
    };

bool parser_needwarn = true;

/**
 * @brief 打印汇编代码前缀
 */
void print_prefix()
{
    std::cout << ".intel_syntax noprefix" << std::endl
              << ".global main" << std::endl
              << ".extern printf" << std::endl
              << std::endl
              << ".data" << std::endl
              << "format_str:" << std::endl
              << "  .asciz \"%d\\n\"" << std::endl
              << std::endl
              << ".text" << std::endl
              << std::endl;
    return;
}

/**
 * @brief 打印函数调用栈(结束leave ret)
 */
void print_postfix()
{
    std::cout << "  leave" << std::endl
              << "  ret" << std::endl
              << std::endl;
}
/**
 * @brief 打印tokens，检查是否分词错误
 */
void display_tokens(std::vector<word> words)
{
    std::vector<word>::iterator iter = words.begin();
    for (; iter != words.end(); iter++)
    {
        trans_wordtype((*iter).wtype);
        std::cout << " " << (*iter).name << std::endl;
    }
}

/**
 * @brief 重载的打印tokens，检查是否分词错误
 */
void display_tokens(std::vector<word>::iterator start, std::vector<word>::iterator end)
{
    std::vector<word>::iterator iter = start;
    for (; iter != end; iter++)
    {
        trans_wordtype((*iter).wtype);
        std::cout << " " << (*iter).name << std::endl;
    }
}

/**
 * @brief 打印/翻译wordtype
 */
void trans_wordtype(WordType wtype)
{
    switch (wtype)
    {
    case WordType::VAR:
        std::cout << "IDENTIFIER";
        break;
    case WordType::MAIN:
        std::cout << "MAIN";
        break;
    case WordType::OP:
        std::cout << "OP";
        break;
    case WordType::INT:
        std::cout << "INT";
        break;
    case WordType::VOID:
        std::cout << "VOID";
        break;
    case WordType::RETURN:
        std::cout << "RETURN";
        break;
    case WordType::LBRANCE:
        std::cout << "LBRANCE";
        break;
    case WordType::RBRANCE:
        std::cout << "RBRANCE";
        break;
    case WordType::LPARENTTHESIS:
        std::cout << "LPARENTTHESIS";
        break;
    case WordType::RPARENTTHESIS:
        std::cout << "RPARENTTHESIS";
        break;
    case WordType::SEMICOLON:
        std::cout << "SEMICOLON";
        break;
    case WordType::COMMA:
        std::cout << "COMMA";
        break;
    case WordType::ASSIGNMET:
        std::cout << "ASSIGNMET";
        break;
    case WordType::PRINT:
        std::cout << "PRINT";
        break;
    case WordType::NUMBER:
        std::cout << "NUMBER";
        break;
    case WordType::MINUS:
        std::cout << "MINUS";
        break;
    case WordType::IF:
        std::cout << "IF";
        break;
    case WordType::WHILE:
        std::cout << "WHILE";
        break;
    case WordType::ELSE:
        std::cout << "ELSE";
        break;
    case WordType::CONTINUE:
        std::cout << "CONTINUE";
        break;
    case WordType::BREAK:
        std::cout << "BREAK";
        break;
    default:
        std::cout << "ERROR";
        break;
    }
}

void trans_op(std::string op)
{
    if (op == "^") // 异或
    {
        std::cout << "  xor eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        // //numberstack.push(var{0, "#number"});
    }
    else if (op == "|") // 或
    {
        std::cout << "  or eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        // //numberstack.push(var{0, "#number"});
    }
    else if (op == "&") // 与
    {
        std::cout << "  and eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "==") // 判等
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  sete al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "!=") // 判不等
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  setne al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == ">") // 大于
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  setg al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "<") // 小于
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  setl al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == ">=") // 大于等于
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  setge al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "<=") // 小于等于
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  setle al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "+") // 加
    {
        std::cout << "  add eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "-" || op == "–") // 减
    {
        std::cout << "  sub eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "*") // 乘
    {
        std::cout << "  imul eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "/") // 除
    {
        std::cout << "  cdq" << std::endl
                  << "  idiv ebx" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "%") // 取余
    {
        std::cout << "  cdq" << std::endl
                  << "  idiv ebx" << std::endl
                  << "  push edx" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "&&") // 逻辑与
    {
        std::cout << "  test eax, eax" << std::endl
                  << "  setne al" << std::endl
                  << "  test ebx, ebx" << std::endl
                  << "  setne bl" << std::endl
                  << "  and al, bl" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "||") // 逻辑或
    {
        std::cout << "  test eax, eax" << std::endl
                  << "  setne al" << std::endl
                  << "  test ebx, ebx" << std::endl
                  << "  setne bl" << std::endl
                  << "  or al, bl" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "!") // 逻辑非
    {
        std::cout << "  cmp eax, 0" << std::endl
                  << "  sete al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
    else if (op == "~") // 按位取反
    {
        std::cout << "  not eax" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }else if (op=="minus"){
        std::cout << "  neg eax" << std::endl
                  << "  push eax" << std::endl;
        //numberstack.push(var{0, "#number"});
    }
}
