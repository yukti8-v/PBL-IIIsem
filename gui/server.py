from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
from datetime import datetime
import json

from config import GUI
from routing import (
    read_locations, read_roads, read_coordinates,
    normalize_blocks, parse_blocks_text, run_route
)
from hospitals import read_hospitals, find_best_hospital
from dispatch_engine import (
    fleet_with_status, read_dispatches,
    create_dispatch, complete_dispatch, clear_dispatches
)
from export import filter_dispatches, export_rows, build_csv, build_pdf

# Simple GET endpoints that just return a function's result as JSON
SIMPLE_GET = {
    "/api/locations": read_locations,
    "/api/roads": read_roads,
    "/api/coordinates": read_coordinates,
    "/api/hospitals": read_hospitals,
    "/api/ambulances": fleet_with_status,
}


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(GUI), **kwargs)

    # ---------- helpers ----------

    def send_json(self, data, status=200):
        body = json.dumps(data).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def send_download(self, body, mimetype, filename):
        self.send_response(200)
        self.send_header("Content-Type", mimetype)
        self.send_header(
            "Content-Disposition", 'attachment; filename="%s"' % filename
        )
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def handle_export(self, parsed):
        params = parse_qs(parsed.query)
        fmt = parsed.path.rsplit("/", 1)[-1]

        items = filter_dispatches(read_dispatches(), params)
        items.reverse()  # newest first, like the dashboard

        if len(items) == 0:
            self.send_json({"error": "No dispatches to export"}, 404)
            return

        rows = export_rows(items)
        stamp = datetime.now().strftime("%Y%m%d_%H%M")

        if fmt == "csv":
            self.send_download(
                build_csv(rows), "text/csv; charset=utf-8",
                "dispatch_log_%s.csv" % stamp
            )
            return

        filters = ", ".join(
            "%s=%s" % (k, v[0]) for k, v in params.items() if v and v[0]
        ) or "none"

        try:
            pdf_bytes = build_pdf(rows, items, filters)
        except ImportError:
            self.send_json(
                {"error": "PDF export needs fpdf2: pip install fpdf2"}, 501
            )
            return

        self.send_download(
            pdf_bytes, "application/pdf", "dispatch_log_%s.pdf" % stamp
        )

    # ---------- POST ----------

    def do_POST(self):
        parsed = urlparse(self.path)

        try:
            length = int(self.headers.get("Content-Length", "0"))
            raw = self.rfile.read(length) if length > 0 else b"{}"
            body = json.loads(raw)

            if parsed.path == "/api/dispatches/clear":
                clear_dispatches()
                self.send_json({"cleared": True})
                return

            if parsed.path == "/api/dispatches/complete":
                self.send_json(complete_dispatch(int(body.get("id"))))
                return

            if parsed.path == "/api/dispatch":
                self.send_json(create_dispatch(body), 201)
                return

            self.send_json({"error": "Not found"}, 404)

        except (ValueError, TypeError, KeyError) as e:
            self.send_json({"error": str(e)}, 400)
        except Exception as e:
            self.send_json({"error": str(e)}, 500)

    # ---------- GET ----------

    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path

        if path in ("/api/export/csv", "/api/export/pdf"):
            try:
                self.handle_export(parsed)
            except Exception as e:
                self.send_json({"error": str(e)}, 500)
            return

        if path in SIMPLE_GET:
            try:
                self.send_json(SIMPLE_GET[path]())
            except Exception as e:
                self.send_json({"error": str(e)}, 500)
            return

        if path == "/api/dispatches":
            try:
                items = read_dispatches()
                items.reverse()
                self.send_json(items)
            except Exception as e:
                self.send_json({"error": str(e)}, 500)
            return

        if path == "/api/nearest-hospital":
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

        if path == "/api/route":
            try:
                params = parse_qs(parsed.query)

                source = int(params["source"][0])
                destination = int(params["destination"][0])

                valid_ids = {x["id"] for x in read_locations()}
                if source not in valid_ids or destination not in valid_ids:
                    raise ValueError("Invalid source or destination")

                blocks = []
                if params.get("blocks"):
                    blocks = normalize_blocks(
                        parse_blocks_text(params["blocks"][0]), valid_ids
                    )

                self.send_json(run_route(source, destination, blocks))

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