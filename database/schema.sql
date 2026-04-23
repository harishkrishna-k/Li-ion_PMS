-- ============================================================================
-- Li-ion Battery Performance Monitoring System - Database Schema (v2.0)
-- ============================================================================
-- Enhanced with time-series optimization, RLS policies, and retention
-- Run in Supabase SQL Editor
-- ============================================================================

-- Create sensor_readings table with optimized indices
CREATE TABLE IF NOT EXISTS sensor_readings (
    id BIGSERIAL PRIMARY KEY,
    device_name TEXT NOT NULL DEFAULT 'Li-ion_PMS_001',
    voltage REAL NOT NULL CHECK (voltage >= 0 AND voltage <= 20),
    current REAL NOT NULL CHECK (current >= -5 AND current <= 10),
    temperature REAL NOT NULL CHECK (temperature >= -20 AND temperature <= 100),
    battery_status TEXT CHECK (
        battery_status IN ('charging', 'discharging', 'full', 'low', 'critical')
    ),
    soc REAL CHECK (soc >= 0 AND soc <= 100),
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- Enable Row Level Security
ALTER TABLE sensor_readings ENABLE ROW LEVEL SECURITY;

-- Index for efficient queries
CREATE INDEX IF NOT EXISTS idx_readings_device_time 
ON sensor_readings(device_name, created_at DESC);
CREATE INDEX IF NOT EXISTS idx_readings_battery_status 
ON sensor_readings(battery_status);
CREATE INDEX IF NOT EXISTS idx_readings_temperature 
ON sensor_readings(temperature) WHERE temperature > 50;

-- RLS Policies
-- Public read access for dashboard
CREATE POLICY "Anyone can read sensor data"
ON sensor_readings FOR SELECT USING (true);

-- Service role can insert/update (device)
CREATE POLICY "Device can insert readings"
ON sensor_readings FOR INSERT WITH CHECK (true);

-- Functions for data aggregation
-- ============================================================================

-- Hourly aggregates function
CREATE OR REPLACE FUNCTION get_hourly_stats(start_time TIMESTAMPTZ, end_time TIMESTAMPTZ)
RETURNS TABLE (
    hour TIMESTAMPTZ,
    avg_voltage REAL,
    min_voltage REAL,
    max_voltage REAL,
    avg_current REAL,
    avg_temperature REAL,
    reading_count BIGINT
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        DATE_TRUNC('hour', created_at) AS hour,
        AVG(voltage) AS avg_voltage,
        MIN(voltage) AS min_voltage,
        MAX(voltage) AS max_voltage,
        AVG(current) AS avg_current,
        AVG(temperature) AS avg_temperature,
        COUNT(*)::BIGINT AS reading_count
    FROM sensor_readings
    WHERE created_at >= start_time AND created_at < end_time
    GROUP BY DATE_TRUNC('hour', created_at)
    ORDER BY hour DESC;
END;
$$ LANGUAGE plpgsql;

-- Daily aggregates function
CREATE OR REPLACE FUNCTION get_daily_stats(days INTEGER DEFAULT 7)
RETURNS TABLE (
    day DATE,
    avg_voltage REAL,
    min_voltage REAL,
    max_voltage REAL,
    total_charge_time INTERVAL,
    total_discharge_time INTERVAL,
    avg_temp REAL,
    reading_count BIGINT
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        DATE(created_at)::DATE AS day,
        AVG(voltage) AS avg_voltage,
        MIN(voltage) AS min_voltage,
        MAX(voltage) AS max_voltage,
        (SELECT SUM(duration) FROM (
            SELECT created_at - LAG(created_at) OVER (ORDER BY created_at) AS duration
            FROM sensor_readings
            WHERE battery_status = 'charging' AND created_at >= NOW() - (days || ' days')::INTERVAL
        ) t WHERE duration > INTERVAL '0') AS total_charge_time,
        (SELECT SUM(duration) FROM (
            SELECT created_at - LAG(created_at) OVER (ORDER BY created_at) AS duration
            FROM sensor_readings
            WHERE battery_status = 'discharging' AND created_at >= NOW() - (days || ' days')::INTERVAL
        ) t WHERE duration > INTERVAL '0') AS total_discharge_time,
        AVG(temperature) AS avg_temp,
        COUNT(*)::BIGINT AS reading_count
    FROM sensor_readings
    WHERE created_at >= NOW() - (days || ' days')::INTERVAL
    GROUP BY DATE(created_at)
    ORDER BY day DESC;
END;
$$ LANGUAGE plpgsql;

-- Battery Health (SOH) calculation
CREATE OR REPLACE FUNCTION calculate_soh(device_name TEXT, days INTEGER DEFAULT 30)
RETURNS TABLE (
    soh_percent REAL,
    cycle_count INTEGER,
    avg_discharge_rate REAL,
    degradation_rate REAL
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        -- Estimate SOH based on capacity loss
        CASE 
            WHEN MAX(voltage) < 12.0 THEN 50  -- Significant degradation
            WHEN MAX(voltage) < 12.5 THEN 80
            ELSE 100
        END AS soh_percent,
        -- Count charge/discharge cycles
        (SELECT COUNT(*) FROM (
            SELECT battery_status, LAG(battery_status) OVER (ORDER BY created_at) AS prev_status
            FROM sensor_readings
            WHERE device_name = calculate_soh.device_name
            AND created_at >= NOW() - (days || ' days')::INTERVAL
        ) t WHERE prev_status = 'discharging' AND battery_status = 'charging') AS cycle_count,
        -- Average discharge rate (voltage drop per hour)
        (SELECT AVG(voltage_rate) FROM (
            SELECT created_at, voltage,
                voltage - LAG(voltage) OVER (ORDER BY created_at) AS voltage_rate
            FROM sensor_readings
            WHERE device_name = calculate_soh.device_name AND battery_status = 'discharging'
            AND created_at >= NOW() - (days || ' days')::INTERVAL
        ) t WHERE voltage_rate < 0) AS avg_discharge_rate,
        -- Degradation rate (voltage loss per cycle)
        (SELECT AVG(degradation) FROM (
            SELECT 
                voltage - LAG(voltage) OVER (PARTITION BY battery_status ORDER BY created_at) AS degradation
            FROM sensor_readings
            WHERE device_name = calculate_soh.device_name
            AND battery_status = 'charging'
            AND created_at >= NOW() - (days || ' days')::INTERVAL
        ) t WHERE degradation IS NOT NULL) AS degradation_rate
    FROM sensor_readings
    WHERE device_name = calculate_soh.device_name
    LIMIT 1;
END;
$$ LANGUAGE plpgsql;

-- Automatic retention policy (delete data older than 90 days)
-- ============================================================================
-- Enable with: SELECT drop_retention_policy('sensor_readings');
-- Then run: SELECT alter_retention_policy('sensor_readings', INTERVAL '90 days');

CREATE OR REPLACE FUNCTION drop_retention_policy(table_name TEXT)
RETURNS void AS $$
BEGIN
    EXECUTE 'ALTER TABLE ' || table_name || ' DROP POLICY IF EXISTS retention_policy';
END;
$$ LANGUAGE plpgsql;

CREATE OR RETENTION POLICY retention_policy ON sensor_readings
FOR DELETE USING (created_at < NOW() - INTERVAL '90 days');

-- Sample data (for testing)
-- ============================================================================
INSERT INTO sensor_readings (device_name, voltage, current, temperature, battery_status, soc, created_at)
VALUES 
    ('Li-ion_PMS_001', 12.6, 1.2, 28.5, 'charging', 85, NOW() - INTERVAL '1 hour'),
    ('Li-ion_PMS_001', 12.5, 1.5, 29.2, 'charging', 80, NOW() - INTERVAL '50 minutes'),
    ('Li-ion_PMS_001', 12.4, 1.8, 30.5, 'charging', 75, NOW() - INTERVAL '40 minutes'),
    ('Li-ion_PMS_001', 12.3, 2.1, 32.1, 'charging', 70, NOW() - INTERVAL '30 minutes'),
    ('Li-ion_PMS_001', 12.2, 2.4, 34.5, 'charging', 65, NOW() - INTERVAL '20 minutes'),
    ('Li-ion_PMS_001', 12.1, 2.8, 36.8, 'charging', 60, NOW() - INTERVAL '10 minutes'),
    ('Li-ion_PMS_001', 11.8, 0.5, 27.3, 'discharging', 55, NOW() - INTERVAL '5 minutes'),
    ('Li-ion_PMS_001', 11.5, 1.2, 28.0, 'discharging', 45, NOW() - INTERVAL '4 minutes'),
    ('Li-ion_PMS_001', 11.2, 1.5, 29.5, 'discharging', 35, NOW() - INTERVAL '3 minutes'),
    ('Li-ion_PMS_001', 10.8, 1.8, 31.0, 'discharging', 25, NOW() - INTERVAL '2 minutes'),
    ('Li-ion_PMS_001', 10.5, 2.0, 32.5, 'discharging', 18, NOW() - INTERVAL '1 minute'),
    ('Li-ion_PMS_001', 12.85, 0.0, 26.0, 'full', 100, NOW());

-- ============================================================================
-- Views for Dashboard
-- ============================================================================

-- Latest readings
CREATE OR REPLACE VIEW latest_readings AS
SELECT * FROM sensor_readings
WHERE device_name = 'Li-ion_PMS_001'
ORDER BY created_at DESC LIMIT 100;

-- Critical alerts (last 24 hours)
CREATE OR REPLACE VIEW critical_alerts AS
SELECT created_at, voltage, temperature, battery_status
FROM sensor_readings
WHERE (temperature > 55 OR voltage < 8.5 OR voltage > 13.0)
AND created_at >= NOW() - INTERVAL '24 hours'
ORDER BY created_at DESC;

-- Health summary
CREATE OR REPLACE VIEW health_summary AS
SELECT 
    device_name,
    MIN(created_at) AS first_reading,
    MAX(created_at) AS last_reading,
    COUNT(*) AS total_readings,
    AVG(voltage) AS avg_voltage,
    MIN(voltage) AS min_voltage,
    MAX(voltage) AS max_voltage,
    AVG(temperature) AS avg_temp,
    MAX(temperature) AS max_temp
FROM sensor_readings
GROUP BY device_name;

-- ============================================================================
-- End of Schema v2.0
-- ============================================================================