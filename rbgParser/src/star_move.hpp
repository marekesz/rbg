#ifndef STAR_MOVE
#define STAR_MOVE

#include<memory>

#include"game_move.hpp"
#include"abstract_dispatcher.hpp"

namespace rbg_parser{

class star_move : public game_move{
        std::unique_ptr<game_move> contained_move;
        star_move(void)=default;
    public:
        star_move(std::unique_ptr<game_move>&& contained_move);
        ~star_move(void)override=default;
        star_move(star_move&&)=default;
        star_move(const star_move&)=delete;
        star_move& operator=(const star_move&)=delete;
        star_move& operator=(star_move&&)=default;
        bool modifies(void)const override;
        std::unique_ptr<game_move> simplify(void)override;
        void accept(abstract_dispatcher& dispatcher)const override;
        uint priority(void)const override{return 3;};
        std::string to_rbg(uint indent)const override;
        std::string to_rbg()const override;
        std::unique_ptr<game_move> flatten(void)override;
        straightness_result compute_k_straightness(void)const override;
        const game_move* get_content(void)const;
        std::unique_ptr<game_move> copy(void)const override;
};

}

#endif
