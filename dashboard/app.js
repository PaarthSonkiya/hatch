// ============================================================================
//  HATCH operator console — app.js
//  Mock data + interactive map + node detail panel.
//  In production: data comes from the TimescaleDB REST API. This prototype
//  uses a deterministic synthetic dataset so the demo is reproducible.
// ============================================================================

// ---------------------------------------------------------------------------
//  Mock data — 10 nodes across two clusters
// ---------------------------------------------------------------------------
const NODES = [
  // Cluster A — NTU campus
  { id: 'A1', site: 'EEE — drainage / loading dock', siteType: 'drain',    lat: 1.3434, lng: 103.6810, state: 'ok',    batt_mv: 4080, stagnation_h: 0,  temp_c: 28.4, rh: 76, fav: 0.40 },
  { id: 'A2', site: 'Hall 2 — common planter',        siteType: 'planter',  lat: 1.3454, lng: 103.6790, state: 'warn',  batt_mv: 3950, stagnation_h: 28, temp_c: 27.9, rh: 81, fav: 0.85 },
  { id: 'A3', site: 'The Hive — rooftop drain',       siteType: 'gutter',   lat: 1.3469, lng: 103.6822, state: 'ok',    batt_mv: 4110, stagnation_h: 6,  temp_c: 31.1, rh: 68, fav: 0.40 },
  { id: 'A4', site: 'SBS — bin centre',               siteType: 'bin',      lat: 1.3447, lng: 103.6837, state: 'alert', batt_mv: 3870, stagnation_h: 52, temp_c: 28.7, rh: 84, fav: 1.00 },
  { id: 'A5', site: 'NIE garden — water feature',     siteType: 'pond',     lat: 1.3478, lng: 103.6788, state: 'warn',  batt_mv: 4060, stagnation_h: 120, temp_c: 28.1, rh: 79, fav: 0.85 },
  // Cluster B — Jurong West HDB (anonymised; demo coords)
  { id: 'B1', site: 'Block 6XX — floor trap',         siteType: 'drain',    lat: 1.3540, lng: 103.7060, state: 'ok',    batt_mv: 4020, stagnation_h: 4,  temp_c: 29.0, rh: 73, fav: 0.40 },
  { id: 'B2', site: 'Block 6XX — bin chute base',     siteType: 'bin',      lat: 1.3530, lng: 103.7080, state: 'alert', batt_mv: 3940, stagnation_h: 40, temp_c: 28.5, rh: 86, fav: 1.00 },
  { id: 'B3', site: 'Block 6XY — rooftop trough',     siteType: 'gutter',   lat: 1.3560, lng: 103.7050, state: 'ok',    batt_mv: 4090, stagnation_h: 2,  temp_c: 31.8, rh: 65, fav: 0.25 },
  { id: 'B4', site: 'Block 6YY — void deck planter',  siteType: 'planter',  lat: 1.3520, lng: 103.7110, state: 'warn',  batt_mv: 3980, stagnation_h: 30, temp_c: 28.2, rh: 80, fav: 0.85 },
  { id: 'B5', site: 'Carpark 6XZ — floor trap',       siteType: 'drain',    lat: 1.3565, lng: 103.7080, state: 'ok',    batt_mv: 4050, stagnation_h: 0,  temp_c: 30.4, rh: 71, fav: 0.40 },
];

const ALERTS = [
  { node: 'B2', when: '2h ago',    label: 'Alert',    desc: 'Aedes wingbeats confirmed (conf 0.86, votes 4/5). Stagnation 40h.' },
  { node: 'A4', when: '5h ago',    label: 'Alert',    desc: 'Aedes wingbeats confirmed (conf 0.81, votes 3/5). Stagnation 52h.' },
  { node: 'A2', when: '11h ago',   label: 'Gate open', desc: 'Env. gate opened (stagnation 24h crossed). Acoustic stage activated.', muted: true },
  { node: 'B4', when: '14h ago',   label: 'Gate open', desc: 'Env. gate opened. No acoustic detection in window.', muted: true },
  { node: 'A5', when: '1d ago',    label: 'Gate open', desc: 'Env. gate opened (long-stagnation feature triggered).', muted: true },
  { node: 'B2', when: '2d ago',    label: 'Resolved', desc: 'Site cleared by town council (response time 6h).', muted: true },
  { node: 'A4', when: '4d ago',    label: 'Alert',    desc: 'Aedes wingbeats confirmed (conf 0.79, votes 3/5).' },
  { node: 'B1', when: '5d ago',    label: 'Gate open', desc: 'Env. gate opened after monsoon rainfall event.', muted: true },
];

// ---------------------------------------------------------------------------
//  Initialise map
// ---------------------------------------------------------------------------
const map = L.map('map', {
  zoomControl: true,
  attributionControl: false,
}).setView([1.349, 103.695], 14);

L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
  maxZoom: 19,
}).addTo(map);

L.control.attribution({ prefix: false }).addAttribution('© OpenStreetMap').addTo(map);

const markers = {};

NODES.forEach(n => {
  const icon = L.divIcon({
    className: '',
    iconSize: [16, 16],
    iconAnchor: [8, 8],
    html: `<span class="hatch-node-icon ${n.state}"></span>`,
  });
  const m = L.marker([n.lat, n.lng], { icon }).addTo(map);
  m.bindPopup(`
    <div class="popup-id">${n.id}</div>
    <div><strong>${n.site}</strong></div>
    <div class="muted" style="margin-top:4px;">${n.temp_c.toFixed(1)} °C / ${n.rh}% RH · stagn. ${n.stagnation_h}h</div>
  `);
  m.on('click', () => selectNode(n.id));
  markers[n.id] = m;
});

// ---------------------------------------------------------------------------
//  Populate left-pane node list
// ---------------------------------------------------------------------------
const nodeRowsEl = document.getElementById('nodeRows');
const nodeCountEl = document.getElementById('nodecount');
nodeCountEl.textContent = NODES.length;

NODES.forEach(n => {
  const row = document.createElement('div');
  row.className = 'node-row';
  row.dataset.id = n.id;
  row.innerHTML = `
    <span class="badge ${n.state}"></span>
    <div class="meta">
      <span class="id">${n.id}</span>
      <span class="site">${n.site}</span>
    </div>
    <span class="batt">${(n.batt_mv / 1000).toFixed(2)}V</span>
  `;
  row.addEventListener('click', () => selectNode(n.id));
  nodeRowsEl.appendChild(row);
});

// ---------------------------------------------------------------------------
//  Populate alert log
// ---------------------------------------------------------------------------
const alertRowsEl = document.getElementById('alertRows');
ALERTS.forEach(a => {
  const row = document.createElement('div');
  row.className = 'alert-row' + (a.muted ? ' muted' : '');
  row.innerHTML = `
    <div class="time">${a.when}</div>
    <div class="body">
      <div class="head">${a.label} · ${a.node}</div>
      <div class="desc">${a.desc}</div>
    </div>
  `;
  alertRowsEl.appendChild(row);
});

// ---------------------------------------------------------------------------
//  Top-line metrics
// ---------------------------------------------------------------------------
document.getElementById('metric-alerts').textContent = NODES.filter(n => n.state === 'alert').length;
document.getElementById('metric-gated').textContent  = NODES.filter(n => n.state === 'warn' || n.state === 'alert').length;
const meanBatt = NODES.reduce((s, n) => s + n.batt_mv, 0) / NODES.length;
document.getElementById('metric-batt').textContent = `${(meanBatt / 1000).toFixed(2)}V`;

// ---------------------------------------------------------------------------
//  Detail panel + node selection
// ---------------------------------------------------------------------------
let currentChart = null;

function selectNode(id) {
  const n = NODES.find(x => x.id === id);
  if (!n) return;

  // Update list selection
  document.querySelectorAll('.node-row').forEach(r => r.classList.toggle('selected', r.dataset.id === id));

  // Centre map + popup
  map.setView([n.lat, n.lng], 16, { animate: true });
  markers[id].openPopup();

  // Render detail panel
  renderDetail(n);
}

function renderDetail(n) {
  const detail = document.getElementById('detailPanel');
  const stateLabel = { ok: 'normal', warn: 'gate open', alert: 'ACTIVE ALERT' }[n.state];
  const stateClass = n.state === 'alert' ? 'alert' : n.state === 'warn' ? 'warn' : '';

  detail.innerHTML = `
    <div class="detail-header">
      <div class="detail-id">NODE ${n.id} · ${n.siteType.toUpperCase()}</div>
      <div class="detail-site">${n.site}</div>
      <div class="detail-coords">${n.lat.toFixed(4)}°N, ${n.lng.toFixed(4)}°E</div>
    </div>
    <div class="detail-grid">
      <div class="cell"><div class="k">State</div><div class="v ${stateClass}">${stateLabel}</div></div>
      <div class="cell"><div class="k">Favorability</div><div class="v">${(n.fav * 100).toFixed(0)}%</div></div>
      <div class="cell"><div class="k">Stagnation</div><div class="v">${n.stagnation_h}h</div></div>
      <div class="cell"><div class="k">Temperature</div><div class="v">${n.temp_c.toFixed(1)} °C</div></div>
      <div class="cell"><div class="k">Humidity</div><div class="v">${n.rh}%</div></div>
      <div class="cell"><div class="k">Battery</div><div class="v">${(n.batt_mv / 1000).toFixed(2)}V</div></div>
    </div>
    <div class="chart-wrap">
      <div class="chart-title">Favorability score · last 24h</div>
      <canvas id="favChart"></canvas>
    </div>
  `;

  // Render mock 24h favorability chart
  const ctx = document.getElementById('favChart');
  if (currentChart) currentChart.destroy();

  // Generate a plausible-looking time series ending at n.fav
  const points = 24;
  const data = [];
  let v = n.fav * 0.4;
  for (let i = 0; i < points; i++) {
    v += (Math.random() - 0.4) * 0.08;
    v = Math.max(0, Math.min(1, v));
    if (i === points - 1) v = n.fav;
    data.push(v);
  }
  const labels = Array.from({ length: points }, (_, i) => `${i - points + 1}h`);

  currentChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels,
      datasets: [{
        data,
        borderColor: '#d94434',
        backgroundColor: 'rgba(217, 68, 52, 0.1)',
        borderWidth: 1.5,
        pointRadius: 0,
        fill: true,
        tension: 0.3,
      }],
    },
    options: {
      maintainAspectRatio: false,
      plugins: { legend: { display: false } },
      scales: {
        x: { display: false },
        y: { 
          min: 0, max: 1, 
          ticks: { color: '#6e7378', font: { family: 'JetBrains Mono', size: 9 } },
          grid: { color: '#2a2f33' },
        },
      },
    },
  });
}

// ---------------------------------------------------------------------------
//  Live clock
// ---------------------------------------------------------------------------
function updateClock() {
  const d = new Date();
  const t = d.toLocaleTimeString('en-SG', { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false });
  document.getElementById('liveTime').textContent = t + ' SGT';
}
setInterval(updateClock, 1000);
updateClock();

// ---------------------------------------------------------------------------
//  Auto-select most concerning node on load (one of the alerts)
// ---------------------------------------------------------------------------
window.addEventListener('load', () => {
  const top = NODES.find(n => n.state === 'alert') || NODES[0];
  setTimeout(() => selectNode(top.id), 300);
});
