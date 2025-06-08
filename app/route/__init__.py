from flask import Blueprint

# 主要頁面路由
esp_bp = Blueprint('esp', __name__)
front_bp = Blueprint('front', __name__)

# 導入路由模組
from app.route import esp_routes
from app.route import front_routes