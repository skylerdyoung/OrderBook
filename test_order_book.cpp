#define DOCTEST_CONFIG_IMPLEMENT
#include "external/doctest/doctest.h"

#include "order_book.hpp"
#include "threaded_order_queue.hpp"

#include <chrono>
#include <thread>
#include <iostream>
#include <string>

// Benchmarks inserting N unmatched orders
static long benchInsert(int N) {
    OrderBook ob;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        ob.addOrder({ std::to_string(i), /*isBuy=*/true, 100.0, 1 });
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// Benchmarks crossing N sells then N buys at the same prices
static long benchCross(int N) {
    OrderBook ob;
    for (int i = 0; i < N; ++i)
        ob.addOrder({ "S" + std::to_string(i), /*isBuy=*/false, 100.0 + (i % 10), 1 });
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        ob.addOrder({ "B" + std::to_string(i), /*isBuy=*/true, 100.0 + (i % 10), 1 });
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// Benchmarks cancelling N resting orders
static long benchCancel(int N) {
    OrderBook ob;
    for (int i = 0; i < N; ++i)
        ob.addOrder({ std::to_string(i), /*isBuy=*/true, 100.0, 1 });
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        ob.cancelOrder(std::to_string(i));
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// Benchmarks replacing quantity on N resting orders
static long benchReplace(int N) {
    OrderBook ob;
    for (int i = 0; i < N; ++i)
        ob.addOrder({ std::to_string(i), /*isBuy=*/true, 100.0, 1 });
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        ob.replaceOrder(std::to_string(i), 2);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// Benchmarks full pipeline via ThreadedQueue
static long benchPipeline(int N, bool interleaved) {
    OrderBook ob;
    ThreadedQueue<Order> queue;
    std::thread worker([&]() {
        while (auto mo = queue.pop()) {
            ob.addOrder(*mo);
        }
        });

    auto start = std::chrono::high_resolution_clock::now();
    if (!interleaved) {
        for (int i = 0; i < N; ++i)
            queue.push({ "S" + std::to_string(i), /*isBuy=*/false, 100.0, 1 });
        for (int i = 0; i < N; ++i)
            queue.push({ "B" + std::to_string(i), /*isBuy=*/true, 100.0, 1 });
    }
    else {
        for (int i = 0; i < N; ++i) {
            queue.push({ "S" + std::to_string(i), /*isBuy=*/false, 100.0, 1 });
            queue.push({ "B" + std::to_string(i), /*isBuy=*/true, 100.0, 1 });
        }
    }
    queue.shutdown();
    worker.join();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

TEST_CASE("Insert speed: 100k orders") {
    std::cout << "\n";

    const int N = 100000;
    long ms = benchInsert(N);
    
    CHECK(ms >= 0);
    std::cout << "[Benchmark] Inserted " << N << " orders in " << ms << " ms\n";
    if (ms > 200) {
        std::cout << "[Warning] Insertion slower than expected (>200ms)\n";
    }
    std::cout << "\n";
}

TEST_CASE("Cross speed: 50k sells then buys") {
    const int N = 50000;
    long ms = benchCross(N);

    CHECK(ms >= 0);
    std::cout << "[Benchmark] Crossed " << N << " sells and " << N << " buys in " << ms << " ms\n";
    if (ms > 300) {
        std::cout << "[Warning] Crossing slower than expected (>300ms)\n";
    }
    std::cout << "\n";
}

TEST_CASE("Cancel speed: 100k orders") {
    const int N = 100000;
    long ms = benchCancel(N);

    CHECK(ms >= 0);
    std::cout << "[Benchmark] Cancelled " << N << " orders in " << ms << " ms\n";
    if (ms > 200) {
        std::cout << "[Warning] Cancellation slower than expected (>200ms)\n";
    }
    std::cout << "\n";
}

TEST_CASE("Replace speed: 100k orders") {
    const int N = 100000;
    long ms = benchReplace(N);

    CHECK(ms >= 0);
    std::cout << "[Benchmark] Replaced qty on " << N << " orders in " << ms << " ms\n";
    if (ms > 200) {
        std::cout << "[Warning] Replace slower than expected (>200ms)\n";
    }
    std::cout << "\n";
}

TEST_CASE("Pipeline speed: 5k batched orders") {
    const int N = 5000;
    long ms = benchPipeline(N, false);

    CHECK(ms >= 0);
    std::cout << "[Benchmark] Pipeline batched " << 2 * N << " orders in " << ms << " ms\n";
    if (ms > 100) {
        std::cout << "[Warning] Batched pipeline slower than expected (>100ms)\n";
    }
    std::cout << "\n";
}

TEST_CASE("Pipeline speed: 5k interleaved orders") {
    const int N = 5000;
    long ms = benchPipeline(N, true);

    CHECK(ms >= 0);
    std::cout << "[Benchmark] Pipeline interleaved " << 2 * N << " orders in " << ms << " ms\n";
    if (ms > 150) {
        std::cout << "[Warning] Interleaved pipeline slower than expected (>150ms)\n";
    }
    std::cout << "\n";
}

int main(int argc, char** argv) {
    doctest::Context context;
    context.setOption("abort-after", 5);
    context.setOption("success", true);

    int res = context.run();
    if (context.shouldExit()) return res;
}