import csv
import random

def get_fan_rate(row):
    # CO 比例
    rate_co = 0.00
    if row['co_raw'] > 3000:
        return 100.00
    elif row['co_raw'] > 500:
        rate_co = (row['co_raw'] - 500) / (3000 - 500) * 100.00

    # 溫度比例
    temp_low = 32
    temp_high = 36
    rate_temp = 0.00
    if row['temperature'] > temp_low:
        if row['temperature'] >= temp_high:
            rate_temp = 100.00
        else:
            rate_temp = (row['temperature'] - temp_low) / (temp_high - temp_low) * 100.00

    # 濕度比例
    hum_low = 20
    hum_high = 50
    rate_hum = 0.00
    if row['humidity'] > hum_low:
        if row['humidity'] >= hum_high:
            rate_hum = 100.00
        else:
            rate_hum = (row['humidity'] - hum_low) / (hum_high - hum_low) * 100.00

    # 取最大比例值
    fan_rate = max(rate_temp, rate_hum, rate_co)
    return round(max(0.00, min(fan_rate, 100.00)), 2)

# 產生並寫入資料
with open(r'model\fan_training_data.csv', mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(['temperature', 'humidity', 'co_raw', 'fan_speed'])

    for _ in range(500):  # 產生200筆資料
        row = {
            'temperature': random.randint(25, 40),
            'humidity': random.randint(10, 60),
            'co_raw': random.randint(200, 4000),
            'water_level': random.randint(0, 1)
        }
        fan_speed = get_fan_rate(row)
        writer.writerow([
            row['temperature'],
            row['humidity'],
            row['co_raw'],
            fan_speed
        ])

print("✅ 已產出 fan_training_data.csv，可用於 Edge Impulse 回歸訓練")
