#pragma once
#include <map>
#include "orderBook.h"
#include "constants.h"
#include "types.h"
#include "kalman.h"
#include <Eigen/Dense>
#include <deque>

class StrategyEngine {
    public:
        double cashBalance = 0.0;
        long long restingBuyPrice = 0;
        long long restingSellPrice = 0;
        long long netPosition = 0; //+ve for long position, -ve for short position
        long long midPrice = 0.0;
        double pnl = 0;
        double downScaledNetPos = 0;
        
        double currentVar = 0.0001;
        double lastMidPrice = 0;
        const double lambda = 0.94;

        long long END_TIME_STAMP = 1764633602583;
        long long START_TIME_STAMP = 0;
        long long timeStamp = 0;

        double gamma;
        double kappa;
        long long orderSize = 0;

        KalmanFilter kf;
        Eigen::VectorXd x0;
        bool kf_initialised = false;

        std::deque<double> priceHistory;
        const int VOL_WINDOW = 100;

        StrategyEngine() {
            int n = 2; // number of states (price only)
            int m = 1; // measurement (mid-price)

            double dt = 1.0; // per tick
            Eigen::MatrixXd A(n,n); A << 1, dt, 0, 1;      // random walk
            Eigen::MatrixXd C(m,n); C << 1, 0;      // measurement = price
            
            double price_noise = 10.0;  // $10 noise
            double drift_noise = 0.001;
            Eigen::MatrixXd Q(n,n); Q << price_noise*price_noise, 0, 0, drift_noise*drift_noise;
            
            double measurement_noise = 0.1;  // $1 measurement noise
            Eigen::MatrixXd R(m,m); R << measurement_noise * measurement_noise;

            Eigen::MatrixXd P(n,n); P << 100.0, 0, 0, 0.1;  // $100 price uncertainty

            x0 = Eigen::VectorXd(n); 
            x0 << 0.0, 0.0; // first mid-price in your backtest

            kf = KalmanFilter(dt, A, C, Q, R, P);
            kf.init(0.0, x0);
        }

        std::vector<TradeSignal> generate_signal(const OrderBook& book);
        void update_position(long long filledQty, long long price, TradeSignal::Side side, OrderBook& book);
        void display_PnL(OrderBook& book);
        void update_PnL(OrderBook& book);
        double updateAndGetVolatility(double midPrice);
        double getTimeFrac();
};