#pragma once

#include "utilities/types.h"
#include <string>
#include <vector>

/// @class ErrorManager
/// @brief Global error management service for application-level error handling
///
/// @details Singleton service that collects, manages, and coordinates error
/// reporting across the Clarity application. Integrates with the trigger system
/// to automatically display error panels when critical issues occur.
///
/// @design_constraints:
/// - Memory efficient with bounded error queue
/// - Non-intrusive - preserves Arduino crash reporting
/// - Priority-aware error handling via trigger system
/// - Automatic cleanup of old errors when queue is full
class ErrorManager
{
public:
    // ========== Constructors and Destructor ==========
    ErrorManager(const ErrorManager&) = delete;
    ErrorManager& operator=(const ErrorManager&) = delete;
    ~ErrorManager() = default;

    // ========== Static Methods ==========
    /**
     * @brief Get the singleton instance of ErrorManager
     * @return Reference to the global ErrorManager instance
     */
    static ErrorManager &Instance();

    // ========== Public Interface Methods ==========
    /**
     * @brief Report an error with specified severity level
     * @param level The severity level (WARNING, ERROR, CRITICAL)
     * @param source The source component reporting the error
     * @param message Detailed error description
     */
    void ReportError(ErrorLevel level, const char *source, const std::string &message);

    /**
     * @brief Report a warning-level error (convenience method)
     * @param source The source component reporting the warning
     * @param message Detailed warning description
     */
    void ReportWarning(const char *source, const std::string &message);

    /**
     * @brief Report a critical error that requires immediate attention
     * @param source The source component reporting the critical error
     * @param message Detailed critical error description
     */
    void ReportCriticalError(const char *source, const std::string &message);

    /**
     * @brief Check if there are any unacknowledged errors in the queue
     * @return true if errors exist, false if queue is empty
     */
    bool HasPendingErrors() const;

    /**
     * @brief Check if there are any unacknowledged critical errors
     * @return true if critical errors exist, false otherwise
     */
    bool HasCriticalErrors() const;

    /**
     * @brief Get a copy of all errors currently in the queue
     * @return Vector containing all current error information
     */
    std::vector<ErrorInfo> GetErrorQueue() const;

    /**
     * @brief Mark an error as acknowledged and optionally remove it
     * @param errorIndex Zero-based index of the error to acknowledge
     * @details Warnings and errors are removed when acknowledged, critical errors remain until cleared
     */
    void AcknowledgeError(size_t errorIndex);

    /**
     * @brief Remove all errors from the queue and deactivate error panel
     */
    void ClearAllErrors();

    /**
     * @brief Determine if the error panel should be displayed
     * @return true if there are pending errors that warrant panel display
     */
    bool ShouldTriggerErrorPanel() const;

    /**
     * @brief Set the error panel activation state
     * @param active true to mark panel as active, false otherwise
     */
    void SetErrorPanelActive(bool active);

    /**
     * @brief Check if the error panel is currently active
     * @return true if error panel is active, false otherwise
     */
    bool IsErrorPanelActive() const;

    /**
     * @brief Process the error queue and manage panel state
     * @details Handles automatic warning dismissal and panel activation logic
     */
    void Process();

private:
    // ========== Constructors and Destructor ==========
    ErrorManager() = default;

    // ========== Private Methods ==========
    /**
     * @brief Trim error queue to maximum size, keeping highest priority errors
     */
    void TrimErrorQueue();

    /**
     * @brief Automatically remove warnings that exceed the dismissal timeout
     */
    void AutoDismissOldWarnings();

    /**
     * @brief Find the highest severity level among unacknowledged errors
     * @return The most severe error level in the queue
     */
    ErrorLevel GetHighestErrorLevel() const;

    // ========== Private Data Members ==========
    static constexpr size_t MAX_ERROR_QUEUE_SIZE = 10;                ///< Memory-constrained device limit
    static constexpr unsigned long WARNING_AUTO_DISMISS_TIME = 10000; ///< 10 seconds for warnings

    std::vector<ErrorInfo> errorQueue_;
    bool errorPanelActive_ = false;
    unsigned long lastWarningDismissalTime_ = 0;
};