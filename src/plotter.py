import matplotlib.pyplot as plt;
import numpy as np;


pnlValues = []
midPriceValues = []
highPNLVal = 0
lowPNLVal = 0
highMPVal = 0
lowMPVal = float('inf')

with open("data/outputLog.csv", "r", encoding="utf-8") as f:
    for line in f:
        newLine = line.split(",")
        if (float(newLine[1]) == 0): continue

        nextPNLVal = float(newLine[0])
        nextMidPriceVal = float(newLine[1])

        pnlValues.append(nextPNLVal)
        midPriceValues.append(nextMidPriceVal)

        highPNLVal = max(highPNLVal, nextPNLVal)
        lowPNLVal = min(lowPNLVal, nextPNLVal)

        highMPVal = max(highMPVal, nextMidPriceVal)
        lowMPVal = min(lowMPVal, nextMidPriceVal)


f.close()

fig, ax1 = plt.subplots(figsize=(15, 7.5))

# --- PnL (left axis) ---
ax1.plot(pnlValues, c="blue", label="PnL")
ax1.axhline(0, ls=":", c="black")
ax1.set_ylabel("PnL", color="blue")
ax1.tick_params(axis="y", labelcolor="blue")
ax1.set_ylim((lowPNLVal+1) * 1.5, (highPNLVal+1) * 1.5)

# --- Mid Price (right axis) ---
ax2 = ax1.twinx()
ax2.plot(midPriceValues, c="red", label="Mid Price")
ax2.set_ylabel("Mid Price", color="red")
ax2.tick_params(axis="y", labelcolor="red")
ax2.set_ylim(lowMPVal * 0.98, highMPVal * 1.02)

# --- Title & layout ---
plt.title("PnL vs Mid Price Over Time")
fig.tight_layout()

plt.show()
