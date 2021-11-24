#ifndef PEREGRINE_CODEGEN_HPP
#define PEREGRINE_CODEGEN_HPP

#include "ast/ast.hpp"

#include <fstream>
#include <string>
#include <string_view>

class Codegen {
    std::ofstream m_file;
    std::string match_arg(std::vector<ast::AstNodePtr> match_item,std::vector<ast::AstNodePtr> case_item);
  public:
    void write(std::string_view code);
    Codegen(std::string outputFilename);

    std::string generate(ast::AstNodePtr astNode);
};

#endif