#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stack>

enum WordType
{
    MAIN = 0,
    RETUTN = 1,
    VAR = 2,
    OP = 3,
    ASSIGNMET = 4,
    FUNC = 5,
    NUMBER = 6,
    INT = 7,
    LPARENTTHESIS = 8, //(
    RPARENTTHESIS = 9, // )
    LBRANCE = 10,      //{
    RBRANCE = 11,      // }
    SEMICOLON = 12,    // ;
    COMMA = 13         //,
};

typedef struct Var
{
    int ptr;          // 栈指针
    std::string name; // 变量名
} var;

typedef struct Word
{
    WordType wtype;
    std::string name;
} word;

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
    {"(", 0},
    {")", 10}};

std::vector<var> vars;
int gptr = -4;
bool iftest = true;
bool ifdisplaywords = false;
bool runreturn = false;
bool needwarn = false;
std::stack<std::string> opstack;
std::stack<var> numberstack;

int findvar(std::string name);
void print_prefix();
void print_postfix();
void process(std::string contents);
std::vector<word> lexical_analysis(std::string line);
void translate(std::vector<word> words);
bool isChar(char ch);
bool isNum(char ch);
bool isSep(char ch);
bool endofWord(char ch);
void displaywords(std::vector<word> showwords);
void caculate(std::vector<word> words);
void trans_println(word inword);
void trans_op(std::string op);

int main(int argc, char *argv[])
{
    // read file
    std::ifstream fin(argv[1]);
    // fin.open(argv[1], std::ios::in);
    if (!fin.is_open())
    {
        std::cerr << "Fail to open file! File: " << argv[1] << std::endl;
        return 1;
    }
    std::stringstream buff;
    buff << fin.rdbuf();
    std::string contents = buff.str();
    fin.close();
    print_prefix();
    process(contents);
    print_postfix();
    return 0;
}

/*打印前缀，main之前的*/
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
              << "main:" << std::endl
              << "  push ebp" << std::endl
              << "  mov ebp, esp" << std::endl
              << "  sub esp, 0x100" << std::endl;
    return;
}

/*打印后缀，return之后的*/
void print_postfix()
{
    std::cout << "  leave" << std::endl
              << "  ret" << std::endl;
}

/*总控*/
void process(std::string contents)
{

    std::string sentence;
    std::vector<word> words;
    for (int i = 0; i < contents.size(); i++)
    {
        // std::cout << sentence << std::endl;
        sentence += contents[i];
        if (contents[i] == ';')
        {
            words = lexical_analysis(sentence);
            translate(words);
            // displaywords(words);
            // std::cout << "############################\n";
            sentence = "";
            words.clear();
            // break;
        }
        if (runreturn)
        {
            return;
        }
    }

    if (sentence != "" and !runreturn)
    {
        words = lexical_analysis(sentence);
        sentence = "";
        words.clear();
    }
}

/*把某行的内容进行词法分析，返回word结构变量{string::value, WordType::wt}并且删除了空格等分隔符*/
std::vector<word> lexical_analysis(std::string line)
{
    bool hasmain = false;
    // if (iftest)
    //     std::cout << line << std::endl;
    std::vector<word> words;

    for (int i = 0; i < line.size(); i++)
    {
        if (isSep(line[i]))
        {
            continue;
        }
        else if (line[i] == ',')
        {
            words.push_back(word{WordType::COMMA, std::string(1, line[i])});
        }
        else if (line[i] == ';')
        {
            words.push_back({WordType::SEMICOLON, ";"});
        }
        else if (line[i] == '+' or line[i] == '-' or line[i] == '*' or line[i] == '/' or line[i] == '%' or line[i] == '–' or line[i] == '^')
        { /* +-/*% & | ^*/
            words.push_back({WordType::OP, std::string(1, line[i])});
        }
        else if (line[i] == '&')
        {
            if ((i + 1) < line.size() and line[i + 1] == '&')
            {
                words.push_back({WordType::OP, "&&"});
                i++;
            }
            else
            {
                words.push_back({WordType::OP, "&"});
            }
        }
        else if (line[i] == '|')
        {
            if (i + 1 < line.size() and line[i + 1] == '|')
            {
                words.push_back({WordType::OP, "||"});
                i++;
            }
            else
            {
                words.push_back({WordType::OP, "|"});
            }
        }
        else if (line[i] == '<' or line[i] == '>' or line[i] == '=' or line[i] == '!')
        {
            if ((i + 1) < line.size() and line[i + 1] == '=')
            {
                words.push_back({WordType::OP, std::string(1, line[i]) + "="});
                i++;
            }
            else
            {
                if (line[i] == '=')
                {
                    words.push_back({WordType::ASSIGNMET, std::string(1, line[i])});
                }

                else
                    words.push_back({WordType::OP, std::string(1, line[i])});
            }
        }
        else if (line[i] == '{' or line[i] == '}' or line[i] == '(' or line[i] == ')')
        {
            if (line[i] == '(')
                words.push_back({WordType::LPARENTTHESIS, std::string(1, line[i])});
            else if (line[i] == ')')
                words.push_back({WordType::RPARENTTHESIS, std::string(1, line[i])});
            else if (line[i] == '{')
                words.push_back({WordType::LBRANCE, std::string(1, line[i])});
            else if (line[i] == '}')
                words.push_back({WordType::RBRANCE, std::string(1, line[i])});
        }
        else if (isNum(line[i]))
        {
            std::string number = "";
            while (i < line.size())
            {
                if (!isNum(line[i]))
                {
                    i--;
                    break;
                }
                number += line[i];
                i++;
            }
            words.push_back({WordType::NUMBER, number});
        }
        else if (isChar(line[i]))
        {
            std::string tmp = "";
            while (i < line.size())
            {
                if (endofWord(line[i]))
                {
                    i--;
                    break;
                }
                tmp += line[i];
                i++;
            }
            // 得到一个word，可能是变量，也可能是关键字
            // 开始辨别是否为关键字或者是变量
            if (tmp == "int")
            {
                words.push_back({WordType::INT, tmp});
            }

            else if (tmp == "main")
            {
                words.push_back({WordType::MAIN, tmp});
                hasmain = true;
            }
            else if (tmp == "return")
            {
                words.push_back({WordType::RETUTN, tmp});
            }

            else if (tmp == "println_int")
            {
                words.push_back({WordType::FUNC, tmp});
            }

            else
            {
                words.push_back({WordType::VAR, tmp});
            }
        }
        else
        {
            std::cout << "Error NonType: " << line[i] << std::endl;
        }
    }

    if (hasmain)
    {
        bool beforemain = true;
        std::vector<word> truewords;
        for (auto wordi : words)
        {
            if (wordi.wtype == WordType::MAIN)
            {
                beforemain = false;
            }
            if (beforemain)
            {
                ;
            }
            else
            {
                if (wordi.wtype == WordType::INT || wordi.wtype == WordType::VAR || wordi.wtype == WordType::SEMICOLON)
                {
                    truewords.push_back(wordi);
                }
            }
        }
        words = truewords;
    }
    if (ifdisplaywords)
    {
        displaywords(words);
    }
    return words;
}

/*执行汇编翻译*/
void translate(std::vector<word> words)
{
    /*打印所翻译的语句方便查错*/
    std::cout << "# ";
    for (auto wordi : words)
    {
        std::cout << wordi.name << " ";
    }
    std::cout << std::endl;

    // 主函数int main(int argc, int argv){... return xx;}如何处理？
    std::vector<word>::iterator iter = words.begin();
    for (; iter != words.end(); iter++)
    {
        if ((*iter).wtype == WordType::INT) // declair int var
        {
            iter++; // 其实没有边界判断，有待完善
            if ((*iter).wtype != WordType::VAR and needwarn)
            {
                std::cout << "Error! After int without var:\n";
                displaywords(words);
                exit(-1);
                // return;
            }
            int vptr = findvar((*iter).name);
        }
        else if ((*iter).wtype == WordType::SEMICOLON) // ; end a sentence
        {
            break;
        }
        else if ((*iter).wtype == WordType::VAR) // var = ，以变量名开头的语句只能是赋值语句
        {
            caculate(std::vector<word>(iter, words.end()));
            break;
        }
        else if ((*iter).wtype == WordType::FUNC)
        {
            iter++;
            if ((*iter).wtype != WordType::LPARENTTHESIS and needwarn)
            {
                std::cout << "Error, println_int without '(':\n";
                displaywords(words);
                exit(-1);
            }
            iter++;
            if ((*iter).wtype == WordType::VAR or (*iter).wtype == WordType::NUMBER)
            {
                trans_println(*iter);
            }
            else
            {
                if (needwarn)
                {
                    std::cout << "Error, println_int(?) without a number or a variable:\n";
                    displaywords(words);
                    exit(-1);
                }
            }
            iter++;
            if ((*iter).wtype != WordType::RPARENTTHESIS and needwarn)
            {
                std::cout << "Error, println_int(a without ')':\n";
                displaywords(words);
                exit(-1);
            }
            break; // 其实可以直接break，如果用例没错且程序没写错的话，println_int(a)后面就是跟着
            /*下面内容有些多余*/
            // iter++;
            // if (((*iter).wtype != WordType::SEMICOLON or iter != words.end()) and needwarn)
            // {
            //     std::cout << "Error, println_int(a) not followed by ';' :\n";
            //     displaywords(words);
            //     exit(-1);
            // }
            // break;
        }
        else if ((*iter).wtype == WordType::RETUTN)
        {
            runreturn = true;
            iter++;
            if ((*iter).wtype == WordType::NUMBER)
            {

                std::cout << "  mov eax, " << (*iter).name << std::endl;
            }
            else if ((*iter).wtype == WordType::VAR)
            {
                std::cout << "  mov eax, [ebp" << findvar((*iter).name) << "]" << std::endl;
            }
            else
            {
                if (needwarn)
                {
                    std::cout << "Error, return not followed by var or number:\n";
                    displaywords(words);
                    exit(-1);
                }
            }
        }
        else
        {
            ;
        }
    }
}

void caculate(std::vector<word> words)
{
    while (!numberstack.empty())
    {
        numberstack.pop();
    }
    while (!opstack.empty())
    {
        opstack.pop();
    }
    std::vector<word>::iterator iter = words.begin();
    int var_ptr = findvar((*iter).name);
    iter++; // 如果var后没有跟=也即type=assignment的word，也即出现了问题
    if ((*iter).wtype != WordType::ASSIGNMET and needwarn)
    {
        std::cout << "Error: start with var but not followed by assignment = :\n";
        displaywords(words);
        exit(-1);
    }

    /*开始计算表达式的值了，同样是使用中缀计算法*/

    iter++;
    WordType before_type = WordType::LPARENTTHESIS;
    for (; iter != words.end(); iter++)
    {
        if ((*iter).wtype == WordType::SEMICOLON) // ; 不处理
        {
            ;
        }
        else if ((*iter).wtype == WordType::NUMBER) // 类型为number，直接入栈（打印）
        {
            numberstack.push(var{0, "#number"});
            std::cout << "  mov eax, " << (*iter).name << std::endl
                      << "  push eax" << std::endl;
        }
        else if ((*iter).wtype == WordType::VAR) // 类型为var,也即变量，需要先取值再入栈
        {
            int varptr = findvar((*iter).name);
            numberstack.push(var{varptr, (*iter).name});
            std::cout << "  mov eax, DWORD PTR [ebp" << varptr << "]" << std::endl
                      << "  push eax" << std::endl;
        }
        else if ((*iter).wtype == WordType::OP || (*iter).wtype == WordType::LPARENTTHESIS || (*iter).wtype == WordType::RPARENTTHESIS)
        {
            std::cout << "  # now is symbol : " << (*iter).name << std::endl;
            if (opstack.empty()) // 栈空，直接入栈
            {
                if (before_type == WordType::LPARENTTHESIS && ((*iter).name == "-" || (*iter).name == "–"))
                {
                    std::cout << "# do somethings" << std::endl;
                    std::cout << "  mov eax, 0" << std::endl
                              << "  push eax" << std::endl;
                    numberstack.push({0, "#number"});
                }
                opstack.push((*iter).name);
            }
            else // 栈非空，比较优先级
            {
                std::string top_op = opstack.top();
                if ((*iter).name == "(")
                {
                    opstack.push("(");
                }
                else if ((*iter).name == ")")
                {
                    std::string tmp_op = "";
                    while (1)
                    {

                        tmp_op = opstack.top();
                        opstack.pop();
                        if (tmp_op == "(")
                            break; // 结束循环，"(" 与 ")"匹配了
                        var var2 = numberstack.top();
                        numberstack.pop();
                        var var1 = numberstack.top();
                        numberstack.pop();
                        std::cout << "  pop ebx" << std::endl
                                  << "  pop eax" << std::endl;
                        trans_op(tmp_op);
                    }
                }
                else if (operatorMap[(*iter).name] > operatorMap[top_op]) // 优先级更高，直接入栈
                {
                    if (before_type == WordType::LPARENTTHESIS && ((*iter).name == "-" || (*iter).name == "–"))
                    {
                        std::cout << "# do somethings" << std::endl;
                        std::cout << "  mov eax, 0" << std::endl
                                  << "  push eax" << std::endl;
                        numberstack.push({0, "#number"});
                    }
                    opstack.push((*iter).name);
                }
                else // 优先级更低，先出栈再入栈
                {
                    while (1)
                    {
                        if (opstack.empty())
                        {
                            break;
                        }
                        top_op = opstack.top();
                        if (operatorMap[top_op] < operatorMap[(*iter).name])
                        {
                            break;
                        }
                        opstack.pop();
                        var var2 = numberstack.top();
                        numberstack.pop();
                        var var1 = numberstack.top();
                        numberstack.pop();
                        std::cout << "  pop ebx" << std::endl
                                  << "  pop eax" << std::endl;
                        trans_op(top_op);
                    }
                    opstack.push((*iter).name);
                }
            }
        }
        before_type = (*iter).wtype;
    }
    // 计算完表达式，最后将栈中的内容全部出栈，并且赋值
    while (!opstack.empty())
    {
        std::string op = opstack.top();
        opstack.pop();
        var var2 = numberstack.top();
        numberstack.pop();
        var var1 = numberstack.top();
        numberstack.pop();
        std::cout << "  pop ebx" << std::endl;
        std::cout << "  pop eax" << std::endl;
        trans_op(op);
    }
    // if (numberstack.size() != 1 and needwarn)
    // {
    //     std::cout << "Error: Stack is not empty!" << std::endl;
    //     exit(-1);
    // }
    std::cout << "  # assignment" << std::endl
              << "  pop eax" << std::endl
              << "  mov DWORD PTR [ebp" << var_ptr << "], eax" << std::endl;
}

/* 根据不同的符号输出不同的汇编代码 */
void trans_op(std::string op)
{
    if (op == "^") // 异或
    {
        std::cout << "  xor eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == "|") // 或
    {
        std::cout << "  or eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == "&") // 与
    {
        std::cout << "  and eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == "==") // 判等
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  sete al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == "!=") // 判不等
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  setne al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == ">") // 大于
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  setg al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == "<") // 小于
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  setl al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == ">=") // 大于等于
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  setge al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == "<=") // 小于等于
    {
        std::cout << "  cmp eax, ebx" << std::endl
                  << "  setle al" << std::endl
                  << "  movzx eax, al" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == "+") // 加
    {
        std::cout << "  add eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == "-" || op == "–") // 减
    {
        std::cout << "  sub eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == "*") // 乘
    {
        std::cout << "  imul eax, ebx" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == "/") // 除
    {
        std::cout << "  cdq" << std::endl
                  << "  idiv ebx" << std::endl
                  << "  push eax" << std::endl;
        numberstack.push(var{0, "#number"});
    }
    else if (op == "%") // 取余
    {
        std::cout << "  cdq" << std::endl
                  << "  idiv ebx" << std::endl
                  << "  push edx" << std::endl;
        numberstack.push(var{0, "#number"});
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
        numberstack.push(var{0, "#number"});
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
        numberstack.push(var{0, "#number"});
    }
}

/*translate printl_int*/
void trans_println(word inword)
{
    if (inword.wtype == WordType::VAR)
    {
        int varptr = findvar(inword.name);
        std::cout << "  push DWORD PTR [ebp" << varptr << "]" << std::endl;
    }
    else
    {
        std::cout << "  mov eax, " << inword.name << std::endl
                  << "  push eax" << std::endl;
    }
    std::cout << "  push offset format_str" << std::endl
              << "  call printf" << std::endl
              << "  add esp, 8" << std::endl;
}

/*根据变量名查找变量的栈指针，如果没找到则new一个*/
int findvar(std::string name)
{
    std::vector<var>::iterator iter = vars.begin();
    for (; iter != vars.end(); iter++)
    {
        if ((*iter).name == name)
        {
            return (*iter).ptr;
        }
    }

    vars.push_back(var{gptr, name});
    std::cout << "  mov DWORD PTR [ebp" << gptr << "], 0 # int " << name << std::endl;
    gptr -= 4;
    return (gptr + 4);
}

bool isChar(char ch)
{
    if (ch >= '0' && ch <= '9')
        return false;
    else
        return true;
}

/*是否为数字>='0' <='9'*/
bool isNum(char ch)
{
    if (ch >= '0' && ch <= '9')
        return true;
    else
        return false;
}

/*是否为分隔符 空格，回车*/
bool isSep(char ch)
{
    if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r')
        return true;
    else
        return false;
}

/*某个单词是否结束，遇到空格、回车、运算符、括号*/
bool endofWord(char ch)
{
    if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '^' || ch == '|' || ch == '&' || ch == '=' || ch == '–' || ch == '>' || ch == '<' || ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' || ch == '{' || ch == '}' || ch == '(' || ch == ')' || ch == ';' || ch == ',' || ch == '!')
        return true;
    else
        return false;
}

void displaywords(std::vector<word> showwords)
{
    std::vector<word>::iterator iter = showwords.begin();
    for (; iter != showwords.end(); iter++)
    {
        std::cout << (*iter).name << "     " << (*iter).wtype << std::endl;
    }
}

// void displaywordsshort(std::vector<word> showwords){

// }
//