// ============================================================
//   REAL-TIME DASHBOARD - ESP32 MQTT INTEGRATION
//   Data langsung dari ESP32 melalui MQTT broker
// ============================================================

// ==================== KONFIGURASI ====================
const CONFIG = {
    // MQTT Broker (sama dengan ESP32)
    mqtt: {
        broker: 'wss://broker.hivemq.com:8000/mqtt',
        clientId: 'dashboard_' + Math.random().toString(16).substr(2, 8),
        topics: ['watermon/all']
    },
    // Jumlah data point untuk grafik
    maxDataPoints: 50,
    // Timeout untuk koneksi
    timeout: 10000
};

// ==================== STATE ====================
let state = {
    connected: false,
    mqttClient: null,
    dataCount: 0,
    lastData: null,
    charts: {
        ph: { labels: [], values: [] },
        tds: { labels: [], values: [] }
    },
    isDemo: false
};

// ==================== DOM REFS ====================
const DOM = {
    // Status
    mqttStatus: document.getElementById('mqttStatus'),
    espStatus: document.getElementById('espStatus'),
    connectionStatus: document.getElementById('connectionStatus'),
    lastUpdate: document.getElementById('lastUpdate'),
    lastMessage: document.getElementById('lastMessage'),
    dataCount: document.getElementById('dataCount'),
    
    // Water Status
    waterStatus: document.getElementById('waterStatus'),
    statusDetail: document.getElementById('statusDetail'),
    
    // Filter Health
    filterHealth: document.getElementById('filterHealth'),
    healthBar: document.getElementById('healthBar'),
    daysLeft: document.getElementById('daysLeft'),
    volumeTotal: document.getElementById('volumeTotal'),
    
    // Sensors
    phValue: document.getElementById('phValue'),
    phStatus: document.getElementById('phStatus'),
    tdsValue: document.getElementById('tdsValue'),
    tdsStatus: document.getElementById('tdsStatus'),
    turbidityValue: document.getElementById('turbidityValue'),
    turbidityStatus: document.getElementById('turbidityStatus'),
    tempValue: document.getElementById('tempValue'),
    tempStatus: document.getElementById('tempStatus'),
    flowRate: document.getElementById('flowRate'),
    pumpStatus: document.getElementById('pumpStatus')
};

// ==================== CHART INIT ====================
function initCharts() {
    const phCtx = document.getElementById('phChart').getContext('2d');
    const tdsCtx = document.getElementById('tdsChart').getContext('2d');

    const phChart = new Chart(phCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'pH',
                data: [],
                borderColor: '#4299e1',
                backgroundColor: 'rgba(66, 153, 225, 0.1)',
                borderWidth: 2,
                tension: 0.3,
                fill: true,
                pointRadius: 2,
                pointBackgroundColor: '#4299e1'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: { duration: 300 },
            plugins: {
                legend: { display: false },
                tooltip: {
                    callbacks: {
                        label: function(context) {
                            return `pH: ${context.parsed.y.toFixed(2)}`;
                        }
                    }
                }
            },
            scales: {
                y: {
                    min: 0,
                    max: 14,
                    grid: { color: 'rgba(0,0,0,0.05)' },
                    title: { display: true, text: 'pH', font: { size: 11 } }
                },
                x: {
                    grid: { display: false },
                    ticks: { 
                        maxTicksLimit: 10,
                        font: { size: 8 }
                    }
                }
            }
        }
    });

    const tdsChart = new Chart(tdsCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'TDS',
                data: [],
                borderColor: '#48bb78',
                backgroundColor: 'rgba(72, 187, 120, 0.1)',
                borderWidth: 2,
                tension: 0.3,
                fill: true,
                pointRadius: 2,
                pointBackgroundColor: '#48bb78'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: { duration: 300 },
            plugins: {
                legend: { display: false },
                tooltip: {
                    callbacks: {
                        label: function(context) {
                            return `TDS: ${context.parsed.y.toFixed(0)} ppm`;
                        }
                    }
                }
            },
            scales: {
                y: {
                    min: 0,
                    max: 500,
                    grid: { color: 'rgba(0,0,0,0.05)' },
                    title: { display: true, text: 'ppm', font: { size: 11 } }
                },
                x: {
                    grid: { display: false },
                    ticks: { 
                        maxTicksLimit: 10,
                        font: { size: 8 }
                    }
                }
            }
        }
    });

    return { phChart, tdsChart };
}

const charts = initCharts();

// ==================== MQTT CLIENT ====================
function initMQTT() {
    try {
        if (typeof Paho === 'undefined') {
            console.error('❌ Paho MQTT library not loaded!');
            updateConnectionStatus(false, 'Library Error');
            setTimeout(initMQTT, 3000);
            return;
        }

        console.log('🔌 Initializing MQTT client...');
        
        state.mqttClient = new Paho.MQTT.Client(
            CONFIG.mqtt.broker,
            CONFIG.mqtt.clientId
        );

        state.mqttClient.onConnectionLost = onConnectionLost;
        state.mqttClient.onMessageArrived = onMessageArrived;

        connectMQTT();
    } catch (e) {
        console.error('❌ MQTT Init Error:', e);
        updateConnectionStatus(false, 'Init Error');
        setTimeout(initMQTT, 5000);
    }
}

function connectMQTT() {
    if (!state.mqttClient) return;

    console.log('🔄 Connecting to MQTT broker...');
    updateConnectionStatus(false, 'Connecting...');

    const options = {
        timeout: 10,
        onSuccess: () => {
            console.log('✅ MQTT Connected!');
            state.connected = true;
            updateConnectionStatus(true, 'Connected');
            
            // Subscribe ke topic ESP32
            CONFIG.mqtt.topics.forEach(topic => {
                state.mqttClient.subscribe(topic);
                console.log(`📡 Subscribed to: ${topic}`);
            });
            
            // Kirim status online
            DOM.espStatus.textContent = 'ESP32: Online';
            DOM.espStatus.className = 'status-badge online';
        },
        onFailure: (error) => {
            console.error('❌ MQTT Connection Failed:', error.errorMessage);
            state.connected = false;
            updateConnectionStatus(false, 'Failed');
            
            // Retry setelah 5 detik
            setTimeout(connectMQTT, 5000);
        }
    };
    
    try {
        state.mqttClient.connect(options);
    } catch (e) {
        console.error('❌ MQTT Connect Error:', e);
        setTimeout(connectMQTT, 5000);
    }
}

function onConnectionLost(response) {
    state.connected = false;
    updateConnectionStatus(false, 'Lost');
    console.log('🔌 MQTT Connection Lost:', response.errorMessage);
    DOM.espStatus.textContent = 'ESP32: Offline';
    DOM.espStatus.className = 'status-badge offline';
    
    // Auto reconnect
    setTimeout(connectMQTT, 5000);
}

// ==================== MESSAGE HANDLER ====================
function onMessageArrived(message) {
    try {
        const data = JSON.parse(message.payloadString);
        console.log('📨 Data received:', data);
        
        // Update state
        state.lastData = data;
        state.dataCount++;
        DOM.dataCount.textContent = `📊 ${state.dataCount} data points`;
        DOM.lastMessage.textContent = `📨 Last message: ${new Date().toLocaleTimeString()}`;
        
        // Update dashboard
        updateDashboard(data);
        updateCharts(data);
        updateESPStatus(data);
        
    } catch (e) {
        console.error('❌ Error parsing message:', e);
    }
}

// ==================== DASHBOARD UPDATE ====================
function updateDashboard(data) {
    // Timestamp
    const now = new Date();
    DOM.lastUpdate.textContent = `⏱️ ${now.toLocaleTimeString('id-ID')}`;
    
    // ===== STATUS AIR =====
    const statusText = DOM.waterStatus.querySelector('.status-text');
    const statusIcon = DOM.waterStatus.querySelector('.status-icon');
    
    if (data.status === 'LAYAK') {
        statusText.textContent = 'LAYAK';
        statusText.className = 'status-text layak';
        statusIcon.textContent = '✅';
        DOM.statusDetail.textContent = '✨ Air layak minum';
        DOM.statusDetail.style.color = '#48bb78';
    } else {
        statusText.textContent = 'TIDAK LAYAK';
        statusText.className = 'status-text tidak-layak';
        statusIcon.textContent = '❌';
        DOM.statusDetail.textContent = '⚠️ Air tidak layak minum';
        DOM.statusDetail.style.color = '#fc8181';
    }
    
    // ===== FILTER HEALTH =====
    if (data.health !== undefined) {
        const health = parseFloat(data.health);
        DOM.filterHealth.textContent = `${health.toFixed(0)}%`;
        DOM.healthBar.style.width = `${Math.min(health, 100)}%`;
        
        DOM.healthBar.className = 'progress-fill ' + 
            (health >= 70 ? 'good' : health >= 40 ? 'warning' : 'danger');
    }
    
    if (data.days_left !== undefined) {
        DOM.daysLeft.textContent = `📅 ${data.days_left} Hari`;
    }
    
    if (data.volume !== undefined) {
        DOM.volumeTotal.textContent = `💧 ${data.volume.toFixed(1)} L`;
    }
    
    // ===== pH =====
    if (data.ph !== undefined) {
        DOM.phValue.textContent = data.ph.toFixed(2);
        const phStatus = DOM.phStatus;
        if (data.ph >= 6.5 && data.ph <= 8.5) {
            phStatus.textContent = '✅ Normal';
            phStatus.className = 'status-indicator normal';
        } else if (data.ph < 6.5) {
            phStatus.textContent = '⚠️ Asam';
            phStatus.className = 'status-indicator warning';
        } else {
            phStatus.textContent = '⚠️ Basa';
            phStatus.className = 'status-indicator danger';
        }
    }
    
    // ===== TDS =====
    if (data.tds !== undefined) {
        DOM.tdsValue.textContent = data.tds.toFixed(0);
        const tdsStatus = DOM.tdsStatus;
        if (data.tds <= 50) {
            tdsStatus.textContent = '✅ Sangat Baik';
            tdsStatus.className = 'status-indicator normal';
        } else if (data.tds <= 200) {
            tdsStatus.textContent = '✅ Baik';
            tdsStatus.className = 'status-indicator normal';
        } else if (data.tds <= 500) {
            tdsStatus.textContent = '⚠️ Cukup';
            tdsStatus.className = 'status-indicator warning';
        } else {
            tdsStatus.textContent = '❌ Tinggi';
            tdsStatus.className = 'status-indicator danger';
        }
    }
    
    // ===== TURBIDITY =====
    if (data.turbidity !== undefined) {
        DOM.turbidityValue.textContent = data.turbidity.toFixed(0);
        const turbStatus = DOM.turbidityStatus;
        if (data.turbidity >= 80) {
            turbStatus.textContent = '✅ Sangat Jernih';
            turbStatus.className = 'status-indicator normal';
        } else if (data.turbidity >= 50) {
            turbStatus.textContent = '✅ Jernih';
            turbStatus.className = 'status-indicator normal';
        } else if (data.turbidity >= 20) {
            turbStatus.textContent = '⚠️ Agak Keruh';
            turbStatus.className = 'status-indicator warning';
        } else {
            turbStatus.textContent = '❌ Keruh';
            turbStatus.className = 'status-indicator danger';
        }
    }
    
    // ===== TEMPERATURE =====
    if (data.temperature !== undefined) {
        DOM.tempValue.textContent = data.temperature.toFixed(1);
        const tempStatus = DOM.tempStatus;
        if (data.temperature >= 15 && data.temperature <= 35) {
            tempStatus.textContent = '✅ Normal';
            tempStatus.className = 'status-indicator normal';
        } else {
            tempStatus.textContent = '⚠️ Tidak Normal';
            tempStatus.className = 'status-indicator warning';
        }
    }
    
    // ===== FLOW RATE =====
    if (data.flow_rate !== undefined) {
        DOM.flowRate.textContent = data.flow_rate.toFixed(1);
    }
    
    // ===== PUMP =====
    if (data.pump) {
        const pumpEl = DOM.pumpStatus;
        if (data.pump === 'ON') {
            pumpEl.innerHTML = '<span class="dot"></span> ON';
            pumpEl.className = 'pump-badge on';
        } else {
            pumpEl.innerHTML = '<span class="dot"></span> OFF';
            pumpEl.className = 'pump-badge off';
        }
    }
}

// ==================== CHARTS UPDATE ====================
function updateCharts(data) {
    const now = new Date().toLocaleTimeString('id-ID');
    
    // Update pH chart
    if (data.ph !== undefined) {
        const phData = state.charts.ph;
        phData.labels.push(now);
        phData.values.push(parseFloat(data.ph));
        
        if (phData.labels.length > CONFIG.maxDataPoints) {
            phData.labels.shift();
            phData.values.shift();
        }
        
        charts.phChart.data.labels = phData.labels;
        charts.phChart.data.datasets[0].data = phData.values;
        charts.phChart.update('none');
    }
    
    // Update TDS chart
    if (data.tds !== undefined) {
        const tdsData = state.charts.tds;
        tdsData.labels.push(now);
        tdsData.values.push(parseFloat(data.tds));
        
        if (tdsData.labels.length > CONFIG.maxDataPoints) {
            tdsData.labels.shift();
            tdsData.values.shift();
        }
        
        charts.tdsChart.data.labels = tdsData.labels;
        charts.tdsChart.data.datasets[0].data = tdsData.values;
        charts.tdsChart.update('none');
    }
}

// ==================== ESP STATUS ====================
function updateESPStatus(data) {
    if (data.esp_status) {
        DOM.espStatus.textContent = `ESP32: ${data.esp_status}`;
        DOM.espStatus.className = `status-badge ${data.esp_status === 'online' ? 'online' : 'offline'}`;
    } else {
        // Jika ada data, berarti ESP32 online
        DOM.espStatus.textContent = 'ESP32: Online';
        DOM.espStatus.className = 'status-badge online';
    }
}

// ==================== CONNECTION STATUS ====================
function updateConnectionStatus(connected, status) {
    const el = DOM.connectionStatus;
    if (connected) {
        el.textContent = '● Online';
        el.className = 'badge badge-success';
        DOM.mqttStatus.textContent = 'MQTT: Online';
        DOM.mqttStatus.className = 'status-badge online';
    } else {
        el.textContent = `● ${status || 'Offline'}`;
        el.className = 'badge badge-danger';
        DOM.mqttStatus.textContent = `MQTT: ${status || 'Offline'}`;
        DOM.mqttStatus.className = 'status-badge offline';
    }
}

// ==================== DEMO DATA (FALLBACK) ====================
function generateDemoData() {
    const now = new Date();
    const basePh = 7.0 + Math.sin(now.getSeconds() / 60 * Math.PI) * 0.2;
    const baseTds = 100 + Math.sin(now.getSeconds() / 30 * Math.PI) * 30;
    
    return {
        ph: Math.max(0, Math.min(14, basePh + (Math.random() - 0.5) * 0.1)),
        tds: Math.max(0, baseTds + (Math.random() - 0.5) * 10),
        turbidity: Math.max(0, Math.min(100, 70 + (Math.random() - 0.5) * 15)),
        temperature: 25 + (Math.random() - 0.5) * 2,
        status: Math.random() > 0.15 ? 'LAYAK' : 'TIDAK LAYAK',
        health: Math.max(0, Math.min(100, 80 + (Math.random() - 0.5) * 15)),
        days_left: Math.floor(Math.random() * 30) + 10,
        volume: 15000 + Math.random() * 5000,
        flow_rate: Math.max(0, 2 + (Math.random() - 0.5) * 3),
        pump: Math.random() > 0.3 ? 'ON' : 'OFF'
    };
}

// ==================== DEMO MODE ====================
let demoInterval = null;

function startDemoMode() {
    console.log('🎯 Starting demo mode (no MQTT)');
    state.isDemo = true;
    updateConnectionStatus(false, 'Demo Mode');
    DOM.espStatus.textContent = 'ESP32: Demo';
    DOM.espStatus.className = 'status-badge warning';
    
    // Update setiap 2 detik dengan data demo
    demoInterval = setInterval(() => {
        const demoData = generateDemoData();
        state.dataCount++;
        DOM.dataCount.textContent = `📊 ${state.dataCount} data points (DEMO)`;
        DOM.lastMessage.textContent = `📨 Demo: ${new Date().toLocaleTimeString()}`;
        updateDashboard(demoData);
        updateCharts(demoData);
    }, 2000);
}

function stopDemoMode() {
    if (demoInterval) {
        clearInterval(demoInterval);
        demoInterval = null;
        state.isDemo = false;
        console.log('🔄 Demo mode stopped');
    }
}

// ==================== INIT ====================
console.log('🚀 Starting Real-Time Dashboard...');
console.log(`📡 MQTT Broker: ${CONFIG.mqtt.broker}`);
console.log(`📊 Max data points: ${CONFIG.maxDataPoints}`);

// Start MQTT
initMQTT();

// Fallback: Jika tidak ada koneksi setelah 10 detik, mulai demo
setTimeout(() => {
    if (!state.connected && !state.isDemo) {
        console.warn('⚠️ No MQTT connection, starting demo mode...');
        startDemoMode();
    }
}, CONFIG.timeout);

// Cek koneksi setiap 10 detik
setInterval(() => {
    if (!state.connected && !state.isDemo) {
        console.log('🔄 Attempting to reconnect MQTT...');
        connectMQTT();
    }
}, 10000);
