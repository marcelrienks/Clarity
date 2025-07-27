Current Workload Analysis

  Core 1 (Minimal Load)

  - GPIO ISR handlers: ~1-10μs per interrupt
  - Queue message posting: ~5-10μs per message
  - Total: Essentially idle except during pin changes

  Core 0 (Heavy Load)

  - Message processing: ~50-100μs per batch
  - Trigger state consolidation: ~20-50μs per batch
  - Panel loading/switching: ~10-50ms (major load)
  - LVGL rendering: ~5-20ms per frame
  - Component updates: ~1-5ms per update
  - Total: 95%+ of system work

  Performance Considerations

  Potential Issues

  1. Core 0 saturation: Could miss messages or drop frames during heavy panel operations
  2. Uneven CPU utilization: Core 1 mostly idle while Core 0 stressed
  3. Latency spikes: Complex panel loads could delay trigger response

  Mitigating Factors

  1. Trigger events are rare: Pin changes happen maybe 1-10 times per minute, not continuously
  2. LVGL is optimized: Double buffering, partial updates, efficient rendering
  3. ESP32 is fast: 240MHz dual-core handles UI workloads well
  4. Queue buffering: Messages can accumulate during heavy operations

  Alternative Load Distribution

  Option A: Core 1 Does State Consolidation

  Core 1: GPIO monitoring + message processing + state consolidation
  Core 0: LVGL + panel operations + action execution

  Option B: Hybrid Processing

  Core 1: GPIO monitoring + trigger state updates
  Core 0: LVGL + panel operations
  Shared: Mutex-protected trigger state registry

  Recommendation: Stick with Current Approach

  The imbalance is actually beneficial for this use case:

  1. Trigger latency is critical: Core 1 dedicated to GPIO ensures immediate ISR response
  2. UI operations can tolerate brief delays: Users won't notice 10-20ms delays during panel switches
  3. Event frequency is low: Trigger processing won't significantly impact Core 0 performance
  4. Simplicity wins: No mutexes, no cross-core synchronization complexity

  The ESP32's dual-core design commonly uses one core for time-critical tasks (GPIO/interrupts) and the other for application logic (UI/networking). Your proposed architecture follows
   this pattern well.

  Performance impact: Minimal. The trigger processing adds maybe 1-2% to Core 0's workload.