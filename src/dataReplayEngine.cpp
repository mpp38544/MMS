#include <map>
#include "json.hpp"
#include <algorithm>
#include "dataReplayEngine.h"
#include "constants.h"

using nlohmann::json;

bool DataReplayEngine::has_more_data(std::ifstream &file) {
    return file.good();
}

void DataReplayEngine::get_next_trade() {
    if (std::getline(tradesFile, nextTradeLine)) {
        std::stringstream ss(nextTradeLine);

        std::string field;

        std::getline(ss, field, ','); //ID

        std::getline(ss, field, ','); //TIMESTAMP
        nextTrade.timestamp = stoll(field);

        std::getline(ss, field, ','); //PRICE
        nextTrade.price = static_cast<long long>(stod(field) * g_priceScale);

        std::getline(ss, field, ','); //VOLUME
        nextTrade.qty = static_cast<long long>(stod(field) * g_qtyScale);

        std::getline(ss, field, ','); //SIDE
        nextTrade.side = field;
    } else {
        nextTrade.timestamp = LLONG_MAX;
    }
}

void DataReplayEngine::get_next_book() {
    
    if(std::getline(bookFile, nextBookLine)) {
        nextBook.bookBids.clear();
        nextBook.bookAsks.clear();

        auto j = json::parse(nextBookLine);
        
        nextBook.timestamp = j["cts"].get<long long>(); //TIMESTAMP
        nextBook.type = (j["type"].get<std::string>() == "snapshot") ? MarketEvent::Type::SNAPSHOT : MarketEvent::Type::DELTA; //SIDE
            
        for (const auto& bid : j["data"]["b"]) {
            long long price = static_cast<long long>(stod(bid[0].get<std::string>()) * g_priceScale);
            long long qty = static_cast<long long>(stod(bid[1].get<std::string>()) * g_qtyScale);

            nextBook.bookBids.push_back({price, qty}); //BookBids
        }

        for (const auto& ask : j["data"]["a"]) {
            long long price = static_cast<long long>(stod(ask[0].get<std::string>()) * g_priceScale);
            long long qty = static_cast<long long>(stod(ask[1].get<std::string>()) * g_qtyScale);

            nextBook.bookAsks.push_back({price, qty}); //BookAsks
        }
    }  else {
        nextBook.timestamp = LLONG_MAX;
    }
}