#!/usr/bin/env python3

"""
Test runner with coverage reporting for Clarity project.
Implements comprehensive testing requirements from docs/todo.md
"""

import os
import sys
import subprocess
import json
import argparse
from pathlib import Path

class CoverageTestRunner:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.build_dir = self.project_root / ".pio" / "build" / "test-coverage"
        self.coverage_dir = self.project_root / "coverage"
        
    def setup_coverage_environment(self):
        """Setup coverage compilation flags and directories"""
        print("Setting up coverage environment...")
        
        # Create coverage directory
        self.coverage_dir.mkdir(exist_ok=True)
        
        # Coverage flags for PlatformIO
        coverage_flags = [
            "-fprofile-arcs",
            "-ftest-coverage",
            "--coverage",
            "-fno-inline",
            "-fno-inline-small-functions",
            "-fno-default-inline"
        ]
        
        return coverage_flags
    
    def run_unit_tests(self):
        """Run all unit tests with coverage"""
        print("Running unit tests with coverage...")
        
        try:
            # Run PlatformIO test with coverage flags
            cmd = [
                "pio.exe", "test", 
                "-e", "test-coverage",  # Change to "test" for simplified setup
                "--verbose"
            ]
            
            result = subprocess.run(
                cmd, 
                cwd=self.project_root,
                capture_output=True,
                text=True
            )
            
            print(f"Unit test output:\\n{result.stdout}")
            if result.stderr:
                print(f"Unit test errors:\\n{result.stderr}")
                
            return result.returncode == 0
            
        except subprocess.CalledProcessError as e:
            print(f"Error running unit tests: {e}")
            return False
    
    def run_integration_tests(self):
        """Run integration tests"""
        print("Running integration tests...")
        
        try:
            cmd = [
                "pio.exe", "test",
                "-e", "test-integration",
                "--verbose"
            ]
            
            result = subprocess.run(
                cmd,
                cwd=self.project_root,
                capture_output=True,
                text=True
            )
            
            print(f"Integration test output:\\n{result.stdout}")
            if result.stderr:
                print(f"Integration test errors:\\n{result.stderr}")
                
            return result.returncode == 0
            
        except subprocess.CalledProcessError as e:
            print(f"Error running integration tests: {e}")
            return False
    
    def generate_coverage_report(self):
        """Generate coverage reports using gcov and lcov"""
        print("Generating coverage report...")
        
        try:
            # Find all .gcno and .gcda files
            gcov_files = list(self.build_dir.rglob("*.gcno"))
            
            if not gcov_files:
                print("No coverage files found. Make sure tests ran with coverage flags.")
                return False
            
            # Run gcov on all source files
            for gcov_file in gcov_files:
                src_file = gcov_file.with_suffix('.cpp')
                if src_file.exists():
                    subprocess.run([
                        "gcov", 
                        str(gcov_file),
                        "-o", str(gcov_file.parent)
                    ], cwd=self.project_root, capture_output=True)
            
            # Generate simple coverage report with gcov only
            print(f"Found {len(gcov_files)} coverage files")
            
            # Create simple text report
            report_file = self.coverage_dir / "coverage_report.txt"
            with open(report_file, 'w') as f:
                f.write("CLARITY PROJECT COVERAGE REPORT\\n")
                f.write("=" * 50 + "\\n\\n")
                f.write("Coverage data found for test files\\n")
                f.write(f"Total coverage files: {len(gcov_files)}\\n")
                for gcov_file in gcov_files:
                    f.write(f"- {gcov_file.name}\\n")
            
            print(f"Coverage report generated: {report_file}")
            return True
            
        except Exception as e:
            print(f"Error generating coverage report: {e}")
            return False
    
    def parse_coverage_results(self):
        """Parse coverage results and return metrics"""
        coverage_file = self.coverage_dir / "coverage_filtered.info"
        
        if not coverage_file.exists():
            return None
            
        try:
            result = subprocess.run([
                "lcov", "--summary", str(coverage_file)
            ], capture_output=True, text=True)
            
            # Parse lcov summary output
            lines = result.stdout.split('\\n')
            coverage_data = {}
            
            for line in lines:
                if 'lines......:' in line:
                    parts = line.split()
                    coverage_data['line_coverage'] = float(parts[1].rstrip('%'))
                elif 'functions..:' in line:
                    parts = line.split()
                    coverage_data['function_coverage'] = float(parts[1].rstrip('%'))
                elif 'branches...:' in line:
                    parts = line.split()  
                    coverage_data['branch_coverage'] = float(parts[1].rstrip('%'))
            
            return coverage_data
            
        except Exception as e:
            print(f"Error parsing coverage results: {e}")
            return None
    
    def validate_coverage_thresholds(self, coverage_data):
        """Validate coverage meets minimum thresholds"""
        if not coverage_data:
            return False
            
        thresholds = {
            'line_coverage': 85.0,
            'function_coverage': 95.0, 
            'branch_coverage': 80.0
        }
        
        passed = True
        print("\\nCoverage Results:")
        print("-" * 40)
        
        for metric, threshold in thresholds.items():
            actual = coverage_data.get(metric, 0.0)
            status = "PASS" if actual >= threshold else "FAIL"
            print(f"{metric}: {actual:.1f}% (threshold: {threshold:.1f}%) - {status}")
            
            if actual < threshold:
                passed = False
        
        return passed
    
    def run_performance_tests(self):
        """Run performance benchmarks"""
        print("Running performance tests...")
        
        try:
            cmd = [
                "pio.exe", "test",
                "-e", "test-performance",
                "--verbose"
            ]
            
            result = subprocess.run(
                cmd,
                cwd=self.project_root,
                capture_output=True,
                text=True
            )
            
            print(f"Performance test output:\\n{result.stdout}")
            if result.stderr:
                print(f"Performance test errors:\\n{result.stderr}")
                
            return result.returncode == 0
            
        except subprocess.CalledProcessError as e:
            print(f"Error running performance tests: {e}")
            return False
    
    def run_memory_tests(self):
        """Run memory tests with sanitizers"""
        print("Running memory tests...")
        
        try:
            cmd = [
                "pio.exe", "test",
                "-e", "test-memory", 
                "--verbose"
            ]
            
            result = subprocess.run(
                cmd,
                cwd=self.project_root,
                capture_output=True,
                text=True
            )
            
            print(f"Memory test output:\\n{result.stdout}")
            if result.stderr:
                print(f"Memory test errors:\\n{result.stderr}")
                
            return result.returncode == 0
            
        except subprocess.CalledProcessError as e:
            print(f"Error running memory tests: {e}")
            return False
    
    def generate_test_report(self, unit_result, integration_result, coverage_data, performance_result=True, memory_result=True):
        """Generate comprehensive test report"""
        report = {
            "test_results": {
                "unit_tests": "PASS" if unit_result else "FAIL",
                "integration_tests": "PASS" if integration_result else "FAIL",
                "performance_tests": "PASS" if performance_result else "FAIL",
                "memory_tests": "PASS" if memory_result else "FAIL"
            },
            "coverage": coverage_data or {},
            "timestamp": subprocess.run(
                ["date", "-Iseconds"], 
                capture_output=True, 
                text=True
            ).stdout.strip()
        }
        
        report_file = self.coverage_dir / "test_report.json"
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"\\nTest report saved: {report_file}")
        return report
    
    def run_all_tests(self, skip_performance=False, skip_memory=False):
        """Run complete test suite with coverage"""
        print("=" * 60)
        print("CLARITY COMPREHENSIVE TEST SUITE")  
        print("=" * 60)
        
        # Setup
        self.setup_coverage_environment()
        
        # Run test categories
        unit_result = self.run_unit_tests()
        integration_result = self.run_integration_tests()
        
        # Optional test categories
        performance_result = True  # Default to pass if skipped
        memory_result = True       # Default to pass if skipped
        
        if not skip_performance:
            performance_result = self.run_performance_tests()
        else:
            print("Skipping performance tests...")
            
        if not skip_memory:
            memory_result = self.run_memory_tests()
        else:
            print("Skipping memory tests...")
        
        # Generate coverage
        coverage_generated = self.generate_coverage_report()
        coverage_data = self.parse_coverage_results() if coverage_generated else None
        
        # Validate coverage thresholds
        coverage_passed = self.validate_coverage_thresholds(coverage_data)
        
        # Generate report
        report = self.generate_test_report(unit_result, integration_result, coverage_data, performance_result, memory_result)
        
        # Summary
        print("\\n" + "=" * 60)
        print("TEST SUMMARY")
        print("=" * 60)
        print(f"Unit Tests: {'PASS' if unit_result else 'FAIL'}")
        print(f"Integration Tests: {'PASS' if integration_result else 'FAIL'}")
        print(f"Performance Tests: {'PASS' if performance_result else 'FAIL'}")
        print(f"Memory Tests: {'PASS' if memory_result else 'FAIL'}")
        print(f"Coverage Generated: {'PASS' if coverage_generated else 'FAIL'}")
        print(f"Coverage Thresholds: {'PASS' if coverage_passed else 'FAIL'}")
        
        # Overall result
        overall_result = (unit_result and 
                         integration_result and
                         performance_result and
                         memory_result and
                         coverage_generated and 
                         coverage_passed)
        
        print(f"\\nOVERALL RESULT: {'PASS' if overall_result else 'FAIL'}")
        
        return 0 if overall_result else 1

def main():
    parser = argparse.ArgumentParser(
        description="Run comprehensive tests with coverage for Clarity project"
    )
    parser.add_argument(
        "--project-root", 
        default=".",
        help="Path to project root directory"
    )
    parser.add_argument(
        "--unit-only",
        action="store_true",
        help="Run only unit tests"
    )
    parser.add_argument(
        "--integration-only", 
        action="store_true",
        help="Run only integration tests"
    )
    parser.add_argument(
        "--no-coverage",
        action="store_true", 
        help="Skip coverage generation"
    )
    parser.add_argument(
        "--performance-only",
        action="store_true",
        help="Run only performance tests"
    )
    parser.add_argument(
        "--memory-only", 
        action="store_true",
        help="Run only memory tests"
    )
    parser.add_argument(
        "--skip-performance",
        action="store_true",
        help="Skip performance tests in full run"
    )
    parser.add_argument(
        "--skip-memory",
        action="store_true", 
        help="Skip memory tests in full run"
    )
    
    args = parser.parse_args()
    
    runner = CoverageTestRunner(args.project_root)
    
    if args.unit_only:
        success = runner.run_unit_tests()
        return 0 if success else 1
    elif args.integration_only:
        success = runner.run_integration_tests()
        return 0 if success else 1
    elif args.performance_only:
        success = runner.run_performance_tests()
        return 0 if success else 1
    elif args.memory_only:
        success = runner.run_memory_tests()
        return 0 if success else 1
    else:
        # Modify run_all_tests to respect skip flags
        return runner.run_all_tests(
            skip_performance=args.skip_performance,
            skip_memory=args.skip_memory
        )

if __name__ == "__main__":
    sys.exit(main())