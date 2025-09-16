#pragma once

#include "utilities/types.h"
#include <string>
#include <vector>

/**
 * @class ErrorManager
 * @brief Global error management service for application-level error handling
 *
 * @details Singleton service that collects, manages, and coordinates error
 * reporting across the Clarity application. Integrates with the trigger system
 * to automatically display error panels when critical issues occur.
 *
 * @design_constraints:
 * - Memory efficient with bounded error queue
 * - Non-intrusive - preserves Arduino crash reporting
 * - Priority-aware error handling via trigger system
 * - Automatic cleanup of old errors when queue is full
 */
class ErrorManager
{
public:
    // ========== Constructors and Destructor ==========
    ErrorManager(const ErrorManager&) = delete;
    ErrorManager& operator=(const ErrorManager&) = delete;
    ~ErrorManager() = default;

    // ========== Static Methods ==========
    static ErrorManager &Instance();

    // ========== Public Interface Methods ==========
    void ReportError(ErrorLevel level, const char *source, const std::string &message);
    void ReportWarning(const char *source, const std::string &message);
    void ReportCriticalError(const char *source, const std::string &message);
    bool HasPendingErrors() const;
    bool HasCriticalErrors() const;
    std::vector<ErrorInfo> GetErrorQueue() const;
    void AcknowledgeError(size_t errorIndex);
    void ClearAllErrors();
    bool ShouldTriggerErrorPanel() const;
    void SetErrorPanelActive(bool active);
    bool IsErrorPanelActive() const;
    void Process();

private:
    // ========== Constructors and Destructor ==========
    ErrorManager() = default;

    // ========== Private Methods ==========
    void TrimErrorQueue();
    void AutoDismissOldWarnings();
    ErrorLevel GetHighestErrorLevel() const;

    // ========== Private Data Members ==========
    static constexpr size_t MAX_ERROR_QUEUE_SIZE = 10;                ///< Memory-constrained device limit
    static constexpr unsigned long WARNING_AUTO_DISMISS_TIME = 10000; ///< 10 seconds for warnings

    std::vector<ErrorInfo> errorQueue_;
    bool errorPanelActive_ = false;
    unsigned long lastWarningDismissalTime_ = 0;
};