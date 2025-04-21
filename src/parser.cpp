#include "parser.hpp"
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

Parser::Parser(std::vector<TOKEN> _tokens) : tokens(std::move(_tokens)), index(0), depth(0), next_line(false) {
    parse_tree_fd = open((parse_tree_directory + "/" + parse_tree_filename).c_str(), O_CREAT | O_WRONLY, 0644);
    if (parse_tree_fd == -1) {
        std::cerr << "Parser:Parser() unable to open parse tree file\n";
    }
}

bool Parser::drawInStart(const std::string& node_name) {
    ++depth;
    next_line = true;
    write(node_name);
    return true;
}

bool Parser::drawInEnd() {
    if (next_line) write("");
    next_line = false;
    --depth;
    return true;
}

void Parser::write(const std::string& node_name) {
    std::string line;
    for (int i = 0; i < depth; ++i) line += '\t';
    if (!node_name.empty()) {
        line += node_name + "\n";
        for (int i = 0; i < depth; ++i) line += '\t';
        line += "|\n";
    } else {
        line += "\n";
    }
    ::write(parse_tree_fd, line.c_str(), line.size());
}

TOKEN Parser::peek() {
    if (index >= tokens.size()) {
        return TOKEN(-1, std::nullopt, TOKEN_CLASS::T_EOF, 0, 0);
    }
    return tokens[index];
}

void Parser::match(const std::string& lexeme) {
    if (peek().t_lexeme == lexeme) {
        ++index;
    } else {
        std::cerr << "[PARSE ERROR] Expected '" << lexeme << "' at line " << peek().line_number << "\n";
        exit(EXIT_FAILURE);
    }
}

void Parser::match(TOKEN_CLASS cls) {
    if (peek().t_class == cls) {
        ++index;
    } else {
        std::cerr << "[PARSE ERROR] Expected token class at line " << peek().line_number << "\n";
        exit(EXIT_FAILURE);
    }
}



void Parser::programme() {
    drawInStart("programme");
    if (peek().t_class != T_EOF) {
        type(); identifier(); programme_1();
    }
    drawInEnd();
}

void Parser::programme_1() {
    drawInStart("programme_1");
    if (peek().t_lexeme == "(") {
        match("("); argList(); match(")"); compStmt(); programme();
    } else if (peek().t_lexeme == "::") {
        match("::"); programme();
    }
    drawInEnd();
}

void Parser::type() {
    drawInStart("type");
    if (peek().t_lexeme == "Adadi" || peek().t_lexeme == "Ashriya" || peek().t_lexeme == "Harf" || peek().t_lexeme == "Mantiqi") {
        ++index;
    } else {
        std::cerr << "[PARSE ERROR] Expected type at line " << peek().line_number << "\n";
        exit(EXIT_FAILURE);
    }
    drawInEnd();
}

void Parser::argList() {
    drawInStart("argList");
    if (peek().t_lexeme == "Adadi" || peek().t_lexeme == "Ashriya" || peek().t_lexeme == "Harf" || peek().t_lexeme == "Mantiqi") {
        type(); identifier();
        if (peek().t_lexeme == ",") {
            match(","); argList();
        }
    }
    drawInEnd();
}

void Parser::declaration() {
    drawInStart("declaration");
    type(); identifier();
    while (peek().t_lexeme == ",") {
        match(","); identifier();
    }
    match("::");
    drawInEnd();
}

void Parser::stmt() {
    drawInStart("stmt");
    if (peek().t_lexeme == "Agar") ifStmt();
    else noIfStmt();
    drawInEnd();
}

void Parser::noIfStmt() {
    drawInStart("noIfStmt");
    if (peek().t_lexeme == "for") forStmt();
    else if (peek().t_lexeme == "while") whileStmt();
    else if (peek().t_lexeme == "{") compStmt();
    else if (peek().t_lexeme == "Wapas") returnStmt();
    else if (peek().t_class == Keyword) declaration();
    else if (peek().t_class == Identifier) { expr(); match("::"); }
    else if (peek().t_lexeme == "::") match("::");
    else {
        std::cerr << "[PARSE ERROR] Invalid statement at line " << peek().line_number << "\n";
        exit(EXIT_FAILURE);
    }
    drawInEnd();
}

void Parser::forStmt() {
    drawInStart("forStmt");
    match("for"); match("("); optExpr(); match("::"); optExpr(); match("::"); optExpr(); match(")"); stmt();
    drawInEnd();
}

void Parser::optExpr() {
    drawInStart("optExpr");
    if (peek().t_class == Identifier) expr();
    drawInEnd();
}

void Parser::whileStmt() {
    drawInStart("whileStmt");
    match("while"); match("("); expr(); match(")"); stmt();
    drawInEnd();
}

void Parser::ifStmt() {
    drawInStart("ifStmt");
    match("Agar"); match("("); expr(); match(")");
    stmt();
    if (peek().t_lexeme == "Wagarna"){
        elsePart();
    }
    drawInEnd();
}
void Parser::elsePart() {
    drawInStart("elsePart");
    match("Wagarna");
    stmt();
    drawInEnd();
}

void Parser::compStmt() {
    drawInStart("compStmt");
    match("{"); stmtList(); match("}");
    drawInEnd();
}

void Parser::stmtList() {
    drawInStart("stmtList");
    if (peek().t_lexeme != "}") {
        stmt(); stmtList();
    }
    drawInEnd();
}

void Parser::returnStmt() {
    drawInStart("returnStmt");
    match("Wapas"); expr(); match("::");
    drawInEnd();
}

void Parser::expr() {
    drawInStart("expr");
    if (peek().t_class == Identifier) {
        identifier(); expr_1();
    }
    else rvalue();
    drawInEnd();
}

void Parser::expr_1() {
    drawInStart("expr_1");
    if (peek().t_lexeme == ":=") {
        match(":="); expr();
    } else {
        rvalue_1();
    }
    drawInEnd();
}

void Parser::rvalue() {
    drawInStart("rvalue");
    mag(); rvalue_1();
    drawInEnd();
}

void Parser::rvalue_1() {
    drawInStart("rvalue_1");
    if (peek().t_lexeme == "==" || peek().t_lexeme == "<" || peek().t_lexeme == ">" ||
        peek().t_lexeme == "<=" || peek().t_lexeme == ">=" || peek().t_lexeme == "!=" ||
        peek().t_lexeme == "<>") {
        compare(); mag(); rvalue_1();
    }
    drawInEnd();
}

void Parser::mag() {
    drawInStart("mag");
    term(); mag_1();
    drawInEnd();
}

void Parser::mag_1() {
    drawInStart("mag_1");
    if (peek().t_lexeme == "+" || peek().t_lexeme == "-") {
        match(peek().t_lexeme.value()); term(); mag_1();
    }
    drawInEnd();
}

void Parser::term() {
    drawInStart("term");
    factor(); term_1();
    drawInEnd();
}

void Parser::term_1() {
    drawInStart("term_1");
    if (peek().t_lexeme == "*" || peek().t_lexeme == "/") {
        match(peek().t_lexeme.value()); factor(); term_1();
    }
    drawInEnd();
}

void Parser::compare() {
    drawInStart("compare");
    match(peek().t_lexeme.value());
    drawInEnd();
}

void Parser::factor() {
    drawInStart("factor");
    if (peek().t_lexeme == "(") {
        match("("); expr(); match(")");
    } else if (peek().t_class == Identifier) {
        identifier();
    } else if (peek().t_class == Number) {
        number();
    } else {
        std::cerr << "[PARSE ERROR] Invalid factor at line " << peek().line_number << "\n";
        exit(EXIT_FAILURE);
    }
    drawInEnd();
}

void Parser::identifier() {
    drawInStart("Identifier");
    match(Identifier);
    drawInEnd();
}

void Parser::number() {
    drawInStart("Number");
    match(Number);
    drawInEnd();
}