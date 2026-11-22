import csv
import subprocess

from config import LOCATIONS, ROADS, COORDS, EXE, ROOT


# ---------- Map data readers ----------

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


def normalize_blocks(raw_pairs, valid_ids):
    blocks = []
    seen = set()

    for pair in raw_pairs:
        a = int(pair[0])
        b = int(pair[1])

        if a not in valid_ids or b not in valid_ids:
            raise ValueError("Invalid blocked road locations")
        if a == b:
            raise ValueError("Blocked road needs two different locations")
        if not road_exists(a, b):
            raise ValueError("Selected road does not exist")

        key = (min(a, b), max(a, b))
        if key in seen:
            continue

        seen.add(key)
        blocks.append((a, b))

    return blocks


def parse_blocks_text(text):
    pairs = []

    for part in text.split(","):
        part = part.strip()
        if part == "":
            continue

        pieces = part.split("-")
        if len(pieces) != 2:
            raise ValueError("Invalid blocked road list")

        pairs.append((int(pieces[0]), int(pieces[1])))

    return pairs


# ---------- Routing (calls the C++ executable) ----------


def run_route(source, destination, blocks=None, algo="dijkstra", stats=False):
    if algo not in ("dijkstra", "astar"):
        raise ValueError("Algorithm must be dijkstra or astar")

    args = [str(EXE), str(source), str(destination)]

    # Add blocked roads
    if blocks is not None:
        for pair in blocks:
            args.append(str(pair[0]))
            args.append(str(pair[1]))

    # Select algorithm and optionally request statistics
    args.extend(["--algo", algo])

    if stats:
        args.append("--stats")

    result = subprocess.run(
        args,
        cwd=str(ROOT),
        capture_output=True,
        text=True,
        timeout=10
    )

    if result.returncode != 0:
        raise ValueError(result.stderr.strip() or "Routing failed")

    # Parse only the first line for route information
    lines = result.stdout.strip().splitlines()

    if not lines:
        raise ValueError("Empty routing output")

    parts = lines[0].split()

    if len(parts) < 4:
        raise ValueError("Invalid routing output")

    found = parts[0] == "1"
    distance = float(parts[1])
    original = float(parts[2])
    count = int(parts[3])
    path = [int(x) for x in parts[4:]]

    if count != len(path):
        raise ValueError("Invalid route path")

    response = {
        "found": found,
        "distance": distance if found else None,
        "originalDistance": original if original >= 0 else None,
        "path": path
    }

    # Parse optional statistics from the second line
    if stats and len(lines) > 1:
        stats_data = {}

        for item in lines[1].split():
            if "=" in item:
                key, value = item.split("=", 1)
                stats_data[key] = value

        response["algorithm"] = stats_data.get("algorithm", algo)
        response["nodesVisited"] = int(
            stats_data.get("nodes_visited", 0)
        )
        response["timeUs"] = float(
            stats_data.get("time_us", 0)
        )

    return response
