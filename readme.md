# Benchy: A lightweight, header-implementation only C++ benchmarking library
## Example
```cpp
#include "benchy.h"
#include <math.h>

int fib(int n) {
  if (n <= 1) return n;
  return fib(n - 1) + fib(n - 2);
}

int main() {
    // Create a bencher that logs to the console.
    benchy::Bencher bencher(benchy::LogLevel::INFO,
                             &std::cout);

    // Run some function and measure its performance.
    bencher.execute("Recursive fibonnaci", 100, []{
        int fib20 = fib(20); 
    });

    // Run code that captures the block it's in 
    // and measure its performance.
    int x = 13;
    int y = 42;
    bencher.execute("Inline fibonnaci", 100, [&x, &y]{
        int z = pow(x, y);
    });

    // Log the results. Equivalent to 
    // Bencher::print_results() since we log to console
    // It will display in an ordered, tab format
    bencher.log_results(); /
}
```

## Documentation
Documentation is coming soon, but i consider
the api clear enough for it not to be a priority.
I would simply say that "print results" print results.

## License
[GNU GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html#license-text)

## TODO's and issues
- Fix negative times for 
  **extremely** quick (~sub 5ns) code on -O3
- Setup tests with github actions
- Make the output tab clearer with lines n' stuff
- Support multithreaded benchmark executions