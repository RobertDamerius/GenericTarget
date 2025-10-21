#include <GenericTarget/GT_ApplicationArguments.hpp>
using namespace gt;


ApplicationArguments::ApplicationArguments(){
    stop = false;
    console = false;
}

void ApplicationArguments::Parse(int argc, char** argv){
    // set default values
    stop = false;
    console = false;
    bool help = false;

    // store all arguments, set flags for known ones
    args.clear();
    for(int i = 0; i < argc; ++i){
        std::string arg(argv[i]);
        args.push_back(arg);
        stop |= (0 == arg.compare("--stop"));
        console |= (0 == arg.compare("--console"));
        help |= (0 == arg.compare("--help"));
    }

    // print help if requested
    if(help){
        PrintHelp();
    }
}

void ApplicationArguments::PrintHelp(void){
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Syntax: GenericTarget [--console] [--help] [--stop]\n");
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("Options:\n");
    GENERIC_TARGET_PRINT_RAW("    --console   Print stdout/stderr to the console instead of redirecting them to a protocol file.\n");
    GENERIC_TARGET_PRINT_RAW("    --help      Show this help page.\n");
    GENERIC_TARGET_PRINT_RAW("    --stop      Stop another possibly running target application and also stop this application.\n");
    GENERIC_TARGET_PRINT_RAW("\n");
    GENERIC_TARGET_PRINT_RAW("\n");
}

