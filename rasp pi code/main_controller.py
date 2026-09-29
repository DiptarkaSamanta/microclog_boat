#!/usr/bin/env python3
"""
microclog_boat - Raspberry Pi Main Controller & Telemetry Server
------------------------------------------------------------------
Serves a web-based real-time dashboard for remote boat control,
processes telemetry from ESP32, and handles logging & navigation logic.
"""

import os
import sys
import time
import json
import logging
from threading import Thread, Lock
from flask import Flask, render_template, request, jsonify

# Setup Logging for Microclog Boat
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    handlers=[
        logging.FileHandler("boat_system.log"),
        logging.StreamHandler(sys.stdout)
    ]
)

app = Flask(__name__)
state_lock = Lock()

# Global Boat System State
boat_state = {
    "status": "ONLINE",
    "thrust": 0,
    "rudder": 90,
    "mode": "MANUAL",  # MANUAL, AUTOPILOT, EMERGENCY_STOP
    "distance_cm": 150.0,
    "battery_v": 12.4,
    "gps": {"lat": 22.5726, "lng": 88.3639},
    "heading": 145,
    "last_update": time.time()
}

class TelemetryBridge(Thread):
    """Bridge thread for handling Serial / UDP communication with ESP32."""
    def __init__(self):
        super().__init__()
        self.daemon = True
        self.running = True

    def run(self):
        logging.info("Telemetry Bridge initialized. Listening for ESP32 controller...")
        while self.running:
            time.sleep(0.5)
            with state_lock:
                # Simulate active sensors update for demo testing
                if boat_state["mode"] == "AUTOPILOT":
                    boat_state["heading"] = (boat_state["heading"] + 2) % 360
                    boat_state["thrust"] = 120
                boat_state["last_update"] = time.time()

bridge = TelemetryBridge()
bridge.start()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/telemetry', methods=['GET'])
def get_telemetry():
    with state_lock:
        return jsonify(boat_state)

@app.route('/api/control', methods=['POST'])
def update_control():
    data = request.json or {}
    with state_lock:
        if 'thrust' in data:
            boat_state['thrust'] = max(-255, min(255, int(data['thrust'])))
        if 'rudder' in data:
            boat_state['rudder'] = max(30, min(150, int(data['rudder'])))
        if 'mode' in data:
            boat_state['mode'] = data['mode']
        
        logging.info(f"Control Updated -> Thrust: {boat_state['thrust']}, Rudder: {boat_state['rudder']}, Mode: {boat_state['mode']}")
    return jsonify({"success": True, "state": boat_state})

if __name__ == '__main__':
    logging.info("Starting microclog_boat Raspberry Pi web control server on port 5000...")
    app.run(host='0.0.0.0', port=5000, debug=False)
