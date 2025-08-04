#!/usr/bin/env python3

import os
import re
import glob

def extract_test_functions(filepath):
    """Extract test function names and runner functions from a test file"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            
        # Find test function definitions
        test_func_pattern = r'void\s+(test_\w+)\s*\([^)]*\)\s*{'
        test_functions = re.findall(test_func_pattern, content)
        
        # Find setup/teardown functions
        setup_pattern = r'void\s+(setUp_\w+)\s*\([^)]*\)\s*{'
        setup_functions = re.findall(setup_pattern, content)
        
        teardown_pattern = r'void\s+(tearDown_\w+)\s*\([^)]*\)\s*{'
        teardown_functions = re.findall(teardown_pattern, content)
        
        # Find runner function
        runner_pattern = r'void\s+(run\w+Tests?)\s*\([^)]*\)\s*{'
        runner_functions = re.findall(runner_pattern, content)
        
        # Find RUN_TEST calls
        run_test_pattern = r'RUN_TEST\s*\(\s*(\w+)\s*\)'
        run_tests = re.findall(run_test_pattern, content)
        
        return {
            'test_functions': test_functions,
            'setup_functions': setup_functions,
            'teardown_functions': teardown_functions,
            'runner_functions': runner_functions,
            'run_tests': run_tests
        }
        
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return {}

def main():
    test_files = glob.glob('test/unit/**/*.cpp', recursive=True)
    test_files.extend(glob.glob('test/test_*.cpp'))
    
    all_functions = {
        'test_functions': [],
        'setup_functions': [],
        'teardown_functions': [],
        'runner_functions': [],
        'run_tests': []
    }
    
    print("EXTRACTING TEST FUNCTIONS FOR UNIFIED RUNNER")
    print("=" * 60)
    
    for test_file in sorted(test_files):
        if os.path.exists(test_file) and 'test_all.cpp' not in test_file:
            result = extract_test_functions(test_file)
            
            if result and any(result.values()):
                print(f"\n{test_file}:")
                
                if result.get('setup_functions'):
                    print(f"  Setup: {', '.join(result['setup_functions'])}")
                    all_functions['setup_functions'].extend(result['setup_functions'])
                
                if result.get('teardown_functions'):
                    print(f"  Teardown: {', '.join(result['teardown_functions'])}")
                    all_functions['teardown_functions'].extend(result['teardown_functions'])
                
                if result.get('runner_functions'):
                    print(f"  Runner: {', '.join(result['runner_functions'])}")
                    all_functions['runner_functions'].extend(result['runner_functions'])
                
                if result.get('run_tests'):
                    print(f"  Tests executed: {len(result['run_tests'])} ({', '.join(result['run_tests'][:3])}{'...' if len(result['run_tests']) > 3 else ''})")
                    all_functions['run_tests'].extend(result['run_tests'])
                
                if result.get('test_functions'):
                    print(f"  Functions defined: {len(result['test_functions'])}")
                    all_functions['test_functions'].extend(result['test_functions'])
    
    print(f"\n" + "=" * 60)
    print(f"SUMMARY:")
    print(f"Total test functions: {len(all_functions['test_functions'])}")
    print(f"Total executed tests: {len(all_functions['run_tests'])}")
    print(f"Setup functions: {len(all_functions['setup_functions'])}")
    print(f"Teardown functions: {len(all_functions['teardown_functions'])}")
    print(f"Runner functions: {len(all_functions['runner_functions'])}")
    
    # Generate extern declarations
    print(f"\n" + "=" * 60)
    print("EXTERN DECLARATIONS FOR UNIFIED RUNNER:")
    print("=" * 60)
    
    for func in sorted(set(all_functions['setup_functions'])):
        print(f"extern void {func}();")
    
    for func in sorted(set(all_functions['teardown_functions'])):
        print(f"extern void {func}();")
    
    for func in sorted(set(all_functions['test_functions'])):
        print(f"extern void {func}();")
    
    for func in sorted(set(all_functions['runner_functions'])):
        print(f"extern void {func}();")

if __name__ == "__main__":
    main()