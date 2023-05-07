#pragma once

#include "parsing_table.h"
#include "lexer.h"
#include <stack>
#include <iostream>

using namespace std;

namespace xd {

    struct parsing_node {
        string token_type;
        string value;
        bool is_epsilon{true};
        vector<shared_ptr<parsing_node>> kids;

        // https://stackoverflow.com/questions/59508678/c-print-tree-not-necessarily-binary-in-a-pretty-way-to-stdout
        void print_sub_tree(const string &prefix) {
            if(kids.empty()) return;
            cout << prefix;
            size_t n_kids = kids.size();
            cout << (n_kids > 1 ? "├── " : "");

            for (size_t i = 0; i < n_kids; ++i) {
                shared_ptr<parsing_node> kid = kids[i];
                if (i < n_kids - 1) {
                    if (i > 0) { // added fix
                    cout << prefix << "├── "; // added fix
                    }
                    bool printStrand = n_kids > 1 && !kid->kids.empty();
                    string newPrefix = prefix + (printStrand ? "│\t" : "\t");
                    cout << kid->value << "\n";
                    kid->print_sub_tree(newPrefix);
                } else {
                    cout << (n_kids > 1 ? prefix : "") << "└── ";
                    cout << kid->value << "\n";
                    kid->print_sub_tree(prefix + "\t");
                }
            }
        }

        void print() {
            cout << value << "\n";
            print_sub_tree("");
            cout << "\n";
        }
    };

    class parser{
        vector<token> input_tokens;
        string start_symbol;
        vector<token> current_tokens;
        size_t current_step{};
        parsing_table parsing_table_;
        stack<string> stack_;
        shared_ptr<parsing_node> ast;
        vector<size_t> history;
        vector<string> errors;

        void jump() {
            if(!is_complete()) {
                current_step ++;
            }
        }

        void DFS(shared_ptr<parsing_node> node) {
            for(shared_ptr<parsing_node>& kid_node : node->kids) {
                DFS(kid_node);
                if(!kid_node->is_epsilon) {
                    node->is_epsilon = false;
                }
            }
        }

        void remove(shared_ptr<parsing_node> node) {
            vector<shared_ptr<parsing_node>> new_kids;
            for(shared_ptr<parsing_node>& kid_node : node->kids) {
                remove(kid_node);
                if(!kid_node->is_epsilon) {
                    new_kids.push_back(kid_node);
                }
            }
            node->kids = std::move(new_kids);
        }

        void remove_kid(shared_ptr<parsing_node> node, shared_ptr<parsing_node> parent) {
            for(shared_ptr<parsing_node>& kid_node : node->kids) {
                remove_kid(kid_node, node);
            }
            if(!parent) return;
            if(node->kids.size() == 1) {
                shared_ptr<parsing_node> kid_node = node->kids[0];
                for(size_t id = 0;id < parent->kids.size();id++) {
                    if(parent->kids[id] == node) {
                        parent->kids[id] = kid_node;
                        return;
                    }
                }
            }
        }

        void set(shared_ptr<parsing_node> node, vector<token>& tokens, int& index) {
            auto& terminals = parsing_table_.get_terminals();
            for(shared_ptr<parsing_node>& kid_node : node->kids) {
                if(find(terminals.begin(), terminals.end(), kid_node->token_type) != terminals.end()) {
                    kid_node->value = tokens[index++].value;
                } else {
                    kid_node->value = kid_node->token_type;
                }
                set(kid_node, tokens, index);
            }
        }

        void print(shared_ptr<parsing_node> node) {
            auto& terminals = parsing_table_.get_terminals();
            if(node->kids.size()) {
                cout << node->value << " : ";
            for(shared_ptr<parsing_node>& kid_node : node->kids) {
                cout << kid_node->value << " ";
            }
            cout << "\n";
            }
            for(shared_ptr<parsing_node>& kid_node : node->kids) {
                print(kid_node);
            }
        }

        string error_message(const string& symbol) {
            return "parser: error at symbol: " + symbol;
        }

        public:

        parser() {
            stack_.push(std::string(STRING_ENDMARKER));
            input_tokens.clear();
            current_tokens.clear();
        }

        const vector<size_t>& get_history() {
            return history;
        }

        bool is_complete() {
            return current_step == current_tokens.size() || stack_.top() == STRING_ENDMARKER;
        }

        void parser_next_step() {
            string top_symbol = stack_.top();
            token current_token = current_tokens[current_step];
            table table_ = parsing_table_.get_table();

            while(!is_complete() && table_[top_symbol][current_token.token_type] == ERROR_TOKEN) {
                errors.push_back(error_message(current_token.token_type));
                jump();
                if(current_step < current_tokens.size()) {
                    current_token = current_tokens[current_step];
                }
            }

            if(!is_complete()) {
                if(table_[top_symbol][current_token.token_type] == SYNCH_TOKEN) {
                    errors.push_back(error_message(current_token.token_type));
                    stack_.pop();
                } else {
                    auto terminals = parsing_table_.get_terminals();

                    if(top_symbol == current_token.token_type) {
                        stack_.pop();
                        jump();
                    } else if(find(terminals.begin(), terminals.end(), top_symbol) != terminals.end() &&
                              find(terminals.begin(), terminals.end(), current_token.token_type) != terminals.end()) {
                        errors.push_back(error_message(current_token.token_type));
                        jump();
                    } else {
                        auto prod_rule = parsing_table_.get_entry(top_symbol, current_token.token_type);
                        auto productions = parsing_table_.get_productions();
                        auto req_rule = productions[prod_rule.first].get_rules()[prod_rule.second];
                        auto entities = req_rule.get_entities();
                        reverse(entities.begin(), entities.end());
                        stack_.pop();
                        
                        for(auto& entity : entities) {
                            if(entity != EPSILON) {
                                stack_.push(entity);
                            }
                        }
                        history.push_back((prod_rule.first << 16U) ^ prod_rule.second);
                    }
                }
            }
        }

        void build_tree() {
            stack<shared_ptr<parsing_node>> parsing_node_stack;
            parsing_node_stack.push(ast);
            
            auto productions = parsing_table_.get_productions();
            auto terminals = parsing_table_.get_terminals();
            auto non_terminals = parsing_table_.get_non_terminals();

            for(unsigned int value : get_history()) {        
                unsigned int left = value >> 16U;
                unsigned int right = value ^ (left << 16U);
                auto prod_rule = std::make_pair(left, right);
                auto req_prod = productions[prod_rule.first];
                auto req_rule = req_prod.get_rules()[prod_rule.second];
                auto parent = req_prod.get_parent();
                auto entities = req_rule.get_entities();
                
                shared_ptr<parsing_node> parent_node = parsing_node_stack.top();

                for(auto& entity : entities) {
                    shared_ptr<parsing_node> kid_node = make_shared<parsing_node>();
                    kid_node->token_type = entity;
                    if(find(terminals.begin(), terminals.end(), entity) != terminals.end()) {
                        kid_node->is_epsilon = false;
                    }
                    parent_node->kids.push_back(kid_node);
                }

                parsing_node_stack.pop();

                reverse(parent_node->kids.begin(), parent_node->kids.end());
                for(shared_ptr<parsing_node>& kid_node : parent_node->kids) {
                    if(find(non_terminals.begin(), non_terminals.end(), kid_node->token_type) != non_terminals.end()) {
                        parsing_node_stack.push(kid_node);
                    }
                }
                reverse(parent_node->kids.begin(), parent_node->kids.end());
            }
        }

        void remove_epsilon_node() {
            DFS(ast);
            remove(ast);
        }

        void remove_one_kid_node() {
            remove_kid(ast, nullptr);
        }

        void print_node() {
            vector<token> tokens = input_tokens;
            int index = 0;
            set(ast, tokens, index);
            ast->print();
        }

        void set_input_tokens(vector<token> tokens) {
            input_tokens = std::move(tokens);
        }

        void restart() {
            while(!stack_.empty()) {
                stack_.pop();
            }
            stack_.push(STRING_ENDMARKER);
            stack_.push(start_symbol);
            current_step = 0;
            current_tokens = input_tokens;
            current_tokens.emplace_back(STRING_ENDMARKER, STRING_ENDMARKER);
            ast = make_shared<parsing_node>();
            ast->token_type = start_symbol;
            ast->value = start_symbol;
        }

        void set_start_symbol(string symbol) {
            start_symbol = std::move(symbol);
        }

        void set_parsing_table(parsing_table table) {
            parsing_table_ = std::move(table);
        }

        const vs& get_errors() {return errors;}
    };
}