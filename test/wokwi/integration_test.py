#!/usr/bin/env python3
"""
Clarity Integration Test Automation
Simple Python test runner for Wokwi-based integration testing
"""

import subprocess
import sys
import os
import time
import re
from pathlib import Path

class ClarityIntegrationTest:
    def __init__(self):
        self.test_dir = Path(__file__).parent
        self.project_root = self.test_dir.parent.parent
        self.firmware_path = self.project_root / ".pio/build/debug-local/firmware.bin"
        self.wokwi_cli = self.find_wokwi_cli()
        
    def find_wokwi_cli(self):
        """Find wokwi-cli executable in common locations"""
        common_paths = [
            "wokwi-cli",  # In PATH
            os.path.expanduser("~/bin/wokwi-cli"),
            os.path.expanduser("~/.wokwi/bin/wokwi-cli"),
        ]
        
        for path in common_paths:
            if subprocess.run(["which", path], capture_output=True).returncode == 0:
                return path
            elif os.path.isfile(path):
                return path
                
        raise FileNotFoundError("wokwi-cli not found in PATH or common locations")
    
    def build_firmware(self):
        """Build the firmware using PlatformIO"""
        print("🔨 Building firmware...")
        result = subprocess.run(
            ["pio", "run", "-e", "debug-local"],
            cwd=self.project_root,
            capture_output=True,
            text=True
        )
        
        if result.returncode != 0:
            print(f"❌ Build failed:\n{result.stderr}")
            return False
            
        if not self.firmware_path.exists():
            print(f"❌ Firmware not found at {self.firmware_path}")
            return False
            
        print(f"✅ Firmware built: {self.firmware_path}")
        return True
    
    def run_wokwi_test(self, timeout=300000):
        """Run the Wokwi integration test"""
        print(f"🚀 Starting Wokwi simulation (timeout: {timeout}ms)...")
        
        # Change to test directory for wokwi.toml
        os.chdir(self.test_dir)
        
        cmd = [self.wokwi_cli, "--timeout", str(timeout)]
        print(f"Command: {' '.join(cmd)}")
        print("\n📡 Serial Monitor Output:")
        print("=" * 50)
        
        # Track test phases based on serial patterns
        test_phases = {
            "startup": False,
            "sensor_interaction": False,  
            "theme_trigger": False,
            "error_system": False,
            "configuration": False
        }
        
        try:
            process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                universal_newlines=True,
                bufsize=1
            )
            
            # Monitor serial output
            for line in process.stdout:
                print(line, end='')
                
                # Track test phase completion
                self.update_test_phases(line, test_phases)
                
            process.wait()
            
            print("\n" + "=" * 50)
            print("📊 Test Phase Summary:")
            self.print_test_summary(test_phases)
            
            if process.returncode == 0:
                print("✅ Integration test completed successfully!")
                return True
            else:
                print("❌ Integration test failed or timed out")
                return False
                
        except KeyboardInterrupt:
            print("\n⏹️ Test interrupted by user")
            process.terminate()
            return False
        except Exception as e:
            print(f"❌ Error running test: {e}")
            return False
    
    def update_test_phases(self, line, phases):
        """Update test phase tracking based on serial patterns"""
        patterns = {
            "startup": [
                "SplashPanel loaded successfully",
                "OemOilPanel loaded successfully"
            ],
            "sensor_interaction": [
                "Pressure reading changed",
                "Temperature reading changed",
                "Gauge animation"
            ],
            "theme_trigger": [
                "Theme changed to Night",
                "KeyPresentActivate",
                "LockEngagedActivate",
                "Theme changed to Day"
            ],
            "error_system": [
                "ErrorOccurredActivate",
                "Error navigation",
                "Error resolution"
            ],
            "configuration": [
                "Config panel",
                "Theme configuration",
                "Config exit"
            ]
        }
        
        for phase, phase_patterns in patterns.items():
            if not phases[phase]:
                for pattern in phase_patterns:
                    if pattern in line:
                        phases[phase] = True
                        break
    
    def print_test_summary(self, phases):
        """Print summary of completed test phases"""
        for phase, completed in phases.items():
            status = "✅" if completed else "❌"
            print(f"  {status} {phase.replace('_', ' ').title()}")
        
        completed_count = sum(phases.values())
        total_count = len(phases)
        print(f"\n📈 Overall Progress: {completed_count}/{total_count} phases completed")
    
    def run_full_test(self):
        """Run the complete integration test"""
        print("🧪 Clarity Integration Test")
        print("=" * 40)
        
        # Check if firmware exists, build if needed
        if not self.firmware_path.exists():
            if not self.build_firmware():
                return False
        else:
            print(f"✅ Using existing firmware: {self.firmware_path}")
        
        # Run the test
        return self.run_wokwi_test()

def main():
    """Main entry point"""
    try:
        test = ClarityIntegrationTest()
        success = test.run_full_test()
        sys.exit(0 if success else 1)
    except Exception as e:
        print(f"❌ Test setup failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()