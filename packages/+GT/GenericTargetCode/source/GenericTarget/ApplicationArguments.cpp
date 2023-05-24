#include <GenericTarget/ApplicationArguments.hpp>
using namespace gt;


ApplicationArguments::ApplicationArguments(){
    stop = false;
    console = false;
}

void ApplicationArguments::Parse(int argc, char** argv){
    // Set default values
    stop = false;
    console = false;
    bool help = false;

    // Scan all arguments, ignore unknown arguments
    for(int i = 1; i < argc; ++i){
        std::string arg(argv[i]);
        stop |= (0 == arg.compare("--stop"));
        console |= (0 == arg.compare("--console"));
        help |= (0 == arg.compare("--help"));
    }

    // Print help if requested
    if(help){
        PrintHelp();
    }
}

void ApplicationArguments::PrintHelp(void){
    PrintRaw("\n");
    PrintRaw("Syntax: GenericTarget [--console] [--help] [--stop]\n");
    PrintRaw("\n");
    PrintRaw("Options:\n");
    PrintRaw("    --console   Print stdout/stderr to the console instead of redirecting them to a protocol file.\n");
    PrintRaw("    --help      Show this help page.\n");
    PrintRaw("    --stop      Stop another possibly running target application and also stop this application.\n");
    PrintRaw("\n");
    PrintRaw("\n");
}

