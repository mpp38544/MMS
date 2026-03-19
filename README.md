# MMS — Market Making Strategy

A high-frequency market making strategy implemented in C++ using the Avellaneda-Stoikov model. Simulates a full order book with dynamic bid-ask spread optimisation based on inventory risk and market volatility. Includes a backtesting engine tested on BTCUSDT historical data, with trade logs and PnL visualisation.

---

## What it does
The system simulates an order book, matches incoming orders against resting quotes, and tracks PnL over the backtest period. It utilises the Avellaneda Market Making Strategy.

---

## Project structure
```
MMS/
├── src/          ← C++ strategy and order book engine
├── data/         ← Historical BTCUSDT data in Parquet format
├── plots/        ← Python matplotlib visualisation scripts
└── CMakeLists.txt
```

---

## Build and run

**Prerequisites**
- CMake 3.15+
- C++17 compiler (GCC or Clang)
- Python 3 with `matplotlib` and `pandas` for visualisation

**Build**
```bash
mkdir build
cd build
cmake ..
make
```

**Run**
```bash
./MMS
```

Trade logs are printed to the terminal. To generate PnL charts:
```bash
cd src
python tandemPlotter.py
```

---

## Results

Backtested on BTCUSDT historical data. PnL curve and trade activity visualised via matplotlib.

---

## Tech stack

Strategy: Avellaneda-Stoikov model
Order book: Custom C++ simulation
Data storage: Parquet
Visualisation: Python + matplotlib

