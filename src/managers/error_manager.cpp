#include "managers/error_manager.h"
#include "utilities/logging.h"
#include <Arduino.h>
#include <algorithm>

ErrorManager &ErrorManager::Instance()
{
    static ErrorManager instance;
    return instance;
}

void ErrorManager::ReportError(ErrorLevel level, const char *source, const std::string &message)
{
    // Process auto-dismiss before adding new error
    AutoDismissOldWarnings();

    // Create error info
    ErrorInfo errorInfo;
    errorInfo.level = level;
    errorInfo.source = source;
    errorInfo.SetMessage(message);
    errorInfo.timestamp = millis();
    errorInfo.acknowledged = false;

    // Add to queue
    errorQueue_.push_back(errorInfo);

    // Trim queue if necessary
    TrimErrorQueue();

    // Log error
    const char *levelStr = (level == ErrorLevel::WARNING) ? "WARNING"
                           : (level == ErrorLevel::ERROR) ? "ERROR"
                                                          : "CRITICAL";
    log_e("[%s] %s: %s", levelStr, source, errorInfo.message);
    log_t("Error reported: %s", levelStr);
}

void ErrorManager::ReportWarning(const char *source, const std::string &message)
{
    ReportError(ErrorLevel::WARNING, source, message);
}

void ErrorManager::ReportCriticalError(const char *source, const std::string &message)
{
    ReportError(ErrorLevel::CRITICAL, source, message);
}

bool ErrorManager::HasPendingErrors() const
{
    return !errorQueue_.empty();
}

bool ErrorManager::HasCriticalErrors() const
{
    return std::any_of(errorQueue_.begin(), errorQueue_.end(), [](const ErrorInfo &error)
                       { return error.level == ErrorLevel::CRITICAL && !error.acknowledged; });
}

std::vector<ErrorInfo> ErrorManager::GetErrorQueue() const
{
    return errorQueue_;
}

void ErrorManager::AcknowledgeError(size_t errorIndex)
{
    if (errorIndex < errorQueue_.size())
    {
        errorQueue_[errorIndex].acknowledged = true;

        // Remove acknowledged warnings and errors (keep critical until explicitly cleared)
        if (errorQueue_[errorIndex].level != ErrorLevel::CRITICAL)
        {
            errorQueue_.erase(errorQueue_.begin() + errorIndex);
        }

        // Update last dismissal time for warnings
        if (errorQueue_.size() > errorIndex && errorQueue_[errorIndex].level == ErrorLevel::WARNING)
        {
            lastWarningDismissalTime_ = millis();
        }
    }
}

void ErrorManager::ClearAllErrors()
{
    errorQueue_.clear();
    errorPanelActive_ = false;
}

bool ErrorManager::ShouldTriggerErrorPanel() const
{
    // Process auto-dismiss first (const_cast for this internal operation)
    const_cast<ErrorManager *>(this)->AutoDismissOldWarnings();

    // Always trigger if we have pending errors - let the trigger system handle the logic
    // of whether to actually switch panels
    return HasPendingErrors();
}

void ErrorManager::SetErrorPanelActive(bool active)
{
    errorPanelActive_ = active;
}

bool ErrorManager::IsErrorPanelActive() const
{
    return errorPanelActive_;
}

void ErrorManager::TrimErrorQueue()
{
    if (errorQueue_.size() > MAX_ERROR_QUEUE_SIZE)
    {
        // Sort by priority (critical > error > warning) and timestamp (newer first)
        std::sort(errorQueue_.begin(), errorQueue_.end(),
                  [](const ErrorInfo &a, const ErrorInfo &b)
                  {
                      if (a.level != b.level)
                      {
                          return static_cast<int>(a.level) > static_cast<int>(b.level);
                      }
                      return a.timestamp > b.timestamp;
                  });

        // Keep only the most recent/important errors
        errorQueue_.resize(MAX_ERROR_QUEUE_SIZE);
    }
}

ErrorLevel ErrorManager::GetHighestErrorLevel() const
{
    if (errorQueue_.empty())
    {
        return ErrorLevel::WARNING; // Default level
    }

    ErrorLevel highest = ErrorLevel::WARNING;
    for (const auto &error : errorQueue_)
    {
        if (!error.acknowledged)
        {
            if (error.level == ErrorLevel::CRITICAL)
            {
                return ErrorLevel::CRITICAL; // Highest possible
            }
            if (error.level == ErrorLevel::ERROR && highest == ErrorLevel::WARNING)
            {
                highest = ErrorLevel::ERROR;
            }
        }
    }
    return highest;
}

/// @brief Auto-dismiss warnings older than timeout
void ErrorManager::AutoDismissOldWarnings()
{
    auto now = millis();
    auto it = errorQueue_.begin();
    while (it != errorQueue_.end())
    {
        if (it->level == ErrorLevel::WARNING && 
            (now - it->timestamp) > WARNING_AUTO_DISMISS_TIME)
        {
            it = errorQueue_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

/// @brief Process error queue and manage error panel state
void ErrorManager::Process()
{
    // Process error queue
    AutoDismissOldWarnings();

    // Check if error panel should be triggered
    if (HasCriticalErrors() && !errorPanelActive_)
    {
        // Trigger error panel through interrupt system
        // This would normally activate the error trigger
        log_i("Critical errors present, error panel should be activated");
    }

    // Check if error panel should be deactivated
    if (!HasPendingErrors() && errorPanelActive_)
    {
        errorPanelActive_ = false;
        log_i("No pending errors, error panel can be deactivated");
    }
}