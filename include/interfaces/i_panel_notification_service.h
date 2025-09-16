#pragma once

// Forward declarations
class IPanel;

/**
 * @interface IPanelNotificationService
 * @brief Interface for panel completion notifications in callback-free architecture
 *
 * @details This interface enables panels to notify completion of async operations
 * (animations, loading) without using std::function callbacks that cause heap
 * fragmentation on ESP32. Components use dependency injection to receive this
 * interface and call it directly when operations complete.
 *
 * @design_pattern Dependency Injection - Injected into panels for testability
 * @memory_safety Zero heap allocation - Interface calls compile to direct calls
 *
 * @implementations:
 * - PanelManager: Main implementation for production use
 * - Mock implementations: For unit testing panel behavior
 */
class IPanelNotificationService {
public:
    virtual ~IPanelNotificationService() = default;
    
    virtual void OnPanelLoadComplete(IPanel* panel) = 0;
    virtual void OnPanelUpdateComplete(IPanel* panel) = 0;
};