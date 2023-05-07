#include <bits/stdc++.h>

#include "parser.h"

using namespace std;
using namespace xd;

int main(int argc, char const *argv[])
{

    freopen("file.txt","w",stdout);

    xd::grammar_parser grammar_parser("grammar.dat");
    if(!grammar_parser.parse()) {
        cout << grammar_parser.get_error();
        return -1;
    }

    auto productions = grammar_parser.get_grammar();

    auto nullables = calc_nullables(productions);
    auto firsts = calc_firsts(productions, nullables);
    auto follows = calc_follows(productions, nullables, firsts, grammar_parser.get_start_symbol());

    auto terminals = grammar_parser.get_terminals();
    auto non_terminals = get_all_non_terminals(productions);

    parsing_table parsing_table(terminals, non_terminals);
    parsing_table.set_firsts(firsts);
    parsing_table.set_follows(follows);
    parsing_table.set_productions(productions);
    parsing_table.build_tables();

    auto err = parsing_table.get_errors();
    if (!err.empty())
    {
        for (auto &e : err)
        {
            std::cout << e << '\n';
        }
        return -1;
    }

    vector<token> tokens({{"void", "void"}, {"identifier", "main"}, {"(", "("}, {")", ")"}, {"{", "{"}, {"integer_constant", "1"}, {"+", "+"}, {"integer_constant", "2"}, {"-", "-"}, {"integer_constant", "3"}, {"+", "+"}, {"integer_constant", "4"}, {"/", "/"}, {"integer_constant", "5"}, {";", ";"}, {"}", "}"}, 
    });

    parser ll1_parser;
    ll1_parser.set_input_tokens(tokens);
    ll1_parser.set_parsing_table(parsing_table);
    ll1_parser.set_start_symbol(grammar_parser.get_start_symbol());

    ll1_parser.restart();

    while(!ll1_parser.is_complete()) {
        ll1_parser.parser_next_step();
    }

    err = ll1_parser.get_errors();
    if (!err.empty())
    {
        for (auto &e : err)
        {
            std::cout << e << '\n';
        }
        return -1;
    }

    ll1_parser.build_tree();
    ll1_parser.remove_epsilon_node();
    ll1_parser.remove_one_kid_node();
    ll1_parser.print_node();

    return 0;
}
