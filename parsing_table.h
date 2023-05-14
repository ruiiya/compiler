#pragma once

#include <unordered_map>
#include <string>
#include "utils.h"

using namespace std;

namespace xd {
    using table = unordered_map<string, unordered_map<string, string>>;

    const string ERROR_TOKEN = "__error__";

    class parsing_table {
        table table_;

        symbols_table firsts;
        symbols_table follows;
        vprod productions;
        vs terminals;
        vs non_terminals;
        vs errors;

        string error_message(string prod, string symbol) {
            return "parsing table: duplicate entry - prod: " + prod + " - symbol: " + symbol;
        }

        public:

        parsing_table() = default;
        parsing_table(vs terminals, vs non_terminals) : terminals(std::move(terminals)), non_terminals(std::move(non_terminals)) {
            this->terminals.push_back(STRING_ENDMARKER);
        }

        void build_tables() {
            for(auto& non_terminal : non_terminals) {
                for(auto& terminal : terminals) {
                    table_[non_terminal][terminal] = ERROR_TOKEN;
                }
            }

            for (size_t prod_no = 0; prod_no < productions.size(); prod_no++) {
                string parent = productions[prod_no].get_parent();
                auto& rules = productions[prod_no].get_rules();

                for(size_t rule_no = 0; rule_no < rules.size(); rule_no++) {
                    string first_entity = rules[rule_no].get_entities()[0];

                    if(find(terminals.begin(), terminals.end(), first_entity) != terminals.end()) {
                        string entry = table_[parent][first_entity];
                        if(entry != ERROR_TOKEN) {
                            errors.push_back(error_message(parent, first_entity));
                        } 

                        table_[parent][first_entity] = to_string((prod_no << 16U) ^ rule_no);
                        
                    } else if(find(non_terminals.begin(), non_terminals.end(), first_entity) != non_terminals.end()) {
                        if(firsts.count(first_entity) != 0) {
                            for(const auto& symbol : firsts[first_entity]) {
                                if(symbol != EPSILON) {
                                    string entry = table_[parent][symbol];
                                    if(entry != ERROR_TOKEN) {
                                        errors.push_back(error_message(parent, symbol));
                                    } 

                                    table_[parent][symbol] = to_string((prod_no << 16U) ^ rule_no);
                                }
                            }
                        }
                    } else if (first_entity == EPSILON) {
                        if (follows.count(parent) != 0U) {
                            for (auto &symbol : follows[parent]) {
                                std::string entry = table_[parent][symbol];
                                if (entry != ERROR_TOKEN) {
                                errors.push_back(error_message(parent, symbol));
                                }

                                table_[parent][symbol] = to_string((prod_no << 16U) ^ rule_no);
                            }
                        }
                    }
                } 
            }
        }

        pair<unsigned int, unsigned int> get_entry (const string& terminal, const string& non_terminal) {
            string entry = table_[terminal][non_terminal];
            unsigned int value = static_cast<unsigned int>(stoul(entry));
            unsigned int left = value >> 16U;
            unsigned int right = value ^ (left << 16U);
            return {left, right};
        }

        void set_firsts(symbols_table firsts) {this->firsts = std::move(firsts);}
        void set_follows (symbols_table follows) {this->follows = std::move(follows);}
        void set_productions (vprod productions) {this->productions = std::move(productions);}

        const table& get_table() {return table_;}
        const symbols_table& get_firsts() {return firsts;}
        const symbols_table& get_follows() {return follows;}
        const vprod& get_productions() {return productions;}
        const vs& get_terminals() {return terminals;}
        const vs& get_non_terminals() {return non_terminals;}
        const vs& get_errors() {return errors;}
    };
}