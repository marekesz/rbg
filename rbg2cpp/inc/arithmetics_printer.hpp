#ifndef ARITHMETICS_PRINTER
#define ARITHMETICS_PRINTER

#include"abstract_dispatcher.hpp"
#include"types.hpp"
#include"token.hpp"
#include<cassert>
#include<map>

namespace rbg_parser{
    class arithmetic_expression;
}

class arithmetics_printer : public rbg_parser::abstract_dispatcher{
        const std::map<rbg_parser::token, uint>& pieces_to_id;
        const std::map<rbg_parser::token, uint>& variables_to_id;
        std::string final_result;
        bool static_content;
        int value;
    public:
        arithmetics_printer(void)=delete;
        arithmetics_printer(const arithmetics_printer&)=delete;
        arithmetics_printer(arithmetics_printer&&)=default;
        arithmetics_printer& operator=(const arithmetics_printer&)=delete;
        arithmetics_printer& operator=(arithmetics_printer&&)=delete;
        ~arithmetics_printer(void)override=default;
        arithmetics_printer(const std::map<rbg_parser::token, uint>& pieces_to_id, const std::map<rbg_parser::token, uint>& variables_to_id);
        void dispatch(const rbg_parser::sum&)override{assert(false);}
        void dispatch(const rbg_parser::concatenation&)override{assert(false);}
        void dispatch(const rbg_parser::star_move&)override{assert(false);}
        void dispatch(const rbg_parser::shift&)override{assert(false);}
        void dispatch(const rbg_parser::ons&)override{assert(false);}
        void dispatch(const rbg_parser::off&)override{assert(false);}
        void dispatch(const rbg_parser::assignment&)override{assert(false);}
        void dispatch(const rbg_parser::player_switch&)override{assert(false);}
        void dispatch(const rbg_parser::keeper_switch&)override{assert(false);}
        void dispatch(const rbg_parser::move_check&)override{assert(false);}
        void dispatch(const rbg_parser::arithmetic_comparison&)override{assert(false);}
        void dispatch(const rbg_parser::integer_arithmetic& m)override;
        void dispatch(const rbg_parser::variable_arithmetic& m)override;
        void dispatch(const rbg_parser::arithmetic_operation& m)override;
        std::string get_final_result(void);
        bool can_be_precomputed(void);
        int precomputed_value(void);
};

#endif
