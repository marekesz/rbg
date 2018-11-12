#ifndef PRECOMPUTED_PATTERN
#define PRECOMPUTED_PATTERN

#include<set>
#include<string>

#include"types.hpp"

class cpp_container;
class actions_compiler;

class precomputed_pattern{
        std::set<uint> allowed_cells;
        uint overall_size;
    public:
        precomputed_pattern(uint overall_size);
        void add_cell(uint cell);
        bool evaluates_to_true(uint cell)const;
        void negate(void);
        void print_inside_transition(cpp_container& output, const actions_compiler& ac)const;
};

#endif
