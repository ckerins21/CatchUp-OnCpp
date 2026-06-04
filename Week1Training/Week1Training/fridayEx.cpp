#include <map>
#include <list>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

using Price = int64_t;
using Quantity = int64_t;
using OrderId = int64_t;

enum class Side { Buy, Sell };

struct Order {
    OrderId  id;
    Side     side;
    Price    price;
    Quantity quantity;
};

class OrderBook {
    std::map<Price, std::list<Order>, std::greater<Price>> bids_;
    std::map<Price, std::list<Order>, std::less<Price>>    asks_;

public:
    void addOrder(Order o);
    void cancelOrder(OrderId id);
    void printBook() const;

private:
    template <typename Book, typename Comparator>
    void matchAgainst(Order& incoming, Book& opposite, Comparator priceOK);
};

template <typename Book, typename Comparator>
void OrderBook::matchAgainst(Order& incoming, Book& opposite, Comparator priceOK) {
    auto it = opposite.begin();
    while (it != opposite.end() && incoming.quantity > 0 && priceOK(incoming.price, it->first)) {
        auto& level = it->second;
        while (!level.empty() && incoming.quantity > 0) {
            Order& resting = level.front();
            Quantity tradeQty = std::min(incoming.quantity, resting.quantity);
            std::cout << "Trade: " << tradeQty
                << " @ " << it->first
                << "  incoming #" << incoming.id
                << " vs resting #" << resting.id << "\n";
            incoming.quantity -= tradeQty;
            resting.quantity -= tradeQty;
            if (resting.quantity == 0)
                level.pop_front();
        }
        if (level.empty())
            it = opposite.erase(it);
        else
            ++it;
    }
}

void OrderBook::addOrder(Order o) {
    if (o.side == Side::Buy) {
        matchAgainst(o, asks_, [](Price bp, Price ap) { return bp >= ap; });
        if (o.quantity > 0) bids_[o.price].push_back(std::move(o));
    }
    else {
        matchAgainst(o, bids_, [](Price sp, Price bp) { return sp <= bp; });
        if (o.quantity > 0) asks_[o.price].push_back(std::move(o));
    }
}

void OrderBook::cancelOrder(OrderId id) {
    for (auto& [p, level] : bids_) level.remove_if([id](const Order& o) { return o.id == id; });
    for (auto& [p, level] : asks_) level.remove_if([id](const Order& o) { return o.id == id; });
}

void OrderBook::printBook() const {
    std::cout << "\n====== ORDER BOOK ======\n";
    std::cout << "  ASKS\n";
    for (auto it = asks_.rbegin(); it != asks_.rend(); ++it) {
        std::cout << "    " << it->first << " : ";
        for (const auto& o : it->second) std::cout << o.quantity << "(#" << o.id << ") ";
        std::cout << "\n";
    }
    std::cout << "  --------------------\n";
    std::cout << "  BIDS\n";
    for (const auto& [p, orders] : bids_) {
        std::cout << "    " << p << " : ";
        for (const auto& o : orders) std::cout << o.quantity << "(#" << o.id << ") ";
        std::cout << "\n";
    }
    std::cout << "========================\n";
}

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "usage: book <orders.csv>\n"; return 1; }
    std::ifstream in(argv[1]);
    if (!in) { std::cerr << "could not open " << argv[1] << "\n"; return 1; }
    OrderBook book;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        OrderId id; char side; Price p; Quantity q;
        if (!(iss >> id >> side >> p >> q)) continue;
        book.addOrder({ id, (side == 'B') ? Side::Buy : Side::Sell, p, q });
    }
    book.printBook();
}