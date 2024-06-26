#include <bits/stdc++.h>

#include "parser.h"

using namespace std;
using namespace xd;

#define NAME "sample"

int main(int argc, char const *argv[])
{
    string input_filename = "sample.vc";
    string output_filename = "sample.vcps";
    string output_filename_non_reduce = "sample_full.vcps";
    string output_filename_bracket = "sample_bracket.vcps";
    string grammar_filename = "grammar.dat";

    if(argc > 1) {
        input_filename = string(argv[1]) + ".vc";
        if(argc > 2) {
            output_filename = string(argv[2]) + ".vcps";
            output_filename_non_reduce = string(argv[2]) + "_full.vcps";
            output_filename_bracket = string(argv[2]) + "_bracket.vcps";
            if(argc > 3) {
                grammar_filename = string(argv[3]) + ".dat";
            }
        }
    }
    ifstream ifs(input_filename);
    if(!ifs.good()){
        cout << "cannot read input file";
        return -1;
    }
    ofstream ofs(output_filename);
    ofstream ofs2(output_filename_non_reduce);
    ofstream ofs3(output_filename_bracket);

    xd::grammar_parser grammar_parser(grammar_filename);
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

    

    lexer lexer;
    token token_;

    vector<token> tokens;

    while((token_ = lexer.get_token(ifs, grammar_parser.get_terminals())).token_type != "__EOF__") {
        if(token_.token_type == "__COMMENT__") {
            continue;
        }
        if(token_.token_type == "__ERROR__") {
            cout << lexer.get_error();
            return -1;
        }
        tokens.push_back(token_);
        // cout << token_.value << " ";
    }


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
    ll1_parser.print_node(ofs2);
    ll1_parser.remove_epsilon_node();
    ll1_parser.remove_one_kid_node();
    ll1_parser.print_node(ofs);
    ll1_parser.print_bracket(ofs3);

    return 0;
}
