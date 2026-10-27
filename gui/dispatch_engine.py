import csv
import json
import threading
from datetime import datetime

from config import (
    AMBULANCES, DISPATCHES, FLEET_STATE,
    SEVERITIES, AVG_SPEED_KMH, CONDITION_NEEDS, CONDITION_FACILITY
)
from routing import read_locations, normalize_blocks, run_route
from hospitals import split_list, read_hospitals, find_best_hospital

LOCK = threading.Lock()


# ---------- Ambulance fleet ----------

def read_ambulances():
    with AMBULANCES.open(encoding="utf-8-sig", newline="") as f:
        fleet = []
        for row in csv.DictReader(f):
            fleet.append({
                "id": int(row["ambulance_id"]),
                "location": int(row["location_id"]),
                "baseStatus": row["status"].strip().upper(),
                "type": row["type"].strip(),
                "capabilities": split_list(row["capabilities"])
            })
        return fleet


def read_busy():
    if not FLEET_STATE.exists():
        return {}
    try:
        with FLEET_STATE.open(encoding="utf-8") as f:
            return json.load(f)
    except ValueError:
        return {}


def save_busy(busy):
    with FLEET_STATE.open("w", encoding="utf-8") as f:
        json.dump(busy, f, indent=2)


def fleet_with_status():
    busy = read_busy()
    fleet = read_ambulances()

    for amb in fleet:
        if amb["baseStatus"] != "AVAILABLE":
            amb["status"] = "UNAVAILABLE"
        elif str(amb["id"]) in busy:
            amb["status"] = "BUSY"
        else:
            amb["status"] = "AVAILABLE"

    return fleet


def pick_ambulance(source, condition, blocks):
    need = CONDITION_NEEDS.get(condition, "Oxygen")

    free = []
    for amb in fleet_with_status():
        if amb["status"] == "AVAILABLE":
            free.append(amb)

    if len(free) == 0:
        raise ValueError("No ambulance is available right now")

    matching = []
    for amb in free:
        if need in amb["capabilities"]:
            matching.append(amb)

    matched = len(matching) > 0
    pool = matching if matched else free

    best_amb = None
    best_leg = None

    for amb in pool:
        if amb["location"] == source:
            leg = {
                "found": True,
                "distance": 0.0,
                "originalDistance": None,
                "path": [source]
            }
        else:
            leg = run_route(amb["location"], source, blocks)

        if not leg["found"]:
            continue

        if best_leg is None or leg["distance"] < best_leg["distance"]:
            best_amb = amb
            best_leg = leg

    if best_amb is None:
        raise ValueError("No ambulance can reach the patient")

    return best_amb, best_leg, matched, need


# ---------- Dispatch storage ----------

def read_dispatches():
    if not DISPATCHES.exists():
        return []
    try:
        with DISPATCHES.open(encoding="utf-8") as f:
            return json.load(f)
    except ValueError:
        return []


def save_dispatches(items):
    with DISPATCHES.open("w", encoding="utf-8") as f:
        json.dump(items, f, indent=2)


# ---------- Dispatch actions ----------

def clear_dispatches():
    with LOCK:
        save_dispatches([])
        save_busy({})


def complete_dispatch(dispatch_id):
    with LOCK:
        items = read_dispatches()
        target = None

        for item in items:
            if item.get("id") == dispatch_id:
                target = item
                break

        if target is None:
            raise ValueError("Dispatch not found")
        if target.get("status") == "Completed":
            raise ValueError("Dispatch already completed")

        target["status"] = "Completed"
        save_dispatches(items)

        busy = read_busy()
        key = str(target.get("ambulanceId"))
        if key in busy:
            del busy[key]
            save_busy(busy)

    return target


def create_dispatch(body):
    """Validate the request, pick hospital + ambulance, save and return
    the dispatch record. Raises ValueError for bad input."""
    name = str(body.get("name", "")).strip()
    condition = str(body.get("condition", "")).strip()
    severity = str(body.get("severity", "")).strip()
    age = int(body.get("age"))
    source = int(body.get("source"))

    if name == "":
        raise ValueError("Patient name is required")
    if age < 0 or age > 120:
        raise ValueError("Enter a valid age")
    if severity not in SEVERITIES:
        raise ValueError("Invalid severity")
    if condition == "":
        raise ValueError("Condition is required")

    valid_ids = {x["id"] for x in read_locations()}
    if source not in valid_ids:
        raise ValueError("Invalid pickup location")

    blocks = normalize_blocks(body.get("blocks") or [], valid_ids)

    hospital_name = ""
    hospital_need = CONDITION_FACILITY.get(condition, "Emergency")
    hospital_matched = True

    if body.get("autoHospital"):
        hospital, route, hospital_matched, hospital_need = (
            find_best_hospital(source, condition, blocks)
        )
        destination = hospital["location"]
        hospital_name = hospital["name"]
    else:
        destination = int(body.get("destination"))
        if destination not in valid_ids:
            raise ValueError("Invalid destination")
        if destination == source:
            raise ValueError("Pickup and hospital must differ")

        route = run_route(source, destination, blocks)

        hospital_matched = False
        for hospital in read_hospitals():
            if hospital["location"] == destination:
                hospital_name = hospital["name"]
                if hospital_need in hospital["facilities"]:
                    hospital_matched = True

    if not route["found"]:
        raise ValueError("No route available for this dispatch")

    with LOCK:
        amb, leg, matched, need = pick_ambulance(source, condition, blocks)

        total = leg["distance"] + route["distance"]
        eta = max(1, round(total / AVG_SPEED_KMH * 60))

        items = read_dispatches()
        now = datetime.now()
        record = {
            "id": len(items) + 1,
            "time": now.strftime("%d %b %H:%M:%S"),
            "createdAt": now.isoformat(),
            "name": name,
            "age": age,
            "condition": condition,
            "severity": severity,
            "source": source,
            "destination": destination,
            "hospitalName": hospital_name,
            "hospitalNeeded": hospital_need,
            "hospitalMatched": hospital_matched,
            "path": route["path"],
            "distance": route["distance"],
            "ambulanceId": amb["id"],
            "ambulanceType": amb["type"],
            "ambulanceFrom": amb["location"],
            "pickupPath": leg["path"],
            "pickupDistance": leg["distance"],
            "totalDistance": total,
            "needed": need,
            "matched": matched,
            "eta": eta,
            "rerouted": len(blocks) > 0,
            "blocked": [list(p) for p in blocks],
            "status": "En Route"
        }
        items.append(record)
        save_dispatches(items)

        busy = read_busy()
        busy[str(amb["id"])] = record["id"]
        save_busy(busy)

    return record