/**
 * Li-ion Battery Performance Monitoring System
 * Dashboard JavaScript - Demo Version with Mock Data
 */

(function() {
    'use strict';

    // Mock sensor data for portfolio demonstration
    // Replace with actual Supabase fetch in production
    const mockData = {
        voltage: 12.6,
        current: 1.24,
        temperature: 28.5,
        soc: 95,
        status: 'charging',
        wifiConnected: true,
        dbConnected: true
    };

    // Historical data for chart
    const historicalData = {
        labels: [],
        voltage: [],
        current: [],
        temperature: []
    };

    // Generate mock historical data
    function generateHistoricalData() {
        const now = Date.now();
        for (let i = 20; i >= 0; i--) {
            const time = new Date(now - i * 5 * 60 * 1000);
            historicalData.labels.push(time.toLocaleTimeString('en-US', {
                hour: '2-digit',
                minute: '2-digit'
            }));
            historicalData.voltage.push(11.8 + Math.random() * 1.2);
            historicalData.current.push(0.5 + Math.random() * 2.5);
            historicalData.temperature.push(25 + Math.random() * 15);
        }
    }

    // Generate initial data
    generateHistoricalData();

    // Update dashboard with mock data
    function updateDashboard() {
        // Add some random variation to simulate live data
        mockData.voltage = +(mockData.voltage + (Math.random() - 0.5) * 0.1).toFixed(2);
        mockData.current = +(mockData.current + (Math.random() - 0.5) * 0.2).toFixed(2);
        mockData.temperature = +(mockData.temperature + (Math.random() - 0.5) * 1).toFixed(1);
        mockData.soc = Math.min(100, Math.max(0, +(mockData.soc + (Math.random() - 0.5) * 2)));

        // Calculate SOC from voltage
        const socFromVoltage = Math.min(100, Math.max(0,
            ((mockData.voltage - 8.0) / (12.9 - 8.0)) * 100
        ));
        mockData.soc = socFromVoltage;

        // Update hero stats
        document.getElementById('voltage').textContent = mockData.voltage.toFixed(1);
        document.getElementById('current').textContent = mockData.current.toFixed(2);
        document.getElementById('temperature').textContent = mockData.temperature.toFixed(1);
        document.getElementById('soc').textContent = Math.round(mockData.soc);

        // Update cards
        document.getElementById('card-voltage').textContent = mockData.voltage.toFixed(1);
        document.getElementById('card-current').textContent = mockData.current.toFixed(2);
        document.getElementById('card-temperature').textContent = mockData.temperature.toFixed(1);
        document.getElementById('card-soc').textContent = Math.round(mockData.soc);

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
        const chargingStatus = document.getElementById('status-charging');
        if (mockData.status === 'charging') {
            chargingStatus.innerHTML = '<span class="dot online"></span> Charging';
        } else {
            chargingStatus.innerHTML = '<span class="dot"></span> Discharging';
        }

        const wifiStatus = document.getElementById('status-wifi');
        wifiStatus.innerHTML = mockData.wifiConnected ?
            '<span class="dot online"></span> WiFi Connected' :
            '<span class="dot"></span> WiFi Disconnected';

        const dbStatus = document.getElementById('status-db');
        dbStatus.innerHTML = mockData.dbConnected ?
            '<span class="dot online"></span> Database Connected' :
            '<span class="dot"></span> Database Disconnected';
    }

    // Initialize Chart.js
    let sensorsChart = null;

    function initChart() {
        const canvas = document.getElementById('sensorsChart');
        if (!canvas) return;

        const ctx = canvas.getContext('2d');

        // Simple chart implementation without Chart.js dependency
        const width = canvas.parentElement.clientWidth - 32;
        const height = 260;
        canvas.width = width;
        canvas.height = height;

        // Store chart reference for updates
        sensorsChart = {
            ctx: ctx,
            width: width,
            height: height,
            data: historicalData
        };

        drawChart();
    }

    function drawChart() {
        if (!sensorsChart) return;

        const { ctx, width, height, data } = sensorsChart;

        // Clear canvas
        ctx.clearRect(0, 0, width, height);

        // Chart area
        const padding = { top: 20, right: 20, bottom: 40, left: 50 };
        const chartWidth = width - padding.left - padding.right;
        const chartHeight = height - padding.top - padding.bottom;

        // Draw grid
        ctx.strokeStyle = '#2a2a4e';
        ctx.lineWidth = 1;

        // Horizontal grid lines
        for (let i = 0; i <= 4; i++) {
            const y = padding.top + (chartHeight / 4) * i;
            ctx.beginPath();
            ctx.moveTo(padding.left, y);
            ctx.lineTo(width - padding.right, y);
            ctx.stroke();
        }

        // Draw data
        const drawLine = (values, color) => {
            ctx.strokeStyle = color;
            ctx.lineWidth = 2;
            ctx.beginPath();

            const maxVal = Math.max(...values, 15);
            const minVal = Math.min(...values, 20);

            values.forEach((val, idx) => {
                const x = padding.left + (idx / (values.length - 1)) * chartWidth;
                const y = padding.top + (1 - (val - minVal) / (maxVal - minVal)) * chartHeight;

                if (idx === 0) {
                    ctx.moveTo(x, y);
                } else {
                    ctx.lineTo(x, y);
                }
            });

            ctx.stroke();
        };

        // Draw voltage (green)
        drawLine(data.voltage, '#00d4aa');

        // Draw current (blue)
        drawLine(data.current, '#3498db');

        // Draw temperature (orange)
        drawLine(data.temperature, '#e74c3c');

        // Draw labels
        ctx.fillStyle = '#a0a0a0';
        ctx.font = '12px sans-serif';
        ctx.textAlign = 'center';

        // X-axis labels (show every 5th)
        data.labels.forEach((label, idx) => {
            if (idx % 5 === 0 || idx === data.labels.length - 1) {
                const x = padding.left + (idx / (data.labels.length - 1)) * chartWidth;
                ctx.fillText(label, x, height - 10);
            }
        });

        // Legend
        ctx.textAlign = 'left';
        ctx.fillText('Voltage (V)', width - 100, 20);
        ctx.fillStyle = '#00d4aa';
        ctx.fillRect(width - 130, 12, 12, 12);

        ctx.fillStyle = '#3498db';
        ctx.fillText('Current (A)', width - 100, 40);
        ctx.fillRect(width - 130, 32, 12, 12);

        ctx.fillStyle = '#e74c3c';
        ctx.fillText('Temp (°C)', width - 100, 60);
        ctx.fillRect(width - 130, 52, 12, 12);
    }

    // Initialize dashboard
    function init() {
        updateDashboard();
        initChart();

        // Simulate live updates every 3 seconds
        setInterval(() => {
            updateDashboard();
            drawChart();
        }, 3000);

        // Handle window resize
        window.addEventListener('resize', drawChart);
    }

    // Start when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();