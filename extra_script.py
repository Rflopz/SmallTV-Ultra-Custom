Import("env")
import os

# Try to find .env file
try:
    with open(".env", "r") as f:
        lines = f.readlines()
        for line in lines:
            if "=" in line:
                key, value = line.strip().split("=", 1)
                # Remove quotes if present, assuming simple key=value or key="value"
                value = value.strip('"').strip("'")
                
                # Check for WIFI_SSID and WIFI_PASSWORD specifically or generic injection
                if key in ["WIFI_SSID", "WIFI_PASSWORD"]:
                    env.Append(CPPDEFINES=[(key, '\\"' + value + '\\"')])
                    print(f"Injected {key} from .env")
except IOError:
    print("Warning: .env file not found. Skipping environment variable injection.")
