#include "matchingEngine.h"
#include <algorithm>

/*
void MatchingEngine::resolve_orderBook_market_cross(OrderBook& book) {
    long long bidQty, askQty;
    long long bestBidPrice = book.get_best_bid_price();
    long long bestAskPrice = book.get_best_ask_price();

    do {
        bidQty = book.get_best_bid_volume();
        askQty = book.get_best_ask_volume();

        long long bottleNeck = std::min(bidQty, askQty);
        
        book.bids[bestBidPrice] -= bottleNeck;
        book.asks[bestAskPrice] -= bottleNeck;    

        if (book.bids.count(bestBidPrice) && book.bids[bestBidPrice] <= 0) book.bids.erase(bestBidPrice);
        if (book.asks.count(bestAskPrice) && book.asks[bestAskPrice] <= 0) book.asks.erase(bestAskPrice); 
        
        bestBidPrice = book.get_best_bid_price();
        bestAskPrice = book.get_best_ask_price();

    } while (bestBidPrice > bestAskPrice);
}
*/


void MatchingEngine::place_limit_order(TradeSignal sig) {
    if (sig.qty == 0) {
        if (sig.side == TradeSignal::Side::BUY) currentBidOrder.active = false;
        if (sig.side == TradeSignal::Side::SELL) currentSellOrder.active = false;
        return;
    }

    Order order;
    order.side = (sig.side == TradeSignal::Side::BUY) ? Order::Side::BUY : Order::Side::SELL;

    order.price = sig.price;
    order.qty = sig.qty;

    if (sig.side == TradeSignal::Side::BUY) {
        currentBidOrder = order;
        currentBidOrder.active = true;
    } else if (sig.side == TradeSignal::Side::SELL) {
        currentSellOrder = order;
        currentSellOrder.active = true;
    }

    if (toggleDebugPrint) std::cout << "placed limit order\n";
}

void MatchingEngine::process_immediate_order_book_cross(OrderBook& book) {
    //Check if OUR trade creates market cross. If so, process.
    //Make sure to check quantity of order against quantity available in orderbook
    if (m_strategyEngine.midPrice == 0) return;
    
    if (currentBidOrder.active == true) {
        long long bestAskPrice = book.get_best_ask_price();
        if (currentBidOrder.price >= bestAskPrice) { //BID FILLED
            long long qty = std::min(book.get_best_ask_volume(), currentBidOrder.qty);
            currentBidOrder.qty -= qty;
            
            book.asks[bestAskPrice] -= qty;
            if (book.asks[bestAskPrice] <= 0) book.asks.erase(bestAskPrice);
            
            bool orderComplete = (currentBidOrder.qty <= 0);
            
            if (orderComplete) {
                currentBidOrder.active = false;
            }
            
            m_strategyEngine.update_position(qty, bestAskPrice, TradeSignal::Side::BUY, book);
            
            if (toggleDebugPrint) {
            std::cout << "BID FILLED @ " << bestAskPrice << "\n";
            }
        }

    } if (currentSellOrder.active == true) {
        long long bestBidPrice = book.get_best_bid_price();
        if (currentSellOrder.price <= bestBidPrice) { //ASK FILLED
            long long qty = std::min(book.get_best_bid_volume(), currentSellOrder.qty);
            currentSellOrder.qty -= qty;
            
            book.bids[bestBidPrice] -= qty;
            if (book.bids[bestBidPrice] <= 0) book.bids.erase(bestBidPrice);
            
            bool orderComplete = (currentSellOrder.qty <= 0);
            if (orderComplete) {
                currentSellOrder.active = false;
            }
                

            m_strategyEngine.update_position(qty, bestBidPrice, TradeSignal::Side::SELL, book);
            
            if (toggleDebugPrint) {
                std::cout << "ASK FILLED @ " << bestBidPrice << "\n"; 
            }
        }
    }

    //if (currentBidOrder.active || currentSellOrder.active) std::cout << "------------------------\n";
}

void MatchingEngine::process_incoming_trade(MarketEvent& trade, OrderBook& book) {
    //Check if it's a bid or ask
    if (m_strategyEngine.midPrice == 0) return;

    if (trade.side == "buy") {
        //Check if it crosses our existing sell order
        if (currentSellOrder.active && trade.price >= currentSellOrder.price) {
            long long qty = std::min(currentSellOrder.qty, trade.qty);

            currentSellOrder.qty -= qty;
            bool orderComplete = (currentSellOrder.qty <= 0);

            if (orderComplete) currentSellOrder.active = false;

            m_strategyEngine.update_position(qty, currentSellOrder.price, TradeSignal::Side::SELL, book);
            if (toggleDebugPrint) {
                std::cout << std::fixed << std::setprecision(6); 
                std::cout << "Passive SELL Fill: " << static_cast<double>(qty) / g_qtyScale << " @ " << static_cast<double>(currentSellOrder.price) / g_priceScale << std::endl;
            }
        }
    }
    else if (trade.side == "sell") {
        if (currentBidOrder.active && trade.price <= currentBidOrder.price) {
            long long qty = std::min(currentBidOrder.qty, trade.qty);

            currentBidOrder.qty -= qty;
            bool orderComplete = (currentBidOrder.qty <= 0);

            if (orderComplete) currentBidOrder.active = false;

            m_strategyEngine.update_position(qty, currentBidOrder.price, TradeSignal::Side::BUY, book);
            if (toggleDebugPrint) {
                std::cout << std::fixed << std::setprecision(6); 
                std::cout << "Passive BUY Fill: " << static_cast<double>(qty) / g_qtyScale << " @ " << static_cast<double>(currentBidOrder.price) / g_priceScale << std::endl;
            }
        }
        //std::cout << std::defaultfloat;
    }     
}