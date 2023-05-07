#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <fstream>
#include <unordered_set>

using namespace std;

namespace xd {
    const string EPSILON = "EPSILON";

    using vs = vector<string>;

    class rule {
        vs entities;

        public:
        rule(const vs& entities) : entities(std::move(entities)) {}
        rule() = default;

        const vs& get_entities() const {
            return entities;
        }

        void set_entities(const vs& entities) {
            this->entities = entities;
        }

        bool prefix(const rule& prefix) const {
            if(prefix.entities.size() > entities.size()) {
                return false;
            }

            for(size_t no = 0; no < prefix.entities.size(); no++) {
                if(prefix.entities[no] != entities[no]) {
                    return false;
                }
            }

            return true;
        }
    };

    using vrule = vector<rule>;

    class production{
        string parent;
        vrule rules;
        public:
        production(string parent, vrule rules) : parent(std::move(parent)), rules(std::move(rules)) {}
        production() = default;

        const vrule& get_rules() const {
            return rules;
        }

        void set_rules(const vrule& rules) {
            this->rules = rules;
        }

        const string get_parent() const {
            return parent;
        }

        void set_parent(const string& parent) {
            this->parent = parent;
        }
    };

    using vprod = vector<production>;

    class grammar_parser {
        ifstream ifs;
        vs terminals;
        vs non_terminals;
        string start_symbol;
        vprod grammar;
        string error;

        static vs split_tok(const string& str) {
            stringstream ss(str);
            string s;
            vs vs;
            while(ss >> s) {
                vs.push_back(s);
            }
            return vs;
        }

        public:
        grammar_parser(const string& filename) : ifs(filename) {}
        ~grammar_parser() {
            if(ifs.is_open()) {
                ifs.close();
            }
        }

        bool parse() {
            enum parse_state {NON, TERMINAL, NON_TERMINAL, START, RULES};
            enum rule_state {LEFT, COLON, ENTITY};

            if(ifs.is_open()) {
                parse_state parse_state_now = NON;
                string line;

                unordered_map<string, vector<vs>> _grammar;

                while(getline(ifs, line)) {
                    if(line.size() == 0 || line[0] == '#') continue;
                    
                    vs tokens = split_tok(line);

                    if(tokens[0] == "NON_TERMINAL") {
                        if(parse_state_now != NON || tokens.size() > 1) {
                            set_error("invalid token NON_TERMINAL");
                            return false;
                        }
                        tokens.pop_back();
                        parse_state_now = NON_TERMINAL;
                    } else if(tokens[0] == "TERMINAL") {
                        if(parse_state_now != NON || tokens.size() > 1) {
                            set_error("invalid token TERMINAL");
                            return false;
                        }
                        tokens.pop_back();
                        parse_state_now = TERMINAL;
                    } else if(tokens[0] == "START") {
                        if(parse_state_now != NON || tokens.size() > 1) {
                            set_error("invalid token START");
                            return false;
                        }
                        tokens.pop_back();
                        parse_state_now = START;
                    } else if(tokens[0] == "RULES") {
                        if(parse_state_now != NON || tokens.size() > 1) {
                            set_error("invalid token RULES");
                            return false;
                        }
                        tokens.pop_back();
                        parse_state_now = RULES;
                    } else if(tokens[0] == "END") {
                        if(parse_state_now == NON || tokens.size() > 1) {
                            set_error("invalid token END");
                            return false;
                        }
                        tokens.pop_back();
                        parse_state_now = NON;
                    }

                    rule_state rule_state_now = LEFT;
                    string rule_parent;
                    vs rule_entities;

                    for(auto token : tokens) {
                        switch (parse_state_now)
                        {
                        case NON:
                            set_error("invalid token" + token);
                            return false;
                            break;

                        case TERMINAL:
                            if(token == EPSILON) {
                                set_error("EPSILON is reserved");
                                return false;
                            }
                            terminals.push_back(token);
                            break;

                        case NON_TERMINAL:
                            if(token == EPSILON) {
                                set_error("EPSILON is reserved");
                                return false;
                            }
                            non_terminals.push_back(token);
                            break;
                        
                        case START:
                            if(!start_symbol.empty() || token == EPSILON) {
                                set_error("ambiguous start symbol");
                                return false;
                            }
                            start_symbol = token;
                            break;
                        
                        case RULES:
                            switch (rule_state_now)
                            {
                            case LEFT:
                                if(token == EPSILON) {
                                    set_error("production cannot start with EPSILON");
                                    return false;
                                }
                                rule_parent = token;
                                rule_state_now = COLON;
                                break;

                            case COLON:
                                if(token != ":") {
                                    set_error("rules syntax error ':' expected: " + token);
                                    return false;
                                }
                                rule_state_now = ENTITY;
                                break;
                            
                            case ENTITY:
                                rule_entities.push_back(token);
                                break;

                            default:
                                break;
                            }
                            break;

                        default:
                            break;
                        }
                    }

                    if(parse_state_now == RULES) {
                        if(rule_state_now == ENTITY) {
                            _grammar[rule_parent].push_back(rule_entities);
                        } else if(rule_state_now == COLON) {
                            set_error("rules syntax error ':' expected");
                            return false;
                        }
                    }
                }

                if (parse_state_now != NON) {
                    set_error("block is incomplete 'END' expected");
                    return false;
                }

                unordered_set<string> _terminals(terminals.begin(), terminals.end()), 
                                _non_terminals(non_terminals.begin(), non_terminals.end());
                
                if(_terminals.size() != terminals.size()) {
                    set_error("inconsistent or duplicate terminals");
                    return false;
                }

                if(_non_terminals.size() != non_terminals.size()) {
                    set_error("inconsistent or duplicate non-terminals");
                    return false;
                }
                for(const auto& terminal : _terminals) {
                    for(const auto& non_terminal : _non_terminals) {
                        if(terminal == non_terminal) {
                            set_error("terminals and non_terminals not disjoint");
                            return false;
                        }
                    }
                }

                for(const auto& _prod : _grammar) {
                    production prod;
                    if(_non_terminals.find(_prod.first) == _non_terminals.end()) {
                        set_error("non-terminal not found:" + _prod.first);
                        return false;
                    }

                    prod.set_parent(_prod.first);

                    vrule rules;
                    for(const auto& _rule : _prod.second) {
                        rule prod_rule;
                        for (const auto& entity: _rule) {
                            if(_non_terminals.find(entity) == _non_terminals.end() &&
                            _terminals.find(entity) == _terminals.end() &&
                            entity != EPSILON) {
                                set_error("rule token is not defined: " + entity);
                                return false;
                            }
                        }
                        prod_rule.set_entities(_rule);
                        rules.push_back(prod_rule);
                    }

                    prod.set_rules(rules);

                    grammar.push_back(prod);
                }

                return true;
            } else {
                error = "grammar: file not found";
                return false;
            }
        }

        void set_error(string mes) {
            error = "grammar: " + mes;
        }

        const vs& get_terminals() {
            return terminals;
        };

        const vs& get_non_terminals() {
            return non_terminals;
        };

        const string& get_start_symbol() {
            return start_symbol;
        };

        const vprod& get_grammar() {
            return grammar;
        };

        const string& get_error() {
            return error;
        };
    };

}