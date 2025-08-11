#pragma once

// Project Includes
#include "utilities/types.h"

/**
 * @interface IPreferenceService
 * @brief Interface for configuration persistence and management
 * 
 * @details This interface abstracts configuration storage and retrieval
 * operations, providing access to persistent application settings.
 * Implementations should handle initialization, loading, saving, and
 * default configuration creation.
 * 
 * @design_pattern Interface Segregation - Focused on preference operations only
 * @testability Enables mocking for unit tests with in-memory configs
 * @dependency_injection Replaces direct PreferenceManager singleton access
 * @storage_backend Implementation-specific (NVS, filesystem, memory, etc.)
 */
class IPreferenceService
{
public:
    virtual ~IPreferenceService() = default;

    // Core Functionality Methods
    
    /**
     * @brief Initialize the preference service and load existing configuration
     */
    virtual void Init() = 0;

    /**
     * @brief Save current configuration to persistent storage
     */
    virtual void SaveConfig() = 0;

    /**
     * @brief Load configuration from persistent storage
     */
    virtual void LoadConfig() = 0;

    /**
     * @brief Create default configuration if none exists
     */
    virtual void CreateDefaultConfig() = 0;

    // Configuration Access Methods
    
    /**
     * @brief Get the current configuration object
     * @return Reference to current configuration settings
     */
    virtual Configs& GetConfig() = 0;

    /**
     * @brief Get the current configuration object (read-only)
     * @return Const reference to current configuration settings
     */
    virtual const Configs& GetConfig() const = 0;

    /**
     * @brief Update the configuration object
     * @param config New configuration settings to apply
     */
    virtual void SetConfig(const Configs& config) = 0;
    
    // Generic preference access methods for dynamic menus
    
    /**
     * @brief Get a preference value by key
     * @param key The preference key
     * @return String representation of the value
     */
    virtual std::string GetPreference(const std::string& key) const = 0;
    
    /**
     * @brief Set a preference value by key
     * @param key The preference key
     * @param value String representation of the value
     */
    virtual void SetPreference(const std::string& key, const std::string& value) = 0;
    
    /**
     * @brief Check if a preference exists
     * @param key The preference key
     * @return true if preference exists
     */
    virtual bool HasPreference(const std::string& key) const = 0;
};