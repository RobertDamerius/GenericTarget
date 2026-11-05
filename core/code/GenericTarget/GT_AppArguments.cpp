#include <GenericTarget/GT_AppArguments.hpp>
using namespace gt;


void AppArguments::Parse(int argc, char** argv){
    args.clear();
    for(int i = 0; i < argc; ++i){
        args.emplace_back(argv[i]);
    }
}
