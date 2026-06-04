import random

NUM_ORDERS = 10_000
MID_PRICE  = 100
SPREAD     = 5

with open("random_orders.csv", "w") as f:
    for i in range(1, NUM_ORDERS + 1):
        side     = random.choice(["B", "S"])
        price    = random.randint(MID_PRICE - SPREAD, MID_PRICE + SPREAD)
        quantity = random.randint(1, 100)
        f.write(f"{i} {side} {price} {quantity}\n")

print(f"Generated {NUM_ORDERS} orders -> random_orders.csv")