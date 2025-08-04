#!/usr/bin/env python3

import os
import subprocess
import re
import glob

def count_tests_in_file(filepath):
    """Count TEST_ASSERT and RUN_TEST calls in a test file"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            
        # Count RUN_TEST calls - these are the actual test executions
        run_test_pattern = r'\bRUN_TEST\s*\(\s*(\w+)\s*\)'
        run_tests = re.findall(run_test_pattern, content)
        
        # Count test function definitions
        test_func_pattern = r'\bvoid\s+(test_\w+)\s*\([^)]*\)\s*{'
        test_functions = re.findall(test_func_pattern, content)
        
        return len(run_tests), len(test_functions), run_tests, test_functions
        
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return 0, 0, [], []

def main():
    test_files = []
    
    # Find all test files
    test_patterns = [
        'test/test_*.cpp',
        'test/unit/**/*.cpp',
        'test/integration/**/*.cpp'
    ]
    
    for pattern in test_patterns:
        test_files.extend(glob.glob(pattern, recursive=True))
    
    total_run_tests = 0
    total_test_functions = 0
    
    print("=" * 80)
    print("COMPREHENSIVE TEST COUNT ANALYSIS")
    print("=" * 80)
    
    for test_file in sorted(test_files):
        if os.path.exists(test_file):
            run_count, func_count, run_tests, test_functions = count_tests_in_file(test_file)
            
            if run_count > 0 or func_count > 0:
                print(f"\n{test_file}:")
                print(f"  RUN_TEST calls: {run_count}")
                print(f"  Test functions: {func_count}")
                
                if run_tests:
                    print(f"  Executed tests: {', '.join(run_tests[:5])}" + 
                          ("..." if len(run_tests) > 5 else ""))
                
                total_run_tests += run_count
                total_test_functions += func_count
    
    print("\n" + "=" * 80)
    print(f"SUMMARY:")
    print(f"Total RUN_TEST calls across all files: {total_run_tests}")
    print(f"Total test functions defined: {total_test_functions}")
    print("=" * 80)
    
    # Also try to get current working test count
    print(f"\nCurrent working test configuration: 19 tests (basic test_all.cpp)")
    print(f"Potential additional tests if conflicts resolved: {total_run_tests - 19}")
    print(f"Target test count if all files could run: ~{total_run_tests}")

if __name__ == "__main__":
    main()