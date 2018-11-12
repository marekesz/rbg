#ifndef GRAPH_BUILDER
#define GRAPH_BUILDER

namespace rbg_parser{

class graph;
class messages_container;

class graph_builder{
    public:
        virtual graph build_graph(messages_container& msg)const=0;
        virtual ~graph_builder(void)=default;
};

}

#endif
