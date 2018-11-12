#include"straightness_helpers.hpp"
#include<algorithm>

namespace rbg_parser{

int repeat(int previous_result, int times){
    if(previous_result==-2)
        return -2;
    if(previous_result==0)
        return 0;
    if(times==0 && previous_result>0)
        return -2;
    if(times==0 && previous_result==-1)
        return 0;
    if(times>0 && previous_result==-1)
        return -1;
    return previous_result*times;
}

int count_max_of_straightness(int previous_result,int current_result){
    if(previous_result==-2 || current_result==-2)
        return -2;
    if(previous_result==-1)
        return current_result;
    if(current_result==-1)
        return previous_result;
    return std::max(previous_result,current_result);
}

int append_results(int r1, int r2){
    if(r1==-1 || r2==-1)
        return -1;
    if(r1==-2 || r2==-2)
        return -2;
    return r1+r2;
}

straightness_result::straightness_result(int self_contained_moves,int beginning_moves,int ending_moves,int only_coming_through_moves):
self_contained_moves(self_contained_moves),
beginning_moves(beginning_moves),
ending_moves(ending_moves),
only_coming_through_moves(only_coming_through_moves){
}

void straightness_result::repeat_result(int times){
    if(times==0){
        self_contained_moves =
            count_max_of_straightness(
                append_results(append_results(beginning_moves,repeat(only_coming_through_moves,times)),ending_moves),
                self_contained_moves
            );
        beginning_moves = append_results(beginning_moves,repeat(only_coming_through_moves,times));
        ending_moves = append_results(repeat(only_coming_through_moves,times),ending_moves);
    }
    else if(times==2){
        self_contained_moves =
            count_max_of_straightness(
                append_results(beginning_moves,ending_moves),
                self_contained_moves
            );
        beginning_moves =
            count_max_of_straightness(
                append_results(beginning_moves,only_coming_through_moves),
                beginning_moves
            );
        ending_moves =
            count_max_of_straightness(
                ending_moves,
                append_results(only_coming_through_moves,ending_moves)

            );
    }
    else if(times>2){
        self_contained_moves =
            count_max_of_straightness(
                append_results(append_results(beginning_moves,repeat(only_coming_through_moves,times-2)),ending_moves),
                self_contained_moves
            );
        beginning_moves =
            count_max_of_straightness(
                append_results(beginning_moves,repeat(only_coming_through_moves,times-1)),
                beginning_moves
            );
        ending_moves =
            count_max_of_straightness(
                ending_moves,
                append_results(repeat(only_coming_through_moves,times-1),ending_moves)

            );
    }
    only_coming_through_moves = repeat(only_coming_through_moves,times);
}

void straightness_result::max_of_results(const straightness_result& rhs){
    self_contained_moves = count_max_of_straightness(self_contained_moves,rhs.self_contained_moves);
    beginning_moves = count_max_of_straightness(beginning_moves,rhs.beginning_moves);
    ending_moves = count_max_of_straightness(ending_moves,rhs.ending_moves);
    only_coming_through_moves = count_max_of_straightness(only_coming_through_moves,rhs.only_coming_through_moves);
}

void straightness_result::concatenate_result(const straightness_result& rhs){
    self_contained_moves = count_max_of_straightness(count_max_of_straightness(self_contained_moves,rhs.self_contained_moves),append_results(beginning_moves,rhs.ending_moves));
    beginning_moves = count_max_of_straightness(append_results(beginning_moves,rhs.only_coming_through_moves),rhs.beginning_moves);
    ending_moves = count_max_of_straightness(append_results(only_coming_through_moves,rhs.ending_moves),ending_moves);
    only_coming_through_moves = append_results(only_coming_through_moves,rhs.only_coming_through_moves);
}

void straightness_result::wrap_in_check(void){
    ending_moves = count_max_of_straightness(ending_moves, only_coming_through_moves);
    beginning_moves = -1;
    self_contained_moves = -1;
    only_coming_through_moves = 0;
}

int straightness_result::final_result(void)const{
    return count_max_of_straightness(count_max_of_straightness(self_contained_moves,beginning_moves),count_max_of_straightness(ending_moves,only_coming_through_moves));
}

straightness_result standard_non_switch(void){
    return straightness_result(-1,-1,-1,0);
}

straightness_result modifier_non_switch(void){
    return straightness_result(-1,-1,-1,1);
}

straightness_result standard_switch(void){
    return straightness_result(-1,0,0,-1);
}

straightness_result empty_move(void){
    return straightness_result(-1,-1,-1,-1);
}

}
