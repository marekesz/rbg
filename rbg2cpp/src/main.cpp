#include<iostream>
#include<string>
#include<sstream>

#include"compiler_options.hpp"
#include"token.hpp"
#include"message.hpp"
#include"game_items.hpp"
#include"cpp_container.hpp"
#include"game_compiler.hpp"

int main(int argc, const char** argv){
    if(argc < 2){
        std::cerr<<"No input specified"<<std::endl;
        std::cerr<<"Usage: \"rbg2cpp [flags] input_file\""<<std::endl;
        std::cerr<<"Available flags:"<<std::endl;
        std::cerr<<"\"-o output_file\" - write output to file with given name; defaults to \"game\""<<std::endl;
        std::cerr<<"\"-Whide\" - do not show warnings"<<std::endl;
        std::cerr<<"\"-Werror\" - treat warnings as errors"<<std::endl;
    }
    else{
        rbg_parser::messages_container msg;
        try{
            std::string input_file_name(argv[argc-1]);
            compiler_options o(argc-2, argv+1);
            std::ifstream t(input_file_name);
            std::stringstream buffer;
            buffer << t.rdbuf();
            std::vector<rbg_parser::token> result = rbg_parser::tokenize(buffer.str(),msg);
            rbg_parser::game_items g = rbg_parser::input_tokens(result,msg);
            rbg_parser::parsed_game pg = g.parse_game(msg);
            if(not o.escalating_warnings() or msg.is_empty()){
                if(not msg.is_empty() and o.showing_warnings())
                    msg.write_as_warnings(std::cout);
                game_compiler compiler(pg, o);
                const auto& c = compiler.compile();
                c.print_files();
            }
            else
                msg.write_as_errors(std::cout);
        }
        catch(rbg_parser::message& m){
            std::cout<<m.as_error()<<std::endl;
        }
        catch(std::exception& e){
            std::cout<<e.what()<<std::endl;
        }
    }
    return 0;
}
