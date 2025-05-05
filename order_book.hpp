// orderbook.hpp
#pragma once

#include <list>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

// --- Order & Trade definitions ---
struct Order {
    std::string  id;
    bool         isBuy;
    double       price;
    uint32_t     qty;
};

struct Trade {
    std::string  bidId;
    std::string  askId;
    double       price;
    uint32_t     qty;
};

class OrderBook {
public:
    void addOrder(const Order& o);
    void cancelOrder(const std::string& id);
    void replaceOrder(const std::string& id, uint32_t newQty);
    const std::vector<Trade>& getTrades() const { return trades_; }
    void printTop() const;

private:
    struct Level {
        std::list<Order> orders;
        uint32_t         totalQty = 0;
        uint32_t         orderCount = 0;
    };

    struct IndexEntry {
        bool                        isBuy;
        double                      price;
        std::list<Order>::iterator  it;
    };

    // bids (desc) and asks (asc)
    std::map<double, Level, std::greater<>> bids_;
    std::map<double, Level>                asks_;
    std::unordered_map<std::string, IndexEntry> idIndex_;
    std::vector<Trade>                     trades_;
};