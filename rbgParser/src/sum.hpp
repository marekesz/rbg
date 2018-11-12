#ifndef SUM
#define SUM

#include<vector>
#include<memory>

#include"game_move.hpp"
#include"abstract_dispatcher.hpp"

namespace rbg_parser{

class sum : public game_move{
        std::vector<std::unique_ptr<game_move>> content;
    public:
        sum(std::vector<std::unique_ptr<game_move>>&& content);
        ~sum(void)override=default;
        sum(void)=default;
        sum(sum&&)=default;
        sum(const sum&)=delete;
        sum& operator=(const sum&)=delete;
        sum& operator=(sum&&)=default;
        bool modifies(void)const override;
        std::unique_ptr<game_move> simplify(void)override;
        void accept(abstract_dispatcher& dispatcher)const override;
        uint priority(void)const override{return 1;};
        std::string to_rbg(uint indent)const override;
        std::string to_rbg()const override;
        std::unique_ptr<game_move> flatten(void)override;
        void gather_sum_elements(std::vector<std::unique_ptr<game_move>>& elements)override;
        straightness_result compute_k_straightness(void)const override;
        const std::vector<std::unique_ptr<game_move>>& get_content(void)const;
        void add_move(std::unique_ptr<game_move> m);
        std::unique_ptr<game_move> copy(void)const override;
};

}

#endif
