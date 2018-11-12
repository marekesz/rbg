#include <iostream>
#include <chrono>
#include <boost/container/small_vector.hpp>
#include "reasoner.hpp"

typedef unsigned int uint;
constexpr int KEEPER = 0;

reasoner::resettable_bitarray_stack cache;
reasoner::game_state initial_state;
unsigned long states_count;

unsigned long perft_state_at_depth(reasoner::game_state& state, uint depth){
    if(depth == 0 and state.get_current_player() != KEEPER){
        ++states_count;
        return 1;
    }
    else{
        if(state.get_current_player() == KEEPER){
            auto any_move = state.get_any_move(cache);
            if(any_move.first){
                state.apply_move(any_move.second);
                return perft_state_at_depth(state, depth);
            }
            else{
                ++states_count;
                if(depth == 0)
                    return 1;
                return 0;
            }
        }
        else{
            ++states_count;
            auto copy_state = state;
            auto legal_moves = copy_state.get_all_moves(cache);
            unsigned long result = 0;
            for(const auto& el: legal_moves){
                auto temp_state = copy_state;
                temp_state.apply_move(el);
                result += perft_state_at_depth(temp_state,depth-1);
            }
            return result;
        }
    }
}

uint perft(uint depth){
    return perft_state_at_depth(initial_state,depth);
}

int main(int argv, char** argc){
    if(argv != 2){
        std::cout << "Perft depth unspecified. Exitting..." << std::endl;
        return 1;
    }
    while(initial_state.get_current_player() == KEEPER){
        auto any_move = initial_state.get_any_move(cache);
        if(any_move.first)
            initial_state.apply_move(any_move.second);
    }

    uint depth = std::stoi(argc[1]);
    std::chrono::steady_clock::time_point start_time(std::chrono::steady_clock::now());
    auto p = perft(depth);
    std::chrono::steady_clock::time_point end_time(std::chrono::steady_clock::now());

    uint ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
    std::cout<<"perft: "<<p<<std::endl;
    std::cout<<"took: "<<ms<<" ms"<<std::endl;
    std::cout<<"visited: "<<states_count<<" states"<<std::endl;
    std::cout << std::fixed << static_cast<double>(states_count)/static_cast<double>(ms)*1000.0 << " states/sec" << std::endl;
    return 0;
}
