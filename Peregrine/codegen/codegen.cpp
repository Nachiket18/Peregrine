#include "codegen.hpp"

#include "ast/ast.hpp"
#include "errors/error.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

Codegen::Codegen(std::string outputFilename) { m_file.open(outputFilename); }

void Codegen::write(std::string_view code) {
    m_file << code;
    m_file.close(); // this will be used only once so we can close it
}

std::string Codegen::generate(ast::AstNodePtr astNode) {
    std::string res;
    switch (astNode->type()) {
        case ast::KAstProgram: {
            auto node = std::dynamic_pointer_cast<ast::Program>(astNode);

            for (auto& stmt : node->statements()) {
                res += generate(stmt) + ";\n";
            }
            break;
        }

        case ast::KAstInteger: {
            auto node = std::dynamic_pointer_cast<ast::IntegerLiteral>(astNode);
            res += node->value();
            break;
        }

        case ast::KAstDecimal: {
            auto node = std::dynamic_pointer_cast<ast::DecimalLiteral>(astNode);
            res += node->value();
            break;
        }
        case ast::KAstString: {
            auto node = std::dynamic_pointer_cast<ast::StringLiteral>(astNode);
            // todo- do this in type check

            res += "\"";
            res += node->value();
            res += "\"";
            break;
        }
        case ast::KAstBool: {
            auto node = std::dynamic_pointer_cast<ast::BoolLiteral>(astNode);
            res += ((node->value() == "True") ? "true" : "false");
            break;
        }
        case ast::KAstPrefixExpr: {
            auto node = std::dynamic_pointer_cast<ast::PrefixExpression>(astNode);
            res += "(" + node->prefix().keyword + " " +
                   generate(node->right()) + ")";
            break;
        }
        case ast::KAstBinaryOp: {
            auto node = std::dynamic_pointer_cast<ast::BinaryOperation>(astNode);
            auto operation = node->op();
            if (operation.keyword == "**") {
                res += "_PEREGRINE_POWER(" + generate(node->left()) + "," +
                       generate(node->right()) + ")";
            } else if (operation.keyword == "//") {
                res += "_PEREGRINE_FLOOR(" + generate(node->left()) + "/" +
                       generate(node->right()) + ")";
            } else {
                res += "(" + generate(node->left()) + " " + node->op().keyword +
                       " " + generate(node->right()) + ")";
            }
            break;
        }
        case ast::KAstBlockStmt: {
            auto node = std::dynamic_pointer_cast<ast::BlockStatement>(astNode);
            auto body = node->statements();
            for (auto& stmp : body) {
                res += generate(stmp) + ";\n";
            }
            break;
        }
        case ast::KAstIfStmt: {
            auto node = std::dynamic_pointer_cast<ast::IfStatement>(astNode);
            res += "if(" + generate(node->condition()) + "){\n" +
                   generate(node->ifBody()) + "}";
            auto elifNode = node->elifs();
            if (elifNode.size() != 0) {
                res += "\n";
                for (auto& body : elifNode) { // making sure that elif exists
                    res += "else if(" + generate(body.first) + "){\n" +
                           generate(body.second) + "}";
                }
            }
            auto elseNode = node->elseBody();
            if (elseNode->type() ==
                ast::KAstBlockStmt) { // making sure that else exists
                res += "\nelse{\n" + generate(elseNode) + "}";
            }
            break;
        }
        case ast::KAstContinueStatement: {
            res += "continue";
            break;
        }
        case ast::KAstBreakStatement: {
            res += "break";
            break;
        }
        case ast::KAstNone: {
            res += "NULL";
            break;
        }
        case ast::KAstPassStatement: {
            res += "\n//pass";// we are making it a comment because ; is added to
                           // each node at the end. we dont want that to happen
                           // because it will result in ;; which is an error
            break;
        }
        case ast::KAstVariableStmt: {
            auto node = std::dynamic_pointer_cast<ast::VariableStatement>(astNode);
            std::string type;
            if (node->varType()->type() != ast::KAstNoLiteral){
                type=generate(node->varType())+" ";
            }
            std::string name = generate(node->name());
            std::string value;
            if (node->value()->type() != ast::KAstNoLiteral){
                value=" = "+generate(node->value());
            }
            res+=type+name+value;
            break;
        }
        case ast::KAstCpp: {
            auto node = std::dynamic_pointer_cast<ast::CppStatement>(astNode);
            res += node->value() +
                   "\n//"; // we are making it a comment because ; is added to
                           // each node at the end. we dont want that to happen
                           // because it will result in ;; which is an error
            break;
        }
        case ast::KAstWhileStmt: {
            auto node = std::dynamic_pointer_cast<ast::WhileStatement>(astNode);
            res += "while(" + generate(node->condition()) + "){\n" +
                   generate(node->body()) + "}";
            break;
        }
        case ast::KAstIdentifier: {
            auto node =
                std::dynamic_pointer_cast<ast::IdentifierExpression>(astNode);
            res += node->value();
            break;
        }
        case ast::KAstScopeStmt: {
            auto node = std::dynamic_pointer_cast<ast::ScopeStatement>(astNode);
            res += "{\n" + generate(node->body()) + "\n}";
            break;
        }
        case ast::KAstReturnStatement: {
            auto node = std::dynamic_pointer_cast<ast::ReturnStatement>(astNode);
            res += "return ";
            auto value = node->returnValue();
            if (value->type() != ast::KAstNoLiteral) {
                res += generate(value);
            }
            break;
        }
        case ast::KAstFunctionCall: {
            auto node = std::dynamic_pointer_cast<ast::FunctionCall>(astNode);
            auto funcName = generate(node->name());
            std::string arg;
            auto x = node->arguments();
            if (x.size() != 0) {
                for (uint64_t i = 0; i < x.size(); ++i) {
                    arg += generate(x[i]) + " ";
                    if (i == x.size() - 1) {
                    } else {
                        arg += ",";
                    }
                }
            }
            res += funcName + "(" + arg + ")";
            break;
        }
        case ast::KAstTypeExpr:{
            auto node = std::dynamic_pointer_cast<ast::TypeExpression>(astNode);
            res+=node->value();
            break;
        }
        case ast::KAstFunctionDef: {
            auto node = std::dynamic_pointer_cast<ast::FunctionDefinition>(astNode);
            auto functionName = generate(node->name());
            auto returnType = generate(node->returnType());
            std::string param;
            if (node->parameters().size() > 0) {
                auto x = node->parameters();
                for (uint64_t i = 0; i < x.size(); ++i) {
                    param +=
                        generate(x[i].p_type) + " " + generate(x[i].p_name);
                    if (i == x.size() - 1) {
                    } else {
                        param += ",";
                    }
                }
            }
            if (functionName == "main" && returnType == "void") {
                // we want the main function to always return 0 if success
                res += "int main(" + param + "){\n" + generate(node->body()) +
                       "return 0;\n}";
            } else {
                res += returnType + " " + functionName + "(" + param + "){\n" +
                       generate(node->body()) + "\n}";
            }

            break;
        }
        case ast::KAstMatchStmt:{
            auto node = std::dynamic_pointer_cast<ast::MatchStatement>(astNode);
            auto to_match=node->matchItem();
            auto cases=node->caseBody();
            auto defaultbody=node->defaultBody();
            res+="\nwhile (true){\n";
            for (uint64_t i=0;i<cases.size();++i){
                auto x=cases[i];
                if (x.first.size()==1 && x.first[0]->type() == ast::KAstNoLiteral){
                    if (i==0){
                        res+=generate(x.second)+"\n";
                    }
                    else{
                        res+="else{\n"+generate(x.second)+"\n}\n";
                    }
                }
                else if (i==0){
                    res+="if ("+match_arg(to_match,x.first)+"){\n"+generate(x.second)+"\n}\n";
                }
                else{
                    res+="else if ("+match_arg(to_match,x.first)+"){\n"+generate(x.second)+"\n}\n";
                }
            }
            if (defaultbody->type() != ast::KAstNoLiteral){
                res+=generate(defaultbody);
            }
            res+="\nbreak;\n}";
            break;
        }
        default: {
            std::cout<<astNode->type()<<"\n";
            std::cerr << fg(style("Error: invalid ast node passed to "
                                  "generate(). This should never happen.",
                                  bold),
                            light_red)
                      << "\n";
            std::cerr << "This is most likely an issue with the compiler "
                         "itself. You can seek help at our discord server: "
                         "https://discord.gg/CAMgzwDJDM"
                      << "\n";
            exit(1);
        }
    }
    return res;
}