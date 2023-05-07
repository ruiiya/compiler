#pragma once

#include "grammar.h"
#include <algorithm>
#include <unordered_map>
#include <set>
#include <functional>
using namespace std;
namespace xd {

    const string STRING_ENDMARKER = "__$__";

    vrule get_rules_with_parent(const vprod& grammar, const string& parent) {
        for(const auto& prod : grammar) {
            if(prod.get_parent() == parent) {
                return prod.get_rules();
            }
        }
        return vrule{};
    }

    vs get_all_non_terminals(const vprod& grammar) {
        vs non_terminals;
        for(const auto& prod : grammar) {
            non_terminals.push_back(prod.get_parent());
        }
        return non_terminals;
    }

    bool has_parent(const vprod& grammar, string parent) {
        return std::any_of(grammar.begin(), grammar.end(), [&](const production& prod) {
            return prod.get_parent() == parent;
        });
    }

    vs get_all_terminals(const vprod& grammar) {
        vs terminals;
        for(const auto& prod : grammar) {
            for(const auto& rule : prod.get_rules()) {
                for(const auto& entity: rule.get_entities()) {
                    if(!has_parent(grammar, entity) && entity != EPSILON) {
                        terminals.push_back(entity);
                    }
                }
            }
        }
        terminals.erase(std::unique(terminals.begin(), terminals.end()), terminals.end());
        return terminals;
    }

    using nullables_table = unordered_map<string, bool>;

    nullables_table calc_nullables(const vprod& grammar) {
        nullables_table nullables;
        auto terminals = get_all_terminals(grammar);
        for(const auto& terminal : terminals) {
            nullables[terminal] = false;
        }

        nullables[EPSILON] = true;

        function<bool(const string&, vs&)> calc_recursive = [&](const string& key, vs& path) {
            if(find(path.begin(), path.end(), key) != path.end()) {
                return false;
            }

            if(nullables.find(key) != nullables.end()) {
                return nullables[key];
            }

            path.push_back(key);
            
            for(const auto& rule: get_rules_with_parent(grammar, key)) {
                bool perma_stop = false;
                for(auto& entity : rule.get_entities()) {
                    if(!calc_recursive(entity, path)) {
                        perma_stop = true;
                        break;
                    }
                }

                if(!perma_stop) {
                    nullables[key] = true;
                    path.pop_back();
                    return true;
                }
            }

            nullables[key] = false;
            path.pop_back();
            return false;
        };

        for(const auto& prod : grammar) {
            vs path;
            calc_recursive(prod.get_parent(), path);
        }

        return nullables;
    }

    using symbols_table = unordered_map<string, vs>;

    symbols_table calc_firsts(const vprod& grammar, nullables_table nullables) {
        symbols_table firsts;
        auto terminals = get_all_terminals(grammar);
        for(const auto& terminal : terminals) {
            firsts[terminal] = {terminal};
        }

        firsts[EPSILON] = {EPSILON};

        bool finish = false;

        function<vs(const string&, vs&)> calc_recursive = [&](const string& key, vs& path) {
            if(!has_parent(grammar, key)) {
                return firsts[key];
            }

            if(find(path.begin(), path.end(), key) != path.end()) {
                return firsts[key];
            }

            if(firsts.find(key) != firsts.end()) {
                return firsts[key];
            }

            path.push_back(key);

            if(firsts.find(key) == firsts.end()) {
                firsts[key] = {};
                finish = false;
            }

            for(const auto& rule: get_rules_with_parent(grammar, key)) {
                bool perma_stop = false;
                for(auto& entity : rule.get_entities()) {
                    vs first = calc_recursive(entity, path);
                    for(const auto& der : first) {
                        if(der != EPSILON && find(firsts[key].begin(), firsts[key].end(), der) == firsts[key].end()) {
                            firsts[key].push_back(der);
                            finish = false;
                        }
                    }

                    if(!nullables.at(entity)) {
                        perma_stop = true;
                        break;
                    }
                }

                if(!perma_stop) {
                    if(find(firsts[key].begin(), firsts[key].end(), EPSILON) == firsts[key].end()) {
                        firsts[key].push_back(EPSILON);
                        finish = false;
                    }
                }
            }

            path.pop_back();
            return firsts[key];
        };

        while(!finish) {
            finish = true;
            for(const auto& prod : grammar) {
                vs path;
                calc_recursive(prod.get_parent(), path);
            }
        }

        for(const auto& terminal : terminals) {
            firsts.erase(terminal);
        }

        firsts.erase(EPSILON);

        return firsts;
    }

    symbols_table calc_follows(const vprod& grammar, const nullables_table& nullables, const symbols_table& firsts, const string& start_symbol) {
        symbols_table follows;
        bool finish = false;

        auto aug_firsts = firsts;
        auto terminals = get_all_terminals(grammar);
        for(const auto& terminal : terminals) {
            aug_firsts[terminal] = {terminal};
        }

        aug_firsts[EPSILON] = {EPSILON};

        for(const auto& prod : grammar) {
            if(prod.get_parent() == start_symbol) {
                follows[prod.get_parent()] = {STRING_ENDMARKER};
            } else {
                follows[prod.get_parent()] = {};
            }
        }

        function<void(const string&)> calc_recursive = [&](const string& key) {
            for(const auto& rule : get_rules_with_parent(grammar, key)) {
                for(auto entity_it = rule.get_entities().begin(); entity_it != rule.get_entities().end(); entity_it++) {
                    auto curr = *entity_it;
                    if(has_parent(grammar, curr)) {
                        auto next_it = entity_it + 1;
                        for(; next_it != rule.get_entities().end(); next_it++) {
                            for(const auto& der : aug_firsts[*next_it]) {
                                if(der != EPSILON && find(follows[curr].begin(), follows[curr].end(), der) == follows[curr].end()) {
                                    follows[curr].push_back(der);
                                    finish = false;
                                }   
                            }

                            if(!nullables.at(*next_it)) {
                                break;
                            }
                        }

                        if(next_it == rule.get_entities().end()) {
                            for(const auto& der : follows[key]) {
                                if(find(follows[curr].begin(), follows[curr].end(), der) == follows[curr].end()) {
                                    follows[curr].push_back(der);
                                    finish = false;
                                }   
                            }
                        }
                    }
                }
            }  
        };

        while(!finish) {
            finish = true;
            calc_recursive(start_symbol);

            for(const auto& prod : grammar) {
                if(prod.get_parent() != start_symbol) {
                    calc_recursive(prod.get_parent());
                }
            }
        }

        return follows;
    }
}