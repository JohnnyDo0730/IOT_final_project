from flask import render_template, request, jsonify
from app.route import esp_bp
from app.service.esp_service import processing_post_data


@esp_bp.post('/esp/post_data')
def post_data():
    try:
        # 從請求中獲取 JSON 數據
        content = request.get_json()

        result = processing_post_data(content)
        return jsonify({"success": True, "result": result["message"]})

    except Exception as e:
        # 返回失敗
        print(f"Error receiving data: {str(e)}")
        return jsonify({"success": False, "error": str(e)})


'''
# DHT11
@esp_bp.post('/esp/dht11')
def dht11():
    try:
        # 從請求中獲取 JSON 數據
        content = request.get_json()

        # 解析 JSON 數據
        temperature = content["temperature"]
        humidity = content["humidity"]

        # 呼叫處理函數
        result = processing_dht11(temperature, humidity)

        # 返回成功
        print(f"Received data: temperature={temperature},humidity={humidity}")
        return jsonify({"success": True, "result": result["message"]})

    except Exception as e:
        # 返回失敗
        print(f"Error receiving data: {str(e)}")
        return jsonify({"success": False, "error": str(e)})


# 水位感測器
@esp_bp.post('/esp/water_level')
def water_level():
    try:
        # 從請求中獲取 JSON 數據
        content = request.get_json()

        # 解析 JSON 數據
        #

        # 呼叫處理函數
        processing_water_level(water_level)

        #返回成功
        print(f"Received data: water_level={water_level}")
        return jsonify({"success": True})
    except Exception as e:
        #返回失敗
        print(f"Error receiving data: {str(e)}")
        return jsonify({"success": False, "error": str(e)})


# 紅外人體檢測
@esp_bp.post('/esp/infrared')
def infrared():
    try:
        # 從請求中獲取 JSON 數據
        content = request.get_json()

        # 解析 JSON 數據
        #

        # 呼叫處理函數
        processing_infrared(infrared)

        #返回成功
        print(f"Received data: infrared={infrared}")
        return jsonify({"success": True})
    except Exception as e:
        #返回失敗
        print(f"Error receiving data: {str(e)}")
        return jsonify({"success": False, "error": str(e)})
'''