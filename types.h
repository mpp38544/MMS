#pragma once
#include <string>
#include <vector>

struct MarketEvent {
    enum Type { SNAPSHOT, DELTA, TRADE };

    Type type;
    long long timestamp, price, qty;
    std::string side;

    std::vector<std::pair<long long, long long>> bookBids, bookAsks;

};

struct TradeSignal {
    enum Side {
        BUY,
        SELL,
        NONE
    };

    Side side = NONE;
    long long price = 0.0;
    long long qty = 0.0;

    long long clientOrderId = 0;
    
};

struct Order {
    long long clientOrderId;
    enum Side {
        BUY,
        SELL
    };

    Side side;
    long long price = 0;
    long long qty = 0;
    bool active = false;
};