#include "lexer.h"

void anylize(const char *filename)
{
    std::ifstream fin(filename);
    // fin.open(argv[1], std::ios::in);
    if (!fin.is_open())
    {
        std::cerr << "Fail to open file! File: " << filename << std::endl;
        exit(-1);
    }
    std::stringstream buff;
    buff << fin.rdbuf();
    std::string contents = buff.str(); // read file into contents
    fin.close();
    line_analize(contents);
    /*display tokens1*/
    // std::cout << "!!!!\n\n";
    // display_tokens(tokens1);
    // std::cout << "########\n\n\n\n\n";
    // std::vector<std::vector<word>::iterator iter= tokens.begin();

    /*display tokens*/
    // std::vector<std::vector<word>>::iterator iter=tokens.begin();
    // for(;iter!=tokens.end();++iter){
    //     // std::vector<word>::iterator iter2=(*iter).begin();
    //     display_tokens(*iter);
    // }
}
void line_analize(std::string line)
{
    std::regex keywordRegex("\\b(int|main|void|return)\\b");
    std::regex mainRex("main");
    std::regex intRex("int");
    std::regex voidRex("void");
    std::regex returnRex("return");
    // std::regex operatorRegex("[\\+\\-\\*/%&|^~<>!]=?|==|!=|<<|>>|&&|\\|\\|");
    std::regex operatorRegex(R"(\|\||&&|==|!=|<=|>=|<<|>>|[-+*/%&|^~<>!])");
    std::regex numberRegex("[0-9]+");
    std::regex assignmentRegex("=");
    std::regex identifierRegex("[a-zA-Z_][a-zA-Z0-9_]*");
    std::regex parenthesisRegex("[\\(\\)\\{\\}\\[\\]]");
    std::regex lpaRegex("\\(");
    std::regex rpaRegex("\\)");
    std::regex lbrRegex("\\{");
    std::regex rbrRegex("\\}");
    std::regex commaRegex(",");
    std::regex semicolonRegex(";");
    // std::regex tokenRegex("(\\b(int|main|void|return)\\b|[\\+\\-\\*/%&|^~<>!]=?|==|!=|<<|>>|&&|\\|\\||=|[a-zA-Z_][a-zA-Z0-9_]*|[\\(\\)\\{\\}\\[\\]]|,|;)");
    std::regex tokenRegex(R"((\b(int|main|void|return)\b|\|\||&&|==|!=|<=|>=|<<|>>|[-+*/%&|^~<>!]|=|[a-zA-Z_][a-zA-Z0-9_]*|[(){}[\]]|,|;|[0-9]+))");

    auto words_begin = std::sregex_iterator(line.begin(), line.end(), tokenRegex);
    auto words_end = std::sregex_iterator();
    word before_word;
    std::vector<word> line_tokens;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i)
    {
        std::smatch match = *i;
        std::string match_str = match.str();

        word tword;

        if (std::regex_match(match_str, intRex))
        {
            tword.wtype = WordType::INT;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else if (std::regex_match(match_str, voidRex))
        {
            tword.wtype = WordType::VOID;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else if (std::regex_match(match_str, returnRex))
        {
            tword.wtype = WordType::RETURN;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else if (std::regex_match(match_str, mainRex))
        {
            tword.wtype = WordType::MAIN;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else if (std::regex_match(match_str, operatorRegex))
        {
            tword.wtype = WordType::OP;
            tword.name = match_str;
            if (tword.name == "-" and ((before_word.wtype == WordType::OP and (before_word.name == "~" or before_word.name == "!")) or (before_word.wtype == WordType::LPARENTTHESIS) or (before_word.wtype == WordType::ASSIGNMET) or (before_word.wtype==WordType::COMMA) or (before_word.wtype==WordType::OP)))
            {
                // std::cout << "before op: " << before_word.name << std::endl;
                // std::cout << "current op: " << tword.name << std::endl;
                // std::cout << "match str: " << match_str << std::endl;
                // tword.wtype = WordType::MINUS;
                tword.name = "minus";
            }
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else if (std::regex_match(match_str, assignmentRegex))
        {
            tword.wtype = WordType::ASSIGNMET;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else if (std::regex_match(match_str, identifierRegex))
        {
            tword.wtype = WordType::VAR;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else if (std::regex_match(match_str, lpaRegex))
        {
            tword.wtype = WordType::LPARENTTHESIS;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else if (std::regex_match(match_str, rpaRegex))
        {
            tword.wtype = WordType::RPARENTTHESIS;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else if (std::regex_match(match_str, lbrRegex))
        {
            tword.wtype = WordType::LBRANCE;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else if (std::regex_match(match_str, rbrRegex))
        {
            tword.wtype = WordType::RBRANCE;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        // else if (std::regex_match(match_str, parenthesisRegex)) {
        // token.type = PARENTHESIS;
        // }
        else if (std::regex_match(match_str, commaRegex))
        {
            tword.wtype = WordType::COMMA;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else if (std::regex_match(match_str, semicolonRegex))
        {
            tword.wtype = WordType::SEMICOLON;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
            tokens.push_back(line_tokens);
            line_tokens.clear();
        }
        else if (std::regex_match(match_str, numberRegex))
        {
            tword.wtype = WordType::NUMBER;
            tword.name = match_str;
            tokens1.push_back(tword);
            line_tokens.push_back(tword);
        }
        else
        {
            std::cout << "Unkonw type!!\n";
            exit(-1);
        }
        before_word = tword;
        // token.value = match_str;
        // tokens.push_back(token);
    }

    // return tokens;
}
