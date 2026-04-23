/**
 * Li-ion Battery Monitoring System v3.0
 * Dashboard JavaScript - Portfolio Edition
 * 
 * This script simulates the behavior of the v3.0 firmware:
 * - Kalman-filtered telemetry
 * - LUT-based SOC calculation
 * - System State Machine transitions
 * - Diagnostic Logging
 */

(function() {
    'use strict';

    // Configuration
    const REFRESH_INTERVAL = 1000; // Faster updates
    
    // System State Machine
    const MODES = {
        IDLE: { label: 'System Idle', color: '#888' },
        CHARGING: { label: 'Charging', color: '#00d4aa' },
        DISCHARGING: { label: 'Discharging', color: '#3498db' },
        FAULT: { label: 'CRITICAL FAULT', color: '#ff4757' }
    };

    const state = {
        voltage: 11.8,
        current: 1.85, // Start with non-zero current
        temperature: 28.5,
        soc: 65,
        mode: 'CHARGING', // Start in active mode
        lastMode: 'IDLE'
    };

    // Li-ion 3S Look-Up Table (matches firmware)
    const socLUT = [
        {v: 12.60, s: 100}, {v: 12.30, s: 90}, {v: 12.10, s: 80}, {v: 11.90, s: 70},
        {v: 11.70, s: 60},  {v: 11.45, s: 50}, {v: 11.30, s: 40}, {v: 11.10, s: 30},
        {v: 10.80, s: 20},  {v: 10.50, s: 10}, {v: 9.00, s: 0}
    ];

    function calculateSOC(voltage) {
        if (voltage >= socLUT[0].v) return 100;
        if (voltage <= socLUT[socLUT.length - 1].v) return 0;
        for (let i = 0; i < socLUT.length - 1; i++) {
            if (voltage <= socLUT[i].v && voltage > socLUT[i+1].v) {
                const vRange = socLUT[i].v - socLUT[i+1].v;
                const sRange = socLUT[i].s - socLUT[i+1].s;
                return socLUT[i+1].s + (voltage - socLUT[i+1].v) * (sRange / vRange);
            }
        }
        return 0;
    }

    // Historical data for chart
    const historicalData = {
        labels: [],
        voltage: [],
        current: [],
        temperature: []
    };

    function addLog(message, type = 'info') {
        const logContainer = document.getElementById('system-logs');
        if (!logContainer) return;

        const now = new Date();
        const timeStr = now.toTimeString().split(' ')[0];
        
        const entry = document.createElement('div');
        entry.className = `log-entry ${type}`;
        entry.innerHTML = `<span class="log-time">${timeStr}</span> ${message}`;
        
        logContainer.prepend(entry);
        if (logContainer.childNodes.length > 50) logContainer.lastChild.remove();
    }

    function updateUI() {
        // Update hero and card values
        const elements = ['voltage', 'current', 'temperature', 'soc'];
        elements.forEach(id => {
            const val = state[id];
            
            // For text display
            let fmt;
            if (id === 'current') {
                fmt = val.toFixed(2);
            } else if (id === 'soc') {
                fmt = Math.round(val);
            } else {
                fmt = val.toFixed(1);
            }
            
            const heroEl = document.getElementById(id);
            const cardEl = document.getElementById(`card-${id}`);
            if (heroEl) heroEl.textContent = fmt;
            if (cardEl) cardEl.textContent = fmt;
            
            // For gauges
            const gauge = document.getElementById(`${id}-gauge`);
            if (gauge) {
                let percent = 0;
                if (id === 'voltage') percent = ((val - 9) / (12.6 - 9)) * 100;
                else if (id === 'current') percent = (Math.abs(val) / 5) * 100;
                else if (id === 'temperature') percent = ((val - 20) / (60 - 20)) * 100;
                else percent = val;
                gauge.style.width = Math.min(100, Math.max(0, percent)) + '%';
            }
        });

        // Update mode status
        const statusText = document.getElementById('status-text');
        const modeDot = document.getElementById('mode-dot');
        const alertBanner = document.getElementById('critical-alert');

        if (statusText) statusText.textContent = MODES[state.mode].label;
        if (modeDot) modeDot.style.backgroundColor = MODES[state.mode].color;

        if (state.mode === 'FAULT') {
            alertBanner.style.display = 'block';
        } else {
            alertBanner.style.display = 'none';
        }

        // Log mode changes
        if (state.mode !== state.lastMode) {
            const type = state.mode === 'FAULT' ? 'error' : (state.mode === 'IDLE' ? 'info' : 'warn');
            addLog(`System transition: ${state.lastMode} -> ${state.mode}`, type);
            state.lastMode = state.mode;
        }
    }

    function simulateData() {
        // Mode logic
        if (state.mode === 'CHARGING') {
            state.voltage += 0.01;
            state.current = 1.85 + (Math.random() - 0.5) * 0.1;
            if (state.voltage >= 12.6) {
                state.mode = 'IDLE';
                addLog("Battery fully charged. Switching to idle.", "info");
            }
        } else if (state.mode === 'DISCHARGING') {
            state.voltage -= 0.01;
            state.current = -2.42 + (Math.random() - 0.5) * 0.2;
            if (state.voltage <= 9.2) {
                state.mode = 'IDLE';
                addLog("Low voltage threshold. Discharging stopped.", "warn");
            }
        } else if (state.mode === 'IDLE') {
            state.current = -0.02 + (Math.random() - 0.5) * 0.01; // Tiny parasitic draw
            state.voltage += (Math.random() - 0.5) * 0.002;
            
            const rand = Math.random();
            if (rand < 0.05) state.mode = 'CHARGING';
            else if (rand < 0.10) state.mode = 'DISCHARGING';
        }

        // Temperature
        if (state.mode !== 'IDLE') {
            state.temperature += 0.05;
        } else {
            state.temperature -= 0.02;
        }
        state.temperature = Math.max(26, Math.min(60, state.temperature));

        // Fault trigger
        if (state.temperature > 55 && state.mode !== 'FAULT') {
            state.mode = 'FAULT';
            addLog("CRITICAL: Over-temperature fault detected!", "error");
        } else if (state.mode === 'FAULT') {
            state.current = 0;
            state.temperature -= 0.1;
            if (state.temperature < 45) {
                state.mode = 'IDLE';
                addLog("System cooled down. Fault cleared.", "info");
            }
        }

        state.soc = calculateSOC(state.voltage);

        // Chart data
        const now = new Date();
        const timeStr = now.getHours().toString().padStart(2, '0') + ":" + 
                        now.getMinutes().toString().padStart(2, '0') + ":" + 
                        now.getSeconds().toString().padStart(2, '0');
        
        historicalData.labels.push(timeStr);
        historicalData.voltage.push(state.voltage);
        historicalData.current.push(state.current); // Allow negative for chart
        historicalData.temperature.push(state.temperature);

        if (historicalData.labels.length > 30) {
            historicalData.labels.shift();
            historicalData.voltage.shift();
            historicalData.current.shift();
            historicalData.temperature.shift();
        }

        updateUI();
        drawChart();
    }

    function drawChart() {
        const canvas = document.getElementById('sensorsChart');
        if (!canvas) return;

        const ctx = canvas.getContext('2d');
        const width = canvas.width = canvas.parentElement.clientWidth;
        const height = canvas.height = 250;

        ctx.clearRect(0, 0, width, height);
        
        const padding = 40;
        const chartW = width - padding * 2;
        const chartH = height - padding * 2;

        // Grid lines
        ctx.strokeStyle = '#2a2a4e';
        ctx.lineWidth = 1;
        ctx.beginPath();
        for(let i=0; i<=4; i++) {
            const y = padding + (chartH/4)*i;
            ctx.moveTo(padding, y); ctx.lineTo(width-padding, y);
        }
        ctx.stroke();

        function drawLine(data, color, min, max) {
            if (data.length < 2) return;
            ctx.strokeStyle = color;
            ctx.lineWidth = 2;
            ctx.beginPath();
            data.forEach((val, i) => {
                const x = padding + (i / (data.length - 1)) * chartW;
                const y = padding + chartH - ((val - min) / (max - min)) * chartH;
                if (i === 0) ctx.moveTo(x, y);
                else ctx.lineTo(x, y);
            });
            ctx.stroke();
        }

        // Draw multiple parameters
        drawLine(historicalData.voltage, '#00d4aa', 8, 14);      // Voltage scale
        drawLine(historicalData.current, '#3498db', -5, 5);      // Current scale (bipolar)
        drawLine(historicalData.temperature, '#ff4757', 20, 70); // Temp scale
    }

    function init() {
        console.log("BMS Dashboard v3.0 Initialized");
        // Pre-fill some data so the chart isn't empty
        for(let i=0; i<20; i++) {
            const time = new Date(Date.now() - (20-i)*2000);
            historicalData.labels.push(time.getHours().toString().padStart(2, '0') + ":" + 
                                       time.getMinutes().toString().padStart(2, '0') + ":" + 
                                       time.getSeconds().toString().padStart(2, '0'));
            historicalData.voltage.push(11.5 + Math.random());
            historicalData.current.push(1.5 + Math.random());
            historicalData.temperature.push(28 + Math.random());
        }
        
        setInterval(simulateData, REFRESH_INTERVAL);
        window.addEventListener('resize', drawChart);
        simulateData();
    }

    document.addEventListener('DOMContentLoaded', init);
})();
