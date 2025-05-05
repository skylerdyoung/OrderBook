// orderbook.cpp
#include "order_book.hpp"
#include <cstdio>
#include <algorithm>

void OrderBook::addOrder(const Order& o) {
    Order incoming = o;

    // 1) match against the opposite side
    if (incoming.isBuy) {
        auto& opp = asks_;
        while (incoming.qty && !opp.empty()) {
            auto it = opp.begin();
            double p = it->first;
            if (incoming.price < p) break;

            Level& lvl = it->second;
            auto  restIt = lvl.orders.begin();
            uint32_t xQty = std::min(incoming.qty, restIt->qty);

            trades_.push_back({ incoming.id, restIt->id, p, xQty });

            incoming.qty -= xQty;
            restIt->qty -= xQty;
            lvl.totalQty -= xQty;

            if (restIt->qty == 0) {
                idIndex_.erase(restIt->id);
                lvl.orders.erase(restIt);
                --lvl.orderCount;
            }
            if (lvl.orders.empty())
                opp.erase(it);
        }
    }
    else {
        auto& opp = bids_;
        while (incoming.qty && !opp.empty()) {
            auto it = opp.begin();
            double p = it->first;
            if (incoming.price > p) break;

            Level& lvl = it->second;
            auto  restIt = lvl.orders.begin();
            uint32_t xQty = std::min(incoming.qty, restIt->qty);

            trades_.push_back({ restIt->id, incoming.id, p, xQty });

            incoming.qty -= xQty;
            restIt->qty -= xQty;
            lvl.totalQty -= xQty;

            if (restIt->qty == 0) {
                idIndex_.erase(restIt->id);
                lvl.orders.erase(restIt);
                --lvl.orderCount;
            }
            if (lvl.orders.empty())
                opp.erase(it);
        }
    }

    // 2) rest any remainder
    if (incoming.qty) {
        if (incoming.isBuy) {
            auto& book = bids_;
            Level& lvl = book[incoming.price];
            lvl.orders.push_back(incoming);
            auto it = std::prev(lvl.orders.end());
            lvl.totalQty += incoming.qty;
            ++lvl.orderCount;
            idIndex_[incoming.id] = { true, incoming.price, it };
        }
        else {
            auto& book = asks_;
            Level& lvl = book[incoming.price];
            lvl.orders.push_back(incoming);
            auto it = std::prev(lvl.orders.end());
            lvl.totalQty += incoming.qty;
            ++lvl.orderCount;
            idIndex_[incoming.id] = { false, incoming.price, it };
        }
    }
}

void OrderBook::cancelOrder(const std::string& id) {
    auto idxIt = idIndex_.find(id);
    if (idxIt == idIndex_.end()) return;

    // extract into locals
    bool   isBuy = idxIt->second.isBuy;
    double price = idxIt->second.price;
    auto   itOrder = idxIt->second.it;

    if (isBuy) {
        auto& lvl = bids_[price];
        lvl.totalQty -= itOrder->qty;
        --lvl.orderCount;
        lvl.orders.erase(itOrder);
        if (lvl.orders.empty())
            bids_.erase(price);
    }
    else {
        auto& lvl = asks_[price];
        lvl.totalQty -= itOrder->qty;
        --lvl.orderCount;
        lvl.orders.erase(itOrder);
        if (lvl.orders.empty())
            asks_.erase(price);
    }

    idIndex_.erase(idxIt);
}

void OrderBook::replaceOrder(const std::string& id, uint32_t newQty) {
    auto idxIt = idIndex_.find(id);
    if (idxIt == idIndex_.end()) return;

    // extract into locals
    const auto& entry = idxIt->second;
    bool        isBuy = entry.isBuy;
    double      price = entry.price;
    auto        itOrder = entry.it;

    if (isBuy) {
        auto& lvl = bids_[price];
        lvl.totalQty += (newQty - itOrder->qty);
        itOrder->qty = newQty;
    }
    else {
        auto& lvl = asks_[price];
        lvl.totalQty += (newQty - itOrder->qty);
        itOrder->qty = newQty;
    }
}

void OrderBook::printTop() const {
    if (!bids_.empty()) {
        const auto& [px, lvl] = *bids_.begin();
        std::printf("BID %.2f  Qty=%u  Orders=%u\n",
            px, lvl.totalQty, lvl.orderCount);
    }
    if (!asks_.empty()) {
        const auto& [px, lvl] = *asks_.begin();
        std::printf("ASK %.2f  Qty=%u  Orders=%u\n",
            px, lvl.totalQty, lvl.orderCount);
    }
}