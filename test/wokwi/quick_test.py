#!/usr/bin/env python3
"""
Clarity Wokwi Integration Test Quick Runner
Simplified Python script for running Wokwi integration tests
"""

import subprocess
import time
import sys
import os

def run_command(cmd, description, timeout=None):
    """Run a command and return success status"""
    print(f"🔄 {description}...")
    try:
        result = subprocess.run(
            cmd, 
            shell=True, 
            capture_output=True, 
            text=True, 
            timeout=timeout,
            cwd=os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
        )
        
        if result.returncode == 0:
            print(f"✅ {description} - Success")
            return True
        else:
            print(f"❌ {description} - Failed")
            if result.stderr:
                print(f"Error: {result.stderr}")
            return False
    except subprocess.TimeoutExpired:
        print(f"⏰ {description} - Timeout")
        return False
    except Exception as e:
        print(f"❌ {description} - Exception: {e}")
        return False

def main():
    print("=" * 50)
    print("Clarity Wokwi Integration Test Runner")
    print("=" * 50)
    
    # Check prerequisites
    print("\n📋 Checking Prerequisites...")
    
    # Check PlatformIO
    if not run_command("pio --version", "PlatformIO installation check"):
        print("Install PlatformIO: https://platformio.org/install")
        return False
    
    # Check Wokwi CLI
    if not run_command("wokwi-cli --version", "Wokwi CLI installation check"):
        print("Install Wokwi CLI: npm install -g @wokwi/cli")
        return False
    
    print("\n🔨 Building Test Firmware...")
    
    # Build the firmware
    if not run_command(
        "pio test -e test-wokwi --without-uploading --without-testing", 
        "Firmware build", 
        timeout=120
    ):
        return False
    
    print("\n🚀 Running Integration Test...")
    print("⏱️  Expected duration: ~7 minutes")
    print("📊 Progress will be shown via serial output...")
    print()
    
    # Run Wokwi simulation
    success = run_command(
        "cd test/wokwi && wokwi-cli run --timeout 420000 diagram.json ../../.pio/build/test-wokwi/firmware.bin",
        "Wokwi integration test",
        timeout=450
    )
    
    print("\n" + "=" * 50)
    if success:
        print("🎉 Integration Test COMPLETED!")
        print("✅ Check the output above for detailed results")
    else:
        print("❌ Integration Test FAILED!")
        print("📋 Check error messages above for troubleshooting")
    
    print("=" * 50)
    return success

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)