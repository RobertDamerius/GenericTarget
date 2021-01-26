#pragma once


#include <GenericTarget.hpp>


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Logger macros
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define Log(...) \
    do { \
        std::string preString = std::string("   [t=") + std::to_string(GenericTarget::GetTime()) + std::string("] "); \
        Console::ColorMessageP(preString, CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK, __VA_ARGS__); \
    } while(0)
#define LogW(...) \
    do { \
        std::string preString = std::string(" W [t=") + std::to_string(GenericTarget::GetTime()) + std::string("] "); \
        Console::WarningP(preString, __VA_ARGS__); \
    } while(0)
#define LogE(...) \
    do { \
        std::string preString = std::string(" E [t=") + std::to_string(GenericTarget::GetTime()) + std::string("] "); \
        Console::ErrorP(preString, __VA_ARGS__); \
    } while(0)
#define LogRaw(...) \
    do { \
        std::string preString(""); \
        Console::ColorMessageP(preString, CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK, __VA_ARGS__); \
    } while(0)


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Type definitions, enums and structs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Windows console colors
typedef enum {
    CONSOLE_COLOR_BLACK = 0,
    CONSOLE_COLOR_DARKBLUE,
    CONSOLE_COLOR_DARKGREEN,
    CONSOLE_COLOR_DARKCYAN,
    CONSOLE_COLOR_DARKRED,
    CONSOLE_COLOR_DARKMAGENTA,
    CONSOLE_COLOR_DARKYELLOW,
    CONSOLE_COLOR_GREY,
    CONSOLE_COLOR_DARKGREY,
    CONSOLE_COLOR_BLUE,
    CONSOLE_COLOR_GREEN,
    CONSOLE_COLOR_CYAN,
    CONSOLE_COLOR_RED,
    CONSOLE_COLOR_MAGENTA,
    CONSOLE_COLOR_YELLOW,
    CONSOLE_COLOR_WHITE
} console_color_t;


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Classes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/**
 *  @brief Class: Console
 *  @details Draw colored text to the console (thread-safe). Colored text is currently only supported for windows OS.
 */
class Console {
    public:
        /**
         *  @brief Print a message to the console using desired foreground and background color.
         *  @param [in] foreground Foreground color.
         *  @param [in] background Background color.
         *  @param [in] format Format string.
         *  @param [in] ... Variable arguments.
         */
        static void ColorMessage(console_color_t foreground, console_color_t background, const char* format, ...);

        /**
         *  @brief Print a message to the console using desired foreground and background color and a specific prefix.
         *  @param [in] preString The string that should be printed before the actual message.
         *  @param [in] foreground Foreground color.
         *  @param [in] background Background color.
         *  @param [in] format Format string.
         *  @param [in] ... Variable arguments.
         */
        static void ColorMessageP(std::string preString, console_color_t foreground, console_color_t background, const char* format, ...);

        /**
         *  @brief Print a default message to the console.
         *  @param [in] format Format string.
         *  @param [in] ... Variable arguments.
         */
        static void Message(const char* format, ...);

        /**
         *  @brief Print a green message to the console.
         *  @param [in] format Format string.
         *  @param [in] ... Variable arguments.
         */
        static void Success(const char* format, ...);

        /**
         *  @brief Print a green message to the console using a specific prefix.
         *  @param [in] preString The string that should be printed before the actual message.
         *  @param [in] format Format string.
         *  @param [in] ... Variable arguments.
         */
        static void SuccessP(std::string preString, const char* format, ...);

        /**
         *  @brief Print a yellow message to the console.
         *  @param [in] format Format string.
         *  @param [in] ... Variable arguments.
         */
        static void Warning(const char* format, ...);

        /**
         *  @brief Print a yellow message to the console using a specific prefix.
         *  @param [in] preString The string that should be printed before the actual message.
         *  @param [in] format Format string.
         *  @param [in] ... Variable arguments.
         */
        static void WarningP(std::string preString, const char* format, ...);

        /**
         *  @brief Print a red message to the console.
         *  @param [in] format Format string.
         *  @param [in] ... Variable arguments.
         */
        static void Error(const char* format, ...);

        /**
         *  @brief Print a red message to the console using a specific prefix.
         *  @param [in] preString The string that should be printed before the actual message.
         *  @param [in] format Format string.
         *  @param [in] ... Variable arguments.
         */
        static void ErrorP(std::string preString, const char* format, ...);

        /**
         *  @brief Print a hexdump.
         *  @param [in] buffer Buffer content.
         *  @param [in] length Buffer size.
         */
        static void HexDump(const uint8_t* buffer, size_t length);

        /**
         *  @brief Print a hexdump.
         *  @param [in] buffer Buffer content.
         */
        static void HexDump(const std::vector<uint8_t>& buffer);

    private:
        static std::mutex mtx; ///< Mutex to protect console prints.

}; /* class: Console */

