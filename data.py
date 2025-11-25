import csv
import random
from datetime import datetime, timedelta

def generate_fake_btc_data(
    start_price=40000,
    num_points=50,
    start_time=datetime(2025, 1, 1),
    interval_minutes=1,
    outfile="fake_btc_data.csv"
):

    rows = []
    current_price = start_price
    timestamp = start_time

    for _ in range(num_points):
        change = random.uniform(-0.015, 0.015) 
        open_price = current_price
        close_price = open_price * (1 + change)

        high_price = max(open_price, close_price) * (1 + random.uniform(0.0, 0.003))
        low_price  = min(open_price, close_price) * (1 - random.uniform(0.0, 0.003))

        volume = random.uniform(5, 250)  

        rows.append([
            timestamp.strftime("%Y-%m-%d %H:%M:%S"),
            round(open_price, 2),
            round(high_price, 2),
            round(low_price, 2),
            round(close_price, 2),
            round(volume, 4)
        ])

        current_price = close_price
        timestamp += timedelta(minutes=interval_minutes)


    with open(outfile, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Timestamp", "Open", "High", "Low", "Close", "Volume"])
        writer.writerows(rows)

    print(f"Generated {num_points} fake BTC data points → {outfile}")


if __name__ == "__main__":
    generate_fake_btc_data()
