#include "benchy.h"
#include <math.h>

int fib(int n) {
  if (n <= 1) return n;
  return fib(n - 1) + fib(n - 2);
}

int main() {
    // Create a bencher that logs to the console.
    benchy::Bencher bencher(benchy::LogLevel::INFO, &std::cout);

    // Run some code and measure its performance.
    bencher.execute("Recursive fibonnaci", 100, []{
        int fib20 = fib(20);
    });

    // Run code inline and measure its performance.
    int x = 13;
    int y = 42;
    bencher.execute("Inline fibonnaci", 100, [&x, &y]{
        int z = pow(x, y);
    });

    // Log the results. Equivalent to Bencher::print_results() since we log to console
    bencher.log_results();
}