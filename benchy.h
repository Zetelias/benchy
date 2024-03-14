#ifndef BENCHY_H
#define BENCHY_H

#include <functional>
#include <istream>
#include <optional>
#include <ostream>
#include <iomanip>
#include <queue>
#include <string>
#include <unordered_map>
#include <chrono>
#include <stdexcept>
#include <string>
#include <chrono>
#include <iomanip>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace benchy
{

enum LogLevel {
    NONE,
    RESULT,
    ERROR,
    WARNING,
    INFO,
};

struct BenchResult {
    double averageDuration;
    double medianDuration;
    double standardDeviation;
    double totalDurations;
    double min;
    double max;
    int iterations;
};

struct EnqueuedBenchmark {
    std::string name;
    int iterations;
    std::function<void()> func;
};

class Bencher {
private:
    // A map that associates the name of a benchmark to it's results
    std::unordered_map<std::string, BenchResult> mResults;

    // An option to a pointer to an ostream to which logs will be sent to.
    // We use an option to a pointer instead of a pointer that can be nullptr
    // to distinguish between pointer becoming null at runtime (unintended error),
    // or the user specifying they don't want logging (intended setting)
    std::optional<std::ostream*> mLogStreamPtr;

    // The level of logging we should use
    LogLevel mLogLevel;

    std::queue<EnqueuedBenchmark> queue;

    // Returns true if the given LogLevel is above our current mLogLevel,
    // if we opted into logging and if the pointer in mLogStreamPtr is not null.
    // that means we have safety guarantees if should_log is true.
    bool should_log(LogLevel logLevel);

public:
    Bencher(LogLevel logLevel, std::optional<std::ostream*> logStream);
    ~Bencher() = default;

    void execute(const std::string& benchmarkName, int iterations, std::function<void()> func);
    void queueExecution(const std::string& benchmarkName, int iterations, std::function<void()> func);
    void runAll();

    void print_results();
    void log_results();

    // Logs a message to the ostream pointed to by mLogStreamPtr if we should log it
    void log(const std::string& message, LogLevel level);
};

bool _is_colorful_terminal(const std::ostream& os);

std::string _format_ns(double ns);

std::string _format_results(const std::string& results);
std::string _format_log(const std::string& message, LogLevel logLevel, bool colorful);
std::string _log_level_to_string(LogLevel logLevel);
std::string _log_level_to_color_code(LogLevel logLevel);
std::string _colorful_word(const std::string& word,  const std::string& color);

std::string _datetime_now_string();

double _empty_loop_overhead_ns(double iterations = 100);

double _vectorAverage(const std::vector<double>& doubleVec);
double _vectorMedian(const std::vector<double>& doubleVec);
double _vectorStdev(const std::vector<double>& doubleVec);
double _vectorSum(const std::vector<double>& doubleVec);
double _vectorMin(const std::vector<double>& doubleVec);
double _vectorMax(const std::vector<double>& doubleVec);


// Ripped from https://github.com/gugu256/ColorC
// but as defines, not as global variables (lol)
#define RESET_ALL "\033[0m"
#define RESET_BACK "\033[49m"
#define RESET_FORE "\033[39m"

#define STYLE_BOLD "\033[1m"
#define STYLE_DIM "\033[2m"
#define STYLE_ITALIC "\033[3m"
#define STYLE_UNDERLINE "\033[4m"
#define STYLE_BLINK "\033[5m"
#define STYLE_FRAMED "\033[51m"
#define STYLE_ENCIRCLED "\033[52m"

#define FORE_BLACK "\033[30m"
#define FORE_RED "\033[31m"
#define FORE_GREEN "\033[32m"
#define FORE_YELLOW "\033[33m"
#define FORE_BLUE "\033[34m"
#define FORE_MAGENTA "\033[35m"
#define FORE_CYAN "\033[36m"
#define FORE_WHITE "\033[37m"

#define BACK_BLACK "\033[40m"
#define BACK_RED "\033[41m"
#define BACK_GREEN "\033[42m"
#define BACK_YELLOW "\033[43m"
#define BACK_BLUE "\033[44m"
#define BACK_MAGENTA "\033[45m"
#define BACK_CYAN "\033[46m"
#define BACK_WHITE "\033[47m"

} // namespace benchy

#endif // BENCHY_H