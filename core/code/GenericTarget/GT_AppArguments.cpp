#include <GenericTarget/GT_AppArguments.hpp>
using namespace gt;


AppArguments::AppArguments(){
    console = false;
}

void AppArguments::Parse(int argc, char** argv){
    // set default values
    console = false;

    // store all arguments, set flags for known ones
    args.clear();
    for(int i = 0; i < argc; ++i){
        std::string arg(argv[i]);
        args.push_back(arg);
        console |= (0 == arg.compare("--console"));
    }
}
