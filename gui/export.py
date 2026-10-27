import csv
import io
from datetime import datetime

from routing import read_locations

EXPORT_COLUMNS = [
    ("id", "ID"),
    ("time", "Time"),
    ("name", "Patient"),
    ("age", "Age"),
    ("condition", "Condition"),
    ("severity", "Severity"),
    ("ambulance", "Ambulance"),
    ("hospitalName", "Hospital"),
    ("route", "Route"),
    ("totalDistance", "Distance (km)"),
    ("eta", "ETA (min)"),
    ("rerouted", "Rerouted"),
    ("status", "Status"),
]


def filter_dispatches(items, params):
    sev = params.get("severity", [""])[0]
    status = params.get("status", [""])[0]
    cond = params.get("condition", [""])[0]

    result = []
    for item in items:
        if sev and item.get("severity") != sev:
            continue
        if status and item.get("status") != status:
            continue
        if cond and item.get("condition") != cond:
            continue
        result.append(item)
    return result


def export_rows(items):
    names = {x["id"]: x["name"] for x in read_locations()}
    rows = []

    for item in items:
        pickup = item.get("pickupPath") or []
        path = item.get("path") or []
        full = pickup + path[1:] if pickup else path
        route_text = " -> ".join(names.get(n, str(n)) for n in full)

        rows.append({
            "id": item.get("id"),
            "time": item.get("time"),
            "name": item.get("name"),
            "age": item.get("age"),
            "condition": item.get("condition"),
            "severity": item.get("severity"),
            "ambulance": "#%s %s" % (
                item.get("ambulanceId"), item.get("ambulanceType", "")
            ),
            "hospitalName": item.get("hospitalName") or "-",
            "route": route_text,
            "totalDistance": "%.2f" % (item.get("totalDistance") or 0),
            "eta": item.get("eta"),
            "rerouted": "Yes" if item.get("rerouted") else "No",
            "status": item.get("status"),
        })
    return rows


def build_csv(rows):
    buf = io.StringIO()
    writer = csv.writer(buf)
    writer.writerow([label for _, label in EXPORT_COLUMNS])
    for row in rows:
        writer.writerow([row[key] for key, _ in EXPORT_COLUMNS])
    # utf-8-sig so Excel opens it correctly
    return buf.getvalue().encode("utf-8-sig")


def pdf_safe(value):
    # built-in PDF fonts only support latin-1
    return str(value).encode("latin-1", "replace").decode("latin-1")


def build_pdf(rows, items, filters_text):
    from fpdf import FPDF  # pip install fpdf2

    pdf = FPDF(orientation="L", unit="mm", format="A4")
    pdf.set_auto_page_break(auto=True, margin=12)
    pdf.add_page()

    pdf.set_font("Helvetica", "B", 16)
    pdf.cell(0, 9, "RescueRoute - Dispatch Report",
             new_x="LMARGIN", new_y="NEXT")

    pdf.set_font("Helvetica", "", 9)
    pdf.cell(
        0, 5,
        pdf_safe("Generated: %s    Filters: %s" % (
            datetime.now().strftime("%d %b %Y %H:%M"), filters_text
        )),
        new_x="LMARGIN", new_y="NEXT"
    )
    pdf.ln(3)

    total = len(items)
    avg_eta = round(sum(i.get("eta", 0) for i in items) / total, 1) if total else 0
    total_km = round(sum(i.get("totalDistance", 0) for i in items), 2)
    by_sev = {}
    for i in items:
        by_sev[i.get("severity")] = by_sev.get(i.get("severity"), 0) + 1
    sev_text = ", ".join("%s: %d" % (k, v) for k, v in by_sev.items()) or "-"

    pdf.set_font("Helvetica", "B", 10)
    pdf.cell(
        0, 6,
        pdf_safe("Total: %d   Avg ETA: %s min   Total distance: %s km   %s"
                 % (total, avg_eta, total_km, sev_text)),
        new_x="LMARGIN", new_y="NEXT"
    )
    pdf.ln(2)

    # 12 columns (the "Rerouted" column is left out of the PDF to save width)
    cols = [c for c in EXPORT_COLUMNS if c[0] != "rerouted"]
    widths = (8, 24, 28, 9, 30, 18, 26, 32, 52, 16, 12, 18)

    pdf.set_font("Helvetica", "", 7)
    with pdf.table(col_widths=widths, text_align="LEFT",
                   line_height=4.5) as table:
        header = table.row()
        for _, label in cols:
            header.cell(label)
        for row in rows:
            r = table.row()
            for key, _ in cols:
                r.cell(pdf_safe(row[key]))

    return bytes(pdf.output())