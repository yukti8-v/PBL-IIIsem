from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
from pathlib import Path
from datetime import datetime
import subprocess
import threading
import json
import csv

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

LOCK = threading.Lock()
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


# ---------- Data readers ----------

def read_locations():
    with LOCATIONS.open(encoding="utf-8-sig", newline="") as f:
        return [
            {"id": int(row["location_id"]), "name": row["name"]}
            for row in csv.DictReader(f)
        ]


def read_roads():
    with ROADS.open(encoding="utf-8-sig", newline="") as f:
        return [
            {
                "source": int(row["source"]),
                "destination": int(row["destination"]),
                "distance": float(row["distance"])
            }
            for row in csv.DictReader(f)
        ]


def read_coordinates():
    # Used later by the map feature (data/coordinates.csv)
    with COORDS.open(encoding="utf-8-sig", newline="") as f:
        return [
            {
                "id": int(row["location_id"]),
                "lat": float(row["lat"]),
                "lng": float(row["lng"])
            }
            for row in csv.DictReader(f)
        ]


def road_exists(a, b):
    for road in read_roads():
        x = road["source"]
        y = road["destination"]
        if (x == a and y == b) or (x == b and y == a):
            return True
    return False


# ---------- Hospitals ----------

def split_list(text):
    items = []
    for part in text.split(";"):
        part = part.strip()
        if part != "":
            items.append(part)
    return items


def read_hospitals():
    with HOSPITALS.open(encoding="utf-8-sig", newline="") as f:
        hospitals = []
        for row in csv.DictReader(f):
            hospitals.append({
                "id": int(row["hospital_id"]),
                "name": row["name"].strip(),
                "location": int(row["location_id"]),
                "status": row["status"].strip().upper(),
                "facilities": split_list(row["facilities"])
            })
        return hospitals


def available_hospitals():
    result = []
    for hospital in read_hospitals():
        if hospital["status"] == "AVAILABLE":
            result.append(hospital)
    return result


def nearest_in_pool(pool, source, block_a, block_b):
    best_hospital = None
    best_route = None

    for hospital in pool:
        if hospital["location"] == source:
            continue

        route = run_route(source, hospital["location"], block_a, block_b)

        if not route["found"]:
            continue

        if best_route is None or route["distance"] < best_route["distance"]:
            best_hospital = hospital
            best_route = route

    return best_hospital, best_route


def find_best_hospital(source, condition, block_a=None, block_b=None):
    need = CONDITION_FACILITY.get(condition, "Emergency")

    available = available_hospitals()
    if len(available) == 0:
        raise ValueError("No hospital is available right now")

    matching = []
    for hospital in available:
        if need in hospital["facilities"]:
            matching.append(hospital)

    matched = True
    best_hospital = None
    best_route = None

    if len(matching) > 0:
        best_hospital, best_route = nearest_in_pool(
            matching, source, block_a, block_b
        )

    if best_hospital is None:
        matched = False
        best_hospital, best_route = nearest_in_pool(
            available, source, block_a, block_b
        )

    if best_hospital is None:
        raise ValueError("No hospital reachable from here")

    return best_hospital, best_route, matched, need


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


def pick_ambulance(source, condition, block_a, block_b):
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
            leg = run_route(amb["location"], source, block_a, block_b)

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


# ---------- Routing ----------

def run_route(source, destination, block_a=None, block_b=None):
    args = [str(EXE), str(source), str(destination)]

    if block_a is not None and block_b is not None:
        args += [str(block_a), str(block_b)]

    result = subprocess.run(
        args,
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        timeout=10
    )

    if result.returncode != 0:
        raise ValueError(result.stderr.strip() or "Routing failed")

    parts = result.stdout.strip().split()
    if len(parts) < 4:
        raise ValueError("Invalid routing output")

    found = parts[0] == "1"
    distance = float(parts[1])
    original = float(parts[2])
    count = int(parts[3])
    path = [int(x) for x in parts[4:]]

    if count != len(path):
        raise ValueError("Invalid route path")

    return {
        "found": found,
        "distance": distance if found else None,
        "originalDistance": original if original >= 0 else None,
        "path": path
    }


# ---------- HTTP handler ----------

class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(GUI), **kwargs)

    def send_json(self, data, status=200):
        body = json.dumps(data).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        parsed = urlparse(self.path)

        try:
            length = int(self.headers.get("Content-Length", "0"))
            raw = self.rfile.read(length) if length > 0 else b"{}"
            body = json.loads(raw)

            if parsed.path == "/api/dispatches/clear":
                with LOCK:
                    save_dispatches([])
                    save_busy({})
                self.send_json({"cleared": True})
                return

            if parsed.path == "/api/dispatches/complete":
                dispatch_id = int(body.get("id"))

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

                self.send_json(target)
                return

            if parsed.path != "/api/dispatch":
                self.send_json({"error": "Not found"}, 404)
                return

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

            block_a = None
            block_b = None

            if (body.get("blockA") is not None
                    and body.get("blockB") is not None):
                block_a = int(body["blockA"])
                block_b = int(body["blockB"])
                if not road_exists(block_a, block_b):
                    raise ValueError("Selected road does not exist")

            hospital_name = ""
            hospital_need = CONDITION_FACILITY.get(condition, "Emergency")
            hospital_matched = True

            if body.get("autoHospital"):
                hospital, route, hospital_matched, hospital_need = (
                    find_best_hospital(
                        source, condition, block_a, block_b
                    )
                )
                destination = hospital["location"]
                hospital_name = hospital["name"]
            else:
                destination = int(body.get("destination"))
                if destination not in valid_ids:
                    raise ValueError("Invalid destination")
                if destination == source:
                    raise ValueError("Pickup and hospital must differ")

                route = run_route(source, destination, block_a, block_b)

                hospital_matched = False
                for hospital in read_hospitals():
                    if hospital["location"] == destination:
                        hospital_name = hospital["name"]
                        if hospital_need in hospital["facilities"]:
                            hospital_matched = True

            if not route["found"]:
                raise ValueError("No route available for this dispatch")

            with LOCK:
                amb, leg, matched, need = pick_ambulance(
                    source, condition, block_a, block_b
                )

                total = leg["distance"] + route["distance"]
                eta = max(1, round(total / AVG_SPEED_KMH * 60))

                items = read_dispatches()
                record = {
                    "id": len(items) + 1,
                    "time": datetime.now().strftime("%d %b %H:%M:%S"),
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
                    "rerouted": block_a is not None,
                    "status": "En Route"
                }
                items.append(record)
                save_dispatches(items)

                busy = read_busy()
                busy[str(amb["id"])] = record["id"]
                save_busy(busy)

            self.send_json(record, 201)

        except (ValueError, TypeError, KeyError) as e:
            self.send_json({"error": str(e)}, 400)
        except Exception as e:
            self.send_json({"error": str(e)}, 500)

    def do_GET(self):
        parsed = urlparse(self.path)

        if parsed.path == "/api/locations":
            try:
                self.send_json(read_locations())
            except Exception as e:
                self.send_json({"error": str(e)}, 500)
            return

        if parsed.path == "/api/roads":
            try:
                self.send_json(read_roads())
            except Exception as e:
                self.send_json({"error": str(e)}, 500)
            return

        if parsed.path == "/api/coordinates":
            try:
                self.send_json(read_coordinates())
            except Exception as e:
                self.send_json({"error": str(e)}, 500)
            return

        if parsed.path == "/api/hospitals":
            try:
                self.send_json(read_hospitals())
            except Exception as e:
                self.send_json({"error": str(e)}, 500)
            return

        if parsed.path == "/api/ambulances":
            try:
                self.send_json(fleet_with_status())
            except Exception as e:
                self.send_json({"error": str(e)}, 500)
            return

        if parsed.path == "/api/dispatches":
            try:
                items = read_dispatches()
                items.reverse()
                self.send_json(items)
            except Exception as e:
                self.send_json({"error": str(e)}, 500)
            return

        if parsed.path == "/api/nearest-hospital":
            try:
                params = parse_qs(parsed.query)
                source = int(params["source"][0])

                condition = "Other"
                if params.get("condition"):
                    condition = params["condition"][0]

                valid_ids = {x["id"] for x in read_locations()}
                if source not in valid_ids:
                    raise ValueError("Invalid source")

                hospital, route, matched, need = find_best_hospital(
                    source, condition
                )

                route["hospitalId"] = hospital["location"]
                route["hospitalName"] = hospital["name"]
                self.send_json(route)

            except (ValueError, KeyError, IndexError) as e:
                self.send_json({"error": str(e)}, 400)
            except Exception as e:
                self.send_json({"error": str(e)}, 500)
            return

        if parsed.path == "/api/route":
            try:
                params = parse_qs(parsed.query)

                source = int(params["source"][0])
                destination = int(params["destination"][0])

                valid_ids = {x["id"] for x in read_locations()}
                if source not in valid_ids or destination not in valid_ids:
                    raise ValueError("Invalid source or destination")

                a_values = params.get("blockA")
                b_values = params.get("blockB")

                if bool(a_values) != bool(b_values):
                    raise ValueError("Both blocked road IDs are required")

                block_a = None
                block_b = None

                if a_values and b_values:
                    block_a = int(a_values[0])
                    block_b = int(b_values[0])

                    if block_a not in valid_ids or block_b not in valid_ids:
                        raise ValueError("Invalid blocked road locations")

                    if not road_exists(block_a, block_b):
                        raise ValueError("Selected road does not exist")

                self.send_json(
                    run_route(source, destination, block_a, block_b)
                )

            except (ValueError, KeyError, IndexError) as e:
                self.send_json({"error": str(e)}, 400)
            except Exception as e:
                self.send_json({"error": str(e)}, 500)
            return

        super().do_GET()


if __name__ == "__main__":
    server = ThreadingHTTPServer(("127.0.0.1", 8000), Handler)
    print("Dashboard: http://127.0.0.1:8000")
    print("Press Ctrl+C to stop.")
    server.serve_forever()