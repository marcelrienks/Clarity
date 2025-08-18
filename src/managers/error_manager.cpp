#include "managers/error_manager.h"
#include <Arduino.h>
#include <algorithm>

ErrorManager &ErrorManager::Instance()
{
    log_v("Instance() called");
    static ErrorManager instance;
    return instance;
}

void ErrorManager::ReportError(ErrorLevel level, const char *source, const std::string &message)
{
    log_v("ReportError() called");
    // Process auto-dismiss before adding new error
    ProcessAutoDismiss();

    // Create error info
    ErrorInfo errorInfo;
    errorInfo.level = level;
    errorInfo.source = source;
    errorInfo.message = message;
    errorInfo.timestamp = millis();
    errorInfo.acknowledged = false;

    // Add to queue
    errorQueue_.push_back(errorInfo);

    // Trim queue if necessary
    TrimErrorQueue();

    // Log error for debugging
    const char *levelStr = (level == ErrorLevel::WARNING) ? "WARNING"
                           : (level == ErrorLevel::ERROR) ? "ERROR"
                                                          : "CRITICAL";
    log_e("[%s] %s: %s", levelStr, source, message.c_str());
}

void ErrorManager::ReportWarning(const char *source, const std::string &message)
{
    log_v("ReportWarning() called");
    ReportError(ErrorLevel::WARNING, source, message);
}

void ErrorManager::ReportCriticalError(const char *source, const std::string &message)
{
    log_v("ReportCriticalError() called");
    ReportError(ErrorLevel::CRITICAL, source, message);
}

bool ErrorManager::HasPendingErrors() const
{
    log_v("HasPendingErrors() called");
    return !errorQueue_.empty();
}

bool ErrorManager::HasCriticalErrors() const
{
    log_v("HasCriticalErrors() called");
    return std::any_of(errorQueue_.begin(), errorQueue_.end(), [](const ErrorInfo &error)
                       { return error.level == ErrorLevel::CRITICAL && !error.acknowledged; });
}

std::vector<ErrorInfo> ErrorManager::GetErrorQueue() const
{
    log_v("GetErrorQueue() called");
    return errorQueue_;
}

void ErrorManager::AcknowledgeError(size_t errorIndex)
{
    log_v("AcknowledgeError() called");
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
    log_v("ClearAllErrors() called");
    errorQueue_.clear();
    errorPanelActive_ = false;
}

bool ErrorManager::ShouldTriggerErrorPanel() const
{
    log_v("ShouldTriggerErrorPanel() called");
    // Process auto-dismiss first (const_cast for this internal operation)
    const_cast<ErrorManager *>(this)->ProcessAutoDismiss();

    // Always trigger if we have pending errors - let the trigger system handle the logic
    // of whether to actually switch panels
    return HasPendingErrors();
}

void ErrorManager::SetErrorPanelActive(bool active)
{
    log_v("SetErrorPanelActive() called");
    errorPanelActive_ = active;
}

void ErrorManager::TrimErrorQueue()
{
    log_v("TrimErrorQueue() called");
    if (errorQueue_.size() > MAX_ERROR_QUEUE_SIZE)
    {
        size_t originalSize = errorQueue_.size();
        log_d("Queue trim needed - before: %zu, max: %zu", originalSize, MAX_ERROR_QUEUE_SIZE);
        
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
        log_d("Queue trimmed - after: %zu, removed: %zu", errorQueue_.size(), originalSize - errorQueue_.size());
    }
}

void ErrorManager::ProcessAutoDismiss()
{
    log_v("ProcessAutoDismiss() called");
    unsigned long currentTime = millis();
    size_t originalCount = errorQueue_.size();

    // Remove warnings that are older than auto-dismiss time
    errorQueue_.erase(std::remove_if(errorQueue_.begin(), errorQueue_.end(),
                                     [currentTime](const ErrorInfo &error)
                                     {
                                         return error.level == ErrorLevel::WARNING && !error.acknowledged &&
                                                (currentTime - error.timestamp) > WARNING_AUTO_DISMISS_TIME;
                                     }),
                      errorQueue_.end());
    
    size_t dismissedCount = originalCount - errorQueue_.size();
    if (dismissedCount > 0)
    {
        log_d("Auto-dismiss - currentTime: %lu, dismissTime: %lu, dismissed: %zu warnings", 
              currentTime, WARNING_AUTO_DISMISS_TIME, dismissedCount);
    }
}

ErrorLevel ErrorManager::GetHighestErrorLevel() const
{
    log_v("GetHighestErrorLevel() called");
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