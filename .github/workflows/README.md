# Clarity GitHub Actions Workflows

This directory contains GitHub Actions workflows for automated testing of the Clarity digital gauge system using Wokwi hardware simulation.

## Workflows Overview

### 1. Basic Hardware Test (`wokwi-basic-test.yml`)
**Trigger**: Every push to main branches and feature/fix branches
**Duration**: ~30 seconds
**Purpose**: Quick hardware validation

**What it tests:**
- GPIO pin initialization
- Button press simulation (4 buttons)
- Analog sensor simulation (pressure/temperature)
- Timing validation
- Long press detection

### 2. Full System Integration Test (`wokwi-full-test.yml`)
**Trigger**: Pull requests to main/develop branches
**Duration**: ~3 minutes
**Purpose**: Complete system validation

**What it tests:**
- All 7 phases of system integration
- Panel lifecycle management (6 panels)
- Trigger system with priorities
- Theme switching (Day/Night)
- Animation system
- Error handling and recovery
- Configuration management
- User interaction flows

### 3. Release Validation Tests (`wokwi-release-test.yml`)
**Trigger**: Release branches and tags
**Duration**: ~4 minutes (both tests)
**Purpose**: Pre-release validation

**What it tests:**
- Runs both basic and full tests
- Comprehensive validation before release
- Manual trigger option for specific test types

## Setup Requirements

### 1. Repository Secrets

The workflows require the following secret to be configured in your GitHub repository:

```
WOKWI_CLI_TOKEN
```

**How to set up:**
1. Go to [Wokwi CI Dashboard](https://wokwi.com/dashboard/ci)
2. Generate a CLI token for your account
3. In your GitHub repository, go to Settings → Secrets and variables → Actions
4. Add a new repository secret named `WOKWI_CLI_TOKEN` with your token value

### 2. Branch Protection Rules (Recommended)

To ensure code quality, set up branch protection rules:

**For `main` branch:**
- Require status checks to pass before merging
- Require "Wokwi Basic Hardware Test" to pass
- Require "Full System Integration Test" to pass (for PRs)

**For `develop` branch:**
- Require status checks to pass before merging
- Require "Wokwi Basic Hardware Test" to pass

## Workflow Behavior

### Commit Push Workflow
```
Push to main/develop/feature/* → Basic Test (30s) → ✅/❌
```

### Pull Request Workflow
```
Open/Update PR → Basic Test (30s) + Full Test (3min) → Comment on PR → ✅/❌
```

### Release Workflow
```
Push to release/* → Basic Test + Full Test (4min) → ✅/❌
Tag v*.*.* → Basic Test + Full Test (4min) → ✅/❌
```

## Workflow Features

### ✅ **Optimized Performance**
- PlatformIO caching for faster builds
- Appropriate timeouts for each test type
- Parallel test execution for releases

### ✅ **Smart Triggering**
- Basic tests on all commits for quick feedback
- Full tests only on PRs to avoid excessive resource usage
- Skips draft PRs unless explicitly requested

### ✅ **Comprehensive Reporting**
- Detailed test output in workflow logs
- Automatic PR comments with test results
- Build artifacts uploaded on failures

### ✅ **Debugging Support**
- Uploads build artifacts on test failures
- Retains test logs for analysis
- Manual trigger options for troubleshooting

## Troubleshooting

### Common Issues

**Test fails with "WOKWI_CLI_TOKEN not set":**
- Verify the secret is configured in repository settings
- Ensure the secret name matches exactly: `WOKWI_CLI_TOKEN`

**Build fails with PlatformIO errors:**
- Check `platformio.ini` configuration
- Verify all required dependencies are specified
- Check the build logs for specific error details

**Wokwi simulation timeout:**
- Check if the timeout values are appropriate
- Verify the firmware boots and runs correctly
- Look for infinite loops or blocking operations in test code

**Test passes locally but fails in CI:**
- Environment differences (paths, permissions)
- Check the uploaded artifacts for debugging
- Verify all dependencies are properly installed

### Manual Test Execution

You can manually trigger workflows from the GitHub Actions tab:

1. Go to Actions → Select workflow
2. Click "Run workflow"
3. Choose branch and options (if available)
4. Click "Run workflow"

## Maintenance

### Updating Test Timeouts
If test performance changes, update timeouts in:
- Workflow files (`timeout-minutes`)
- Test script (`TIMEOUT` values)
- Documentation

### Adding New Test Types
1. Create new PlatformIO environment in `platformio.ini`
2. Update test script to support new test mode
3. Add workflow or extend existing ones
4. Update documentation

### Performance Monitoring
Monitor workflow execution times and adjust timeouts if needed:
- Basic tests should complete in < 1 minute
- Full tests should complete in < 5 minutes
- Release tests should complete in < 10 minutes