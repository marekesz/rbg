#ifndef SWITCH
#define SWITCH

#include<cassert>

#include"game_move.hpp"
#include"abstract_dispatcher.hpp"

namespace rbg_parser{

class player_switch : public game_move{
        token to_player;
    public:
        player_switch(const token& to_player);
        ~player_switch(void)override=default;
        player_switch(void)=default;
        player_switch(player_switch&&)=default;
        player_switch(const player_switch&)=delete;
        player_switch& operator=(const player_switch&)=delete;
        player_switch& operator=(player_switch&&)=default;
        std::unique_ptr<game_move> simplify(void)override{return std::unique_ptr<game_move>(new player_switch(std::move(*this)));};
        void accept(abstract_dispatcher& dispatcher)const override;
        uint priority(void)const override{return 3;};
        std::string to_rbg(uint)const override;
        std::string to_rbg()const override;
        std::unique_ptr<game_move> flatten(void)override;
        straightness_result compute_k_straightness(void)const override;
        bool has_finisher(void)const override{return true;};
        const token& get_player(void)const;
        std::unique_ptr<game_move> copy(void)const override;
};

class keeper_switch : public game_move{
        bool deterministic;
    public:
        keeper_switch(bool deterministic);
        ~keeper_switch(void)override=default;
        keeper_switch(void)=default;
        keeper_switch(keeper_switch&&)=default;
        keeper_switch(const keeper_switch&)=delete;
        keeper_switch& operator=(const keeper_switch&)=delete;
        keeper_switch& operator=(keeper_switch&&)=default;
        std::unique_ptr<game_move> simplify(void)override{return std::unique_ptr<game_move>(new keeper_switch(std::move(*this)));};
        void accept(abstract_dispatcher& dispatcher)const override;
        uint priority(void)const override{return 3;};
        std::string to_rbg(uint)const override;
        std::string to_rbg()const override;
        std::unique_ptr<game_move> flatten(void)override;
        straightness_result compute_k_straightness(void)const override;
        bool has_finisher(void)const override{return true;};
        bool is_deterministic(void)const;
        std::unique_ptr<game_move> copy(void)const override;
};

}

#endif
