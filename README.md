# Order Book

A simple, high-performance C++ order book and matching engine for buy/sell orders, with support for thread-safe order submission and built-in performance tests.

---

## Features

* **Order Book Management**: Insert, cancel, and replace orders in O(1) time using linked lists and iterators.
* **Matching Engine**: Cross incoming buy and sell orders with price-time priority, producing trade executions.
* **Thread-Safe Submission**: Decouple input from matching using a `ThreadedQueue` with producer/consumer semantics.
* **Tests & Benchmarks**: uses [doctest](https://github.com/onqtam/doctest) (via `external/doctest/doctest.h`) for lightweight C++ testing and performance benchmarks.

---

## Repository Structure

```text
order-book/
├── order_book.hpp            # OrderBook class + Order/Trade definitions
├── order_book.cpp            # Matching engine implementation
├── threaded_order_queue.hpp  # Thread-safe queue for order submission
├── test_order_book.cpp       # Doctest benchmarks and speed tests
└── README.md                 # Project overview and instructions
```

---

## Getting Started

I recommend Visual Studio 2022 for the easiest setup and debugging experience.

1. **Clone the repository**

   ```powershell
   git clone https://github.com/skylerdyoung/OrderBook.git
   cd OrderBook
   ```

2. **Open the solution in VS2022**

   * Double-click `OrderBook.sln` (or **File → Open → Project/Solution**).
   * Confirm it loads the **OrderBook** project.

3. **Configure for Release (x64)**

   * In the toolbar, set **Solution Configuration** to `Release` and **Solution Platform** to `x64`.

4. **Build the solution**

   * Press **Ctrl+Shift+B** or go to **Build → Build Solution**.

5. **Run the performance tests**

   * In **Solution Explorer**, right-click **OrderBook** → **Debug → Start new instance**.
   * The console will print lines like:

     ```
     [Benchmark] Inserted 100000 orders in XX ms
     [Benchmark] Crossed 50000 sells and 50000 buys in YY ms
     ```

---

## Usage Example

```cpp
#include "order_book.hpp"

int main() {
    OrderBook ob;
    ob.addOrder({"o1", true, 100.0, 5});   // BUY 5 @ 100.0
    ob.addOrder({"o2", false,  99.5, 3});  // SELL 3 @ 99.5
    ob.printTop();                         // shows best bid/ask
    return 0;
}
```

---

## Contributing

1. Fork the repo
2. Create a feature branch (`git checkout -b feature/foo`)
3. Commit your changes (`git commit -am 'Add foo'`)
4. Push the branch (`git push origin feature/foo`)
5. Open a Pull Request

Please follow the existing style, write clear commit messages, and add tests.

---

## License

This project is released under the MIT License. See [LICENSE](LICENSE.txt) for details.
