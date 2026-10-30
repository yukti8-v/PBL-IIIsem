const $ = id => document.getElementById(id);

const AVG_SPEED_KMH = 30;

const CONDITION_FACILITY = {
    "Cardiac Arrest": "Cardiology",
    "Road Accident": "Trauma",
    "Stroke": "ICU",
    "Burns": "BurnUnit",
    "Breathing Difficulty": "ICU",
    "Pregnancy / Delivery": "Maternity",
    "Child Emergency": "Pediatrics",
    "Other": "Emergency"
};

let locations = [];
let roads = [];
let currentRoute = null;
let activeBlocks = [];

/* ---------- Helpers ---------- */

function nameOf(id) {
    const item = locations.find(x => x.id === Number(id));
    return item ? item.name : `Location ${id}`;
}

function toast(message, type) {
    const t = $("toast");
    t.textContent = message;
    t.className = "toast show";

    if (type === "success") {
        t.classList.add("success");
    }

    setTimeout(() => {
        t.classList.remove("show");
    }, 3000);
}

function showError(message) {
    toast(message);
    $("result").innerHTML = `<p class="error">${message}</p>`;
}

function setLoading(button, isLoading, text) {
    if (button.dataset.original === undefined) {
        button.dataset.original = button.textContent;
    }

    button.disabled = isLoading;

    if (isLoading) {
        button.textContent = text;
    } else {
        button.textContent = button.dataset.original;
    }
}

function etaMinutes(km) {
    return Math.max(1, Math.round((km / AVG_SPEED_KMH) * 60));
}

function setStatus(text, cls) {
    const el = $("statusStat");
    el.textContent = text;
    el.className = cls || "";
}

async function request(url) {
    const response = await fetch(url);
    const data = await response.json();

    if (!response.ok) {
        throw new Error(data.error || "Request failed");
    }

    return data;
}

async function postJson(url, body) {
    const response = await fetch(url, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(body)
    });
    const data = await response.json();

    if (!response.ok) {
        throw new Error(data.error || "Request failed");
    }

    return data;
}

/* ---------- Loading data ---------- */

async function loadLocations() {
    try {
        locations = await request("/api/locations");
        $("locationCount").textContent = locations.length;

        for (const id of ["source", "destination", "roadStart"]) {
            const select = $(id);
            select.innerHTML = "";

            locations.forEach(loc => {
                const option = document.createElement("option");
                option.value = loc.id;
                option.textContent = loc.name;
                select.appendChild(option);
            });
        }

        const clock = locations.find(x => x.name === "Clock Tower");
        const hospital = locations.find(x => x.name === "City Hospital");

        if (clock) {
            $("source").value = clock.id;
            $("roadStart").value = clock.id;
        }
        if (hospital) {
            $("destination").value = hospital.id;
        }

        await loadRoads();
        refreshNeighbors();
        await loadFleet();
        await loadHospitals();
        await loadDispatches();
    } catch (error) {
        showError("Cannot load locations: " + error.message);
    }
}

async function loadRoads() {
    try {
        roads = await request("/api/roads");
    } catch (error) {
        showError("Cannot load roads: " + error.message);
    }
}

function refreshNeighbors() {
    const from = Number($("roadStart").value);
    const select = $("roadEnd");
    select.innerHTML = "";

    roads.forEach(road => {
        let neighbor = null;

        if (road.source === from) {
            neighbor = road.destination;
        } else if (road.destination === from) {
            neighbor = road.source;
        }

        if (neighbor !== null) {
            const option = document.createElement("option");
            option.value = neighbor;
            option.textContent =
                `${nameOf(neighbor)} (${road.distance} km)`;
            select.appendChild(option);
        }
    });
}

/* ---------- Rendering ---------- */

function buildChips(path) {
    const wrap = document.createElement("div");
    wrap.className = "chips";

    path.forEach((id, i) => {
        const chip = document.createElement("span");
        chip.className = "chip";

        if (i === 0) {
            chip.classList.add("start");
        }
        if (i === path.length - 1) {
            chip.classList.add("end");
        }

        chip.textContent = nameOf(id);
        wrap.appendChild(chip);

        if (i < path.length - 1) {
            const arrow = document.createElement("span");
            arrow.className = "arrow";
            arrow.textContent = "→";
            wrap.appendChild(arrow);
        }
    });

    return wrap;
}

function buildMetric(label, value) {
    const box = document.createElement("div");
    box.className = "metric";

    const small = document.createElement("small");
    small.textContent = label;

    const strong = document.createElement("strong");
    strong.textContent = value;

    box.append(small, strong);
    return box;
}

function renderRoute(data, blocked) {
    currentRoute = data;

    if (!data.found) {
        showError("No route available.");
        $("distanceStat").textContent = "N/A";
        $("etaStat").textContent = "N/A";
        setStatus("No Route", "badge-failed");
        return;
    }

    const eta = etaMinutes(data.distance);

    $("distanceStat").textContent = `${data.distance.toFixed(2)} km`;
    $("etaStat").textContent = `${eta} min`;

    if (blocked) {
        setStatus("Rerouted", "badge-rerouted");
    } else {
        setStatus("Normal", "badge-normal");
    }

    $("result").replaceChildren();

    const title = document.createElement("h3");
    if (blocked) {
        title.textContent = "Alternative Route Calculated";
    } else {
        title.textContent = "Shortest Route Found";
    }

    const metrics = document.createElement("div");
    metrics.className = "metrics";
    metrics.append(
        buildMetric("Distance", `${data.distance.toFixed(2)} km`),
        buildMetric("Est. Time", `${eta} min`),
        buildMetric("Stops", String(data.path.length)),
        buildMetric("Hops", String(data.path.length - 1))
    );

    if (blocked &&
        data.originalDistance !== null &&
        data.originalDistance !== undefined) {
        const extra = data.distance - data.originalDistance;
        metrics.append(
            buildMetric(
                "Original",
                `${data.originalDistance.toFixed(2)} km`
            ),
            buildMetric("Detour", `+${extra.toFixed(2)} km`)
        );
    }

    $("result").append(title, buildChips(data.path), metrics);
}

function appendDispatchInfo(record) {
    const box = document.createElement("div");
    box.className = "dispatch-info";

    const heading = document.createElement("h3");
    heading.textContent =
        `Ambulance #${record.ambulanceId} (${record.ambulanceType}) assigned`;
    box.appendChild(heading);

    const leg1 = document.createElement("p");
    leg1.textContent =
        `1) ${nameOf(record.ambulanceFrom)} → ${nameOf(record.source)} ` +
        `(pickup): ${record.pickupDistance.toFixed(2)} km`;
    box.appendChild(leg1);

    let hospitalLabel = nameOf(record.destination);
    if (record.hospitalName) {
        hospitalLabel = record.hospitalName;
    }

    const leg2 = document.createElement("p");
    leg2.textContent =
        `2) ${nameOf(record.source)} → ${hospitalLabel} ` +
        `(hospital): ${record.distance.toFixed(2)} km`;
    box.appendChild(leg2);

    const total = document.createElement("p");
    total.textContent =
        `Total: ${record.totalDistance.toFixed(2)} km, ` +
        `ETA ${record.eta} min`;
    box.appendChild(total);

    if (record.matched === false) {
        const warn = document.createElement("p");
        warn.className = "warn-text";
        warn.textContent =
            `⚠ No free ambulance with ${record.needed} capability. ` +
            `Nearest available ambulance assigned instead.`;
        box.appendChild(warn);
    }

    if (record.hospitalMatched === false) {
        const warn = document.createElement("p");
        warn.className = "warn-text";
        warn.textContent =
            `⚠ ${hospitalLabel} has no ${record.hospitalNeeded} facility. ` +
            `Best available hospital used instead.`;
        box.appendChild(warn);
    }

    $("result").appendChild(box);
}

/* ---------- Route actions ---------- */

async function findRoute() {
    const source = $("source").value;
    const destination = $("destination").value;

    if (source === destination) {
        showError("Choose different source and destination.");
        return;
    }

    const btn = $("findRoute");
    setLoading(btn, true, "Calculating...");

    try {
        const data = await request(
            `/api/route?source=${source}&destination=${destination}`
        );

        clearBlocks();
        renderRoute(data, false);
    } catch (error) {
        showError(error.message);
    } finally {
        setLoading(btn, false);
    }
}

async function nearestHospital() {
    const source = $("source").value;
    const btn = $("nearest");
    setLoading(btn, true, "Searching...");

    try {
    const data = await request(
      `/api/nearest-hospital?source=${source}`
    );

    $("destination").value = data.hospitalId;
    clearBlocks();
    renderRoute(data, false);

    // Member 4: Highlight the best match card and show the badge
    if (typeof setBestMatchHospital === "function") {
      setBestMatchHospital(data.hospitalId);
    }
  } catch (error) {
    showError(error.message);
  } finally {
    setLoading(btn, false);
  }
}

/* ---------- Road closures (one or many roads) ---------- */

function blocksQuery() {
    return activeBlocks.map(pair => `${pair[0]}-${pair[1]}`).join(",");
}

function updateClosureStat() {
    const stat = $("closureStat");

    if (activeBlocks.length === 0) {
        stat.textContent = "None";
        stat.style.color = "";
    } else if (activeBlocks.length === 1) {
        stat.textContent =
            `${nameOf(activeBlocks[0][0])} → ${nameOf(activeBlocks[0][1])}`;
        stat.style.color = "#ef5350";
    } else {
        stat.textContent = `${activeBlocks.length} roads`;
        stat.style.color = "#ef5350";
    }
}

function renderBlockList() {
    const list = $("blockList");
    list.replaceChildren();

    activeBlocks.forEach((pair, index) => {
        const chip = document.createElement("span");
        chip.className = "block-chip";
        chip.textContent = `${nameOf(pair[0])} ↔ ${nameOf(pair[1])}`;

        const remove = document.createElement("button");
        remove.className = "chip-x";
        remove.textContent = "×";
        remove.title = "Remove this closure";
        remove.addEventListener("click", () => {
            removeBlock(index);
        });

        chip.appendChild(remove);
        list.appendChild(chip);
    });
}

function pairInList(a, b) {
    return activeBlocks.some(pair =>
        (pair[0] === a && pair[1] === b) ||
        (pair[0] === b && pair[1] === a)
    );
}

function addBlock() {
    const a = Number($("roadStart").value);
    const b = Number($("roadEnd").value);

    if ($("roadEnd").value === "" || a === b) {
        showError("Choose a valid road to block.");
        return false;
    }

    if (pairInList(a, b)) {
        toast("That road is already in the closure list.");
        return false;
    }

    activeBlocks.push([a, b]);
    renderBlockList();
    updateClosureStat();
    return true;
}

function addBlockClick() {
    if (addBlock() === true) {
        toast("Road added to closure. Add more or recalculate.", "success");
    }
}

function removeBlock(index) {
    activeBlocks.splice(index, 1);
    renderBlockList();
    updateClosureStat();
}

function clearBlocks() {
    activeBlocks = [];
    renderBlockList();
    updateClosureStat();
}

async function blockAndReroute() {
    const source = $("source").value;
    const destination = $("destination").value;

    if (source === destination) {
        showError("Choose different source and destination.");
        return;
    }

    if (activeBlocks.length === 0) {
        if (addBlock() === false) {
            return;
        }
    }

    const btn = $("reroute");
    setLoading(btn, true, "Recalculating...");

    try {
        const data = await request(
            `/api/route?source=${source}&destination=${destination}` +
            `&blocks=${blocksQuery()}`
        );

        renderRoute(data, true);
    } catch (error) {
        showError(error.message);
    } finally {
        setLoading(btn, false);
    }
}

function resetClosure() {
    clearBlocks();
    findRoute();
}

/* ---------- Ambulance fleet ---------- */

function renderFleet(fleet) {
    const grid = $("fleetGrid");
    grid.replaceChildren();

    let freeCount = 0;

    fleet.forEach(amb => {
        if (amb.status === "AVAILABLE") {
            freeCount = freeCount + 1;
        }

        const card = document.createElement("div");
        card.className = "amb";

        const title = document.createElement("h4");
        title.textContent = `Ambulance #${amb.id}`;
        card.appendChild(title);

        const sub = document.createElement("small");
        sub.textContent = `${amb.type} · at ${nameOf(amb.location)}`;
        card.appendChild(sub);

        const badge = document.createElement("span");
        badge.className = `amb-status st-${amb.status}`;
        badge.textContent = amb.status;
        card.appendChild(badge);

        const caps = document.createElement("div");
        caps.className = "caps";
        amb.capabilities.forEach(cap => {
            const c = document.createElement("span");
            c.className = "cap";
            c.textContent = cap;
            caps.appendChild(c);
        });
        card.appendChild(caps);

        grid.appendChild(card);
    });

    $("fleetStat").textContent = `${freeCount} / ${fleet.length}`;
}

async function loadFleet() {
    try {
        const fleet = await request("/api/ambulances");
        renderFleet(fleet);
    } catch (error) {
        toast("Cannot load fleet: " + error.message);
    }
}

/* ---------- Hospitals ---------- */

function highlightHospitals() {
    const need = CONDITION_FACILITY[$("condition").value];
    const cards = document.querySelectorAll("#hospitalGrid .amb");

    cards.forEach(card => {
        const facilities = card.dataset.facilities.split(";");
        const open = card.dataset.status === "AVAILABLE";
        const match = open && facilities.includes(need);
        card.classList.toggle("match", match);
    });
}

function renderHospitals(hospitals) {
  const grid = $("hospitalGrid");
  grid.replaceChildren();

  let openCount = 0;

  hospitals.forEach(hospital => {
    if (hospital.status === "AVAILABLE") {
      openCount = openCount + 1;
    }

    const card = document.createElement("div");
    card.className = "amb hospital-card";
    card.dataset.status = hospital.status;
    card.dataset.hospitalId = hospital.id;
    card.dataset.locationId = hospital.location;
    card.dataset.facilities = (hospital.facilities || []).join(";");

    // "Best match" badge placeholder
    const bestPill = document.createElement("span");
    bestPill.className = "best-match-pill";
    bestPill.style.display = "none";
    bestPill.textContent = "★ Best Match";
    card.appendChild(bestPill);

    const title = document.createElement("h4");
    title.textContent = hospital.name;
    card.appendChild(title);

    const sub = document.createElement("small");
    sub.textContent = `at ${nameOf(hospital.location)}`;
    card.appendChild(sub);

    const badge = document.createElement("span");
    badge.className = `amb-status st-${hospital.status}`;
    badge.textContent = hospital.status;
    card.appendChild(badge);

    // Bed & ICU calculations
    const totalBeds = hospital.total_beds || (hospital.beds ? hospital.beds + 10 : 40);
    const availBeds = hospital.beds ?? 0;
    const bedPct = Math.min(100, Math.round((availBeds / totalBeds) * 100));

    const totalIcu = hospital.total_icu || (hospital.icu ? hospital.icu + 4 : 10);
    const availIcu = hospital.icu ?? 0;
    const icuPct = Math.min(100, Math.round((availIcu / totalIcu) * 100));

    // Progress Bars Container
    const barsContainer = document.createElement("div");
    barsContainer.style.cssText = "display: flex; flex-direction: column; gap: 8px; margin: 10px 0;";
    barsContainer.innerHTML = `
      <div class="progress-group">
        <div class="progress-header" style="display:flex; justify-content:space-between; font-size:11px; color:#aaa;">
          <span>Beds</span>
          <span><strong>${availBeds}</strong>/${totalBeds} (${bedPct}%)</span>
        </div>
        <div class="progress-bg" style="background:#2b2b30; height:6px; border-radius:3px; overflow:hidden;">
          <div class="progress-bar-fill fill-bed" style="width:${bedPct}%; height:100%; background:#38bdf8;"></div>
        </div>
      </div>
      <div class="progress-group">
        <div class="progress-header" style="display:flex; justify-content:space-between; font-size:11px; color:#aaa;">
          <span>ICU</span>
          <span><strong>${availIcu}</strong>/${totalIcu} (${icuPct}%)</span>
        </div>
        <div class="progress-bg" style="background:#2b2b30; height:6px; border-radius:3px; overflow:hidden;">
          <div class="progress-bar-fill fill-icu" style="width:${icuPct}%; height:100%; background:#f43f5e;"></div>
        </div>
      </div>
    `;
    card.appendChild(barsContainer);

    const caps = document.createElement("div");
    caps.className = "caps";
    (hospital.facilities || []).forEach(facility => {
      const c = document.createElement("span");
      c.className = "cap";
      c.textContent = facility;
      caps.appendChild(c);
    });
    card.appendChild(caps);

    grid.appendChild(card);
  });

  $("hospitalStat").textContent = `${openCount} / ${hospitals.length}`;
  highlightHospitals();
}
function setBestMatchHospital(hospitalIdOrLocation) {
  const cards = document.querySelectorAll("#hospitalGrid .hospital-card");
  cards.forEach(card => {
    const isMatch = (card.dataset.hospitalId == hospitalIdOrLocation || card.dataset.locationId == hospitalIdOrLocation);
    card.classList.toggle("is-best-match", isMatch);
    
    const pill = card.querySelector(".best-match-pill");
    if (pill) {
      pill.style.display = isMatch ? "inline-block" : "none";
    }
  });
}

async function loadHospitals() {
    try {
        const hospitals = await request("/api/hospitals");
        renderHospitals(hospitals);
    } catch (error) {
        toast("Cannot load hospitals: " + error.message);
    }
}

/* ---------- Dispatch ---------- */

function makeCell(text) {
    const td = document.createElement("td");
    td.textContent = text;
    return td;
}

async function completeDispatch(id) {
    try {
        await postJson("/api/dispatches/complete", { id: id });
        await loadDispatches();
        await loadFleet();
        toast("Dispatch completed. Ambulance is free again.", "success");
    } catch (error) {
        toast(error.message);
    }
}

async function loadDispatches() {
    try {
        const items = await request("/api/dispatches");
        const body = $("logBody");
        body.replaceChildren();

        if (items.length === 0) {
            const tr = document.createElement("tr");
            const td = document.createElement("td");
            td.colSpan = 10;
            td.className = "empty";
            td.textContent = "No dispatches yet.";
            tr.appendChild(td);
            body.appendChild(tr);
            return;
        }

        items.forEach(item => {
            const tr = document.createElement("tr");

            tr.appendChild(makeCell(item.id));
            tr.appendChild(makeCell(item.time));
            tr.appendChild(makeCell(`${item.name} (${item.age})`));
            tr.appendChild(makeCell(item.condition));

            const sevCell = document.createElement("td");
            const sev = document.createElement("span");
            sev.className = `sev sev-${item.severity}`;
            sev.textContent = item.severity;
            sevCell.appendChild(sev);
            tr.appendChild(sevCell);

            let ambText = "—";
            if (item.ambulanceId !== undefined) {
                ambText = `#${item.ambulanceId} ${item.ambulanceType}`;
            }
            tr.appendChild(makeCell(ambText));

            let destText = nameOf(item.destination);
            if (item.hospitalName) {
                destText = item.hospitalName;
            }

            let routeText = `${nameOf(item.source)} → ${destText}`;
            if (item.rerouted) {
                routeText = routeText + " (rerouted)";
            }
            tr.appendChild(makeCell(routeText));

            let totalKm = item.distance;
            if (item.totalDistance !== undefined) {
                totalKm = item.totalDistance;
            }
            tr.appendChild(makeCell(`${totalKm.toFixed(2)} km`));
            tr.appendChild(makeCell(`${item.eta} min`));

            const statusCell = document.createElement("td");
            if (item.status === "En Route") {
                const doneBtn = document.createElement("button");
                doneBtn.className = "small";
                doneBtn.textContent = "Complete";
                doneBtn.addEventListener("click", () => {
                    completeDispatch(item.id);
                });
                statusCell.appendChild(doneBtn);
            } else {
                const badge = document.createElement("span");
                badge.className = "amb-status st-AVAILABLE";
                badge.textContent = "Completed";
                statusCell.appendChild(badge);
            }
            tr.appendChild(statusCell);

            body.appendChild(tr);
        });
    } catch (error) {
        toast("Cannot load log: " + error.message);
    }
}

async function dispatchAmbulance() {
    const name = $("patientName").value.trim();
    const ageText = $("patientAge").value;

    if (name === "") {
        showError("Enter the patient name.");
        return;
    }
    if (ageText === "") {
        showError("Enter the patient age.");
        return;
    }

    const payload = {
        name: name,
        age: Number(ageText),
        condition: $("condition").value,
        severity: $("severity").value,
        source: Number($("source").value),
        destination: Number($("destination").value),
        autoHospital: $("autoHospital").checked
    };

    payload.blocks = activeBlocks;

    const btn = $("dispatchBtn");
    setLoading(btn, true, "Dispatching...");

    try {
        const record = await postJson("/api/dispatch", payload);

        $("destination").value = record.destination;

        renderRoute(
            {
                found: true,
                distance: record.distance,
                originalDistance: null,
                path: record.path
            },
            record.rerouted
        );

        $("distanceStat").textContent =
            `${record.totalDistance.toFixed(2)} km`;
        $("etaStat").textContent = `${record.eta} min`;

        appendDispatchInfo(record);

        $("patientName").value = "";
        $("patientAge").value = "";

        await loadDispatches();
        await loadFleet();
        toast(
            `Ambulance #${record.ambulanceId} dispatched. ` +
            `ETA ${record.eta} min.`,
            "success"
        );
    } catch (error) {
        showError(error.message);
    } finally {
        setLoading(btn, false);
    }
}

async function clearLog() {
    try {
        await postJson("/api/dispatches/clear", {});
        await loadDispatches();
        await loadFleet();
        toast("Dispatch log cleared. All ambulances freed.", "success");
    } catch (error) {
        toast(error.message);
    }
}

/* ---------- Sidebar active highlight ---------- */

function setupSidebar() {
    const sections = document.querySelectorAll("main section");
    const links = document.querySelectorAll(".sidebar nav a");

    const observer = new IntersectionObserver(entries => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                links.forEach(link => {
                    const match =
                        link.getAttribute("href") === `#${entry.target.id}`;
                    link.classList.toggle("active", match);
                });
            }
        });
    }, { rootMargin: "-40% 0px -55% 0px" });

    sections.forEach(section => observer.observe(section));
}

/* ---------- Events ---------- */

function on(id, eventName, handler) {
    const element = $(id);

    if (element === null) {
        console.warn("Missing element in index.html: " + id);
        return;
    }

    element.addEventListener(eventName, handler);
}

on("findRoute", "click", findRoute);
on("nearest", "click", nearestHospital);
on("reroute", "click", blockAndReroute);
on("reset", "click", resetClosure);
on("addBlock", "click", addBlockClick);
on("roadStart", "change", refreshNeighbors);
on("dispatchBtn", "click", dispatchAmbulance);
on("clearLog", "click", clearLog);
on("condition", "change", highlightHospitals);

setupSidebar();
loadLocations();