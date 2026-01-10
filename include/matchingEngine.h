#pragma once
#include <iostream>
#include "orderBook.h"
#include "strategyEngine.h"
#include "types.h"

class MatchingEngine {
    private:
        StrategyEngine& m_strategyEngine;

        Order currentBidOrder{0, Order::Side::BUY, 0, 0, false};
        Order currentSellOrder{0, Order::Side::SELL, 0, 0, false};
    
    public:
        MatchingEngine(StrategyEngine& stratEng) : m_strategyEngine(stratEng) {}

        void resolve_orderBook_market_cross(OrderBook& book);
        void place_limit_order(TradeSignal sig);
        void process_immediate_order_book_cross(OrderBook& book);
        void process_incoming_trade(MarketEvent& trade, OrderBook& book);
};