#include <GenericTarget/GT_Common.hpp>


const std::string_view gt::builtTimestamp{__DATE__ " " __TIME__};

double gt::get_log_elapsed_seconds(void){
    static const auto start = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
}
