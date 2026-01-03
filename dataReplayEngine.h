#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "types.h"

class DataReplayEngine {
    public:
        std::ifstream bookFile;
        std::ifstream tradesFile;
        std::string nextTradeLine;
        std::string nextBookLine;
        MarketEvent nextTrade;
        MarketEvent nextBook;
        int lineNum = 2;

        int totalBookFileBytes;
        int totalTradesFileBytes;

        DataReplayEngine(const std::string& bookFileName, const std::string& tradeFileName) : bookFile(bookFileName), tradesFile(tradeFileName)  {
            nextTrade.type = MarketEvent::Type::TRADE;

            if (!bookFile.is_open()) {
                std::cerr << "Error: Could not open file " << bookFileName << "\n";
                return;
            }

            if (!tradesFile.is_open()) {
                std::cerr << "Error: Could not open file " << tradeFileName << "\n";
                return;
            }

            bookFile.seekg(0, std::ios::end);
            totalBookFileBytes = bookFile.tellg();
            bookFile.seekg(0, std::ios::beg);

            tradesFile.seekg(0, std::ios::end);
            totalTradesFileBytes = tradesFile.tellg();
            tradesFile.seekg(0, std::ios::beg);

            std::getline(tradesFile, nextTradeLine);
            get_next_trade();
            get_next_book();
        }

        bool has_more_data(std::ifstream &file);
        void get_next_event();
        void get_next_trade();
        void get_next_book();
};