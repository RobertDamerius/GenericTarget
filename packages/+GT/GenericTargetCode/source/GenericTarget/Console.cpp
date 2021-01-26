#include <Console.hpp>


std::mutex Console::mtx;


static void __console_print(FILE *stream, std::string preString, console_color_t foreground, console_color_t background, const char *format, va_list *argptr){
    #ifndef    __WIN32__
    if(preString.size()) fprintf(stream, "%s", preString.c_str());
    if(argptr) vfprintf(stream, format, *argptr);
    else fprintf(stream, "%s", format);
    (void)foreground;
    (void)background;
    #else
    HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD color = (foreground | (background << 4));
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hstdout, &csbi);
    SetConsoleTextAttribute(hstdout, color);
    if(preString.size()) fprintf(stream, "%s", preString.c_str());
    if(argptr) vfprintf(stream, format, *argptr);
    else fprintf(stream, "%s", format);
    SetConsoleTextAttribute(hstdout, csbi.wAttributes);
    #endif
    fflush(stream);
}

void Console::ColorMessageP(std::string preString, console_color_t foreground, console_color_t background, const char* format, ...){
    Console::mtx.lock();
    va_list argptr;
    va_start(argptr, format);
    __console_print(stdout, preString, foreground, background, format, &argptr);
    va_end(argptr);
    Console::mtx.unlock();
}

void Console::ColorMessage(console_color_t foreground, console_color_t background, const char* format, ...){
    Console::mtx.lock();
    va_list argptr;
    va_start(argptr, format);
    __console_print(stdout, std::string(""), foreground, background, format, &argptr);
    va_end(argptr);
    Console::mtx.unlock();
}

void Console::Message(const char* format, ...){
    Console::mtx.lock();
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
    Console::mtx.unlock();
}

void Console::SuccessP(std::string preString, const char* format, ...){
    Console::mtx.lock();
    va_list argptr;
    va_start(argptr, format);
    __console_print(stdout, preString, CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK, format, &argptr);
    va_end(argptr);
    Console::mtx.unlock();
}

void Console::Success(const char* format, ...){
    Console::mtx.lock();
    va_list argptr;
    va_start(argptr, format);
    __console_print(stdout, std::string(""), CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK, format, &argptr);
    va_end(argptr);
    Console::mtx.unlock();
}

void Console::WarningP(std::string preString, const char* format, ...){
    Console::mtx.lock();
    va_list argptr;
    va_start(argptr, format);
    __console_print(stdout, preString, CONSOLE_COLOR_YELLOW, CONSOLE_COLOR_BLACK, format, &argptr);
    va_end(argptr);
    Console::mtx.unlock();
}

void Console::Warning(const char* format, ...){
    Console::mtx.lock();
    va_list argptr;
    va_start(argptr, format);
    __console_print(stdout, std::string(""), CONSOLE_COLOR_YELLOW, CONSOLE_COLOR_BLACK, format, &argptr);
    va_end(argptr);
    Console::mtx.unlock();
}

void Console::ErrorP(std::string preString, const char* format, ...){
    Console::mtx.lock();
    va_list argptr;
    va_start(argptr, format);
    __console_print(stderr, preString, CONSOLE_COLOR_RED, CONSOLE_COLOR_BLACK, format, &argptr);
    va_end(argptr);
    Console::mtx.unlock();
}

void Console::Error(const char* format, ...){
    Console::mtx.lock();
    va_list argptr;
    va_start(argptr, format);
    __console_print(stderr, std::string(""), CONSOLE_COLOR_RED, CONSOLE_COLOR_BLACK, format, &argptr);
    va_end(argptr);
    Console::mtx.unlock();
}

void Console::HexDump(const uint8_t* buffer, size_t length){
    Console::mtx.lock();
    __console_print(stdout, std::string(""), CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK, "Offset(h)  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", nullptr);
    for(size_t n = 0; n < length; n++){
        if(!(n % 16)){
            if(n){
                __console_print(stdout, std::string(""), CONSOLE_COLOR_GREY, CONSOLE_COLOR_BLACK, "  ", nullptr);
                std::string plainText;
                for(int i = 0; i < 16; i++){
                    unsigned char c = buffer[n - 16 + i];
                    plainText.push_back((c < 0x20) ? '.' : c);
                }
                __console_print(stdout, std::string(""), CONSOLE_COLOR_CYAN, CONSOLE_COLOR_DARKBLUE, plainText.c_str(), nullptr);
            }
            char strOffset[16];
            sprintf(strOffset, " %08X ", (uint32_t)n);
            __console_print(stdout, std::string("\n"), CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK, strOffset, nullptr);
        }
        char str[4];
        sprintf(str," %02X", buffer[n]);
        __console_print(stdout, std::string(""), CONSOLE_COLOR_GREY, CONSOLE_COLOR_BLACK, str, nullptr);
        if(n == (length - 1)){
            std::string wspace = "  ";
            for(int i = (int)(n % 16); i < 15; i++)
                wspace += "   ";
            __console_print(stdout, std::string(""), CONSOLE_COLOR_GREY, CONSOLE_COLOR_BLACK, wspace.c_str(), nullptr);
            std::string plainText;
            for(size_t k = length - (n % 16) - 1; k < length; k++)
                plainText.push_back((buffer[k] < 0x20) ? '.' : buffer[k]);
            __console_print(stdout, std::string(""), CONSOLE_COLOR_CYAN, CONSOLE_COLOR_DARKBLUE, plainText.c_str(), nullptr);
        }
    }
    __console_print(stdout, std::string(""), CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK, "\n", nullptr);
    Console::mtx.unlock();
}

void Console::HexDump(const std::vector<uint8_t>& buffer){
    HexDump((const uint8_t*)&(buffer[0]), buffer.size());
}

