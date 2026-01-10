#include <map>
#include <string>
#include <functional> // Required for std::greater and std::less
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "strategyEngine.h"
#include "orderBook.h"
#include "constants.h"
#include "mth.h"
#include <cmath>

std::vector<TradeSignal> StrategyEngine::generate_signal(const OrderBook &book) {

    /*
    Avellaneda Stoikov Strategy
    s = current market mid-price
    q = inventory quantity of base asset (positive/negative for long/short positions)
    σ = market volatility
    T = normalized closing time (1)
    t = current time as a fraction of T
    δa, δb = symmetrical bid/ask spread
    γ = inventory risk aversion parameter
    κ = order book liquidity parameter
    */

    std::vector<TradeSignal> signalOutputs;
    
    long long bestBid = book.get_best_bid_price();
    long long bestAsk = book.get_best_ask_price();
    double mP = (static_cast<double>(bestAsk) + static_cast<double>(bestBid)) / (2.0 * g_priceScale);

    if (!kf_initialised) {
        x0 << mP, 0.0;
        kf.init(0.0, x0);
        kf_initialised = true;
    }

    // Update KF with new observation
    Eigen::VectorXd y(1);
    y << mP;
    kf.update(y);

    double fairPrice = kf.state()[0];       // filtered fair price
    double drift = kf.state()[1];   // estimated short-term drift

    double q = static_cast<double>(netPosition) / g_qtyScale;
    // 1. Get Volatility in Dollars (e.g., 1200.0)
    // Your function currently returns midPrice * dailyStdDev, which is correct for this.
    double sigma_dollars = updateAndGetVolatility(mP); 
    double sigma_sq_dollars = sigma_dollars * sigma_dollars;

    double time_left = 1.0 - getTimeFrac();

    // 2. Reservation Price (Dollar Space)
    // Because sigma_sq is in dollars, the result of this is a dollar adjustment.
    // No /mP needed here!
    double inventorySkew = (q * gamma * sigma_sq_dollars * time_left);

    // SAFETY: Cap the skew so it doesn't move your price by more than, say, 1% of the asset value.
    double maxSkew = mP * 0.01;
    inventorySkew = std::clamp(inventorySkew, -maxSkew, maxSkew);

    double reservationPrice = fairPrice - inventorySkew;

    // 3. Spread (Dollar Space)
    // We use sigma_sq_dollars here too.
    double spread = (gamma * sigma_sq_dollars * time_left) + (2.0 / gamma) * std::log(1.0 + (gamma / kappa));

    // 4. Hard Floor (2 basis points)
    double minSpread = mP * 0.0002; 
    spread = std::max(spread, minSpread);

    // 5. Target Prices
    long long targetBid = static_cast<long long>((reservationPrice - spread / 2.0) * g_priceScale);
    long long targetAsk = static_cast<long long>((reservationPrice + spread / 2.0) * g_priceScale);

    // Ensure we stay as a Maker (do not cross the spread)
    targetBid = std::min(targetBid, bestBid);
    targetAsk = std::max(targetAsk, bestAsk);

    bool canBuy = (q < g_maxPos);
    bool canSell = (q > -g_maxPos);

    // Generate sell signal
    if (targetAsk == restingSellPrice && targetBid == restingBuyPrice) {
        return signalOutputs; // No meaningful change
    }

    TradeSignal sellSignal;
    if (canSell) {
        if (targetAsk != restingSellPrice) {
            sellSignal.side = TradeSignal::SELL;
            sellSignal.price = targetAsk;
            sellSignal.qty = orderSize;
            
            restingSellPrice = targetAsk;
            signalOutputs.push_back(sellSignal);
        }
    } else {
        restingSellPrice = 0;
        sellSignal.side = TradeSignal::SELL;
        sellSignal.qty = 0;
        signalOutputs.push_back(sellSignal);
    }
    
    // Generate buy signal
    TradeSignal buySignal;
    if (canBuy) {
        if (targetBid != restingBuyPrice) {
            buySignal.side = TradeSignal::BUY;
            buySignal.price = targetBid;
            buySignal.qty = orderSize;
            
            restingBuyPrice = targetBid;
            signalOutputs.push_back(buySignal);
        }
    } else {
        restingBuyPrice = 0;
        buySignal.side = TradeSignal::BUY;
        buySignal.qty = 0;
        signalOutputs.push_back(buySignal);
    }
    
    return signalOutputs;
}


void StrategyEngine::update_position(long long filledQty, long long price, TradeSignal::Side side, OrderBook& book) {
    double price_dollars = static_cast<double>(price) / g_priceScale;
    double qty_btc = static_cast<double>(filledQty) / g_qtyScale;
    double trade_value = price_dollars * qty_btc;
    
    if (side == TradeSignal::Side::BUY) {
        netPosition += filledQty;
        cashBalance -= (trade_value);  // Pay price + fee
        
    } else {
        netPosition -= filledQty;
        cashBalance += (trade_value);  // Receive price - fee
    }
}

void StrategyEngine::update_PnL(OrderBook& book) {
    double bestAskPrice = static_cast<double>(book.get_best_ask_price()) / g_priceScale;
    double bestBidPrice = static_cast<double>(book.get_best_bid_price()) / g_priceScale;

    midPrice = (bestAskPrice + bestBidPrice) / 2.0;
    
    downScaledNetPos = static_cast<double>(netPosition) / g_qtyScale;

    double unrealised = downScaledNetPos * midPrice;
    double downScaledPnL = cashBalance + unrealised;
    pnl = downScaledPnL;
}

void StrategyEngine::display_PnL(OrderBook& book) {
    if (book.bids.empty() || book.asks.empty()) {
        std::cout << "Empty bids/asks." << "\n";
        return;
    }

    std::cout << "--------------------------------------------------------------------------\n";
    std::cout << "cash balance: " << cashBalance << ", netPos: " << downScaledNetPos << "\n";

    std::cout << std::fixed << std::setprecision(6); 
    std::cout << " Pos: " << downScaledNetPos 
              << " Cash: " << cashBalance 
              << " PnL: " << pnl << "\n";
    std::cout << "--------------------------------------------------------------------------\n";
    
    //std::cout << std::fixed << std::setprecision(6); 
    //std::cout << "PnL: " << downScaledPnL << "";
}

double StrategyEngine::updateAndGetVolatility(double midPrice) {
    if (lastMidPrice <= 0.0) {
        lastMidPrice = midPrice;
        return std::sqrt(currentVar);
    }

    // 1. Calculate Log Return
    double logReturn = std::log(midPrice / lastMidPrice);
    
    // 2. Update Variance (Recursive EWMA)
    // We use the square of the return to capture magnitude of move
    currentVar = (lambda * currentVar) + ((1.0 - lambda) * (logReturn * logReturn));
    
    lastMidPrice = midPrice;

    // 3. Return Standard Deviation (Sigma)
    // Note: AS model usually uses sigma^2, so you could just return variance
    double dailyStdDev = std::sqrt(currentVar) * std::sqrt(70000.0);
    return midPrice * dailyStdDev;
}

double StrategyEngine::getTimeFrac() {
    if (START_TIME_STAMP == 0) START_TIME_STAMP = timeStamp;
    double t_frac = 0.0;
    if (END_TIME_STAMP > START_TIME_STAMP) {
        t_frac = static_cast<double>(timeStamp - START_TIME_STAMP) / 
                 static_cast<double>(END_TIME_STAMP - START_TIME_STAMP);
    }
    return t_frac;
}