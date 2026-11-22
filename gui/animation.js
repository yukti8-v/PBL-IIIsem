/* =========================================================
   RescueRoute — Ambulance Animation
   ========================================================= */

function animateRoute(
    path,
    marker,
    speed = 1,
    onLegChange = () => {},
    onArrive = () => {}
) {
    if (!Array.isArray(path) || path.length === 0) {
        throw new Error("A non-empty route path is required.");
    }

    if (!marker || typeof marker.setLatLng !== "function") {
        throw new Error("A valid Leaflet ambulance marker is required.");
    }

    const allowedSpeeds = [1, 5, 10];

    if (!allowedSpeeds.includes(Number(speed))) {
        throw new Error("Speed must be 1, 5, or 10.");
    }

    const points = path
        .map(id => window.RescueMap?.coordinateFor(id))
        .filter(Boolean);

    if (points.length !== path.length) {
        throw new Error(
            "Some route locations have no coordinates on the map."
        );
    }

    let cancelled = false;
    let segmentIndex = 0;
    let frameId = null;
    let lastTimestamp = null;
    let segmentElapsed = 0;

    // Duration for each road segment at normal speed.
    const SEGMENT_DURATION_MS = 1200;

    function finish() {
        if (cancelled) {
            return;
        }

        cancelled = true;
        marker.setLatLng([points[points.length - 1].lat,
                          points[points.length - 1].lng]);

        onArrive();
    }

    function step(timestamp) {
        if (cancelled) {
            return;
        }

        if (lastTimestamp === null) {
            lastTimestamp = timestamp;
        }

        const delta = Math.min(timestamp - lastTimestamp, 100);
        lastTimestamp = timestamp;

        if (segmentIndex >= points.length - 1) {
            finish();
            return;
        }

        segmentElapsed += delta * Number(speed);

        const progress = Math.min(
            segmentElapsed / SEGMENT_DURATION_MS,
            1
        );

        const start = points[segmentIndex];
        const end = points[segmentIndex + 1];

        const lat = start.lat + (end.lat - start.lat) * progress;
        const lng = start.lng + (end.lng - start.lng) * progress;

        marker.setLatLng([lat, lng]);

        if (progress >= 1) {
            segmentIndex += 1;
            segmentElapsed = 0;

            onLegChange({
                segmentIndex,
                locationId: path[segmentIndex],
                locationName: points[segmentIndex]?.name
            });

            if (segmentIndex >= points.length - 1) {
                finish();
                return;
            }
        }

        frameId = requestAnimationFrame(step);
    }

    frameId = requestAnimationFrame(step);

    // Return a cancellation handle so simultaneous animations can
    // be stopped independently.
    return {
        cancel() {
            cancelled = true;

            if (frameId !== null) {
                cancelAnimationFrame(frameId);
            }
        }
    };
}

/*
 * Animate two distinct route legs:
 * 1. Ambulance location -> patient pickup
 * 2. Patient pickup -> hospital
 *
 * The paths must be arrays of location IDs.
 */
function animateDispatch({
    pickupPath,
    hospitalPath,
    marker,
    speed = 1,
    onLegChange = () => {},
    onArrive = () => {}
}) {
    let currentAnimation = null;

    const cancel = () => {
        if (currentAnimation) {
            currentAnimation.cancel();
        }
    };

    const firstLeg = pickupPath || [];
    const secondLeg = hospitalPath || [];

    if (!firstLeg.length && !secondLeg.length) {
        throw new Error("No pickup or hospital route was provided.");
    }

    const startHospitalLeg = () => {
        if (secondLeg.length === 0) {
            onArrive();
            return;
        }

        window.RescueMap?.drawLeg(
            secondLeg,
            "#8fc47a"
        );

        currentAnimation = animateRoute(
            secondLeg,
            marker,
            speed,
            info => onLegChange({
                leg: "hospital",
                ...info
            }),
            () => onArrive()
        );
    };

    if (firstLeg.length > 0) {
        window.RescueMap?.drawLeg(
            firstLeg,
            "#38bdf8",
            "8 5"
        );

        currentAnimation = animateRoute(
            firstLeg,
            marker,
            speed,
            info => onLegChange({
                leg: "pickup",
                ...info
            }),
            startHospitalLeg
        );
    } else {
        startHospitalLeg();
    }

    return { cancel };
}