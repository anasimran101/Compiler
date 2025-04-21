#pragma once

#include <vector>
#include "lexer.hpp"




class Parser {
public:
    Parser(std::vector<TOKEN>);
    void programme();

private:

    std::vector<TOKEN> tokens;
    size_t index;

    //parse tree vars
    int depth;
    int parse_tree_fd = -1;
    bool next_line=true;
    const std::string parse_tree_directory = "output";
    const std::string parse_tree_filename = "parse_tree.txt";

    TOKEN peek();
    void match(const std::string&);
    void match(TOKEN_CLASS);


    bool drawInStart(const std::string&);
    bool drawInEnd();
    void write(const std::string&);

    void programme_1();
    void type();
    void argList();
    void declaration();
    void stmt();
    void noIfStmt();
    void forStmt();
    void optExpr();
    void whileStmt();
    void ifStmt();
    void elsePart();
    void compStmt();
    void stmtList();
    void returnStmt();
    void expr();
    void expr_1();
    void rvalue();
    void rvalue_1();
    void mag();
    void mag_1();
    void term();
    void term_1();
    void compare();
    void factor();
    void identifier();
    void number();
};