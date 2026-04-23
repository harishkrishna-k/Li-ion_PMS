/**
 * Li-ion Battery Monitoring System
 * Dashboard with Chart.js Integration
 */

(function() {
    'use strict';

    // Configuration
    const SUPABASE_URL = 'https://your-project.supabase.co';
    const SUPABASE_KEY = 'your-anon-key';
    const REFRESH_INTERVAL = 5000;

    // Mock data for demonstration
    const mockData = {
        voltage: 12.6,
        current: 1.24,
        temperature: 28.5,
        soc: 95,
        status: 'charging',
        wifiConnected: true,
        dbConnected: true
    };

    // Historical data
    const historicalData = {
        labels: [],
        voltage: [],
        current: [],
        temperature: []
    };

    // Generate initial data
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

    // Update dashboard
    function updateDashboard() {
        // Simulate small variations
        mockData.voltage = +(mockData.voltage + (Math.random() - 0.5) * 0.1).toFixed(2);
        mockData.current = +(mockData.current + (Math.random() - 0.5) * 0.2).toFixed(2);
        mockData.temperature = +(mockData.temperature + (Math.random() - 0.5) * 1).toFixed(1);
        
        // Update SOC from voltage
        const socFromVoltage = Math.min(100, Math.max(0,
            ((mockData.voltage - 8.0) / (12.9 - 8.0)) * 100
        ));
        mockData.soc = socFromVoltage;

        // Hero stats
        document.getElementById('voltage').textContent = mockData.voltage.toFixed(1);
        document.getElementById('current').textContent = mockData.current.toFixed(2);
        document.getElementById('temperature').textContent = mockData.temperature.toFixed(1);
        document.getElementById('soc').textContent = Math.round(mockData.soc);

        // Card values
        document.getElementById('card-voltage').textContent = mockData.voltage.toFixed(1);
        document.getElementById('card-current').textContent = mockData.current.toFixed(2);
        document.getElementById('card-temperature').textContent = mockData.temperature.toFixed(1);
        document.getElementById('card-soc').textContent = Math.round(mockData.soc);

        // Gauge fills
        document.getElementById('voltage-gauge').style.width =
            ((mockData.voltage / 14.4) * 100) + '%';
        document.getElementById('current-gauge').style.width =
            Math.min(100, (mockData.current / 5 * 100)) + '%';

        const tempGauge = document.getElementById('temp-gauge');
        tempGauge.style.width = (mockData.temperature / 80 * 100) + '%';
        tempGauge.className = 'gauge-fill ' +
            (mockData.temperature > 50 ? 'danger' : mockData.temperature > 40 ? 'warning' : '');

        document.getElementById('soc-gauge').style.width = mockData.soc + '%';

        // Status indicators
        const statusText = document.getElementById('status-text');
        if (statusText) {
            statusText.textContent = mockData.status === 'charging' ? 'Charging' : 'Discharging';
        }

        // Update chart
        updateChart();
    }

    // Chart instance
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

        const { ctx, width, height, data } = chart;

        // Add new data point periodically
        if (Math.random() > 0.7) {
            const now = new Date();
            historicalData.labels.push(now.toLocaleTimeString('en-US', {
                hour: '2-digit',
                minute: '2-digit'
            }));
            historicalData.voltage.push(mockData.voltage);
            historicalData.current.push(mockData.current);
            historicalData.temperature.push(mockData.temperature);

            // Keep last 30 points
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

        const { ctx, width, height, data } = chart;
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
        ctx.fillStyle = '#00d4aa';
        ctx.fillRect(width - 55, 12, 10, 10);

        ctx.fillStyle = '#a0a0a0';
        ctx.fillText('A', width - 40, 40);
        ctx.fillStyle = '#3498db';
        ctx.fillRect(width - 55, 32, 10, 10);

        ctx.fillStyle = '#a0a0a0';
        ctx.fillText('°C', width - 40, 60);
        ctx.fillStyle = '#e74c3c';
        ctx.fillRect(width - 55, 52, 10, 10);
    }

    // Initialize
    function init() {
        generateInitialData();
        updateDashboard();
        initChart();

        // Update intervals
        setInterval(updateDashboard, 3000);

        // Window resize handler
        window.addEventListener('resize', () => {
            initChart();
        });
    }

    // Start
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();