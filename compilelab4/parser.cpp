#include "parser.h"
std::vector<Func *> funcs;
int g_if_label=0;
int g_while_label=0;
/**
 * @brief 解析tokens，识别出某些tokens在干什么，然后传入相应tokens给对应的函数
 * @return void
 * @note 由于lab3不涉及全局变量的定义、include、define等，也即parse这里只需要识别出函数声明、定义即可
 * @todo lab4可能需要修改分割函数部分（{}得配对）
 */
void parser()
{
    print_prefix();
    // std::vector<std::vector<word>>::iterator iter = tokens.begin();
    std::vector<word>::iterator iter = tokens1.begin();
    std::vector<word>::iterator last_iter = iter;
    // std::vector<word> t_tokens;
    std::vector<arg> args; // 免得再处理一次
    std::string funcname;
    // 提前注册println_int函数
    std::vector<arg> tmp;
    tmp.push_back(arg({WordType::INT, word{WordType::VAR, "d"}}));
    declareFunc("println_int", tmp, WordType::VOID);
    for (; iter != tokens1.end(); iter++)
    { // undo 未考虑错误情况！未考虑全局变量定义等！只考虑了函数定义、声明
        if (((*iter).wtype == WordType::INT or (*iter).wtype == WordType::VOID) and ((*(iter + 1)).wtype == VAR or (*(iter + 1)).wtype == MAIN) and (*(iter + 2)).wtype == LPARENTTHESIS)
        {
            // funcname = (*(iter + 1)).name;
            std::vector<word>::iterator t_iter = iter + 3; // skip type funcname, now iter=="("
            std::vector<word>::iterator t_last = t_iter;
            arg t_arg;
            bool flag = false;
            while ((*t_iter).wtype != WordType::RPARENTTHESIS) // find right )
            {

                if ((*t_iter).wtype == WordType::INT)
                {
                    t_arg.argtype = WordType::INT;
                    // undo 其实没有错误处理
                }
                else if ((*t_iter).wtype == WordType::COMMA)
                {
                    args.push_back(t_arg);
                }
                else if ((*t_iter).wtype == WordType::VAR)
                {
                    t_arg.argword = *(t_iter);
                    // t_tokens.push_back(*t_iter);
                    flag = true;
                }
                else
                {
                    ; // undo 其实没有错误处理
                }
                t_iter++;
            }
            if (flag) // 由于最后一个参数没有逗号，所以需要单独push
            {
                args.push_back(t_arg);
            }
            t_iter++;                                     // skip ")"
            if ((*(t_iter)).wtype == WordType::SEMICOLON) // func declare or func define
            {                                             // func declare
                declareFunc(iter, t_iter, args);          // iter=type , t_iter=";"
                args.clear();
                iter = t_iter;
                last_iter = iter;
            }
            else
            { // func define
                if ((*t_iter).wtype != WordType::LBRANCE)
                {
                    if (parser_needwarn)
                    {
                        std::cout << "ERROR not belong to func define or not belong to func declare" << std::endl;
                    }
                }
                else
                {
                    int lbrnum = 0;
                    bool flag = false;
                    while (flag == false or lbrnum != 0)
                    {
                        if ((*t_iter).wtype == WordType::RBRANCE)
                        {
                            lbrnum--;
                        }
                        if ((*t_iter).wtype == WordType::LBRANCE)
                        {
                            lbrnum++;
                            flag = true;
                        }
                        t_iter++;
                    }
                    t_iter--;
                    // std::cout << "xixi "<< (*iter).name << std::endl;//to delete
                    defineFunc(iter, t_iter, args); // iter starts from int/void ,t_iter ends with "}"
                    iter = t_iter;
                    last_iter = iter;
                    args.clear();
                }
            }
        }
        else if ((*iter).wtype == WordType::SEMICOLON)
        { // maybe concerned to much（single ";"--meaningless）
            ;
        }
        else
        {
            std::cout << "not start with int or void" << std::endl;
        }
    }
}

/**
 * @brief 查找某个变量是否在某个函数中存在，如果存在返回栈指针，反之添加该变量到该函数中
 * @param name 变量名
 * @param func 函数指针
 * @return 栈指针(int)
 */
int findVar(std::string name, Func *func, bool strict = false)
{
    std::vector<var>::iterator iter = func->info->vars.begin();
    for (; iter != func->info->vars.end(); iter++)
    {
        if ((*iter).name == name)
        {
            return (*iter).ptr;
        }
    }
    if (strict)
    {
        return 1;
    }

    func->info->vars.push_back(var{func->info->fptr, name});
    std::cout << "  mov DWORD PTR [ebp" << func->info->fptr << "], 0 # int " << name << std::endl;
    func->info->fptr -= 4;
    return (func->info->fptr + 4);
}

/**
 * @brief 查找某个变量是否在参数列表中，返回其所在的位置，不存在则返回0
 * @param name 要查找的变量名
 * @param func 函数指针
 */
int findArg(std::string name, Func *func)
{
    auto iter = func->info->args.begin();
    int count = 1;
    for (; iter != func->info->args.end(); iter++)
    {
        // std::cout << "  count: " << count << " name: " << name << "arg: " << (*iter).argword.name << std::endl;
        if ((*iter).argword.name == name)
        {
            return count;
        }
        else
        {
            count++;
        }
    }
    return 0;
}

/**
 * @brief 查找某个函数是否存在，如果存在返回函数指针，反之添加该函数到函数列表中
 */
Func *findFunc(std::string name)
{
    std::vector<Func *>::iterator iter = funcs.begin();
    for (; iter != funcs.end(); iter++)
    {
        if (name == (*iter)->name)
        {
            return *iter;
        }
    }
    // exit(-2);
    return nullptr;

    // Func *f = new Func();
    // f->name = name;
    // funcs.push_back(f);
    // return f;
}

/**
 * @brief 扫描某个函数定义的变量的个数
 * @param start, end 函数定义起始和结束迭代器（直接skip完了(args)和"{"）
 * @return 定义的变量（int）的个数
 * @note 类似下面的getArgs函数(考虑到int a=b+c, d=add(a,b)这种,所以不能够直接数逗号)
 */
int scanDefineVarNum(std::vector<word>::iterator start, std::vector<word>::iterator end)
{
    int count = 0;
    auto iter = start;
    // std::vector<word> tmp;
    for (; iter != end; iter++)
    {
        if ((*iter).wtype == WordType::INT)
        {
            int lpa = 0;
            while ((*iter).wtype != WordType::SEMICOLON)
            {
                if (WordType::COMMA == (*iter).wtype and lpa == 0)
                {
                    count++;
                }
                else if (WordType::LPARENTTHESIS == (*iter).wtype)
                {
                    lpa++;
                }
                else if (WordType::RPARENTTHESIS == (*iter).wtype)
                {
                    lpa--;
                }
                iter++;
            }
            count++; // 最后一个无，
        }
        else
        {
            ;
        }
    }
    return count;
}

/**
 * @brief 函数声明
 * @param start 函数声明起始迭代器(包括type, fname)
 * @param end 函数声明结束迭代器(截至到";"-->依据传入的数据，但是实际上并不需要,在parse中传的数据会到;)
 * @param args 参数列表
 */
void declareFunc(std::vector<word>::iterator start, std::vector<word>::iterator end, std::vector<arg> args)
{
    Func *f = new Func();
    f->info = new FuncInfo();
    f->type = (*(start)).wtype;
    start++;
    f->name = (*(start)).name;
    f->info->fptr = -4;
    f->info->args = args;
    f->info->argnum = args.size();
    funcs.push_back(f);
}

/**
 * @brief 函数声明（主要是针对println_int）
 * @param fname 函数名
 * @param args 参数列表
 * @param ftype 函数类型
 * @note 函数声明，针对println_int(内置函数);没有内容打印（不同于自定义函数）
 */
void declareFunc(std::string fname, std::vector<arg> args, WordType ftype)
{
    Func *f = new Func();
    f->info = new FuncInfo();
    f->type = ftype;
    f->name = fname;
    f->info->fptr = -4;
    f->info->args = args;
    f->info->argnum = args.size();
    funcs.push_back(f);
}

/**
 * @brief 函数定义，包括函数声明和函数内容，函数内容以句子为单位进行parse（不包括;）
 * @param start 函数声明起始迭代器(包括fname)
 * @param end 函数声明结束迭代器(截至到"}")
 * @param args 参数列表
 */
void defineFunc(std::vector<word>::iterator start, std::vector<word>::iterator end, std::vector<arg> args)
{
    std::cout << "  # define func: " << (*(start)).name << std::endl;
    // start init func
    Func *f = new Func();
    f->info = new FuncInfo();
    f->type = (*(start)).wtype;
    start++;
    f->name = (*(start)).name;
    f->info->fptr = -4;
    f->info->args = args;
    f->info->argnum = args.size();
    funcs.push_back(f);
    // end init func
    while ((*start).wtype != WordType::LBRANCE) // skip args, 因为已经提前处理过了
    {
        start++;
    }
    start++; // skip {
    int defineVarNum = scanDefineVarNum(start, end);
    std::cout << f->name << ": #func " << f->name << std::endl
              << "  push ebp # keep ebp" << std::endl
              << "  mov ebp, esp # keep esp" << std::endl
              << "  sub esp, " << 4 * (defineVarNum + 1) << "# leave space for local var, define var num: " << defineVarNum << std::endl;
    std::vector<word> words;
    std::vector<word> t_words;
    if (0)
    { // 被 parseFunc2 替代
      // for (std::vector<word>::iterator iter = start; iter != end; iter++) // devide into by ;, process a sentence once
      // {
      //     if ((*iter).wtype == WordType::IF)
      //     {
      //         // to do
      //         auto ifelsewords = findIfElse(iter, end);
      //         int iter_count = std::stoi(ifelsewords[0][0].name);
      //         iter += iter_count;
      //         iter--; // because will do iter++
      //         // parseIfElse();
      //     }
      //     if ((*iter).wtype == WordType::WHILE)
      //     {
      //         // to do
      //         auto whilesentence = findWhile(iter, end);
      //         int iter_count = std::stoi(whilesentence[0][0].name);
      //         iter += iter_count;
      //         iter--;
      //         // parseWhile();
      //     }

        //     while ((*iter).wtype != WordType::SEMICOLON)
        //     {
        //         words.push_back(*iter);
        //         iter++;
        //     }
        //     if (!words.empty())
        //     {

        //         // parseFunc(words, f, 0);
        //     }

        //     words.clear();
        // }
    }
    std::cout << "  # recongize func body, in function defineFunc" << std::endl;
    parseFunc2(start, end, f, g_if_label, g_while_label);
    print_postfix();
}

/**
 * @brief 主要是copy自defineFunc，但是删去了打印部分、init Func部分
 * @brief 主要是while和if else的body部分会调用 -- 其实类似一个函数只不过没有参数之类的
 * @brief 这个函数主要是识别if else、while 或者 ； 然后调用parseFunc函数
 * @param start, end 函数内容起始和结束迭代器
 * @param func 函数指针，指向当前正在处理的函数
 * @
 */
void parseFunc2(std::vector<word>::iterator start, std::vector<word>::iterator end, Func *func, int if_label_index, int while_label_index)
{
    std::vector<word> words;
    auto iter = start;
    bool flag = false;
    while (iter != end)
    {
        // 1. judge if else
        // 2. judge while
        // 3. judge ;
        if ((*iter).wtype == WordType::IF)
        {
            std::cout << "  # recongize if" << std::endl;                        // to delete
            std::cout << "  # now if label is: " << if_label_index << std::endl; // to delete
            auto ifelsewords = findIfElse(iter, end);
            int iter_count = std::stoi(ifelsewords[0][0].name);
            iter += iter_count;
            // parse ifelse
            // std::cout << "!!!\n"; // to delete
            // std::cout << "!!!length tokens\n";
            // display_tokens(ifelsewords[0]);
            // std::cout << "!!!condition tokens\n";
            // display_tokens(ifelsewords[1]);
            // std::cout << "!!!if tokens\n";
            // display_tokens(ifelsewords[2]);
            // std::cout << "!!!else tokens\n";
            // display_tokens(ifelsewords[3]);
            // std::cout << "!!! end of check\n";// end to delete
            if_label_index = if_label_index + 1;
            g_if_label+=1;
            parseIfElse(ifelsewords[1], ifelsewords[2], ifelsewords[3], func, if_label_index, while_label_index);
        }
        else if ((*iter).wtype == WordType::WHILE)
        {
            std::cout << "  # recongize while" << std::endl; // to delete
            auto whilesentence = findWhile(iter, end);
            int iter_count = std::stoi(whilesentence[0][0].name);
            iter += iter_count;
            while_label_index = while_label_index + 1;
            g_while_label+=1;
            parseWhile(whilesentence[1], whilesentence[2], func, if_label_index, while_label_index);
        }
        else
        {
            // std::cout << "??" << (iter==end) << std::endl; // to delete
            // std::cout << "..." << (iter==(end-1)) << std::endl;// to delete
            if (iter == end - 1 and (*iter).wtype == WordType::RBRANCE)
            {
                return;
                // break;
            }
            std::cout << "  # recongize neigther if nor while" << std::endl;
            // read until meet ;
            while ((*iter).wtype != WordType::SEMICOLON)
            {
                words.push_back(*iter);
                iter++;
            }
            iter++; // skip ;
            parseFunc(words, func, while_label_index);
            words.clear();
        }
    }
    if (!words.empty())
    { // impossible case?
        std::cout << " # not empty???" << std::endl;
        parseFunc(words, func, while_label_index);
    }
}

/**
 * @brief 找到if else，并返回其vector(二维，0为iter计数, 1为condition, 2为if, 3为else)
 * @param words 待寻找的vector，来自处理函数的内部，也可能是if else、while里面的，总而言之是来自{}里面的
 * @return ifelse包括的vector
 * @details 首先判断是不是if开头的，然后再处理bool表达式，之后再看一下是否有{},\\
 *  之后再判断有没有else，有的话就加到else的vector里面去，之后return，然后返回。可以供函数
 * @note return 的if words，else words 都是含分号的，condition words不含分号，直接调用calcuExpression即可
 */
std::vector<std::vector<word>> findIfElse(std::vector<word> words)
{
    // word count={WordType::INT, "count"};
    std::vector<std::vector<word>> ifelse;
    std::vector<word> condition_words;
    std::vector<word> if_words;
    std::vector<word> else_words;
    auto iter = words.begin();
    if (words.empty())
    {
        std::cout << "  # empty" << std::endl;
        return ifelse;
    }
    if ((*iter).wtype != WordType::IF)
    {
        std::cout << "  # not start with if" << std::endl;
        return ifelse;
    }
    else
    {
        iter++; // skip if
    }
    // 1st, get bool expression (condition words)
    int lpanum = 0; // (
    bool flag = false;
    while (flag == false or lpanum != 0)
    {
        if ((*iter).wtype == WordType::LPARENTTHESIS)
        {
            lpanum++;
            flag = true;
        }
        else if ((*iter).wtype == WordType::RPARENTTHESIS)
        {
            lpanum--;
        }
        condition_words.push_back(*iter);
        iter++;
    }
    iter++;                                         // skip )
    condition_words.pop_back();                     // remove last )
    condition_words.erase(condition_words.begin()); // remove first (
    // 2nd get if words
    flag = false;
    int lbrnum = 0;
    if ((*iter).wtype == WordType::LBRANCE)
    {
        while (lbrnum != 0 or flag == false)
        {
            if ((*iter).wtype == WordType::LBRANCE)
            {
                lbrnum++;
                flag = true;
            }
            else
            {
                lbrnum--;
            }
            if_words.push_back(*iter);
            iter++;
        }
        // remove {}
        if_words.pop_back();              // pop }
        if_words.erase(if_words.begin()); // pop {
    }
    else
    {
        // read until SEMICOLON ';'
        while ((*iter).wtype != WordType::SEMICOLON)
        {
            if_words.push_back(*iter);
            iter++;
        }
    }
    iter++; // skip ; or skip }
    // 3. judge whether else follows
    if ((*iter).wtype == WordType::ELSE)
    {
        // do what if does except condition words
        lbrnum = 0; // not necessary, the end loop condition is lbrnum==0
        flag = false;
        if ((*iter).wtype == WordType::LBRANCE)
        {
            while (lbrnum != 0 or flag == false)
            {
                if ((*iter).wtype == WordType::LBRANCE)
                {
                    lbrnum++;
                    flag = true;
                }
                else
                {
                    lbrnum--;
                }
                else_words.push_back(*iter);
                iter++;
            }
            // remove {}
            else_words.pop_back();                // pop }
            else_words.erase(else_words.begin()); // pop {
        }
        else
        {
            // read until SEMICOLON ';'
            while ((*iter).wtype != WordType::SEMICOLON)
            {
                else_words.push_back(*iter);
                iter++;
            }
        }
        iter++; // skip ; or skip }
    }

    // 4. return vectors
    int tmp = iter - words.begin();
    ifelse.push_back(std::vector<word>{word{WordType::INT, std::to_string(tmp)}}); // 1. push count
    ifelse.push_back(condition_words);                                             // 2. push condition vector
    ifelse.push_back(if_words);                                                    // 3. push if vector
    ifelse.push_back(else_words);                                                  // 4. push else vector
    return ifelse;
}

/**
 * @brief reload findIFELSE, input is iterator
 * @param begin the begin iterator, most case it should be WordType::IF
 */
std::vector<std::vector<word>> findIfElse(std::vector<word>::iterator begin, std::vector<word>::iterator end)
{
    // word count={WordType::INT, "count"};
    std::vector<std::vector<word>> ifelse;
    std::vector<word> condition_words;
    std::vector<word> if_words;
    std::vector<word> else_words;
    // auto iter = words.begin();
    auto iter = begin;
    if (begin == end)
    {
        // std::cout << "  # empty, if else" << std::endl; // to delete
        return ifelse;
    }
    if ((*iter).wtype != WordType::IF)
    {
        // std::cout << "  # not start with if" << std::endl; // to delete
        return ifelse;
    }
    else
    {
        iter++; // skip if
    }
    // 1st, get bool expression (condition words)
    int lpanum = 0; // (
    bool flag = false;
    while (flag == false or lpanum != 0)
    {
        if ((*iter).wtype == WordType::LPARENTTHESIS)
        {
            lpanum++;
            flag = true;
        }
        else if ((*iter).wtype == WordType::RPARENTTHESIS)
        {
            lpanum--;
        }
        condition_words.push_back(*iter);
        iter++;
    }
    // iter++;                                         // skip ) // already skipped
    condition_words.pop_back();                     // remove last )
    condition_words.erase(condition_words.begin()); // remove first (
    // 2nd get if words
    flag = false;
    int lbrnum = 0;
    if ((*iter).wtype == WordType::LBRANCE)
    {

        // std::cout << "  # {} follows behind if\n"; // to delete
        while (lbrnum != 0 or flag == false)
        {
            // std::cout << "  ## so? " << (*iter).name << "flag:" << flag << " lbrnum: " << lbrnum << std::endl; // to delete
            if ((*iter).wtype == WordType::LBRANCE)
            {
                lbrnum++;
                flag = true;
            }
            else if ((*iter).wtype == WordType::RBRANCE)
            {
                lbrnum--;
            }
            if_words.push_back(*iter);
            iter++;
        }
        // remove {}
        if_words.pop_back();              // pop }
        if_words.erase(if_words.begin()); // pop {
    }
    else
    {
        // std::cout << "  # no {} follows behind if\n"; // to delete
        // read until SEMICOLON ';'
        while ((*iter).wtype != WordType::SEMICOLON)
        {
            if_words.push_back(*iter);
            iter++;
        }
        iter++;
    }
    // iter++; // skip ; or skip } // already skipped
    // 3. judge whether else follows
    if ((*iter).wtype == WordType::ELSE)
    {
        iter++;
        // do what if does except condition words
        lbrnum = 0; // not necessary, the end loop condition is lbrnum==0
        flag = false;
        if ((*iter).wtype == WordType::LBRANCE)
        {
            std::cout << "  # else followed by {" << std::endl; // to delete
            while (lbrnum != 0 or flag == false)
            {
                if ((*iter).wtype == WordType::LBRANCE)
                {
                    lbrnum++;
                    flag = true;
                }
                else if ((*iter).wtype == WordType::RBRANCE)
                {
                    lbrnum--;
                }
                else_words.push_back(*iter);
                iter++;
            }
            // remove {}
            else_words.pop_back();                // pop }
            else_words.erase(else_words.begin()); // pop {
        }
        else
        {
            std::cout << "  # else not followed by {}" << ", but followed by" << (*iter).name << std::endl; // to delete
            // read until SEMICOLON ';'
            while ((*iter).wtype != WordType::SEMICOLON)
            {
                else_words.push_back(*iter);
                iter++;
            }
            iter++;
        }
        // iter++; // skip ; or skip } // already skipped
    }

    std::cout << "  # IN FUNC findifelse\n"; // to delete
    // display_tokens(if_words); // to delete
    std::cout << "  # END func findifelse\n"; // to delete
    // 4. return vectors
    int tmp = iter - begin;
    ifelse.push_back(std::vector<word>{word{WordType::INT, std::to_string(tmp)}}); // 1. push count
    ifelse.push_back(condition_words);                                             // 2. push condition vector
    ifelse.push_back(if_words);                                                    // 3. push if vector
    ifelse.push_back(else_words);                                                  // 4. push else vector
    return ifelse;
}

/**
 * @brief 找到while，并返回其vector
 * @param words 待寻找的vector的迭代器
 * @return while包括的vector
 * @note 返回的vector，首先存的是长度（iter走了多少），然后才是condition语句(bool expression),然后是while里面的主要语句
 * @note 注意while body里面仍然没有去掉 ; (SEMICOLON)
 */
std::vector<std::vector<word>> findWhile(std::vector<word>::iterator begin, std::vector<word>::iterator end)
{
    auto iter = begin;
    std::vector<std::vector<word>> while_sentence;
    if (begin == end)
    {
        std::cout << "  # begin = end in findWhile";
        return while_sentence;
    }
    std::vector<word> condition_words;
    std::vector<word> while_words;
    // input iter.wtype must be while, so ignore the judgement
    iter++; // skip 'while'
    // 1. get condition words
    int lbrnum = 0; // {
    int lpanum = 0; // (
    bool flag = false;
    if ((*iter).wtype != WordType::LPARENTTHESIS)
    {
        std::cout << "ERROR, after while not followed by (, lparentthesis" << std::endl;
        exit(-10);
    }
    while (lpanum != 0 or flag == false)
    { // get condition words
        if ((*iter).wtype == WordType::LPARENTTHESIS)
        {
            lpanum++;
            flag = true;
        }
        else if ((*iter).wtype == WordType::RPARENTTHESIS)
        {
            lpanum--;
        }
        condition_words.push_back(*iter);
        iter++;
    }
    // iter++; // skip )
    condition_words.pop_back();                     // remove tail )
    condition_words.erase(condition_words.begin()); // remove start (
    // 2. get while body
    flag = false;
    while (lbrnum != 0 or flag == false)
    { // get while body, {} match
        if ((*iter).wtype == WordType::LBRANCE)
        {
            lbrnum++;
            flag = true;
        }
        else if ((*iter).wtype == WordType::RBRANCE)
        {
            lbrnum--;
        }
        while_words.push_back(*iter);
        iter++;
    } // already skip }
    while_words.pop_back();                                                                         // remove tail }
    while_words.erase(while_words.begin());                                                         // remove start {
    while_sentence.push_back(std::vector<word>{word{WordType::INT, std::to_string(iter - begin)}}); // push length
    while_sentence.push_back(condition_words);
    while_sentence.push_back(while_words);
    return while_sentence;
}

/**
 * @brief 处理if else，主要是打印label，具体处理是递归处理的，不过需要传递函数
 */
void parseIfElse(std::vector<word> condition_words, std::vector<word> if_words, std::vector<word> else_words, Func *func, int if_label_index, int while_label_index)
{
    // ;
    // deal with condition
    std::cout << "  condition_if_" << if_label_index << ":" << std::endl; // 其实没必要，但是写出来方便理解以及debug
    calcuExpression(func, condition_words.begin(), condition_words.end());
    std::cout << "  pop eax" << std::endl; // pop value to eax
    std::cout << "  test eax,eax" << std::endl;
    // std::cout << " jne if_" << if_lable_index << "true" << std::endl; // 冗余了，fall=true的话
    std::cout << "  je if_" << if_label_index << "_false" << std::endl;
    // deal with if sentence
    std::cout << "  if_" << if_label_index << "_true:" << std::endl;
    // 根据;来分割处理，调用parseFunc函数？（但是如果还是遇到了if else怎么办？）——调用declareFunc函数？--重写一个函数（copy过来，删去函数初始化部分）
    // display_tokens(if_words); // to delete
    parseFunc2(if_words.begin(), if_words.end(), func, if_label_index, while_label_index);
    std::cout << "  jmp if_" << if_label_index << "_end" << std::endl;
    std::cout << "  if_" << if_label_index << "_false:" << std::endl;
    if (!else_words.empty())
    {
        std::cout << "  # followed else" << std::endl; // to delete
        // display_tokens(else_words); // to delete
        parseFunc2(else_words.begin(), else_words.end(), func, if_label_index, while_label_index);
        std::cout << "  jmp if_" << if_label_index << "_end" << std::endl;
    }
    std::cout << "  if_" << if_label_index << "_end:" << std::endl;
}

/**
 *
 */
void parseWhile(std::vector<word> condition_words, std::vector<word> while_words, Func *func, int if_label_index, int while_label_index)
{
    // to do
    std::cout << "  condition_while_" << while_label_index << ":" << std::endl;
    calcuExpression(func, condition_words.begin(), condition_words.end());
    std::cout << "  pop eax" << std::endl; // pop value to eax
    std::cout << "  test eax,eax" << std::endl;
    std::cout << "  je while_" << while_label_index << "_false" << std::endl;
    // while body
    // true
    std::cout << "  while_" << while_label_index << "_true:" << std::endl;
    parseFunc2(while_words.begin(), while_words.end(), func, if_label_index, while_label_index);
    std::cout << "  jmp condition_while_" << while_label_index << std::endl;
    // false
    std::cout << "  while_" << while_label_index << "_false:" << std::endl;
}

int findComma(std::vector<word> words)
{
    int lpanum = 0;
    int ret = 0;
    auto iter = words.begin();
    for (; iter != words.end(); iter++, ret++)
    {
        if ((*iter).wtype == WordType::COMMA)
        {
            if (lpanum == 0)
            { //()匹配完了且遇到了,也即第一个逗号直接返回
                // iter++;
                ret++;
                std::cout << "  # found comma" << std::endl;
                return ret;
            }
        }
        else if ((*iter).wtype == WordType::LPARENTTHESIS)
        {
            lpanum++;
        }
        else if ((*iter).wtype == WordType::RPARENTTHESIS)
        {
            lpanum--;
        }
    }
    std::cout << "  # no comma found" << std::endl;
    return ret;
}
/**
 * @brief 解析函数的语句（一句作为输入）
 * @note 语句不带分号";"
 * @param words 语句（无分号）
 * @param f 函数指针
 */
void parseFunc(std::vector<word> words, Func *func, int while_label_index)
{
    std::vector<word>::iterator iter = words.begin();
    // std::cout << "  # in parseFunc" << std::endl;
    int middle = findComma(words);
    // std::cout << "  # ok? " << std::endl;
    bool is_definevar = false;
    std::vector<word> beforecomma;

    // std::cout << "  # beforecomma: ";
    for (iter = words.begin(); iter != (words.begin() + middle - ((middle + words.begin()) != (words.end()))); iter++)
    {
        beforecomma.push_back(*iter);
    }
    std::cout << "  # before comma should be: " << (*(beforecomma.end() - 1)).name << std::endl; // to delete

    iter = words.begin();
    // undo, 这里都是直接第一个word就判断了，使用iter更好还是使用word[0]更好呢？
    if ((*iter).wtype == WordType::INT)
    {
        is_definevar = true;
        defineVars(func, words); // 赋值自带了comma的处理
    }
    else if ((*iter).wtype == WordType::RETURN) // return
    {
        returnFunc(beforecomma, func);
    }
    else if ((*iter).wtype == WordType::CONTINUE)
    { // continue
        continueFunc(while_label_index);
    }
    else if ((*iter).wtype == WordType::BREAK)
    { // break
        breakFunc(while_label_index);
    }
    else if (findFunc((*iter).name) != nullptr) // func call
    {
        std::string fname = (*iter).name;
        callFunc(getArgs(iter, words.begin() + middle), fname, func);
    }
    else if (findVar((*iter).name, func, true) != 1 or findArg((*iter).name, func) != 0) // var assignment
    {
        if ((*iter).name == "b")
        { // to delete
            std::cout << "  # test b?\n";
        }
        assignVar(func, beforecomma); // 和define相比，这里没有int，且define可能有多个，多个变量定义、赋值，且define可能没有赋值
    }
    else
    {
        std::cout << "  # ERROR no match line type(var define, var assignment, func call)" << std::endl;
        // display_tokens(words);
        std::cout << "  # *****\n";
    }
    // std::cout << "  # aftercomma: ";

    if (words.begin() + middle != words.end() and !is_definevar)
    {
        std::vector<word> aftercomma;
        std::cout << "  # so middle symbol is :  " << (*(words.begin() + middle)).name << std::endl; // to delete
        for (iter = words.begin() + middle; iter != words.end(); iter++)
        {
            aftercomma.push_back(*iter);
        }
        parseFunc(aftercomma, func, while_label_index);
    }
}

/**
 * @brief 变量定义，多个变量，利用逗号拆解为多次单个变量的定义
 * @param func, 函数指针
 * @param words 变量定义语句（无分号）
 * @note words里面无分号(类似int a=4+5, b=5+6)
 */
void defineVars(Func *func, std::vector<word> words)
{
    std::vector<word>::iterator iter = words.begin();
    iter++; // skip int
    std::vector<word> define_var;
    int lpa = 0;
    while (iter != words.end())
    { // 每个common处理一次
        if ((*iter).wtype == WordType::LPARENTTHESIS)
        {
            lpa++;
        }
        if ((*iter).wtype == WordType::RPARENTTHESIS)
        {
            lpa--;
        }
        if ((*iter).wtype == WordType::COMMA and lpa == 0)
        {
            defineVar(func, define_var);
            define_var.clear();
        }
        else
        {
            define_var.push_back(*iter);
        }
        iter++;
    }
    if (!define_var.empty()) // 最后一个不会有,
    {
        defineVar(func, define_var);
        define_var.clear();
    }
}

/**
 * @brief 单个变量的定义，如果涉及赋值，调用赋值函数
 * @param func 函数指针
 * @param words 变量定义的tokens 无逗号
 * @note words无逗号（类似a=4+5）
 */
void defineVar(Func *func, std::vector<word> words)
{
    if (words.size() == 1) // 纯声明，无定义与赋值
    {
        findVar(words[0].name, func);
    }
    else if (words[1].wtype == WordType::ASSIGNMET)
    {
        findVar(words[0].name, func); // declare first
        assignVar(func, words);       // then assign
    }
}

/**
 * @brief 变量赋值，调用表达式计算函数
 * @param func 函数指针
 * @param words tokens 包含"="以及变量名
 * @note words 包含=以及变量名(类似a=4+5)
 */
void assignVar(Func *func, std::vector<word> words)
{

    int var_ptr = findVar(words[0].name, func, true);
    int arg_ptr = findArg(words[0].name, func);
    std::cout << "  # assign in\n";
    calcuExpression(func, words.begin() + 2, words.end());
    std::cout << "  # assign out\n";
    std::cout << "  # assignment" << std::endl // undo 可以打印tokens的注释，只不过这样会显得很乱
              << "  pop eax # after calExp, pop eax" << std::endl;
    if (var_ptr == 1)
    { // arg
    std::cout << "mov DWORD PTR [ebp+" << 4 * (arg_ptr + 1) << "], eax" << std::endl;
    }
    else
    {
        std::cout << "  mov DWORD PTR [ebp" << var_ptr << "], eax" << std::endl;
    }
}

/**
 * @brief 表达式计算（涉及到函数调用时，需要递归使用）
 * @param func 函数指针
 * @param start 起始迭代器
 * @param end 结束迭代器
 * @note 函数调用时，主要是其参数也可能是表达式，故需要递归调用。递归的终止条件？-->不再有函数调用？
 * @note 主要麻烦在识别其参数？用某个函数来识别其参数？
 * @note tokens 也即表达式类似(eg1: a+b, eg2: func(func(func(a))), eg3: a+(b+e)|5^6)
 * @warning 使用完calcuExpression后，需要pop eax得到表达式的结果！！
 */
void calcuExpression(Func *func, std::vector<word>::iterator start, std::vector<word>::iterator end)
{
    std::stack<std::string> opstack; // op stack of one func
    // std::stack<var> numberstack;     // numberstack of one func (useless, actually)
    // 按照compile2的思路--number stack其实还是没用（增加函数调用）
    auto iter = start;
    WordType before_type = WordType::LPARENTTHESIS;
    std::string before_op;
    for (; iter != end; iter++)
    {
        if ((*iter).wtype == WordType::NUMBER) // number
        {
            // numberstack.push(var{0, "#number"});
            std::cout << "  mov eax, " << (*iter).name << std::endl
                      << "  push eax" << std::endl;
        }
        else if (((*iter).wtype == VAR)) // 可能为函数调用，也可能就是变量
        {
            // undo, 这里感觉可以优化代码，比如findVar了两次
            // 已改，但是如果函数调用多的话，findFunc写前面可能会更快？
            int varptr = findVar((*iter).name, func, true);
            int argindex = findArg((*iter).name, func);

            if (0 != argindex)
            { // 参数列表里面去找
                // numberstack.push(var{0, (*iter).name});
                std::cout << "  mov eax, DWORD PTR [ebp+" << 4 * (argindex + 1) << "] # come from argslist[" << argindex - 1 << "]" << std::endl
                          << "  push eax" << std::endl;
            }
            else if (1 != varptr)
            { // 变量名
                // numberstack.push(var{varptr, (*iter).name});
                std::cout << "  mov eax, DWORD PTR [ebp" << varptr << "]" << std::endl
                          << "  push eax" << std::endl;
            }
            else if (nullptr != findFunc((*iter).name))
            { // 函数调用

                std::string fname = (*iter).name;
                std::cout << "  # ready to call func" << fname << std::endl;
                // display_tokens(iter, end);
                std::vector<std::vector<word>> args = getArgs(iter, end);
                callFunc(args, fname, func);
                if (findFunc((*iter).name)->type == WordType::INT)
                {
                    std::cout << "  push eax # in calExp, after funcall, push return value" << std::endl;
                    // numberstack.push(var{0, "number"});
                }

                // 更新iter到函数结束的位置
                iter += updataIter(iter, end);
            }
            else
            {
                std::cout << "ERROR, no match type in calcuExpr, token as follows" << std::endl;
                display_tokens(start, iter);
                exit(-3);
            }
            // std::cout<<"xswl\n"; //need delete
        }
        else if ((*iter).wtype == WordType::OP or (*iter).wtype == WordType::LPARENTTHESIS or (*iter).wtype == WordType::RPARENTTHESIS) // op or ( or ),这里的()不是函数调用中的()
        {
            std::cout << "  # now is symbol : " << (*iter).name << std::endl;
            if (opstack.empty()) // 栈空，直接入栈
            {
                std::cout << "  # stack empty, directly push in stack" << std::endl;
                opstack.push((*iter).name);
            }
            else // 栈非空，比较优先级
            {
                std::string top_op = opstack.top();
                if ((*iter).name == "(") // ( 直接入栈
                {
                    opstack.push("(");
                }
                else if ((*iter).name == ")") // pop直到遇到(
                {
                    std::string tmp_op = "";
                    while (1)
                    {
                        tmp_op = opstack.top();
                        opstack.pop();
                        if (tmp_op == "(")
                            break; // 结束循环，"(" 与 ")"匹配了
                        if (tmp_op == "!" or tmp_op == "~" or tmp_op == "minus")
                        { // 一元运算符 只pop一次
                            std::cout << "  pop eax" << std::endl;
                        }
                        else
                        {
                            std::cout << "  pop ebx" << std::endl
                                      << "  pop eax" << std::endl;
                        }
                        trans_op(tmp_op);
                    }
                }
                else if (operatorMap[(*iter).name] > operatorMap[top_op]) // 优先级更高，直接入栈
                {
                    std::cout << "  # directly push stack: (stack top is)" << top_op << std::endl; // need delete
                    opstack.push((*iter).name);
                }
                else // 优先级更低或相等，先出栈再入栈
                {
                    if (operatorMap[(*iter).name] == operatorMap[top_op] and ((*iter).name == "minus" or (*iter).name == "~" or (*iter).name == "!"))
                    {
                        std::cout << "  # continue " << top_op << " " << (*iter).name << ", push directly" << std::endl; // need delete
                        opstack.push((*iter).name);
                    }
                    else
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
                            if (top_op == "!" or top_op == "~" or top_op == "minus") // 一元运算符 只pop一次
                            {
                                std::cout << "  pop eax" << std::endl;
                            }
                            else // 二元运算符， pop 两次
                            {
                                std::cout << "  pop ebx" << std::endl
                                          << "  pop eax" << std::endl;
                            }
                            trans_op(top_op);
                        }
                        opstack.push((*iter).name);
                    }
                }
            }
        }
        else
        {
            std::cout << "ERROR\n";
            std::cout << (*iter).name << std::endl;
            std::cout << "  # now is symbol : ??" << (*iter).name << std::endl;
        }
    }
    while (!opstack.empty())
    {
        std::string op = opstack.top();
        opstack.pop();
        if (op == "!" or op == "~" or op == "minus")
        {
            std::cout << "  pop eax" << std::endl;
        }
        else
        {
            std::cout << "  pop ebx" << std::endl;
            std::cout << "  pop eax" << std::endl;
        }
        trans_op(op);
    }
}

/**
 * @brief 获取函数参数
 * @param start 起始迭代器(从fname开始)
 * @param end 结束迭代器（可能是某条expression语句结束-->可能含有的内容多余其参数，也可能是单纯的函数调用-->所有内容恰好包括参数+")"）
 * @return 返回参数的tokens（也是表达式）
 * @note 根据输入的tokens（起始为funcname，终止为表达式的结尾），找出该func的args（args可能也是嵌套的函数调用...）
 * @note 可能的tokens1：add(a,b) , 可能的tokens2: add(a+b)+c...
 */
std::vector<std::vector<word>> getArgs(std::vector<word>::iterator start, std::vector<word>::iterator end) // 在calcuExpression中调用，获取函数参数
{
    std::string fname = (*start).name;
    Func *f = findFunc(fname);
    auto iter = start;
    iter++; // skip function name
    if ((*iter).wtype != WordType::LPARENTTHESIS)
    {
        std::cout << "ERROR in find func args, funcname not followed by (" << std::endl;
        display_tokens(iter, end);
        exit(-4);
    }
    std::vector<std::vector<word>> result;
    int args_num = f->info->argnum;
    if (args_num == 0)
    {
        return result;
    }
    int lpa_num = 0; //( count

    std::vector<word> tmp;
    for (; iter != end; iter++)
    {

        if ((*iter).wtype == WordType::LPARENTTHESIS)
        {
            lpa_num++;
            if (lpa_num > 1)
            {
                tmp.push_back(*iter);
            }
        }
        else if ((*iter).wtype == WordType::RPARENTTHESIS)
        {
            lpa_num--;
            if (lpa_num >= 1)
            {
                tmp.push_back(*iter);
            }
        }
        else if ((*iter).wtype == WordType::COMMA)
        {

            if (lpa_num == 1)
            { // 找到了第一个参数
                result.push_back(tmp);
                tmp.clear();
            }
            else
            {
                tmp.push_back(*iter);
                ; // do nothing
            }
        }
        else
        {
            tmp.push_back(*iter);
            ; // do nothing
        }
        if (lpa_num == 0)
        {
            if (tmp.size() != 0)
            {
                result.push_back(tmp);
                tmp.clear();
            }
            // break;
            if (result.size() == args_num)
            {
                break;
            }
            else
            {
                // std::cout << "ERROR func args num not match" << std::endl;
                // std::cout << "  # result num: " << result.size() << std::endl;
                // display_tokens(iter, end);
                // exit(-5);
            }
        }
    }
    // f->info->argnum;
    return result;
}

/**
 * @brief 更新iter，配合findArg使用（在calExp中！）
 * @note 需要步长
 */
int updataIter(std::vector<word>::iterator start, std::vector<word>::iterator end)
{
    int step = 0;
    std::string fname = (*start).name;
    Func *f = findFunc(fname);
    auto iter = start;
    iter++; // skip function name
    step++;
    if ((*iter).wtype != WordType::LPARENTTHESIS)
    {
    }
    int args_num = f->info->argnum;
    if (args_num == 0)
    {
        return step + 1; // skip "("
    }
    int lpa_num = 0; //( count

    for (; iter != end; iter++, step++)
    {
        if ((*iter).wtype == WordType::LPARENTTHESIS)
        {
            lpa_num++;
        }
        else if ((*iter).wtype == WordType::RPARENTTHESIS)
        {
            lpa_num--;
        }
        else if ((*iter).wtype == WordType::COMMA)
        {
            if (lpa_num == 1)
            { // 找到了第一个参数
            }
            else
            {
                ; // do nothing
            }
        }
        else
        {
            ; // do nothing
        }
        if (lpa_num == 0)
        {
            break;
        }
    }
    // f->info->argnum;
    return step;
}

/**
 * @brief 函数调用
 * @param args tokens字流，传入给calcuExpression计算
 * @param fname 函数名
 * @param fatherfunc 函数指针，存的是去哪里找变量
 * @note none
 */
void callFunc(std::vector<std::vector<word>> args, std::string fname, Func *fatherfunc)
{
    std::reverse(args.begin(), args.end()); // 参数从右至左入栈，调用函数
    std::vector<std::vector<word>>::iterator iter = args.begin();
    Func *f = findFunc(fname);
    if (f->info->argnum != args.size())
    {
        std::cout << "ERROR func args num not match" << std::endl;
        std::cout << "  # result num: " << f->info->argnum << std::endl;
        // display_tokens(iter, args.end());
        // exit(-5);
    }
    for (; iter != args.end(); iter++)
    {
        std::vector<word>::iterator s_iter = (*iter).begin();
        std::vector<word>::iterator e_iter = (*iter).end();
        calcuExpression(fatherfunc, s_iter, e_iter); // 计算完参数的值后需要pop到eax，然后push入栈
        std::cout << "  pop eax # after caculExpression of func arg" << std::endl
                  << "  push eax # push arg into stack, ready call func" << std::endl; // 这样看来似乎没有多大意义？
    }
    if (f->name == "println_int") // println_int 的push的fname和存的fname不一样
    {
        std::cout << "  push offset format_str" << std::endl;
        std::cout << "  call " << "printf" << "# call func: " << fname << std::endl;
    }
    else
    { // fname
        std::cout << "  call " << fname << "# call func: " << fname << std::endl;
    }
    std::cout << "  add esp, " << f->info->argnum * 4 << "# stack balance" << std::endl;
    if (f->name == "println_int")
    {
        std::cout << "  add esp, 4" << std::endl;
    }
}

/**
 * @brief 处理return语句
 * @param words tokens,包括了return关键字（从函数parse来的，不含";"）
 * @param func 函数指针
 * @note 这里对return语句的处理主要就是涉及表达式计算，要给eax赋值为返回值（int）的情况
 */
void returnFunc(std::vector<word> words, Func *func)
{
    if (func->type == WordType::VOID)
    {
        return;
    }
    auto iter = words.begin();
    iter++; // skip return
    calcuExpression(func, iter, words.end());
    std::cout << "  pop eax # pop return value after calExp" << std::endl;
}

/**
 * @brief 打印continue控制语句
 * @param while_label_index while循环的label
 */
void continueFunc(int while_label_index)
{
    std::cout << "  jmp condition_while_" << while_label_index << std::endl;
}

/**
 * @brief 打印break控制语句
 * @param while_label_index while循环的label
 */
void breakFunc(int while_label_index)
{
    std::cout << "  jmp while_" << while_label_index << "_false" << std::endl;
}

// end of file