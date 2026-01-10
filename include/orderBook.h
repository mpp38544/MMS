#pragma once
#include <map>
#include "dataReplayEngine.h"
#include "types.h"

class OrderBook {
    private:
    // Bid Map: Price (key) -> Volume (value). Sorted DESCENDING (highest price first).
    
    // Ask Map: Price (key) -> Volume (value). Sorted ASCENDING (lowest price first).
    long long lastUpdateId = 0;
    
    public:
        using BidMap = std::map<long long, long long, std::greater<long long> >; 
        using AskMap = std::map<long long, long long, std::less<long long> >;
        AskMap asks;
        BidMap bids;

        void process_l2_update(long long price, long long qty, const std::string& side, long long updateId);
        long long get_best_bid_price() const;
        long long get_best_ask_price() const;
        long long get_best_bid_volume() const;
        long long get_best_ask_volume() const;
        void print_top_of_book() const;
        void print_book() const;
        void apply_update(MarketEvent& e);
        void apply_delta(MarketEvent& e);

};