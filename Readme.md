# IOT Final Project

這是我們的物聯網期末專案，包含初始化環境步驟、Git 使用流程與命名規則等說明。

## 📋 專案架構

![螢幕擷取畫面 2026-02-26 115606](https://github.com/user-attachments/assets/5033ff26-d68f-4dd7-b552-afeb2c959e69)
![螢幕擷取畫面 2025-06-08 175159](https://github.com/user-attachments/assets/45ecf2e7-a048-42bb-8f88-c7a800ae0aa5)

```
project/
│
├── app/                        # 應用程式主目錄
│   ├── __init__.py             # 應用程式初始化
│   ├── data/                   # 資料相關檔案
│   │   ├── insert_fake_data.sql # 插入假資料的 SQL 檔案
│   │   ├── schema.sql          # 資料庫 schema 定義檔案
│   │   └── insert_fake_data.py # 插入假資料的 Python 腳本
│   ├── route/                  # 路由控制
│   │   ├── __init__.py         # 路由初始化
│   │   ├── front_routes.py     # 前台頁面路由
│   │   └── esp_routes.py       # ESP32 相關路由
│   ├── service/                # 服務層
│   │   └── __init__.py         # 服務層初始化
│   │   ├── front_service.py     # 前台頁面服務層
│   │   └── esp_service.py       # ESP32 相關服務層
│   ├── templates/              # HTML 模板
│   │   └── index.html          # 主頁面模板
│   └── util/                   # 工具函數
│       └── db.py               # 資料庫連接工具
│
├── esp32/                      # ESP32 韌體程式碼
│   └── final/                  # 最終使用之版本
│       └── final.ino           # ESP32主程式檔案
│
├── run.py                      # 應用程式入口點
├── Pipfile                     # 依賴管理
├── Pipfile.lock                # 依賴版本鎖定
├── instance/                   # 資料庫實例目錄
├── self/                       # 私人檔案目錄 (不追蹤版本)
├── .gitignore                  # Git 忽略檔案設定
└── Readme.md                   # 專案說明文件
```

## 🚀 快速開始

### 補充說明
- 私人檔案(沒有要上傳的)，請放在project_root/self裡面
- 其他不想上傳的檔案可在.gitignore中新增路徑來自動忽略

### 環境需求

- Python 3.12
- pipenv (用於管理虛擬環境和依賴)

### 安裝步驟

1. 複製專案到本地

```bash
git init
git remote add origin https://github.com/JohnnyDo0730/IOT_final_project.git
git pull origin main
```

2. 安裝依賴

```bash
pip install pipenv
pipenv install
```

3. 啟動虛擬環境

```bash
pipenv shell
```

4. 啟動應用程式（強制使用當前虛擬環境執行）

```bash
pipenv run python run.py
```

應用程式將在 http://localhost:5000 啟動

5. 退出虛擬環境

```bash
exit
```

---

## 🔃 Git 常用操作流程

### 獲取遠端更新（**每次開始作業前**）

```bash
git pull origin main
```

### 新增或更新檔案後，將變更提交與上傳

```bash
git checkout -b feature/some-feature-name
git add .
git commit -m "簡要說明本次變更"
git push origin feature/some-feature-name
```

---
🌿 分支命名原則
請依據用途建立分支，建議格式如下：

| 類型 | 命名範例                    | 說明      |
| -- | ----------------------- | ------- |
| 功能 | `feature/login-system`  | 新增功能    |
| 修復 | `fix/login-bug`         | 修正錯誤    |
| 重構 | `refactor/db-structure` | 調整架構或重構 |
| 文件 | `docs/update-readme`    | 修改說明文件  |
| 測試 | `test/db-connection`    | 測試功能相關  |

---

 ✏️ Commit 命名建議

請用簡潔明確的方式命名 commit 訊息，例如：

* `add login feature`
* `fix bug in signup logic`
* `update ER diagram`
* `refactor query structure`
* `docs: update README.md`

---

## 🔧 技術架構

- **前端**：HTML、CSS、JavaScript
- **後端**：Python、Flask、ESP32
- **資料庫**：SQLite (內建於 Python 標準庫)
- **硬體**：ESP32
- **環境管理**：pipenv

## 📝 開發注意事項

1. 資料庫檔案會自動在 `instance` 目錄下創建
2. 初始化資料庫可使用以下命令(只有第一次需要執行)：
   ```bash
   pipenv shell
   flask init-db
   ```
3. 測試資料庫可使用以下命令(新增測試假資料)：
   ```bash
   pipenv shell
   python app\data\insert_fake_data.py
   ```
4. 開發新功能時，請遵循以下步驟：
   - 創建新分支
   - 在適當目錄下添加功能模組
   - 更新相關路由
   - 提交變更並發起 Pull Request
