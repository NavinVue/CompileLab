// #include "utils.h"
#include "lexer.h"
#include "parser.h"

std::vector<std::vector<word>> tokens; // 存储tokens，词法分析结果
std::vector<word> tokens1;

int main(int argc, char *argv[])
{
    // read file
    anylize(argv[1]);
    parser();
    return 0;
}