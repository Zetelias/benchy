#include "benchy.h"
#include <bits/chrono.h>
#include <chrono>
#include <cstdio>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

benchy::Bencher::Bencher(LogLevel logLevel, std::optional<std::ostream*> logStreamPtr) {
    this->mLogLevel = logLevel;

    if (logLevel == LogLevel::NONE) {
        this->mLogStreamPtr = std::nullopt;
    } else {
        this->mLogStreamPtr = logStreamPtr;
    }
}

void benchy::Bencher::execute(const std::string& benchmarkName, int iterations, std::function<void()> func) {
    log("Executing benchmark: " + benchmarkName, LogLevel::INFO);

    double overhead = _empty_loop_overhead_ns(1000);
    std::vector<double> results;
    for (int i = 0; i < iterations; i++) {
        auto funcStart = std::chrono::high_resolution_clock::now();
        func();    
        auto funcEnd = std::chrono::high_resolution_clock::now();
        results.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(funcEnd - funcStart).count() - overhead);
    }

    mResults[benchmarkName] = BenchResult {
        _vectorAverage(results),
        _vectorMedian(results),
        _vectorStdev(results),
        _vectorSum(results),
        _vectorMin(results),
        _vectorMax(results),
        iterations
    };

    log("Done running benchmark: " + benchmarkName + " average is " + _format_ns(mResults[benchmarkName].averageDuration), LogLevel::INFO);
}

void benchy::Bencher::queueExecution(const std::string& benchmarkName, int iterations, std::function<void()> func) {
    queue.push(
        EnqueuedBenchmark {
            benchmarkName,
            iterations, 
            func,
        }
    );
}

void benchy::Bencher::runAll() {
    while (!queue.empty()) {
        EnqueuedBenchmark front = queue.front();
        execute(front.name, front.iterations, front.func);
        queue.pop();
    }
}

void benchy::Bencher::print_results() {
    std::cout << std::left << std::setw(20) << "Benchmark"
              << std::setw(15) << "Avg Duration"
              << std::setw(15) << "Median Duration"
              << std::setw(15) << "Std Deviation"
              << std::setw(15) << "Total Duration"
              << std::setw(15) << "Min"
              << std::setw(15) << "Max"
              << std::setw(10) << "Iterations" << std::endl;
        
    for (const auto& [name, result] : mResults) {
        std::cout << std::left << std::setw(20) << name
                  << std::setw(15) << _format_ns(result.averageDuration)
                  << std::setw(15) << _format_ns(result.medianDuration)
                  << std::setw(15) << _format_ns(result.standardDeviation)
                  << std::setw(15) << _format_ns(result.totalDurations)
                  << std::setw(15) << _format_ns(result.min)
                  << std::setw(15) << _format_ns(result.max)
                  << std::setw(10) << result.iterations << std::endl;
    }
}

void benchy::Bencher::log_results() {
    if (!should_log(LogLevel::RESULT))
        return;

    std::stringstream ss;

    ss << std::left << std::setw(20) << "Benchmark"
              << std::setw(15) << "Avg Duration"
              << std::setw(15) << "Median Duration"
              << std::setw(15) << "Std Deviation"
              << std::setw(15) << "Total Duration"
              << std::setw(15) << "Min"
              << std::setw(15) << "Max"
              << std::setw(10) << "Iterations" << std::endl;
        
    for (const auto& [name, result] : mResults) {
        ss << std::left << std::setw(20) << name
                  << std::setw(15) << _format_ns(result.averageDuration)
                  << std::setw(15) << _format_ns(result.medianDuration)
                  << std::setw(15) << _format_ns(result.standardDeviation)
                  << std::setw(15) << _format_ns(result.totalDurations)
                  << std::setw(15) << _format_ns(result.min)
                  << std::setw(15) << _format_ns(result.max)
                  << std::setw(10) << result.iterations << std::endl;
    }
    *mLogStreamPtr.value() << _format_results(ss.str()) << std::endl;
}

bool benchy::Bencher::should_log(LogLevel logLevel) {
    return (mLogStreamPtr == std::nullopt ||    // 1. Did we opt out of logging?
            mLogStreamPtr.value() == nullptr || // 2. Is the pointer to the log stream null?
            mLogLevel >= logLevel);             // 3. Is mLogLevel >= to what we should log?
} 

void benchy::Bencher::log(const std::string& message, LogLevel logLevel) {
    if (!should_log(logLevel))
        return;

    *mLogStreamPtr.value() << _format_log(message, logLevel, _is_colorful_terminal(*mLogStreamPtr.value())) << std::endl;
}

std::string benchy::_format_log(const std::string& message, LogLevel logLevel, bool colorful = false) {
    std::stringstream ss; 
    if (colorful) {
        ss << _colorful_word("[" + _log_level_to_string(logLevel) + " " +
                              _datetime_now_string() + "]", _log_level_to_color_code(logLevel)) + " ";
    } else {
        ss << "[" + _log_level_to_string(logLevel) + " " + _datetime_now_string() + "] ";
    }
    ss << message;
    return ss.str();
}

std::string benchy::_format_results(const std::string &results) {
    return _colorful_word("[" + _log_level_to_string(LogLevel::RESULT) + " " + 
    _datetime_now_string() + "]", _log_level_to_color_code(LogLevel::RESULT))
    + "\n" + results;
}

bool benchy::_is_colorful_terminal(const std::ostream &os) {
    if (&os == &std::cout || &os == &std::cerr || &os == &std::clog) {
        #ifdef _WIN32
            return true;
        #else
            return isatty(fileno(stdout)); 
        #endif
    }
    return false; 
}

std::string benchy::_format_ns(double ns) {
    constexpr double ns_per_ms = 1e6;
    constexpr double ns_per_sec = 1e9;
    constexpr double ns_per_min = 6e10;
    constexpr double ns_per_hour = 3.6e12;
    constexpr double ns_per_day = 8.64e13;

    std::ostringstream oss;

    if (std::abs(ns) < ns_per_ms) {
        oss << std::setprecision(2) << std::fixed << ns << "ns";
    } else if (std::abs(ns) < ns_per_sec) {
        oss << std::setprecision(2) << std::fixed << ns / ns_per_ms << "ms";
    } else if (std::abs(ns) < ns_per_min) {
        oss << std::setprecision(2) << std::fixed << ns / ns_per_sec << "s";
    } else if (std::abs(ns) < ns_per_hour) {
        oss << std::setprecision(2) << std::fixed << ns / ns_per_min << "min";
    } else if (std::abs(ns) < ns_per_day) {
        oss << std::setprecision(2) << std::fixed << ns / ns_per_hour << "h";
    } else {
        oss << std::setprecision(2) << std::fixed << ns / ns_per_day << "days";
    }

    return oss.str();
}

std::string benchy::_log_level_to_string(LogLevel logLevel) {
    switch (logLevel) {
        case LogLevel::NONE:
            return "NONE";
        case LogLevel::RESULT:
            return "RESULT";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::INFO:
            return "INFO";
        default:
            throw std::runtime_error("Unknown LogLevel");
    }
}


std::string benchy::_log_level_to_color_code(LogLevel logLevel) {
    switch (logLevel) {
        case LogLevel::NONE:
            return FORE_BLACK;
        case LogLevel::RESULT:
            return FORE_GREEN;
        case LogLevel::ERROR:
            return FORE_RED;
        case LogLevel::WARNING:
            return FORE_YELLOW;
        case LogLevel::INFO:
            return FORE_WHITE;
        default:
            throw std::runtime_error("Unknown LogLevel");
    }
}

double benchy::_empty_loop_overhead_ns(double iterations) {
    auto now = std::chrono::high_resolution_clock::now();
    std::vector<std::chrono::nanoseconds> results;
    for (int i = 0; i < iterations; i++) {
        auto now = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        results.push_back(end - now);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - now).count() / iterations;
}


std::string benchy::_colorful_word(const std::string &word, const std::string& color) {
    return color + word + RESET_FORE;
}

std::string benchy::_datetime_now_string() {
    // Get current time
    auto now = std::chrono::system_clock::now();

    // Convert to time_t (seconds since epoch)
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // Convert to local time
    std::tm local_tm = *std::localtime(&now_time_t);

    // Format as string
    std::stringstream ss;
    ss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

double benchy::_vectorAverage(const std::vector<double>& doubleVec) {
    return _vectorSum(doubleVec) / doubleVec.size();
}

double benchy::_vectorMedian(const std::vector<double>& doubleVec) {
    // Make a copy of the vector and sort it
    std::vector<double> sortedVec = doubleVec;
    std::sort(sortedVec.begin(), sortedVec.end());

    // Calculate the median
    if (sortedVec.size() % 2 == 0) {
        // If the size of the vector is even, average the middle two elements
        int midIndex1 = sortedVec.size() / 2 - 1;
        int midIndex2 = sortedVec.size() / 2;
        return (sortedVec[midIndex1] + sortedVec[midIndex2]) / 2.0;
    } else {
        // If the size of the vector is odd, return the middle element
        return sortedVec[sortedVec.size() / 2];
    }
}

double benchy::_vectorStdev(const std::vector<double>& doubleVec) {
    // Calculate the mean
    double mean = _vectorSum(doubleVec) / doubleVec.size();

    // Calculate the sum of squares of differences from the mean
    double sumOfSquares = 0.0;
    for (double value : doubleVec) {
        double diff = value - mean;
        sumOfSquares += diff * diff;
    }

    // Calculate the variance
    double variance = sumOfSquares / doubleVec.size();

    // Calculate the standard deviation as the square root of the variance
    return std::sqrt(variance);
}

double benchy::_vectorSum(const std::vector<double>& doubleVec) {
    double sum = 0;
    for (double i : doubleVec) {
        sum += i;
    }
    return sum;
}

double benchy::_vectorMin(const std::vector<double>& doubleVec) {
    if (doubleVec.empty()) {
        // Handle empty vector case
        throw std::invalid_argument("Cannot find minimum of an empty vector");
    }

    double minVal = doubleVec[0]; // Assume the first element as the minimum
    for (double val : doubleVec) {
        if (val < minVal) {
            minVal = val; // Update minVal if a smaller value is found
        }
    }
    return minVal;
}

double benchy::_vectorMax(const std::vector<double>& doubleVec) {
    if (doubleVec.empty()) {
        // Handle empty vector case
        throw std::invalid_argument("Cannot find maximum of an empty vector");
    }

    double maxVal = doubleVec[0]; // Assume the first element as the maximum
    for (double val : doubleVec) {
        if (val > maxVal) {
            maxVal = val; // Update maxVal if a larger value is found
        }
    }
    return maxVal;
}