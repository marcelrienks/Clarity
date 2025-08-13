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
    /// @brief Get singleton instance
    /// @return Reference to the global ErrorManager instance
    static ErrorManager &Instance();

    // Error reporting interface
    /// @brief Report an error with specified level
    /// @param level Error severity level
    /// @param source Component/manager reporting the error
    /// @param message Human-readable error description
    void ReportError(ErrorLevel level, const char *source, const std::string &message);

    /// @brief Report a warning (convenience method)
    /// @param source Component/manager reporting the warning
    /// @param message Human-readable warning description
    void ReportWarning(const char *source, const std::string &message);

    /// @brief Report a critical error (convenience method)
    /// @param source Component/manager reporting the critical error
    /// @param message Human-readable error description
    void ReportCriticalError(const char *source, const std::string &message);

    // Error queue management
    /// @brief Check if there are any pending errors
    /// @return True if error queue is not empty
    bool HasPendingErrors() const;

    /// @brief Check if there are any critical errors
    /// @return True if any critical errors exist in queue
    bool HasCriticalErrors() const;

    /// @brief Get copy of current error queue
    /// @return Vector containing all pending errors
    std::vector<ErrorInfo> GetErrorQueue() const;

    /// @brief Acknowledge a specific error by index
    /// @param errorIndex Index of error to acknowledge
    void AcknowledgeError(size_t errorIndex);

    /// @brief Clear all errors from the queue
    void ClearAllErrors();

    // Integration with trigger system
    /// @brief Check if error panel should be triggered
    /// @return True if error panel should be displayed
    bool ShouldTriggerErrorPanel() const;

    /// @brief Set error panel active state
    /// @param active True when error panel is currently displayed
    void SetErrorPanelActive(bool active);

  private:
    static constexpr size_t MAX_ERROR_QUEUE_SIZE = 10;                ///< Memory-constrained device limit
    static constexpr unsigned long WARNING_AUTO_DISMISS_TIME = 10000; ///< 10 seconds for warnings

    std::vector<ErrorInfo> errorQueue_;
    bool errorPanelActive_ = false;
    unsigned long lastWarningDismissalTime_ = 0;

    /// @brief Private constructor for singleton pattern
    ErrorManager() = default;

    /// @brief Remove old errors when queue is full
    void TrimErrorQueue();

    /// @brief Auto-dismiss warnings older than timeout
    void ProcessAutoDismiss();

    /// @brief Get highest error level in queue
    /// @return Highest severity error level currently in queue
    ErrorLevel GetHighestErrorLevel() const;
};