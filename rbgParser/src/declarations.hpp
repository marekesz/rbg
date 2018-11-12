#ifndef DECLARATIONS
#define DECLARATIONS

#include<set>
#include<map>
#include<string>

#include"token.hpp"

namespace rbg_parser{

class declarations{
        std::map<token, uint> players_names;
        std::set<token> pieces_names;
        std::map<token, uint> variables_names;
        std::set<token> edges_names;
    public:
        declarations(
            std::map<token, uint>&& players_names,
            std::set<token>&& pieces_names,
            std::map<token, uint>&& variables_names);
        const std::set<token>& get_legal_pieces(void)const;
        const std::map<token, uint>& get_legal_players(void)const;
        const std::map<token, uint>& get_legal_variables(void)const;
        const std::set<token>& get_legal_edges(void)const;
        uint get_player_bound(const token& player_name)const;
        uint get_variable_bound(const token& variable_name)const;
        void add_edge_label(const token& name);
        std::string to_rbg(void)const;
};

}

#endif
