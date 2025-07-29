Import("env")

def generate_coverage_report(source, target, env):
    print("Generating coverage report...")
    env.Execute("gcov -o .pio/build/test/test *.cpp")

env.AddPostAction("$BUILD_DIR/${PROGNAME}", generate_coverage_report)
