#ifndef STRAIGHTNESS_HELPERS
#define STRAIGHTNESS_HELPERS

namespace rbg_parser{

// maximum of modifiers in move
// -1 = couldn't construct any move given type
// -2 = infinity
// non-negative = count
class straightness_result{
        int self_contained_moves;
        int beginning_moves;
        int ending_moves;
        int only_coming_through_moves;
        straightness_result(int self_contained_moves,int beginning_moves,int ending_moves,int only_coming_through_moves);
    public:
        void repeat_result(int times=0);
        void max_of_results(const straightness_result& rhs);
        void concatenate_result(const straightness_result& rhs);
        void wrap_in_check(void);
        int final_result(void)const;
        friend straightness_result standard_non_switch(void);
        friend straightness_result modifier_non_switch(void);
        friend straightness_result standard_switch(void);
        friend straightness_result empty_move(void);
};

straightness_result standard_non_switch(void);
straightness_result modifier_non_switch(void);
straightness_result standard_switch(void);
straightness_result empty_move(void);

}

#endif
