import requests
import time
import sys
import math

# --- CONFIGURATION ---
API_KEY = "8uhhAvWqcgxWHMBuvjnK"
ZONE = "US-NY-NYIS"  # New York ISO Zone Key
UPDATE_INTERVAL = 2  # Seconds

def get_carbon_intensity():
    """Fetches real-time carbon data or returns a mock fallback."""
    url = f"https://api.electricitymap.org/v3/carbon-intensity/latest?zone={ZONE}"
    headers = {"auth-token": API_KEY}
    
    try:
        response = requests.get(url, headers=headers, timeout=5)
        if response.status_code == 200:
            data = response.json()
            return data['carbonIntensity']
        else:
            raise Exception("API Error")
    except Exception:
        # FALLBACK: Simulate a 24-hour cycle (200-400g range)
        # Peak carbon usually occurs during evening ramps
        simulated_time = time.time() / 3600  # Hours since epoch
        mock_intensity = 300 + 100 * math.sin(simulated_time * (2 * math.pi / 24))
        return round(mock_intensity, 2)

def main():
    try:
        while True:
            intensity = get_carbon_intensity()
            # Send to stdout for C++ to read via pipe
            # We use a unique prefix so C++ can distinguish tasks from telemetry
            print(f"CARBON {intensity}", flush=True)
            time.sleep(UPDATE_INTERVAL)
    except KeyboardInterrupt:
        sys.exit(0)

if __name__ == "__main__":
    main()