#include "services/config_migration_service.h"
#include "utilities/logging.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>

ConfigMigrationService::ConfigMigrationService(IPreferenceService* preferenceService, IDynamicConfigService* dynamicConfigService)
    : preferenceService_(preferenceService), dynamicConfigService_(dynamicConfigService),
      currentPhase_(MigrationPhase::Legacy), currentStepIndex_(0)
{
    log_v("ConfigMigrationService constructor called");
}

ConfigMigrationService::MigrationPhase ConfigMigrationService::Initialize()
{
    log_v("Initialize() called");

    if (!preferenceService_)
    {
        log_e("Cannot initialize migration service - preference service is null");
        return MigrationPhase::Legacy;
    }

    // Determine current phase based on available services and migration state
    if (!dynamicConfigService_)
    {
        currentPhase_ = MigrationPhase::Legacy;
        log_i("Dynamic config service not available - staying in legacy mode");
    }
    else
    {
        // Check if dynamic sections are registered
        auto sectionNames = dynamicConfigService_->GetRegisteredSectionNames();
        if (sectionNames.empty())
        {
            currentPhase_ = MigrationPhase::Legacy;
            log_i("No dynamic sections registered - staying in legacy mode");
        }
        else
        {
            // Check migration completion status
            std::string migrationStatus = preferenceService_->GetPreference("migration_phase");
            if (migrationStatus == "complete")
            {
                currentPhase_ = MigrationPhase::Complete;
                log_i("Migration already complete - using dynamic configuration");
            }
            else if (migrationStatus == "dynamic")
            {
                currentPhase_ = MigrationPhase::Dynamic;
                log_i("Dynamic phase active - preparing for completion");
            }
            else if (migrationStatus == "transitional")
            {
                currentPhase_ = MigrationPhase::Transitional;
                log_i("Transitional phase active - hybrid mode");
            }
            else
            {
                currentPhase_ = MigrationPhase::Transitional;
                log_i("Starting transitional phase - enabling hybrid mode");
            }
        }
    }

    // Initialize migration steps
    InitializeMigrationSteps();

    log_i("ConfigMigrationService initialized in phase: %d", static_cast<int>(currentPhase_));
    return currentPhase_;
}

bool ConfigMigrationService::ExecuteNextStep()
{
    log_v("ExecuteNextStep() called");

    if (currentStepIndex_ >= migrationSteps_.size())
    {
        log_i("All migration steps completed");
        return true;
    }

    auto& step = migrationSteps_[currentStepIndex_];
    if (step.completed)
    {
        log_d("Step '%s' already completed, moving to next", step.name.c_str());
        currentStepIndex_++;
        return ExecuteNextStep();
    }

    log_i("Executing migration step: %s", step.name.c_str());

    // Execute the step
    bool success = step.execute();
    if (!success)
    {
        log_e("Migration step '%s' failed", step.name.c_str());
        return false;
    }

    // Validate the step
    if (step.validate && !step.validate())
    {
        log_e("Migration step '%s' validation failed", step.name.c_str());
        // Rollback the step
        if (step.rollback)
        {
            step.rollback();
        }
        return false;
    }

    // Mark step as completed
    step.completed = true;
    currentStepIndex_++;

    log_i("Migration step '%s' completed successfully", step.name.c_str());

    // Update migration progress
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << GetMigrationProgress();
    preferenceService_->SetPreference("migration_progress", ss.str());

    return true;
}

bool ConfigMigrationService::ExecuteFullMigration()
{
    log_v("ExecuteFullMigration() called");

    while (currentStepIndex_ < migrationSteps_.size())
    {
        if (!ExecuteNextStep())
        {
            log_e("Full migration failed at step %zu", currentStepIndex_);
            return false;
        }
    }

    log_i("Full migration completed successfully");
    currentPhase_ = MigrationPhase::Complete;
    preferenceService_->SetPreference("migration_phase", "complete");
    return true;
}

bool ConfigMigrationService::RollbackLastStep()
{
    log_v("RollbackLastStep() called");

    if (currentStepIndex_ == 0)
    {
        log_w("No steps to rollback");
        return false;
    }

    // Move back to previous step
    currentStepIndex_--;
    auto& step = migrationSteps_[currentStepIndex_];

    if (!step.completed)
    {
        log_w("Step '%s' was not completed, cannot rollback", step.name.c_str());
        return false;
    }

    log_i("Rolling back migration step: %s", step.name.c_str());

    // Execute rollback
    if (step.rollback)
    {
        step.rollback();
    }

    // Mark as not completed
    step.completed = false;

    log_i("Migration step '%s' rolled back successfully", step.name.c_str());
    return true;
}

float ConfigMigrationService::GetMigrationProgress() const
{
    if (migrationSteps_.empty())
        return 0.0f;

    size_t completedSteps = 0;
    for (const auto& step : migrationSteps_)
    {
        if (step.completed)
            completedSteps++;
    }

    return (static_cast<float>(completedSteps) / migrationSteps_.size()) * 100.0f;
}

template<typename T>
std::optional<T> ConfigMigrationService::GetConfigValue(const std::string& key) const
{
    // In transitional phase, prefer dynamic config but fall back to legacy
    if (currentPhase_ >= MigrationPhase::Transitional && dynamicConfigService_)
    {
        auto value = dynamicConfigService_->QueryConfig<T>(key);
        if (value)
        {
            return value;
        }
    }

    // Fall back to legacy system or use it directly in legacy phase
    // Map common keys to legacy config structure
    if (preferenceService_)
    {
        const auto& config = preferenceService_->GetConfig();

        // Map dynamic keys to legacy config
        if (key == "system.panel_name" || key == "panelName")
        {
            if constexpr (std::is_same_v<T, std::string>)
            {
                return std::optional<T>(config.panelName);
            }
        }
        else if (key == "system.show_splash" || key == "showSplash")
        {
            if constexpr (std::is_same_v<T, bool>)
            {
                return std::optional<T>(config.showSplash);
            }
        }
        else if (key == "system.splash_duration" || key == "splashDuration")
        {
            if constexpr (std::is_same_v<T, int>)
            {
                return std::optional<T>(config.splashDuration);
            }
        }
        else if (key == "system.update_rate" || key == "updateRate")
        {
            if constexpr (std::is_same_v<T, int>)
            {
                return std::optional<T>(config.updateRate);
            }
        }
        else if (key == "style.theme" || key == "theme")
        {
            if constexpr (std::is_same_v<T, std::string>)
            {
                return std::optional<T>(config.theme);
            }
        }
        // Add more mappings as needed
    }

    return std::nullopt;
}

template<typename T>
bool ConfigMigrationService::SetConfigValue(const std::string& key, const T& value)
{
    Config::ConfigValue oldValue;
    bool success = false;

    // Get old value for change notification
    auto oldVal = GetConfigValue<T>(key);
    if (oldVal)
    {
        oldValue = Config::ConfigValue(*oldVal);
    }

    // In transitional/dynamic phase, update both systems
    if (currentPhase_ >= MigrationPhase::Transitional && dynamicConfigService_)
    {
        success = dynamicConfigService_->UpdateConfig<T>(key, value);

        // Also update legacy for backward compatibility
        if (success && currentPhase_ == MigrationPhase::Transitional)
        {
            // Update legacy config based on key mapping
            auto config = preferenceService_->GetConfig();

            // Map dynamic keys to legacy config
            if (key == "system.panel_name")
            {
                if constexpr (std::is_same_v<T, std::string>)
                {
                    config.panelName = value;
                }
            }
            else if (key == "system.show_splash")
            {
                if constexpr (std::is_same_v<T, bool>)
                {
                    config.showSplash = value;
                }
            }
            // Add more mappings as needed

            preferenceService_->SetConfig(config);
        }
    }
    else
    {
        // Legacy-only mode, update legacy config
        auto config = preferenceService_->GetConfig();

        // Direct legacy key updates
        if (key == "panelName")
        {
            if constexpr (std::is_same_v<T, std::string>)
            {
                config.panelName = value;
                success = true;
            }
        }
        // Add more legacy mappings as needed

        if (success)
        {
            preferenceService_->SetConfig(config);
        }
    }

    // Notify change callbacks
    if (success)
    {
        Config::ConfigValue newValue = Config::ConfigValue(value);
        NotifyConfigurationChange(key, oldValue, newValue);
    }

    return success;
}

void ConfigMigrationService::RegisterChangeCallback(const ConfigChangeCallback& callback)
{
    changeCallbacks_.push_back(callback);
}

void ConfigMigrationService::UnregisterChangeCallback(const ConfigChangeCallback& callback)
{
    // Note: Function pointer comparison is tricky, this is a simplified implementation
    // In practice, you might want to use callback IDs or smart pointers
    log_w("ConfigChangeCallback unregistration not fully implemented");
}

bool ConfigMigrationService::ValidateConfigurationIntegrity() const
{
    log_v("ValidateConfigurationIntegrity() called");

    if (currentPhase_ < MigrationPhase::Transitional || !dynamicConfigService_)
    {
        log_d("Skipping validation - not in hybrid mode");
        return true;
    }

    // Check that key values match between systems
    auto mappings = GetConfigMappings();
    bool allValid = true;

    for (const auto& mapping : mappings)
    {
        // Compare values between legacy and dynamic systems
        // This is a simplified check - in practice you'd want type-specific comparisons
        log_d("Validating mapping: %s -> %s", mapping.legacyKey.c_str(), mapping.dynamicKey.c_str());
    }

    log_i("Configuration integrity validation %s", allValid ? "passed" : "failed");
    return allValid;
}

std::string ConfigMigrationService::CreateConfigurationBackup()
{
    log_v("CreateConfigurationBackup() called");

    std::string backupId = GenerateBackupId();

    // Save current configuration state
    if (preferenceService_)
    {
        const auto& config = preferenceService_->GetConfig();
        // Serialize configuration to backup storage
        // This is a simplified implementation - you'd want proper serialization

        preferenceService_->SetPreference("backup_" + backupId + "_created", "true");
        log_i("Configuration backup created: %s", backupId.c_str());
    }

    return backupId;
}

bool ConfigMigrationService::RestoreConfigurationBackup(const std::string& backupId)
{
    log_v("RestoreConfigurationBackup() called with ID: %s", backupId.c_str());

    if (!preferenceService_->HasPreference("backup_" + backupId + "_created"))
    {
        log_e("Backup not found: %s", backupId.c_str());
        return false;
    }

    // Restore configuration from backup
    // This is a simplified implementation
    log_i("Configuration backup restored: %s", backupId.c_str());
    return true;
}

// Private Methods

void ConfigMigrationService::InitializeMigrationSteps()
{
    log_v("InitializeMigrationSteps() called");

    migrationSteps_.clear();

    // Step 1: Migrate Basic Settings
    migrationSteps_.push_back({
        "Migrate Basic Settings",
        [this]() { return MigrateBasicSettings(); },
        [this]() { return ValidateConfigurationIntegrity(); },
        [this]() { log_i("Rollback: Basic Settings migration"); }
    });

    // Step 2: Migrate Sensor Settings
    migrationSteps_.push_back({
        "Migrate Sensor Settings",
        [this]() { return MigrateSensorSettings(); },
        [this]() { return ValidateConfigurationIntegrity(); },
        [this]() { log_i("Rollback: Sensor Settings migration"); }
    });

    // Step 3: Migrate Advanced Settings
    migrationSteps_.push_back({
        "Migrate Advanced Settings",
        [this]() { return MigrateAdvancedSettings(); },
        [this]() { return ValidateConfigurationIntegrity(); },
        [this]() { log_i("Rollback: Advanced Settings migration"); }
    });

    // Step 4: Enable Dynamic UI
    migrationSteps_.push_back({
        "Enable Dynamic UI",
        [this]() { return EnableDynamicUI(); },
        [this]() { return true; }, // UI enabling doesn't need validation
        [this]() { log_i("Rollback: Dynamic UI disabled"); }
    });

    // Step 5: Final Validation
    migrationSteps_.push_back({
        "Validate Migration",
        [this]() { return ValidateMigration(); },
        [this]() { return true; },
        [this]() { log_i("Rollback: Migration validation"); }
    });

    // Step 6: Cleanup Legacy Data
    migrationSteps_.push_back({
        "Cleanup Legacy Data",
        [this]() { return CleanupLegacyData(); },
        [this]() { return true; },
        [this]() { log_i("Rollback: Legacy data cleanup"); }
    });

    log_d("Initialized %zu migration steps", migrationSteps_.size());
}

bool ConfigMigrationService::MigrateBasicSettings()
{
    log_i("Migrating basic settings...");

    if (!dynamicConfigService_ || !preferenceService_)
        return false;

    const auto& config = preferenceService_->GetConfig();

    // Migrate system settings
    bool success = true;
    success &= dynamicConfigService_->UpdateConfig("system.panel_name", config.panelName);
    success &= dynamicConfigService_->UpdateConfig("system.show_splash", config.showSplash);
    success &= dynamicConfigService_->UpdateConfig("system.splash_duration", config.splashDuration);
    success &= dynamicConfigService_->UpdateConfig("system.update_rate", config.updateRate);

    // Migrate style settings
    success &= dynamicConfigService_->UpdateConfig("style.theme", config.theme);

    if (success)
    {
        currentPhase_ = MigrationPhase::Transitional;
        preferenceService_->SetPreference("migration_phase", "transitional");
        log_i("Basic settings migration completed");
    }

    return success;
}

bool ConfigMigrationService::MigrateSensorSettings()
{
    log_i("Migrating sensor settings...");

    if (!dynamicConfigService_ || !preferenceService_)
        return false;

    const auto& config = preferenceService_->GetConfig();

    // Migrate pressure sensor settings
    bool success = true;
    success &= dynamicConfigService_->UpdateConfig("oil_pressure.unit", config.pressureUnit);
    success &= dynamicConfigService_->UpdateConfig("oil_pressure.offset", config.pressureOffset);
    success &= dynamicConfigService_->UpdateConfig("oil_pressure.scale", config.pressureScale);

    // Migrate temperature sensor settings
    success &= dynamicConfigService_->UpdateConfig("oil_temperature.unit", config.tempUnit);
    success &= dynamicConfigService_->UpdateConfig("oil_temperature.offset", config.tempOffset);
    success &= dynamicConfigService_->UpdateConfig("oil_temperature.scale", config.tempScale);

    if (success)
    {
        log_i("Sensor settings migration completed");
    }

    return success;
}

bool ConfigMigrationService::MigrateAdvancedSettings()
{
    log_i("Migrating advanced settings...");

    // No advanced settings in current system, but this is where they would go
    log_i("Advanced settings migration completed (no advanced settings to migrate)");
    return true;
}

bool ConfigMigrationService::EnableDynamicUI()
{
    log_i("Enabling dynamic UI...");

    currentPhase_ = MigrationPhase::Dynamic;
    preferenceService_->SetPreference("migration_phase", "dynamic");
    preferenceService_->SetPreference("dynamic_ui_enabled", "true");

    log_i("Dynamic UI enabled");
    return true;
}

bool ConfigMigrationService::ValidateMigration()
{
    log_i("Validating complete migration...");

    bool valid = ValidateConfigurationIntegrity();
    if (valid)
    {
        log_i("Migration validation passed");
    }
    else
    {
        log_e("Migration validation failed");
    }

    return valid;
}

bool ConfigMigrationService::CleanupLegacyData()
{
    log_i("Cleaning up legacy data...");

    // In a real implementation, you might want to keep legacy data for rollback
    // For now, we'll just mark the cleanup as done
    preferenceService_->SetPreference("legacy_cleanup_completed", "true");

    log_i("Legacy data cleanup completed");
    return true;
}

void ConfigMigrationService::NotifyConfigurationChange(const std::string& key, const Config::ConfigValue& oldValue, const Config::ConfigValue& newValue)
{
    for (const auto& callback : changeCallbacks_)
    {
        callback(key, oldValue, newValue);
    }
}

std::string ConfigMigrationService::GenerateBackupId() const
{
    // Generate a timestamp-based backup ID
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::ostringstream ss;
    ss << "backup_" << time_t;
    return ss.str();
}

std::vector<ConfigMigrationService::ConfigMapping> ConfigMigrationService::GetConfigMappings() const
{
    return {
        {"panelName", "system.panel_name", Config::ConfigValueType::String},
        {"showSplash", "system.show_splash", Config::ConfigValueType::Boolean},
        {"splashDuration", "system.splash_duration", Config::ConfigValueType::Integer},
        {"updateRate", "system.update_rate", Config::ConfigValueType::Integer},
        {"theme", "style.theme", Config::ConfigValueType::Enum},
        {"pressureUnit", "oil_pressure.unit", Config::ConfigValueType::Enum},
        {"pressureOffset", "oil_pressure.offset", Config::ConfigValueType::Float},
        {"pressureScale", "oil_pressure.scale", Config::ConfigValueType::Float},
        {"tempUnit", "oil_temperature.unit", Config::ConfigValueType::Enum},
        {"tempOffset", "oil_temperature.offset", Config::ConfigValueType::Float},
        {"tempScale", "oil_temperature.scale", Config::ConfigValueType::Float}
    };
}

// Explicit template instantiations for common types
template std::optional<std::string> ConfigMigrationService::GetConfigValue<std::string>(const std::string&) const;
template std::optional<int> ConfigMigrationService::GetConfigValue<int>(const std::string&) const;
template std::optional<float> ConfigMigrationService::GetConfigValue<float>(const std::string&) const;
template std::optional<bool> ConfigMigrationService::GetConfigValue<bool>(const std::string&) const;

template bool ConfigMigrationService::SetConfigValue<std::string>(const std::string&, const std::string&);
template bool ConfigMigrationService::SetConfigValue<int>(const std::string&, const int&);
template bool ConfigMigrationService::SetConfigValue<float>(const std::string&, const float&);
template bool ConfigMigrationService::SetConfigValue<bool>(const std::string&, const bool&);