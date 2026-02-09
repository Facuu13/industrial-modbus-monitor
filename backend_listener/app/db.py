import os
import sqlite3
from pathlib import Path

DB_PATH = Path(os.getenv("DB_PATH", "/data/telemetry.db"))

def get_conn():
    DB_PATH.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(DB_PATH)
    conn.execute("PRAGMA journal_mode=WAL;")
    conn.execute("PRAGMA synchronous=NORMAL;")
    return conn

def init_db():
    with get_conn() as conn:
        conn.execute("""
        CREATE TABLE IF NOT EXISTS telemetry (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            ts_ingest INTEGER NOT NULL,
            topic TEXT NOT NULL,
            device_id TEXT NOT NULL,
            payload TEXT NOT NULL
        );
        """)
        conn.execute("CREATE INDEX IF NOT EXISTS idx_telemetry_device_ts ON telemetry(device_id, ts_ingest);")
        conn.commit()

def insert_telemetry(ts_ingest: int, topic: str, device_id: str, payload: str):
    with get_conn() as conn:
        conn.execute(
            "INSERT INTO telemetry (ts_ingest, topic, device_id, payload) VALUES (?, ?, ?, ?)",
            (ts_ingest, topic, device_id, payload),
        )
        conn.commit()
