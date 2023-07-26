#include <GenericTarget/Common.hpp>


#if __linux__
const std::string strOS("Linux");
#elif __FreeBSD__
const std::string strOS("FreeBSD");
#elif __ANDROID__
const std::string strOS("Android");
#elif __APPLE__
const std::string strOS("macOS");
#elif _WIN32
const std::string strOS("Windows");
#else
const std::string strOS("unknown");
#endif
const std::string strVersion("20230726");
const std::string strCompilerVersion(__VERSION__);
const std::string strBuilt(__DATE__ " " __TIME__);

