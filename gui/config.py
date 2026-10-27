from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
GUI = ROOT / "gui"
EXE = ROOT / "routing_cli.exe"

LOCATIONS = ROOT / "data" / "locations.csv"
ROADS = ROOT / "data" / "roads.csv"
AMBULANCES = ROOT / "data" / "ambulances.csv"
HOSPITALS = ROOT / "data" / "hospitals.csv"
COORDS = ROOT / "data" / "coordinates.csv"
DISPATCHES = ROOT / "data" / "dispatches.json"
FLEET_STATE = ROOT / "data" / "ambulance_status.json"

SEVERITIES = ["Critical", "Serious", "Stable"]
AVG_SPEED_KMH = 30

CONDITION_NEEDS = {
    "Cardiac Arrest": "Cardiac",
    "Road Accident": "Trauma",
    "Stroke": "ICU",
    "Burns": "BurnSupport",
    "Breathing Difficulty": "Oxygen",
    "Pregnancy / Delivery": "Maternity",
    "Child Emergency": "Pediatric",
    "Other": "Oxygen"
}

CONDITION_FACILITY = {
    "Cardiac Arrest": "Cardiology",
    "Road Accident": "Trauma",
    "Stroke": "ICU",
    "Burns": "BurnUnit",
    "Breathing Difficulty": "ICU",
    "Pregnancy / Delivery": "Maternity",
    "Child Emergency": "Pediatrics",
    "Other": "Emergency"
}