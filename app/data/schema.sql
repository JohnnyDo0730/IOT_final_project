CREATE TABLE low_frequency (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    temperature INTEGER,
    humidity INTEGER,
    co_raw INTEGER,
    water_level BOOLEAN,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE high_frequency (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    is_light_on BOOLEAN,
    distance REAL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);