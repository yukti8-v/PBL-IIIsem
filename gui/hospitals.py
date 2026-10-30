import csv

from config import HOSPITALS, CONDITION_FACILITY
from routing import run_route


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
            h_id = int(row["hospital_id"])
            
            # Read from CSV if present, otherwise generate realistic values
            total_beds = int(row.get("total_beds") or (40 + (h_id * 7) % 30))
            beds = int(row.get("beds") or (total_beds - 5 - (h_id * 3) % 15))
            
            total_icu = int(row.get("total_icu") or (10 + (h_id * 3) % 10))
            icu = int(row.get("icu") or (total_icu - 2 - (h_id * 2) % 6))

            hospitals.append({
                "id": h_id,
                "name": row["name"].strip(),
                "location": int(row["location_id"]),
                "status": row["status"].strip().upper(),
                "facilities": split_list(row["facilities"]),
                "beds": max(0, beds),
                "total_beds": total_beds,
                "icu": max(0, icu),
                "total_icu": total_icu
            })
        return hospitals


def available_hospitals():
    result = []
    for hospital in read_hospitals():
        if hospital["status"] == "AVAILABLE":
            result.append(hospital)
    return result


def nearest_in_pool(pool, source, blocks):
    best_hospital = None
    best_route = None

    for hospital in pool:
        if hospital["location"] == source:
            continue

        route = run_route(source, hospital["location"], blocks)

        if not route["found"]:
            continue

        if best_route is None or route["distance"] < best_route["distance"]:
            best_hospital = hospital
            best_route = route

    return best_hospital, best_route


def find_best_hospital(source, condition, blocks=None):
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
            matching, source, blocks
        )

    if best_hospital is None:
        matched = False
        best_hospital, best_route = nearest_in_pool(
            available, source, blocks
        )

    if best_hospital is None:
        raise ValueError("No hospital reachable from here")

    return best_hospital, best_route, matched, need