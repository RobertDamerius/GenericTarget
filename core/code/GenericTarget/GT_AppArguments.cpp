#include <GenericTarget/GT_AppArguments.hpp>
using namespace gt;


AppArguments::AppArguments(){
    console = false;
}

void AppArguments::Parse(int argc, char** argv){
    // set default values
    console = false;
    bool help = false;

    // store all arguments, set flags for known ones
    args.clear();
    for(int i = 0; i < argc; ++i){
        std::string arg(argv[i]);
        args.push_back(arg);
        console |= (0 == arg.compare("--console"));
        help |= (0 == arg.compare("--help"));
    }

    // print help if requested
    if(help){
        PrintHelp();
    }
}

void AppArguments::PrintHelp(void){
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Syntax: GenericTarget [--console] [--help] [--stop]\n");
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Options:\n");
    GENERIC_TARGET_PRINT_RAW("    --console   Print stdout/stderr to the console instead of redirecting them to a protocol file.\n");
    GENERIC_TARGET_PRINT_RAW("    --help      Show this help page.\n");
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("\n");
}

