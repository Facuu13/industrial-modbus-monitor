import os
import json
import sqlite3
from typing import Any, Dict, List, Optional

from fastapi import FastAPI, Query, HTTPException

DB_PATH = os.getenv("DB_PATH", "/data/telemetry.db")

app = FastAPI(title="Industrial Modbus Monitor API", version="0.1.0")


def get_conn():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


@app.get("/health")
def health():
    return {"status": "ok", "db": DB_PATH}


@app.get("/devices")
def devices(limit: int = Query(200, ge=1, le=2000)):
    with get_conn() as conn:
        rows = conn.execute(
            """
            SELECT DISTINCT device_id
            FROM telemetry
            ORDER BY device_id
            LIMIT ?
            """,
            (limit,),
        ).fetchall()

    return {"devices": [r["device_id"] for r in rows]}


@app.get("/latest")
def latest(device_id: str = Query(..., min_length=1)):
    with get_conn() as conn:
        row = conn.execute(
            """
            SELECT id, ts_ingest, device_id, topic, payload
            FROM telemetry
            WHERE device_id = ?
            ORDER BY id DESC
            LIMIT 1
            """,
            (device_id,),
        ).fetchone()

    if not row:
        raise HTTPException(status_code=404, detail="device_id not found")

    payload = row["payload"]
    try:
        payload_obj = json.loads(payload)
    except json.JSONDecodeError:
        payload_obj = payload  # fallback

    return {
        "id": row["id"],
        "ts_ingest": row["ts_ingest"],
        "device_id": row["device_id"],
        "topic": row["topic"],
        "payload": payload_obj,
    }


@app.get("/recent")
def recent(limit: int = Query(50, ge=1, le=500)):
    with get_conn() as conn:
        rows = conn.execute(
            """
            SELECT id, ts_ingest, device_id, topic, payload
            FROM telemetry
            ORDER BY id DESC
            LIMIT ?
            """,
            (limit,),
        ).fetchall()

    items: List[Dict[str, Any]] = []
    for r in rows:
        payload = r["payload"]
        try:
            payload_obj = json.loads(payload)
        except json.JSONDecodeError:
            payload_obj = payload

        items.append(
            {
                "id": r["id"],
                "ts_ingest": r["ts_ingest"],
                "device_id": r["device_id"],
                "topic": r["topic"],
                "payload": payload_obj,
            }
        )

    return {"items": items}
