import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import os

# --- Configuration ---
day = "day1"
kappa_idx = 1
file_path = f"outputData/{day}/{day}k{kappa_idx}.csv"
save_path = f"outputData/{day}/result_plot.png"
REPLAY_SPEED = 5000 

# 1. Load the data
try:
    df = pd.read_csv(file_path, names=['PnL', 'MidPrice', 'Inventory'], header=None)
    print(f"Loaded {len(df)} lines. Replaying...")
except Exception as e:
    print(f"Error loading file: {e}")
    exit()

# 2. Setup the Plot
fig, (ax1, ax3) = plt.subplots(2, 1, figsize=(14, 8), 
                               gridspec_kw={'height_ratios': [3, 1]}, 
                               sharex=True)
ax2 = ax1.twinx()

# Initialize lines
pnl_line, = ax1.plot([], [], color='#1f77b4', lw=1.5, label="Cumulative PnL")
mid_line, = ax2.plot([], [], color='#ff7f0e', alpha=0.3, lw=1.0, label="BTC Price")
inv_line, = ax3.plot([], [], color='purple', lw=1.0, label="Inventory (q)")

ax3.axhline(0, color='black', lw=1, ls='--')

# Styling
ax1.set_title(f"Backtest Replay: {day} (k={kappa_idx})", fontsize=14, fontweight='bold')
ax1.set_ylabel("PnL (USDT)", color='#1f77b4', fontsize=12)
ax2.set_ylabel("Mid Price (USD)", color='#ff7f0e', fontsize=12)
ax3.set_ylabel("Position (BTC)", color='purple', fontsize=12)
ax3.set_xlabel("Ticks (Data Points)", fontsize=12)

ax1.grid(True, alpha=0.3)
ax2.get_yaxis().get_major_formatter().set_useOffset(False)

# Add legends
ax1.legend(loc='upper left')
ax2.legend(loc='upper right')
ax3.legend(loc='lower left')

# 3. Animation Function
def update(frame):
    end_idx = (frame + 1) * REPLAY_SPEED 
    
    # Check if we've hit the end
    if end_idx >= len(df):
        end_idx = len(df)
        ani.event_source.stop() # Stop the animation timer
        print(f"Replay finished. Final PnL: {df['PnL'].iloc[-1]:.2f}")
        plt.savefig(save_path, dpi=300) # Auto-save the final result
        print(f"Plot saved to {save_path}")

    subset = df.iloc[:end_idx]
    if subset.empty: 
        return pnl_line, mid_line, inv_line
    
    x = subset.index
    
    # Update Data
    pnl_line.set_data(x, subset['PnL'])
    mid_line.set_data(x, subset['MidPrice'])
    inv_line.set_data(x, subset['Inventory'])

    # Dynamic Scaling
    ax1.set_xlim(0, len(df)) # Set full scale immediately for better perspective
    
    # PnL scaling
    ax1.relim()
    ax1.autoscale_view(scalex=False, scaley=True)
    
    # MidPrice scaling
    p_min, p_max = subset['MidPrice'].min(), subset['MidPrice'].max()
    p_pad = (p_max - p_min) * 0.1 if p_max != p_min else 10.0
    ax2.set_ylim(p_min - p_pad, p_max + p_pad)

    # Inventory scaling
    i_min, i_max = subset['Inventory'].min(), subset['Inventory'].max()
    # Ensure symmetric inventory view so 0 is centered
    limit = max(abs(i_min), abs(i_max)) * 1.2
    ax3.set_ylim(-limit - 0.001, limit + 0.001)

    return pnl_line, mid_line, inv_line

# 4. Run the Animation
total_frames = (len(df) // REPLAY_SPEED) + 1
ani = FuncAnimation(fig, update, frames=total_frames, 
                    interval=10, repeat=False, blit=False)

plt.tight_layout()
plt.show() # This will keep the window open after the animation finishes