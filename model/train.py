import pandas as pd
from sklearn.linear_model import LinearRegression
from sklearn.model_selection import train_test_split
import joblib
import matplotlib.pyplot as plt

# 讀入資料
df = pd.read_csv('model/fan_training_data.csv')

# 特徵與標籤
X = df[['temperature', 'humidity', 'co_raw']]
y = df['fan_speed']

# 切分資料（80% 訓練，20% 測試）
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# 建立與訓練模型
model = LinearRegression()
model.fit(X_train, y_train)

# 測試集評估
score = model.score(X_test, y_test)
print(f"模型 R^2 分數（測試集）: {score:.4f}")
print("係數（temperature, humidity, co_raw）:", model.coef_)
print("截距:", model.intercept_)

# 畫圖觀察預測效果
y_pred = model.predict(X_test)
plt.scatter(y_test, y_pred)
plt.xlabel('True Fan Speed')
plt.ylabel('Predicted Fan Speed')
plt.title('Linear Regression Prediction')
plt.grid(True)
plt.show()

# 儲存模型
joblib.dump(model, 'model/fan_regression_model.pkl')
print("✅ 模型已儲存為 model/fan_regression_model.pkl")
