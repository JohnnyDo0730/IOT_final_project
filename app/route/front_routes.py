from flask import render_template, request, jsonify
from app.route import front_bp
from app.service.front_service import processing_get_data

@front_bp.route('/')
def index():
    return render_template('index.html')

@front_bp.route('/front/get_data')
def get_data():
    try:
        # 呼叫處理函數
        data = processing_get_data()

        # 返回資料
        print(f"Success request data")
        return jsonify({"success": True, "data": data})
    except Exception as e:
        # 返回失敗
        print(f"Error request data: {str(e)}")
        return jsonify({"success": False, "error": str(e)})
        
'''
@front_bp.post('/front/activate')
def activate():
    try:
        # 從請求中獲取 JSON 數據
        content = request.get_json()

        # 解析 JSON 數據
        # 應為開啟或關閉

        # 呼叫處理函數
        processing_activate(activate)

        # 返回成功
        print(f"Success activate: {activate}")
        return jsonify({"success": True})
    except Exception as e:
        # 返回失敗
        print(f"Error activate: {str(e)}")
        return jsonify({"success": False, "error": str(e)})
'''


