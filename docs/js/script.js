/**
 * Li-ion Battery Monitoring System
 * Dashboard JavaScript
 * 
 * IMPORTANT: This file generates MOCK DATA for demonstration purposes.
 * The actual ESP32 firmware sends data to Supabase, but this frontend
 * does NOT fetch from the cloud - it simulates data locally for the portfolio.
 * 
 * To connect to real Supabase data, you would need to:
 * 1. Add @supabase/supabase-js library
 * 2. Configure your Supabase URL and key
 * 3. Replace mock data functions with actual API calls
 * 
 * Example real implementation:
 *   const { data, error } = await supabase
 *     .from('sensor_readings')
 *     .select('*')
 *     .order('created_at', { ascending: false })
 *     .limit(10);
 */

(function() {
    'use strict';

    // Configuration
    const REFRESH_INTERVAL = 3000;
    const IS_MOCK_DATA = true;  // This dashboard uses mock data

    // Mock sensor data - generated locally for portfolio demo only!
    const mockData = {
        voltage: 12.6,
        current: 1.24,
        temperature: 28.5,
        soc: 95,
        status: 'charging',
        wifiConnected: true,
        dbConnected: true
    };

    // Generate initial mock historical data
    const historicalData = {
        labels: [],
        voltage: [],
        current: [],
        temperature: []
    };

    function generateInitialData() {
        const now = Date.now();
        for (let i = 30; i >= 0; i--) {
            const time = new Date(now - i * 5 * 60 * 1000);
            historicalData.labels.push(time.toLocaleTimeString('en-US', {
                hour: '2-digit',
                minute: '2-digit'
            }));
            historicalData.voltage.push(11.5 + Math.random() * 1.5);
            historicalData.current.push(0.5 + Math.random() * 2.5);
            historicalData.temperature.push(25 + Math.random() * 15);
        }
    }

    // Update dashboard with mock data
    function updateDashboard() {
        // Add some random variation to simulate "live" data
        mockData.voltage = +(mockData.voltage + (Math.random() - 0.5) * 0.1).toFixed(2);
        mockData.current = +(mockData.current + (Math.random() - 0.5) * 0.2).toFixed(2);
        mockData.temperature = +(mockData.temperature + (Math.random() - 0.5) * 1).toFixed(1);
        
        // Update SOC from voltage
        const socFromVoltage = Math.min(100, Math.max(0,
            ((mockData.voltage - 8.0) / (12.9 - 8.0)) * 100
        )));
        mockData.soc = socFromVoltage;

        // Update hero stats
        const heroEls = ['voltage', 'current', 'temperature', 'soc'];
        heroEls.forEach(id => {
            const el = document.getElementById(id);
            if (el) el.textContent = id === 'voltage' ? mockData.voltage.toFixed(1) :
                                         id === 'current' ? mockData.current.toFixed(2) :
                                         id === 'temperature' ? mockData.temperature.toFixed(1) :
                                         Math.round(mockData.soc);
        });

        // Update cards
        const cardEls = ['card-voltage', 'card-current', 'card-temperature', 'card-soc'];
        cardEls.forEach(id => {
            const el = document.getElementById(id);
            if (el) el.textContent = id === 'card-voltage' ? mockData.voltage.toFixed(1) :
                                       id === 'card-current' ? mockData.current.toFixed(2) :
                                       id === 'card-temperature' ? mockData.temperature.toFixed(1) :
                                       Math.round(mockData.soc);
        });

        // Update gauges
        document.getElementById('voltage-gauge').style.width =
            ((mockData.voltage / 14.4) * 100) + '%';
        document.getElementById('current-gauge').style.width =
            Math.min(100, (mockData.current / 5 * 100)) + '%';

        const tempGauge = document.getElementById('temp-gauge');
        tempGauge.style.width = (mockData.temperature / 80 * 100) + '%';
        tempGauge.className = 'gauge-fill ' +
            (mockData.temperature > 50 ? 'danger' : mockData.temperature > 40 ? 'warning' : '');

        document.getElementById('soc-gauge').style.width = mockData.soc + '%';

        // Update status indicators
        const statusText = document.getElementById('status-text');
        if (statusText) statusText.textContent = mockData.status === 'charging' ? 'Charging' : 'Discharging';
    }

    // Chart functions
    let chart = null;

    function initChart() {
        const canvas = document.getElementById('sensorsChart');
        if (!canvas) return;

        const ctx = canvas.getContext('2d');
        const width = canvas.parentElement.clientWidth - 32;
        const height = 260;
        canvas.width = width;
        canvas.height = height;

        chart = { ctx, width, height };
        drawChart();
    }

    function updateChart() {
        if (!chart) return;

        // Occasionally add new mock data point
        if (Math.random() > 0.7) {
            historicalData.labels.push(new Date().toLocaleTimeString('en-US', {
                hour: '2-digit',
                minute: '2-digit'
            }));
            historicalData.voltage.push(mockData.voltage);
            historicalData.current.push(mockData.current);
            historicalData.temperature.push(mockData.temperature);

            if (historicalData.labels.length > 30) {
                historicalData.labels.shift();
                historicalData.voltage.shift();
                historicalData.current.shift();
                historicalData.temperature.shift();
            }
        }

        drawChart();
    }

    function drawChart() {
        if (!chart) return;

        const { ctx, width, height } = chart;
        const { labels, voltage, current, temperature } = historicalData;

        ctx.clearRect(0, 0, width, height);

        const padding = { top: 20, right: 20, bottom: 40, left: 50 };
        const chartWidth = width - padding.left - padding.right;
        const chartHeight = height - padding.top - padding.bottom;

        // Grid
        ctx.strokeStyle = '#2a2a4e';
        ctx.lineWidth = 1;
        for (let i = 0; i <= 4; i++) {
            const y = padding.top + (chartHeight / 4) * i;
            ctx.beginPath();
            ctx.moveTo(padding.left, y);
            ctx.lineTo(width - padding.right, y);
            ctx.stroke();
        }

        // Draw lines
        const drawLine = (values, color) => {
            ctx.strokeStyle = color;
            ctx.lineWidth = 2;
            ctx.beginPath();

            const maxVal = Math.max(...values, 15);
            const minVal = Math.min(...values, 20);

            values.forEach((val, idx) => {
                const x = padding.left + (idx / (values.length - 1)) * chartWidth;
                const y = padding.top + (1 - (val - minVal) / (maxVal - minVal)) * chartHeight;
                if (idx === 0) ctx.moveTo(x, y);
                else ctx.lineTo(x, y);
            });
            ctx.stroke();
        };

        drawLine(voltage, '#00d4aa');
        drawLine(current, '#3498db');
        drawLine(temperature, '#e74c3c');

        // Labels
        ctx.fillStyle = '#a0a0a0';
        ctx.font = '11px sans-serif';
        ctx.textAlign = 'center';

        labels.forEach((label, idx) => {
            if (idx % 6 === 0 || idx === labels.length - 1) {
                const x = padding.left + (idx / (labels.length - 1)) * chartWidth;
                ctx.fillText(label, x, height - 10);
            }
        });

        ctx.textAlign = 'left';
        ctx.fillText('V', width - 40, 20);
        ctx.fillRect(width - 55, 12, 10, 10);
        ctx.fillText('A', width - 40, 40);
        ctx.fillRect(width - 55, 32, 10, 10);
        ctx.fillText('°C', width - 40, 60);
        ctx.fillRect(width - 55, 52, 10, 10);
    }

    // Initialize
    function init() {
        console.log('Dashboard initialized');
        console.log('NOTE: This demo uses MOCK DATA for portfolio demonstration.');
        console.log('To connect to real Supabase, update with actual API calls.');
        
        generateInitialData();
        updateDashboard();
        initChart();

        setInterval(() => {
            updateDashboard();
            updateChart();
        }, REFRESH_INTERVAL);

        window.addEventListener('resize', () => {
            initChart();
        });
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();