from app.util.db import get_db

def processing_get_data():
    
    # 從資料庫獲取基礎資料(溫度、濕度、水位) (近N筆)
    db = get_db()
    low_data = db.execute('SELECT temperature, humidity, co_raw, water_level, timestamp FROM low_frequency ORDER BY id DESC LIMIT 10').fetchall()
    high_data = db.execute('SELECT is_light_on, distance, timestamp FROM high_frequency ORDER BY id DESC LIMIT 10').fetchall()
    if not low_data or not high_data:
        raise ValueError("No data found in the database")
    else:
        low_data = [dict(row) for row in low_data]
        high_data = [dict(row) for row in high_data]

    # 判斷風扇速率
    fan_rate = get_fan_rate(low_data)
    
    # 判斷警報
    alarms = get_alarm_status(low_data, high_data)
    #print(f"Alarms: {alarms}")

    # 返回資料
    return {
            "low_frequency": low_data,
            "high_frequency": high_data,
            "fan_rate": fan_rate,
            "alarms": alarms
    }


def get_fan_rate(low_data):

    if low_data[-1]['co_raw'] > 3000:
        return 100.00  # 風扇全速運轉
    
    temp_low = 32
    temp_high = 36
    rate_temp = 0.00
    if low_data[-1]['temperature'] > temp_low:
        if low_data[-1]['temperature'] >= temp_high:
            rate_temp = 100.00
        else:
            rate_temp = (low_data[-1]['temperature'] - temp_low) / (temp_high - temp_low) * 100.00

    hum_low = 20
    hum_high = 50
    rate_hum = 0.00
    if low_data[-1]['humidity'] > hum_low:
        if low_data[-1]['humidity'] >= hum_high:
            rate_hum = 100.00
        else:
            rate_hum = (low_data[-1]['humidity'] - hum_low) / (hum_high - hum_low) * 100.00

    fan_rate = max(rate_temp, rate_hum) # 風扇速率(0-100%)
    fan_rate = max(0.00, min(fan_rate, 100.00))  # 確保在0-100%之間
    return fan_rate
    '''
    // 風扇Duty
    int calcFanDuty(int temp, int hum, int coRaw) {
    if (coRaw > 3000) return 255;

    int dutyT = 0, dutyH = 0;
    int temp_low = 32, temp_high = 36;
    if (temp > temp_low) {
        if (temp >= temp_high) dutyT = 255;
        else dutyT = (int)(255.0 * (temp - temp_low) / (temp_high - temp_low));
    }

    int hum_low = 20, hum_high = 50;
    if (hum > hum_low) {
        if (hum >= hum_high) dutyH = 255;
        else dutyH = (int)(255.0 * (hum - hum_low) / (hum_high - hum_low));
    }

    int duty = max(dutyT, dutyH);
    duty = constrain(duty, 0, 255);
    return duty;
    '''

def get_alarm_status(low_data, high_data):
    temperature_alarm = False
    humidity_alarm = False
    co_alarm = False
    water_level_alarm = False
    distance_alarm = False

    if low_data[0]['temperature'] > 36:
        temperature_alarm = True
    if low_data[0]['humidity'] > 50:
        humidity_alarm = True
    if low_data[0]['co_raw'] > 3000:
        co_alarm = True
    if low_data[0]['water_level'] == 1:
        water_level_alarm = True
    if high_data[0]['distance'] - high_data[1]['distance'] > 130:
        distance_alarm = True

    return {
        "temperature_alarm": temperature_alarm,
        "humidity_alarm": humidity_alarm,
        "co_alarm": co_alarm,
        "water_level_alarm": water_level_alarm,
        "distance_alarm": distance_alarm
    }
    

    





'''
def processing_activate(activate):
    raise NotImplementedError
'''


'''
def get_sensor_data():
    raise NotImplementedError


def get_device_status():
    raise NotImplementedError
'''



