#pragma once

static inline constexpr long long g_priceScale = 100;  // Back to $0.01 precision
static inline constexpr long long g_qtyScale = 1000000;
static inline constexpr long long g_orderSize = static_cast<long long>(0.001 * g_qtyScale);  // 0.001 BTC
static inline constexpr double g_maxPos = 0.1;  // 0.1 BTC max position

static inline constexpr bool toggleDebugPrint = false;  // Turn ON debug