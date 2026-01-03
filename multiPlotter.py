import matplotlib.pyplot as plt
import os

# --- Configuration ---
days = ["day1", "day2", "day3", "day4", "day5"]
testIteration = [1, 2, 3, 4, 5]
colors = ["#1f77b4", "#ff7f0e", "#2ca02c", "#d62728", "#9467bd"]

# Loop through each day to create a separate plot
for day in days:
    folder_path = f"outputData/{day}"
    
    # Create a new figure for each day
    fig, ax1 = plt.subplots(figsize=(15, 8))
    mid_price_plotted = False
    ax2 = None

    # Loop through our 5 thread results for the specific day
    for i in range(5):
        file_name = f"{day}k{i+1}.csv"
        full_path = os.path.join(folder_path, file_name)
        
        if not os.path.exists(full_path):
            print(f"Warning: {full_path} not found. Skipping.")
            continue

        pnl_values = []
        mid_prices = []

        with open(full_path, "r") as f:
            for line in f:
                parts = line.strip().split(",")
                if len(parts) < 2 or float(parts[1]) == 0:
                    continue
                pnl_values.append(float(parts[0]))
                mid_prices.append(float(parts[1]))

        if not pnl_values:
            continue

        # Plot PnL for this Kappa
        ax1.plot(pnl_values, color=colors[i], label=f"testIteration {testIteration[i]}", alpha=0.8)

        # Plot Mid Price on the twin axis only once (from the first valid file)
        if not mid_price_plotted:
            ax2 = ax1.twinx()
            ax2.plot(mid_prices, color="black", label="BTC Mid Price", alpha=0.15, linestyle='--')
            ax2.set_ylabel("BTC Mid Price", color="black")
            mid_price_plotted = True

    # --- Formatting for the specific day ---
    ax1.set_title(f"Market Making Performance Comparison - {day.upper()}")
    ax1.set_xlabel("Ticks / Strategy Updates")
    ax1.set_ylabel("Realized PnL (USDT)")
    ax1.axhline(0, color='black', linewidth=1, linestyle=':')
    ax1.legend(loc='upper left')

    if mid_price_plotted and ax2:
        ax2.legend(loc='upper right')

    plt.tight_layout()
    
    # Save the plot as a file
    save_filename = f"performance_{day}.png"
    plt.savefig(save_filename)
    print(f"Saved: {save_filename}")
    
    # Close the plot to free up memory for the next loop iteration
    plt.close(fig)