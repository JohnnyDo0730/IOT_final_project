from app.util.db import get_db

def processing_post_data(content):
    # 解析 JSON 數據
    is_light_on = content["is_light_on"]
    distance = content["distance"]

    if "temperature" in content:
            temperature = content["temperature"]
            humidity = content["humidity"]
            water_level = content["water_level"]
            co_raw = content["co_raw"]

            # 儲存到資料庫
            save_data(is_light_on, distance, temperature, humidity, water_level, co_raw)

            print("\n================================================")
            print(f"Received Air Sensor: temperature={temperature}, humidity={humidity}, CO={co_raw}")
            print(f"Received State Sensor: water_level={water_level}, is_light_on={is_light_on}")
            print(f"Received Ultrasonic Sensor: distance={distance}")
            print("================================================\n")
    else:
        save_data(is_light_on, distance)

        print("\n================================================")
        print(f"Received Sensor: is_light_on={is_light_on}, distance={distance}")
        print("================================================\n")


    return {"success": True, "message": "Data processed successfully"}

def save_data(is_light_on, distance, temperature=None, humidity=None, water_level=None, co_raw=None):
    db = get_db()

    try:
        # 儲存到低頻率資料庫 (每5輪才儲存一次)
        if temperature is not None and humidity is not None and water_level is not None and co_raw is not None:
            db.execute(
                'INSERT INTO low_frequency (temperature, humidity, water_level, co_raw) VALUES (?, ?, ?, ?)',
                (temperature, humidity, water_level, co_raw)
            )
        
        # 儲存到高頻率資料庫 (每輪都要)
        db.execute(
            'INSERT INTO high_frequency (is_light_on, distance) VALUES (?, ?)',
            (is_light_on, distance)
        )

        db.commit()
        print("資料儲存成功")
        return {"success": True, "message": "資料儲存成功"}

    except Exception as e:
        db.rollback()
        print(f"資料儲存失敗: {e}")
        raise e


'''
# 處理DHT11
def processing_dht11(temperature, humidity):
    # 測試
    print(f"temperature: {temperature}, humidity: {humidity}")

    # 儲存到資料庫
    #
    

    # 更新風扇模式
    #update_fan_mode(temperature, humidity)

    return {"success": True, "message": "DHT11 data processed successfully"}

# 處理水位感測器
def processing_water_level(water_level):
    # 儲存到資料庫
    #

    # 更新水位狀態與水龍頭開關
    #update_water_level(water_level)

    return {"success": True, "message": "Water level data processed successfully"}

# 處理紅外人體檢測
def processing_infrared(infrared):
    # 儲存到資料庫
    #

    # 更新紅外人體檢測狀態
    #update_infrared(infrared)

    return {"success": True, "message": "Infrared data processed successfully"}




# 更新風扇模式
def update_fan_mode(temperature, humidity):
    raise NotImplementedError

# 更新水位狀態與水龍頭開關
def update_water_level(water_level):
    raise NotImplementedError

# 更新紅外人體檢測狀態
def update_infrared(infrared):
    raise NotImplementedError

'''



