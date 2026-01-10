#include <map>
#include <iostream>
#include <string>
#include "orderBook.h"
#include "constants.h"


void OrderBook::apply_update(MarketEvent& e) {
    if (e.type == MarketEvent::Type::SNAPSHOT) {
        bids.clear();
        asks.clear();
    }

    for (const auto& bid : e.bookBids) {
        bids[bid.first] = bid.second;
        if (bids[bid.first] <= 0) bids.erase(bid.first);
    }

    for (const auto& ask : e.bookAsks) {
        asks[ask.first] = ask.second;
        if (asks[ask.first] <= 0) asks.erase(ask.first);
    }

}

long long OrderBook::get_best_bid_price() const {
    if (bids.empty()) return LLONG_MIN;
    return bids.begin()->first;
}

long long OrderBook::get_best_ask_price() const {
    if (asks.empty()) return LLONG_MAX;
    return asks.begin()->first;
}

long long OrderBook::get_best_bid_volume() const {
    if (bids.empty()) return 0;
    return bids.begin()->second;
}
long long OrderBook::get_best_ask_volume() const {
    if (asks.empty()) return 0;
    return asks.begin()->second;
}

void OrderBook::print_top_of_book() const {
    std::cout << "--------------------------------------------------------------------------" << "\n";
    std::cout << "BIDS\t\t\t\tASKS" << "\n";
    std::cout << "--------------------------------------------------------------------------" << "\n";
    std::cout << "Price:\tQuantity:\t\tPrice:\tQuantity:" << "\n";
    std::cout << "--------------------------------------------------------------------------" << "\n";
    std::cout << std::fixed << std::setprecision(8);
    
    double topBidPrice = static_cast<double>(bids.begin()->first) / g_priceScale;
    double topBidQty = static_cast<double>(bids.begin()->second) / g_qtyScale;

    double topAskPrice = static_cast<double>(asks.begin()->first) / g_priceScale;
    double topAskQty = static_cast<double>(asks.begin()->second) / g_qtyScale;

    std::cout << topBidPrice << "\t" << topBidQty << "\t\t\t" << topAskPrice << "\t" << topAskQty << "\n";
}

void OrderBook::print_book() const {
    size_t mx = std::max(bids.size(), asks.size());
    auto itBid = bids.begin();
    auto itAsk = asks.begin();

    std::cout << "--------------------------------------------------------------------------\n";
    std::cout << "BIDS" << "\t\t\t\t\t\t" << "ASKS" << "\n";
    std::cout << "--------------------------------------------------------------------------\n";
    std::cout << "Price:\t\tQuantity:\t\t\tPrice:\t\tQuantity:" << "\n";
    std::cout << "--------------------------------------------------------------------------\n";
    std::cout << std::fixed << std::setprecision(8);

    for (int i = 0; i < mx; i++) {
        if (itBid != bids.end()) {
            std::cout << static_cast<double>((itBid)->first) / g_priceScale << " " << static_cast<double>((itBid)->second) / g_qtyScale << " ";
            itBid++;
        } else std::cout << "\t\t\t";
        std::cout << "\t\t\t";

        if (itAsk != asks.end()) {
        std::cout << static_cast<double>((itAsk)->first) / g_priceScale << " " << static_cast<double>((itAsk)->second) / g_qtyScale;
            itAsk++;
        } 
        std::cout << "\n";
    }
}
