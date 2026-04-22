-- ============================================================================
-- Li-ion Battery Performance Monitoring System - Database Schema
-- ============================================================================
-- This SQL creates the sensor_readings table in Supabase PostgreSQL
--
-- Usage:
-- 1. Open Supabase SQL Editor
-- 2. Copy and run this script
-- 3. Note down your URL and anon key from Settings > API
-- ============================================================================

-- Create sensor_readings table
CREATE TABLE IF NOT EXISTS sensor_readings (
    id BIGSERIAL PRIMARY KEY,
    device_name TEXT DEFAULT 'Li-ion_PMS_001',
    voltage REAL NOT NULL,
    current REAL NOT NULL,
    temperature REAL NOT NULL,
    battery_status TEXT CHECK (battery_status IN ('charging', 'discharging', 'full', 'low')),
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- Enable Row Level Security (RLS)
ALTER TABLE sensor_readings ENABLE ROW LEVEL SECURITY;

-- Create policy for anonymous reads (for dashboard)
CREATE POLICY "Allow public read access"
ON sensor_readings
FOR SELECT
USING (true);

-- Create policy for authenticated inserts (for device)
CREATE POLICY "Allow service role insert"
ON sensor_readings
FOR INSERT
WITH CHECK (true);

-- Create policy for authenticated updates (for device)
CREATE POLICY "Allow service role update"
ON sensor_readings
FOR UPDATE
USING (true);

-- Create policy for authenticated deletes (for device)
CREATE POLICY "Allow service role delete"
ON sensor_readings
FOR DELETE
USING (true);

-- Create index on created_at for faster queries
CREATE INDEX IF NOT EXISTS idx_sensor_readings_created_at
ON sensor_readings(created_at DESC);

-- Create index on device_name
CREATE INDEX IF NOT EXISTS idx_sensor_readings_device
ON sensor_readings(device_name);

-- Create index on battery_status
CREATE INDEX IF NOT EXISTS idx_sensor_readings_status
ON sensor_readings(battery_status);

-- Insert sample data (for testing dashboard)
INSERT INTO sensor_readings (device_name, voltage, current, temperature, battery_status, created_at)
VALUES
    ('Li-ion_PMS_001', 12.6, 1.2, 28.5, 'charging', NOW() - INTERVAL '1 hour'),
    ('Li-ion_PMS_001', 12.5, 1.5, 29.0, 'charging', NOW() - INTERVAL '50 minutes'),
    ('Li-ion_PMS_001', 12.4, 1.8, 30.2, 'charging', NOW() - INTERVAL '40 minutes'),
    ('Li-ion_PMS_001', 12.3, 2.1, 32.1, 'charging', NOW() - INTERVAL '30 minutes'),
    ('Li-ion_PMS_001', 12.2, 2.4, 34.5, 'charging', NOW() - INTERVAL '20 minutes'),
    ('Li-ion_PMS_001', 12.1, 2.8, 36.8, 'charging', NOW() - INTERVAL '10 minutes'),
    ('Li-ion_PMS_001', 12.85, 0.5, 27.3, 'full', NOW() - INTERVAL '5 minutes');

-- ============================================================================
-- View for latest readings (optional)
-- ============================================================================

CREATE OR REPLACE VIEW latest_readings AS
SELECT
    device_name,
    voltage,
    current,
    temperature,
    battery_status,
    created_at
FROM sensor_readings
WHERE device_name = 'Li-ion_PMS_001'
ORDER BY created_at DESC
LIMIT 10;

-- ============================================================================
-- Function to get hourly averages (for analytics)
-- ============================================================================

CREATE OR REPLACE VIEW hourly_averages AS
SELECT
    device_name,
    DATE_TRUNC('hour', created_at) AS hour,
    AVG(voltage) AS avg_voltage,
    AVG(current) AS avg_current,
    AVG(temperature) AS avg_temperature,
    COUNT(*) AS reading_count
FROM sensor_readings
WHERE created_at > NOW() - INTERVAL '24 hours'
GROUP BY device_name, DATE_TRUNC('hour', created_at)
ORDER BY hour DESC;

-- ============================================================================
-- End of Schema
-- ============================================================================