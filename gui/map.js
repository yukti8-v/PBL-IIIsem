/* RescueRoute interactive map. Requires Leaflet. */
(function () {
    "use strict";

    const state = {
        map: null,
        locations: [],
        coordinates: [],
        roads: [],
        hospitals: [],
        fleet: [],
        locationById: new Map(),
        coordinateById: new Map(),
        locationMarkers: new Map(),
        hospitalMarkers: new Map(),
        ambulanceMarkers: new Map(),
        routeLayer: null,
        blockedLayer: null,
        roadLayer: null,
        roadsLayerGroup: null,
        allBounds: null,
        fallbackCoordinates: false
    };

    // Load Leaflet CSS
    function addCss() {
        if (!document.querySelector('link[data-rescueroute-leaflet]')) {
            const link = document.createElement('link');
            link.rel = 'stylesheet';
            link.href = 'https://unpkg.com/leaflet@1.9.4/dist/leaflet.css';
            link.dataset.rescuerouteLeaflet = 'true';
            document.head.appendChild(link);
        }
    }

    // Load Leaflet JavaScript
    function loadLeaflet() {
        addCss();

        if (window.L) return Promise.resolve();

        if (window.__rescueLeafletPromise) {
            return window.__rescueLeafletPromise;
        }

        window.__rescueLeafletPromise = new Promise((resolve, reject) => {
            const script = document.createElement('script');

            script.src =
                'https://unpkg.com/leaflet@1.9.4/dist/leaflet.js';

            script.onload = resolve;

            script.onerror = () =>
                reject(
                    new Error(
                        'Could not load Leaflet. Check your internet connection.'
                    )
                );

            document.head.appendChild(script);
        });

        return window.__rescueLeafletPromise;
    }

    // Create the map panel if it doesn't exist
    function ensurePanel() {
        let section = document.getElementById('liveMap');

        if (!section) {
            section = document.createElement('section');
            section.id = 'liveMap';
            section.className = 'panel rescueroute-map-panel';

            section.innerHTML = `
                <div class="section-heading">
                    <div>
                        <p class="eyebrow">LIVE OPERATIONS</p>
                        <h2>Live Map</h2>
                    </div>

                    <div class="map-controls">
                        <label for="animationSpeed">
                            Animation speed
                        </label>

                        <select id="animationSpeed">
                            <option value="1">1×</option>
                            <option value="5">5×</option>
                            <option value="10">10×</option>
                        </select>

                        <button
                            type="button"
                            id="recenterMap"
                            class="small"
                        >
                            Recenter Map
                        </button>
                    </div>
                </div>

                <p id="mapMessage" class="map-message">
                    Loading locations…
                </p>

                <div
                    id="map"
                    role="application"
                    aria-label="RescueRoute live map"
                ></div>

                <p class="map-legend">
                    <span class="legend-dot legend-hospital"></span>
                    Hospital

                    <span class="legend-dot legend-available"></span>
                    Available ambulance

                    <span class="legend-dot legend-busy"></span>
                    Busy ambulance

                    <span class="legend-line"></span>
                    Road / Route

                    <span class="legend-line legend-blocked"></span>
                    Closed road
                </p>

                <p
                    id="ambulanceEta"
                    class="map-message"
                    aria-live="polite"
                ></p>
            `;

            const main = document.querySelector('main');

            if (main) {
                main.insertBefore(section, main.firstChild);
            } else {
                document.body.appendChild(section);
            }
        } else if (!document.getElementById('map')) {
            const map = document.createElement('div');
            map.id = 'map';
            section.appendChild(map);
        }

        // Map styling
        if (!document.getElementById('rescueroute-map-style')) {
            const style = document.createElement('style');

            style.id = 'rescueroute-map-style';

            style.textContent = `
                #liveMap {
                    margin: 18px 0;
                }

                #map {
                    height: 480px;
                    min-height: 320px;
                    width: 100%;
                    border-radius: 14px;
                    z-index: 1;
                    background: #182127;
                }

                .map-controls {
                    display: flex;
                    align-items: center;
                    gap: 8px;
                    flex-wrap: wrap;
                }

                .map-controls select {
                    padding: 7px 10px;
                    border-radius: 8px;
                }

                .map-message {
                    color: #b9c2c9;
                    font-size: .9rem;
                    margin: 8px 0;
                }

                .map-legend {
                    display: flex;
                    align-items: center;
                    gap: 12px;
                    flex-wrap: wrap;
                    color: #b9c2c9;
                    font-size: .8rem;
                }

                .legend-dot {
                    display: inline-block;
                    width: 10px;
                    height: 10px;
                    border-radius: 50%;
                    margin-right: 4px;
                }

                .legend-hospital {
                    background: #f0b34b;
                }

                .legend-available {
                    background: #42c995;
                }

                .legend-busy {
                    background: #ef6464;
                }

                .legend-line {
                    display: inline-block;
                    width: 20px;
                    border-top: 3px solid #55c7e8;
                    margin-right: -8px;
                }

                .legend-blocked {
                    border-color: #ef6464;
                    border-top-style: dashed;
                }

                .rr-marker {
                    display: flex;
                    align-items: center;
                    justify-content: center;
                    width: 28px;
                    height: 28px;
                    border: 2px solid #fff;
                    border-radius: 50%;
                    box-shadow: 0 2px 8px #0008;
                    font-size: 15px;
                }

                .rr-ambulance {
                    background: #1688d4;
                    border-radius: 8px;
                    width: 32px;
                    height: 26px;
                }

                .rr-ambulance.busy {
                    background: #d84d4d;
                }

                .rr-hospital {
                    background: #e9a72c;
                    color: #1b2024;
                }

                .rr-location {
                    background: #344955;
                }

                .rescueroute-map-panel .section-heading {
                    display: flex;
                    justify-content: space-between;
                    align-items: center;
                    gap: 12px;
                    flex-wrap: wrap;
                }

                .road-popup button {
                    margin-top: 6px;
                }
            `;

            document.head.appendChild(style);
        }
    }

    // Return the first available property
    function firstDefined(obj, keys) {
        for (const key of keys) {
            if (
                obj &&
                obj[key] !== undefined &&
                obj[key] !== null &&
                obj[key] !== ''
            ) {
                return obj[key];
            }
        }

        return null;
    }

    function getId(obj) {
        return Number(
            firstDefined(obj, [
                'id',
                'locationId',
                'location_id',
                'node',
                'nodeId'
            ])
        );
    }

    // Read coordinates returned by /api/coordinates
    function parseCoordinate(row) {
        const id = getId(row);

        const lat = Number(
            firstDefined(row, [
                'lat',
                'latitude',
                'Latitude',
                'LAT',
                'y'
            ])
        );

        const lng = Number(
            firstDefined(row, [
                'lng',
                'lon',
                'long',
                'longitude',
                'Longitude',
                'LON',
                'LONG',
                'x'
            ])
        );

        if (
            !Number.isFinite(id) ||
            !Number.isFinite(lat) ||
            !Number.isFinite(lng)
        ) {
            return null;
        }

        if (Math.abs(lat) > 90 || Math.abs(lng) > 180) {
            return null;
        }

        return { id, lat, lng };
    }

    function coordinateFor(id) {
        const key = Number(id);

        if (state.coordinateById.has(key)) {
            return state.coordinateById.get(key);
        }

        return null;
    }

    // Create icons for map markers
    function icon(className, symbol) {
        return window.L.divIcon({
            className: '',
            html: `<div class="rr-marker ${className}">${symbol}</div>`,
            iconSize: [32, 32],
            iconAnchor: [16, 16]
        });
    }

    function message(text) {
        const el = document.getElementById('mapMessage');

        if (el) {
            el.textContent = text;
        }
    }

    function idName(id) {
        const item = state.locationById.get(Number(id));

        return item ? item.name : `Location ${id}`;
    }

    function edgeKey(a, b) {
        return [Number(a), Number(b)]
            .sort((x, y) => x - y)
            .join('-');
    }

    // Get the coordinates for a road
    function roadCoordinates(road) {
        const a = firstDefined(road, [
            'source',
            'from',
            'start',
            'u'
        ]);

        const b = firstDefined(road, [
            'destination',
            'to',
            'end',
            'v'
        ]);

        const ca = coordinateFor(a);
        const cb = coordinateFor(b);

        if (!ca || !cb) {
            return null;
        }

        return {
            a: Number(a),
            b: Number(b),
            points: [
                [ca.lat, ca.lng],
                [cb.lat, cb.lng]
            ],
            road
        };
    }

    // Draw the existing roads from roads.csv
    function renderRoads() {
        if (!state.map || !window.L) return;

        if (state.roadsLayerGroup) {
            state.roadsLayerGroup.remove();
        }

        state.roadsLayerGroup = window.L.layerGroup().addTo(state.map);

        state.roads.forEach(road => {
            const edge = roadCoordinates(road);

            if (!edge) return;

            const line = window.L.polyline(edge.points, {
    color: '#87939b',
    weight: 2,
    opacity: 0.45
}).addTo(state.roadsLayerGroup);

            line.bindPopup(`
                <div class="road-popup">
                    <strong>
                        ${idName(edge.a)} ↔ ${idName(edge.b)}
                    </strong>
                    <br>
                    ${road.distance ?? ''} km
                    <br>
                    <button
                        type="button"
                        data-a="${edge.a}"
                        data-b="${edge.b}"
                    >
                        Block / reopen road
                    </button>
                </div>
            `);

            line.on('popupopen', event => {
                const btn = event.popup
                    .getElement()
                    .querySelector('button[data-a]');

                if (btn) {
                    btn.addEventListener('click', () => {
                        document.dispatchEvent(
                            new CustomEvent('rescueroute:block-road', {
                                detail: {
                                    source: Number(btn.dataset.a),
                                    destination: Number(btn.dataset.b)
                                }
                            })
                        );

                        state.map.closePopup();
                    });
                }
            });
        });
    }

    // Display all locations
    function renderLocations() {
        if (!state.map || !window.L) return;

        state.locationMarkers.forEach(marker => marker.remove());
        state.locationMarkers.clear();

        state.locationById = new Map(
            state.locations.map(item => [Number(item.id), item])
        );

        state.locations.forEach(item => {
            const c = coordinateFor(item.id);

            if (!c) return;

            const marker = window.L.marker(
                [c.lat, c.lng],
                { icon: icon('rr-location', '•') }
            )
                .bindTooltip(
                    item.name || `Location ${item.id}`,
                    { direction: 'top' }
                )
                .addTo(state.map);

            state.locationMarkers.set(Number(item.id), marker);
        });
    }

    // Display hospitals
    function renderHospitals() {
        if (!state.map || !window.L) return;

        state.hospitalMarkers.forEach(marker => marker.remove());
        state.hospitalMarkers.clear();

        state.hospitals.forEach(hospital => {
            const c = coordinateFor(
                firstDefined(hospital, [
                    'location',
                    'locationId',
                    'id'
                ])
            );

            if (!c) return;

            const available =
                String(hospital.status || 'AVAILABLE').toUpperCase()
                === 'AVAILABLE';

            const marker = window.L.marker(
                [c.lat, c.lng],
                { icon: icon('rr-hospital', '✚') }
            )
                .bindPopup(`
                    <strong>${hospital.name || 'Hospital'}</strong>
                    <br>
                    Status: ${hospital.status || 'Unknown'}
                    <br>
                    ${(hospital.facilities || []).join(', ')}
                `)
                .addTo(state.map);

            state.hospitalMarkers.set(
                hospital.id ?? hospital.location,
                marker
            );

            if (!available) {
                marker.setOpacity(0.6);
            }
        });
    }

    // Ambulance marker appearance
    function ambulanceIcon(amb) {
        const busy =
            String(amb.status || '').toUpperCase() !== 'AVAILABLE';

        const symbol = /bike|motor/i.test(amb.type || '')
            ? '🏍️'
            : '🚑';

        return icon(
            `rr-ambulance${busy ? ' busy' : ''}`,
            symbol
        );
    }

    // Update ambulance positions
    function updateAmbulances(fleet) {
        state.fleet = Array.isArray(fleet) ? fleet : [];

        if (!state.map || !window.L) return;

        const present = new Set();

        state.fleet.forEach(amb => {
            const id = String(amb.id);

            present.add(id);

            const c = coordinateFor(amb.location);

            if (!c) return;

            let marker = state.ambulanceMarkers.get(id);

            const popup = `
                <strong>Ambulance #${amb.id}</strong>
                <br>
                Type: ${amb.type || 'Standard'}
                <br>
                Status: ${amb.status || 'Unknown'}
                <br>
                At: ${idName(amb.location)}
            `;

            if (!marker) {
                marker = window.L.marker(
                    [c.lat, c.lng],
                    {
                        icon: ambulanceIcon(amb),
                        zIndexOffset: 1000
                    }
                ).addTo(state.map);

                state.ambulanceMarkers.set(id, marker);
            } else {
                marker.setLatLng([c.lat, c.lng]);
                marker.setIcon(ambulanceIcon(amb));
            }

            marker.bindPopup(popup);
        });

        state.ambulanceMarkers.forEach((marker, id) => {
            if (!present.has(id)) {
                marker.remove();
                state.ambulanceMarkers.delete(id);
            }
        });
    }

    function markerForAmbulance(id) {
        return state.ambulanceMarkers.get(String(id)) || null;
    }

    // Draw a calculated route
    function drawRoute(path, options = {}) {
        if (!state.map || !window.L || !Array.isArray(path)) {
            return;
        }

        if (state.routeLayer) {
            state.routeLayer.remove();
        }

        const points = path
            .map(coordinateFor)
            .filter(Boolean)
            .map(c => [c.lat, c.lng]);

        if (points.length < 1) return;

        state.routeLayer = window.L.polyline(points, {
            color: options.color || '#55c7e8',
            weight: 6,
            opacity: 0.95,
            lineJoin: 'round'
        }).addTo(state.map);

        if (points.length > 1) {
            state.map.fitBounds(
                state.routeLayer.getBounds(),
                {
                    padding: [30, 30],
                    maxZoom: 15
                }
            );
        } else {
            state.map.setView(points[0], 14);
        }
    }

    function clearRoute() {
        if (state.routeLayer) {
            state.routeLayer.remove();
            state.routeLayer = null;
        }
    }

    // Highlight blocked roads
    function setBlockedRoads(blocks) {
        if (!state.map || !window.L) return;

        if (state.blockedLayer) {
            state.blockedLayer.remove();
        }

        state.blockedLayer = window.L.layerGroup().addTo(state.map);

        (blocks || []).forEach(pair => {
            const edge = roadCoordinates({
                source: pair[0],
                destination: pair[1]
            });

            if (edge) {
                window.L.polyline(edge.points, {
                    color: '#ef6464',
                    weight: 7,
                    dashArray: '8 8',
                    opacity: 1
                }).addTo(state.blockedLayer);
            }
        });
    }

    // Recenter map
    function recenter() {
        if (!state.map) return;

        if (state.allBounds && state.allBounds.isValid()) {
            state.map.fitBounds(
                state.allBounds,
                { padding: [30, 30] }
            );
        } else {
            state.map.setView([20.5937, 78.9629], 5);
        }
    }

    function fitMapToLocations() {
        const points = [...state.coordinateById.values()]
            .map(c => [c.lat, c.lng]);

        if (!state.map || points.length === 0) return;

        state.allBounds = window.L.latLngBounds(points);

        recenter();
    }

    // Fetch JSON from the backend
    async function json(url) {
        const response = await fetch(url);

        if (!response.ok) {
            throw new Error(`${url} returned ${response.status}`);
        }

        return response.json();
    }

    // Initialize the map
    async function init() {
        ensurePanel();

        try {
            await loadLeaflet();

            state.map = window.L.map('map', {
                preferCanvas: true
            }).setView([20.5937, 78.9629], 5);

            // FIXED: OpenStreetMap tiles instead of CARTO API-key tiles
            window.L.tileLayer(
                'https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png',
                {
                    attribution: '&copy; OpenStreetMap contributors',
                    maxZoom: 19
                }
            ).addTo(state.map);

            const [
                locations,
                coordinates,
                roads,
                hospitals,
                fleet
            ] = await Promise.all([
                json('/api/locations'),
                json('/api/coordinates'),
                json('/api/roads'),
                json('/api/hospitals'),
                json('/api/ambulances')
            ]);

            state.locations = Array.isArray(locations)
                ? locations
                : [];

            state.coordinates = Array.isArray(coordinates)
                ? coordinates
                : [];

            state.roads = Array.isArray(roads)
                ? roads
                : [];

            state.hospitals = Array.isArray(hospitals)
                ? hospitals
                : [];

            state.locationById = new Map(
                state.locations.map(item => [Number(item.id), item])
            );

            state.coordinateById.clear();

            state.coordinates.forEach(row => {
                const c = parseCoordinate(row);

                if (c) {
                    state.coordinateById.set(c.id, c);
                }
            });

            if (state.coordinateById.size === 0) {
                message(
                    'No valid coordinates found. Check coordinates.csv ' +
                    'and its location_id,lat,lng headers.'
                );
            } else {
                renderRoads();
                renderLocations();
                renderHospitals();
                updateAmbulances(fleet);
                fitMapToLocations();

                message(
                    `Map ready: ${state.coordinateById.size} locations plotted. ` +
                    'Click a road to add/remove a closure.'
                );
            }

            const recenterBtn = document.getElementById('recenterMap');

            if (recenterBtn) {
                recenterBtn.addEventListener('click', recenter);
            }

            setTimeout(() => state.map.invalidateSize(), 100);

        } catch (error) {
            message(`Map could not start: ${error.message}`);
            console.error('RescueRoute map error:', error);
        }
    }

    // Expose functions for script.js and animation.js
    window.RescueMap = {
        coordinateFor,
        drawRoute,
        clearRoute,
        setBlockedRoads,
        updateAmbulances,
        markerForAmbulance,
        recenter,
        fitMapToLocations,

        setLocations(items) {
            state.locations = items || [];

            state.locationById = new Map(
                state.locations.map(x => [Number(x.id), x])
            );

            renderLocations();
            renderRoads();
        },

        setRoads(items) {
            state.roads = items || [];
            renderRoads();
        },

        setHospitals(items) {
            state.hospitals = items || [];
            renderHospitals();
        }
    };

    if (document.readyState === 'loading') {
        document.addEventListener(
            'DOMContentLoaded',
            init,
            { once: true }
        );
    } else {
        init();
    }
})();