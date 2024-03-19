#include <GenericTarget/GT_Common.hpp>


#if __linux__
const std::string gt::strOS("Linux");
#elif __FreeBSD__
const std::string gt::strOS("FreeBSD");
#elif __ANDROID__
const std::string gt::strOS("Android");
#elif __APPLE__
const std::string gt::strOS("macOS");
#elif _WIN32
const std::string gt::strOS("Windows");
#else
const std::string gt::strOS("unknown");
#endif
const std::string gt::strVersion("20240319");
const std::string gt::strCompilerVersion(__VERSION__);
const std::string gt::strBuilt(__DATE__ " " __TIME__);

