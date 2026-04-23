-- ============================================================================
-- Li-ion BMS - Database Schema (Optimized)
-- ============================================================================
-- Note: For production, create material views separately to avoid 
-- performance issues with large datasets
-- ============================================================================

-- Main sensor readings table
CREATE TABLE IF NOT EXISTS sensor_readings (
    id BIGSERIAL PRIMARY KEY,
    device_name TEXT NOT NULL DEFAULT 'Li-ion_PMS_001',
    voltage REAL NOT NULL CHECK (voltage >= 0 AND voltage <= 20),
    current REAL NOT NULL CHECK (current >= -5 AND current <= 10),
    temperature REAL NOT NULL CHECK (temperature >= -20 AND temperature <= 100),
    battery_status TEXT CHECK (battery_status IN ('charging', 'discharging', 'full', 'low', 'critical')),
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- Basic indices
CREATE INDEX IF NOT EXISTS idx_readings_device_time ON sensor_readings(device_name, created_at DESC);
CREATE INDEX IF NOT EXISTS idx_readings_status ON sensor_readings(battery_status);

-- RLS
ALTER TABLE sensor_readings ENABLE ROW LEVEL SECURITY;
CREATE POLICY "Public read" ON sensor_readings FOR SELECT USING (true);
CREATE POLICY "Device insert" ON sensor_readings FOR INSERT WITH CHECK (true);

-- ============================================================================
-- MATERIALIZED VIEW: Must be created manually for performance
-- Run this in Supabase SQL Editor:
-- ============================================================================
/*
CREATE MATERIALIZED VIEW hourly_stats AS
SELECT 
    DATE_TRUNC('hour', created_at) AS hour,
    device_name,
    AVG(voltage) AS avg_voltage,
    MIN(voltage) AS min_voltage,
    MAX(voltage) AS max_voltage,
    AVG(current) AS avg_current,
    AVG(temperature) AS avg_temperature,
    COUNT(*) AS reading_count
FROM sensor_readings
WHERE created_at > NOW() - INTERVAL '7 days'
GROUP BY DATE_TRUNC('hour', created_at), device_name
WITH DATA;

CREATE UNIQUE INDEX ON hourly_stats(hour, device_name);

-- Refresh hourly stats (schedule this via Edge Function or cron)
*/
-- ============================================================================

-- Sample INSERT (for testing)
INSERT INTO sensor_readings (voltage, current, temperature, battery_status)
VALUES (12.6, 1.2, 28.5, 'charging');

-- Note: Due to limitations in this demo, full functions are not included.
-- For production, implement materialized views in Supabase dashboard.