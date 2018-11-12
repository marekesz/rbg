#ifndef ABSTRACT_DISPATCHER
#define ABSTRACT_DISPATCHER

namespace rbg_parser{

class sum;
class concatenation;
class star_move;
class shift;
class ons;
class off;
class assignment;
class player_switch;
class keeper_switch;
class move_check;
class arithmetic_comparison;
class integer_arithmetic;
class variable_arithmetic;
class arithmetic_operation;

class abstract_dispatcher{
    public:
        virtual void dispatch(const sum&)=0;
        virtual void dispatch(const concatenation&)=0;
        virtual void dispatch(const star_move&)=0;
        virtual void dispatch(const shift&)=0;
        virtual void dispatch(const ons&)=0;
        virtual void dispatch(const off&)=0;
        virtual void dispatch(const assignment&)=0;
        virtual void dispatch(const player_switch&)=0;
        virtual void dispatch(const keeper_switch&)=0;
        virtual void dispatch(const move_check&)=0;
        virtual void dispatch(const arithmetic_comparison&)=0;
        virtual void dispatch(const integer_arithmetic&)=0;
        virtual void dispatch(const variable_arithmetic&)=0;
        virtual void dispatch(const arithmetic_operation&)=0;
        virtual ~abstract_dispatcher(void)=default;
};

}

#endif
