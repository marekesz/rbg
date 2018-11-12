#ifndef OFFS
#define OFFS

#include<cassert>

#include"game_move.hpp"
#include"token.hpp"
#include"abstract_dispatcher.hpp"

namespace rbg_parser{

class off : public game_move{
        token off_piece;
        off(token&& off_piece);
        off(void)=default;
    public:
        ~off(void)override=default;
        off(off&&)=default;
        off(const off&)=delete;
        off& operator=(const off&)=delete;
        off& operator=(off&&)=default;
        std::unique_ptr<game_move> simplify(void)override{return std::unique_ptr<game_move>(new off(std::move(*this)));};
        void accept(abstract_dispatcher& dispatcher)const override;
        uint priority(void)const override{return 4;};
        std::string to_rbg(uint)const override;
        std::string to_rbg()const override;
        std::unique_ptr<game_move> flatten(void)override;
        straightness_result compute_k_straightness(void)const override;
        bool check_if_redundant(std::set<token>&, bool& already_met_off)const override;
        const token& get_piece(void)const;
        bool is_lazy(void)const;
        std::unique_ptr<game_move> copy(void)const override;
        friend std::unique_ptr<game_move> make_offs_sum(std::vector<token>&& legal_offs);
};

std::unique_ptr<game_move> make_offs_sum(std::vector<token>&& legal_offs);

}

#endif
