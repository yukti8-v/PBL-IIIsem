import csv
import json
import uuid
from datetime import datetime
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlparse

ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / "data"
DATA.mkdir(exist_ok=True)

QUEUE_FILE = DATA / "member2_queue.json"
RESOURCE_FILE = DATA / "member2_resources.json"
DISPATCH_FILE = DATA / "member2_dispatches.json"
HOSPITALS_CSV = DATA / "hospitals.csv"
AMBULANCES_CSV = DATA / "ambulances.csv"


def load_json(path, default):
    try:
        with path.open("r", encoding="utf-8") as f:
            return json.load(f)
    except (FileNotFoundError, json.JSONDecodeError):
        return default


def save_json(path, value):
    with path.open("w", encoding="utf-8") as f:
        json.dump(value, f, indent=2)


def read_csv(path):
    if not path.exists():
        return []
    with path.open("r", encoding="utf-8-sig", newline="") as f:
        return list(csv.DictReader(f))


def pick(row, *keys, default=""):
    for key in keys:
        if key in row and row[key] not in (None, ""):
            return row[key]
    return default


def to_int(value, default=0):
    try:
        return int(float(str(value).strip()))
    except (TypeError, ValueError):
        return default


def hospitals():
    result = []
    for i, row in enumerate(read_csv(HOSPITALS_CSV), 1):
        result.append({
            "hospital_id": str(pick(row, "hospital_id", "hospitalId", "id", default=f"H{i}")),
            "name": str(pick(row, "name", "hospital_name", "hospitalName", default=f"Hospital {i}")),
            "status": str(pick(row, "status", default="AVAILABLE")),
            "facilities": str(pick(row, "facilities", "facility", "specialities", default="Emergency;Trauma;ICU;Cardiology;BurnUnit;Maternity;Pediatrics"))
        })
    if not result:
        names = [
            ("H1", "City Hospital", "Emergency;Trauma;ICU"),
            ("H2", "General Hospital", "Emergency;ICU"),
            ("H3", "Trauma Centre", "Emergency;Trauma;ICU"),
            ("H4", "Heart Care Hospital", "Emergency;Cardiology;ICU"),
            ("H5", "Children Hospital", "Emergency;Pediatrics"),
            ("H6", "Burn Care Hospital", "Emergency;BurnUnit;ICU"),
            ("H7", "Maternity Hospital", "Emergency;Maternity")
        ]
        result = [{"hospital_id": a, "name": b, "status": "AVAILABLE", "facilities": c} for a, b, c in names]
    return result


def ambulances():
    result = []
    for i, row in enumerate(read_csv(AMBULANCES_CSV), 1):
        result.append({
            "ambulance_id": str(pick(row, "ambulance_id", "ambulanceId", "id", default=i)),
            "status": str(pick(row, "status", default="AVAILABLE"))
        })
    if not result:
        result = [{"ambulance_id": str(i), "status": "AVAILABLE"} for i in range(1, 13)]
    return result


def resources():
    saved = load_json(RESOURCE_FILE, {})
    changed = False
    for h in hospitals():
        hid = h["hospital_id"]
        if hid not in saved:
            saved[hid] = {
                "name": h["name"],
                "totalBeds": 100, "availableBeds": 100,
                "totalICU": 20, "availableICU": 20,
                "totalVentilators": 10, "availableVentilators": 10,
                "totalDoctors": 30, "availableDoctors": 30
            }
            changed = True
    if changed or not RESOURCE_FILE.exists():
        save_json(RESOURCE_FILE, saved)
    return saved


def get_queue():
    items = load_json(QUEUE_FILE, [])
    now = datetime.now()
    for p in items:
        if p.get("status") == "Waiting":
            try:
                minutes = (now - datetime.fromisoformat(p["createdAt"])).total_seconds() / 60
            except (KeyError, ValueError):
                minutes = 0
            p["waitingMinutes"] = round(max(0, minutes), 1)
            if minutes >= 30:
                p["severity"] = "Critical"
                p["escalated"] = True
            elif minutes >= 15 and p.get("severity") == "Stable":
                p["severity"] = "Serious"
                p["escalated"] = True
            p["goldenHourRemaining"] = max(0, round(p.get("goldenHourMin", 60) - minutes, 1))
    save_json(QUEUE_FILE, items)
    order = {"Critical": 0, "Serious": 1, "Stable": 2}
    return sorted(
        [p for p in items if p.get("status") == "Waiting"],
        key=lambda p: (order.get(p.get("severity"), 3), p.get("createdAt", ""))
    )


def assign_waiting():
    # Update waiting-time/escalation values, then load the COMPLETE queue.
    get_queue()
    queue = load_json(QUEUE_FILE, [])
    dispatches = load_json(DISPATCH_FILE, [])
    fleet = ambulances()
    hospital_list = hospitals()
    res = resources()
    assigned = []

    busy_ids = {
        str(d.get("ambulanceId"))
        for d in dispatches
        if d.get("status") == "Dispatched"
    }

    order = {"Critical": 0, "Serious": 1, "Stable": 2}
    waiting = sorted(
        [p for p in queue if p.get("status") == "Waiting"],
        key=lambda p: (order.get(p.get("severity"), 3), p.get("createdAt", ""))
    )

    for patient in waiting:
        available = [
            a for a in fleet
            if str(a["ambulance_id"]) not in busy_ids
            and str(a.get("status", "AVAILABLE")).upper() == "AVAILABLE"
        ]
        if not available:
            break

        needed = {
            "Cardiac Arrest": "Cardiology",
            "Road Accident": "Trauma",
            "Stroke": "ICU",
            "Burns": "BurnUnit",
            "Pregnancy / Delivery": "Maternity",
            "Child Emergency": "Pediatrics"
        }.get(patient.get("condition"), "Emergency")

        choices = [
            h for h in hospital_list
            if h["status"].upper() == "AVAILABLE"
            and needed in h["facilities"].split(";")
            and h["hospital_id"] in res
            and res[h["hospital_id"]]["availableBeds"] > 0
        ]
        if not choices:
            choices = [
                h for h in hospital_list
                if h["status"].upper() == "AVAILABLE"
                and h["hospital_id"] in res
                and res[h["hospital_id"]]["availableBeds"] > 0
            ]
        if not choices:
            continue

        hospital = choices[0]
        r = res[hospital["hospital_id"]]
        r["availableBeds"] = max(0, r["availableBeds"] - 1)
        icu_reserved = patient.get("severity") == "Critical" and r["availableICU"] > 0
        if icu_reserved:
            r["availableICU"] -= 1

        ambulance_id = str(available[0]["ambulance_id"])
        dispatch = {
            "id": str(uuid.uuid4())[:8],
            "patientId": patient["id"],
            "patientName": patient["name"],
            "condition": patient.get("condition", "Other"),
            "severity": patient.get("severity", "Stable"),
            "hospital": hospital["name"],
            "hospitalId": hospital["hospital_id"],
            "ambulanceId": ambulance_id,
            "icuReserved": icu_reserved,
            "status": "Dispatched",
            "createdAt": datetime.now().isoformat()
        }
        dispatches.append(dispatch)
        busy_ids.add(ambulance_id)
        patient["status"] = "Dispatched"
        patient["dispatchId"] = dispatch["id"]
        assigned.append(dispatch)

    save_json(QUEUE_FILE, queue)
    save_json(RESOURCE_FILE, res)
    save_json(DISPATCH_FILE, dispatches)
    return assigned


def add_patient(body, should_assign=True):
    name = str(body.get("name", "")).strip()
    condition = str(body.get("condition", "Other"))
    severity = str(body.get("severity", "Stable"))
    if not name:
        raise ValueError("Patient name is required")
    if severity not in ("Critical", "Serious", "Stable"):
        raise ValueError("Invalid severity")

    patient = {
        "id": str(uuid.uuid4())[:8],
        "name": name,
        "age": to_int(body.get("age", 20), 20),
        "condition": condition,
        "severity": severity,
        "source": body.get("source", "Manual Registration"),
        "heartRate": body.get("heartRate", ""),
        "spo2": body.get("spo2", ""),
        "goldenHourMin": to_int(body.get("goldenHourMin", 60), 60),
        "createdAt": datetime.now().isoformat(),
        "status": "Waiting",
        "escalated": False
    }
    items = load_json(QUEUE_FILE, [])
    items.append(patient)
    save_json(QUEUE_FILE, items)
    assigned = assign_waiting() if should_assign else []
    return {"patient": patient, "assigned": assigned}


def complete_dispatch(dispatch_id):
    dispatches = load_json(DISPATCH_FILE, [])
    res = resources()
    found = None
    for d in dispatches:
        if d.get("id") == dispatch_id and d.get("status") == "Dispatched":
            d["status"] = "Completed"
            found = d
            break
    if not found:
        raise ValueError("Active dispatch not found")

    r = res.get(found["hospitalId"])
    if r:
        r["availableBeds"] = min(r["totalBeds"], r["availableBeds"] + 1)
        if found.get("icuReserved"):
            r["availableICU"] = min(r["totalICU"], r["availableICU"] + 1)

    save_json(RESOURCE_FILE, res)
    save_json(DISPATCH_FILE, dispatches)
    newly_assigned = assign_waiting()
    return {"completed": found, "newlyAssigned": newly_assigned}


PAGE = r"""
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Emergency Response Dashboard</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#0b1020;color:#e8ecf7;font-family:Arial,sans-serif;padding:24px}
.wrap{max-width:1200px;margin:auto}h1{margin:0 0 8px;color:#fff}h2{font-size:20px;margin:0 0 16px}
.sub{color:#aab4ce;margin-bottom:24px}.panel{background:#151d33;border:1px solid #293653;border-radius:14px;padding:20px;margin:18px 0}
.formgrid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px}
input,select,button{padding:12px;border-radius:8px;border:1px solid #394968;background:#0e1629;color:#fff;width:100%}
button{background:#6d5dfc;border:0;font-weight:bold;cursor:pointer}button.secondary{background:#263653}button.danger{background:#b54b59}
.actions{display:flex;gap:10px;flex-wrap:wrap;margin-top:12px}.actions button{width:auto}
table{width:100%;border-collapse:collapse;min-width:650px}th,td{text-align:left;padding:11px;border-bottom:1px solid #2b3854;font-size:14px}
th{color:#aebce0} .tablewrap{overflow-x:auto}.muted{color:#9ba8c4}.status{font-weight:bold;color:#80e0a5}
#message{margin-top:12px;color:#ffd479;white-space:pre-wrap}
</style>
</head>
<body><div class="wrap">
<h1>Campus Emergency Response System</h1>
<div class="sub">Member 2 • Emergency registration, priority queue, hospital resources and ambulance dispatch</div>

<section class="panel">
<h2>Register Emergency Patient</h2>
<div class="formgrid">
<input id="name" placeholder="Patient name *">
<input id="age" type="number" min="1" placeholder="Age" value="20">
<select id="condition">
<option>Road Accident</option><option>Cardiac Arrest</option><option>Stroke</option>
<option>Burns</option><option>Breathing Difficulty</option><option>Pregnancy / Delivery</option>
<option>Child Emergency</option><option>Other</option>
</select>
<select id="severity"><option>Critical</option><option>Serious</option><option selected>Stable</option></select>
<input id="heartRate" type="number" placeholder="Heart rate (optional)">
<input id="spo2" type="number" placeholder="SpO2 (optional)">
</div>
<div class="actions"><button onclick="addPatient()">Add Patient</button><button class="secondary" onclick="refreshAll()">Refresh Dashboard</button></div>
<div id="message"></div>
</section>

<section class="panel">
<h2>Disaster Mode</h2>
<div class="muted">Create five sample emergency cases for demonstration.</div>
<div class="actions"><button class="danger" onclick="disaster()">Create 5 Emergency Cases</button></div>
</section>

<section class="panel">
<h2>Waiting Priority Queue</h2>
<div class="tablewrap"><table><thead><tr><th>Name</th><th>Condition</th><th>Severity</th><th>Wait (min)</th><th>Golden hour left</th></tr></thead><tbody id="queue"></tbody></table></div>
</section>

<section class="panel">
<h2>Hospital Resources</h2>
<div class="tablewrap"><table><thead><tr><th>Hospital</th><th>Beds</th><th>ICU</th><th>Ventilators</th><th>Doctors</th></tr></thead><tbody id="hospitals"></tbody></table></div>
</section>

<section class="panel">
<h2>Ambulance Dispatches</h2>
<div class="tablewrap"><table><thead><tr><th>Patient</th><th>Hospital</th><th>Ambulance</th><th>Status</th><th>Action</th></tr></thead><tbody id="dispatches"></tbody></table></div>
</section>
</div>
<script>
const $ = id => document.getElementById(id);
async function api(path, options={}) {
  const response = await fetch(path, {headers:{"Content-Type":"application/json"}, ...options});
  const data = await response.json();
  if (!response.ok) throw new Error(data.error || "Request failed");
  return data;
}
function esc(s){return String(s ?? "").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));}
async function refreshAll(){
  try{
    const [q,h,d]=await Promise.all([api("/api/queue"),api("/api/hospitals"),api("/api/dispatches")]);
    $("queue").innerHTML=q.length?q.map(p=>`<tr><td>${esc(p.name)}</td><td>${esc(p.condition)}</td><td>${esc(p.severity)}</td><td>${p.waitingMinutes ?? 0}</td><td>${p.goldenHourRemaining ?? 60}</td></tr>`).join(""):`<tr><td colspan="5" class="muted">No patients waiting</td></tr>`;
    $("hospitals").innerHTML=h.map(x=>{let r=x.resources;return `<tr><td>${esc(x.name)}</td><td>${r.availableBeds}/${r.totalBeds}</td><td>${r.availableICU}/${r.totalICU}</td><td>${r.availableVentilators}/${r.totalVentilators}</td><td>${r.availableDoctors}/${r.totalDoctors}</td></tr>`}).join("");
    $("dispatches").innerHTML=d.length?d.slice().reverse().map(x=>`<tr><td>${esc(x.patientName)}</td><td>${esc(x.hospital)}</td><td>${esc(x.ambulanceId)}</td><td class="status">${esc(x.status)}</td><td>${x.status==="Dispatched"?`<button class="secondary" onclick="completeDispatch('${esc(x.id)}')">Complete</button>`:"—"}</td></tr>`).join(""):`<tr><td colspan="5" class="muted">No ambulance dispatches yet</td></tr>`;
  }catch(e){$("message").textContent=e.message;}
}
async function addPatient(){
  try{
    const name=$("name").value.trim();
    if(!name){$("message").textContent="Enter patient name first.";return;}
    const result=await api("/api/patient",{method:"POST",body:JSON.stringify({
      name,age:$("age").value,condition:$("condition").value,severity:$("severity").value,
      heartRate:$("heartRate").value,spo2:$("spo2").value
    })});
    $("message").textContent=`Patient registered. New dispatches: ${result.assigned.length}`;
    $("name").value=""; await refreshAll();
  }catch(e){$("message").textContent=e.message;}
}
async function disaster(){
  try{
    const result=await api("/api/disaster",{method:"POST",body:"{}"});
    $("message").textContent=`Disaster mode: ${result.created} cases created; ${result.assigned} dispatched.`;
    await refreshAll();
  }catch(e){$("message").textContent=e.message;}
}
async function completeDispatch(id){
  try{
    const result=await api("/api/complete",{method:"POST",body:JSON.stringify({id})});
    $("message").textContent=`Dispatch completed. Newly assigned: ${result.newlyAssigned.length}`;
    await refreshAll();
  }catch(e){$("message").textContent=e.message;}
}
refreshAll();
setInterval(refreshAll,5000);
</script>
</body></html>
"""


class Handler(BaseHTTPRequestHandler):
    def send_data(self, data, status=200, content_type="application/json"):
        payload = data.encode("utf-8") if isinstance(data, str) else json.dumps(data).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", content_type + ("; charset=utf-8" if "charset" not in content_type else ""))
        self.send_header("Content-Length", str(len(payload)))
        self.end_headers()
        self.wfile.write(payload)

    def do_GET(self):
        path = urlparse(self.path).path
        if path == "/":
            return self.send_data(PAGE, content_type="text/html")
        if path == "/api/queue":
            return self.send_data(get_queue())
        if path == "/api/hospitals":
            result = []
            res = resources()
            for h in hospitals():
                if h["hospital_id"] in res:
                    result.append({"name": h["name"], "resources": res[h["hospital_id"]]})
            return self.send_data(result)
        if path == "/api/dispatches":
            return self.send_data(load_json(DISPATCH_FILE, []))
        return self.send_data({"error": "Not found"}, 404)

    def do_POST(self):
        try:
            length = int(self.headers.get("Content-Length", 0))
            body = json.loads(self.rfile.read(length) or b"{}")
            path = urlparse(self.path).path
            if path == "/api/patient":
                return self.send_data(add_patient(body), 201)
            if path == "/api/disaster":
                cases = [
                    ("Disaster Patient 1", "Road Accident", "Critical"),
                    ("Disaster Patient 2", "Cardiac Arrest", "Critical"),
                    ("Disaster Patient 3", "Burns", "Serious"),
                    ("Disaster Patient 4", "Stroke", "Critical"),
                    ("Disaster Patient 5", "Breathing Difficulty", "Stable")
                ]
                created = [
                    add_patient({"name": n, "age": 25, "condition": c,
                                 "severity": s, "source": "Disaster Zone"},
                                should_assign=False)["patient"]
                    for n, c, s in cases
                ]
                assigned = assign_waiting()
                return self.send_data({"created": len(created), "assigned": len(assigned)}, 201)
            if path == "/api/complete":
                return self.send_data(complete_dispatch(str(body.get("id", ""))))
            return self.send_data({"error": "Not found"}, 404)
        except Exception as e:
            return self.send_data({"error": str(e)}, 400)

    def log_message(self, fmt, *args):
        print("%s - %s" % (self.address_string(), fmt % args))


if __name__ == "__main__":
    resources()
    print("Member 2 backend is running!")
    print("Open http://127.0.0.1:8001 in your browser")
    ThreadingHTTPServer(("127.0.0.1", 8001), Handler).serve_forever()
