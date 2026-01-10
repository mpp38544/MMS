#include "orderBook.h"
#include "dataReplayEngine.h"
#include <string>
#include "matchingEngine.h"
#include "strategyEngine.h"
#include <future>
#include <vector>
#include <thread>

std::vector<std::string> bookFiles = {"data/2025-12-01_BTCUSDT_ob200.data", "data/2025-12-02_BTCUSDT_ob200.data", "data/2025-12-03_BTCUSDT_ob200.data", "data/2025-12-04_BTCUSDT_ob200.data", "data/2025-12-05_BTCUSDT_ob200.data", "data/2025-12-06_BTCUSDT_ob200.data"};
std::vector<std::string> tradesFiles = {"data/BTCUSDT_2025-12-01.csv", "data/BTCUSDT_2025-12-02.csv", "data/BTCUSDT_2025-12-03.csv", "data/BTCUSDT_2025-12-04.csv", "data/BTCUSDT_2025-12-05.csv", "data/BTCUSDT_2025-12-06.csv"};

void run_sim(StrategyEngine& strategyEngine, MatchingEngine& matchingEngine, OrderBook& book, const std::string& outputFile, const std::string& bookFile, const std::string& tradesFile);

double run_singleStrategy(double orderSize, double g, double k, std::string outputFile, std::string bookFile, std::string tradesFile) {
    OrderBook book;
    StrategyEngine strategyEngine;
    strategyEngine.gamma = g;
    strategyEngine.kappa = k;
    strategyEngine.orderSize = static_cast<long long>(orderSize * g_qtyScale);

    MatchingEngine matchingEngine(strategyEngine);
    run_sim(strategyEngine, matchingEngine, book, outputFile, bookFile, tradesFile);

    return strategyEngine.pnl;
} 

void run_sim(StrategyEngine& strategyEngine, MatchingEngine& matchingEngine, OrderBook& book, const std::string& outputFile, const std::string& bookFile, const std::string& tradesFile) {
    DataReplayEngine dataReplayEngine(bookFile, tradesFile);
    std::ofstream pnlLog(outputFile);

    long long totalBytes = dataReplayEngine.totalBookFileBytes + dataReplayEngine.totalTradesFileBytes;
    int cnt = 0;

    while (dataReplayEngine.has_more_data(dataReplayEngine.bookFile) || dataReplayEngine.has_more_data(dataReplayEngine.tradesFile)) {

        //Compare timestamps of book and trade data
        
        MarketEvent currentEvent;

        if (dataReplayEngine.nextTrade.timestamp <= dataReplayEngine.nextBook.timestamp) {
            //Process next trade and load next data
            currentEvent = dataReplayEngine.nextTrade;
            matchingEngine.process_incoming_trade(currentEvent, book);

            dataReplayEngine.get_next_trade();
            strategyEngine.timeStamp = dataReplayEngine.nextTrade.timestamp;
            
        } else if (dataReplayEngine.nextBook.timestamp < dataReplayEngine.nextTrade.timestamp) {
            currentEvent = dataReplayEngine.nextBook;

            book.apply_update(currentEvent);
            dataReplayEngine.get_next_book();
            strategyEngine.timeStamp = dataReplayEngine.nextBook.timestamp;
        }
        
        std::vector<TradeSignal> sigs = strategyEngine.generate_signal(book);
        
        if (!book.bids.empty() && !book.asks.empty()) {
            for (auto sig : sigs) {
                matchingEngine.place_limit_order(sig);
                //matchingEngine.process_immediate_order_book_cross(book);
            }
        }

        if (cnt % 10000 == 0) {
            pnlLog.flush();
            // // 1. Get current positions
            // long long currentBookPos = dataReplayEngine.bookFile.tellg();
            // long long currentTradePos = dataReplayEngine.tradesFile.tellg();
            
            // // Handle tellg() returning -1 at EOF
            // if (currentBookPos < 0) currentBookPos = dataReplayEngine.totalBookFileBytes;
            // if (currentTradePos < 0) currentTradePos = dataReplayEngine.totalTradesFileBytes;

            // // 2. Calculate percentage
            // double percentage = (static_cast<double>(currentBookPos + currentTradePos) / totalBytes) * 100.0;

            // // 3. Build a visual bar
            // int barWidth = 20;
            // int pos = barWidth * (percentage / 100.0);

            // 4. Print with \r (Carriage Return)
            // std::cout << "\r";
            // for (int i = 0; i < barWidth; ++i) {
            //     if (i < pos) std::cout << "=";
            //     else if (i == pos) std::cout << ">";
            //     else std::cout << " ";
            // }
            //std::cout << "] " << std::fixed << std::setprecision(1) << percentage << "% " << std::flush;
            // std::cout << std::fixed << std::setprecision(4) << strategyEngine.pnl << std::flush;
        }
        cnt++;
        
        strategyEngine.update_PnL(book);
        if (strategyEngine.midPrice != 0) pnlLog << strategyEngine.pnl << "," << strategyEngine.midPrice << "," << strategyEngine.downScaledNetPos << "\n";
    }
    
    // strategyEngine.display_PnL(book);
    // std::cout << "\n";
    pnlLog.close();
    dataReplayEngine.bookFile.close();
    dataReplayEngine.tradesFile.close();
}

//CURRENTLY, SINGLE THREAD, SINGLE TEST VAL;
int main() {
    std::vector<std::string> outputs = {"day1", "day2", "day3", "day4", "day5"};
    std::vector<double> test_vals{0.0005, 0.001, 0.0015, 0.002, 0.0025};
    constexpr double gamma = 0.00005;
    constexpr double kappa = 1.5;

    for (int startDay = 5; startDay < 6; startDay++) {
        std::vector<std::future<double>> futures;
        for (int thread = 0; thread < 1; thread++) {
            int day = (startDay + thread) % 5;
            std::string bookFile = bookFiles[day];
            std::string tradesFile = tradesFiles[day];

            double testVal = test_vals[thread];

            std::string outputLog = "outputData/" + outputs[day] + "/" + outputs[day] + "k" + std::to_string(thread+1) + ".csv";
            
            futures.push_back(std::async(std::launch::async, run_singleStrategy, testVal, gamma, kappa, outputLog, bookFile, tradesFile));

        }

        std::cout << "Starting Batch for StartDay " << (startDay + 1) << "..." << std::endl;
        for (int i = 0; i < futures.size(); i++) {
            double resultPnl = futures[i].get(); 
            
            double progress = (static_cast<double>(i + 1) / futures.size()) * 100.0;
            int barWidth = 20;
            int pos = barWidth * (progress / 100.0);

            std::cout << "\rBatch Progress: [";
            for (int b = 0; b < barWidth; ++b) {
                if (b < pos) std::cout << "=";
                else if (b == pos) std::cout << ">";
                else std::cout << " ";
            }   
            std::cout << "] " << (i + 1) << "/" << futures.size() << " Sims Done" << std::flush;
        }
        std::cout << "\nFinished Batch " << (startDay + 1) << "\n" << std::endl;
    }

    return 0;
}


// double gamma = 0.3; //g_invRiskFactor;
// double kappa = 1.0; //g_k;