from pathlib import Path
p = Path("gui/server.py")
s = p.read_text(encoding="utf-8-sig")
s = s.replace(
    "from export import filter_dispatches, export_rows, build_csv, build_pdf",
    "from export import filter_dispatches, export_rows, build_csv, build_pdf\n"
    "from member2_backend import (get_queue, add_to_queue, process_queue, "
    "create_disaster, get_hospital_resources, update_hospital_resources, "
    "reserve_resources, release_resources)"
)
s = s.replace(
    '            if parsed.path == "/api/dispatches/clear":',
    '            if parsed.path == "/api/queue":\n'
    '                patient = add_to_queue(body)\n'
    '                assigned = process_queue()\n'
    '                self.send_json({"patient": patient, "dispatches": assigned}, 201)\n'
    '                return\n\n'
    '            if parsed.path == "/api/disaster":\n'
    '                self.send_json(create_disaster(body), 201)\n'
    '                return\n\n'
    '            if parsed.path == "/api/hospital-resources":\n'
    '                self.send_json(update_hospital_resources(body))\n'
    '                return\n\n'
    '            if parsed.path == "/api/dispatches/clear":'
)
s = s.replace(
    '                self.send_json(complete_dispatch(int(body.get("id"))))',
    '                completed = complete_dispatch(int(body.get("id")))\n'
    '                release_resources(completed)\n'
    '                assigned = process_queue()\n'
    '                self.send_json({"completed": completed, "newDispatches": assigned})'
)
s = s.replace(
    '                self.send_json(create_dispatch(body), 201)',
    '                dispatch = create_dispatch(body)\n'
    '                reserve_resources(dispatch)\n'
    '                self.send_json(dispatch, 201)'
)
s = s.replace(
    '        if path in SIMPLE_GET:',
    '        if path == "/api/queue":\n'
    '            try:\n'
    '                self.send_json(get_queue())\n'
    '            except Exception as e:\n'
    '                self.send_json({"error": str(e)}, 500)\n'
    '            return\n'
    '        if path == "/api/hospital-resources":\n'
    '            try:\n'
    '                self.send_json(get_hospital_resources())\n'
    '            except Exception as e:\n'
    '                self.send_json({"error": str(e)}, 500)\n'
    '            return\n\n'
    '        if path in SIMPLE_GET:'
)
p.write_text(s, encoding="utf-8")
print("Member 2 backend files and API routes added.")
