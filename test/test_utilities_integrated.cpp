#include <unity.h>
#include "test_utilities.h"

// Local mock state for utilities testing
static uint32_t mock_current_time = 0;
static uint32_t mock_delay_called_with = 0;
static uint32_t mock_tick_increment = 0;
static bool mock_timer_handler_called = false;
static uint32_t mock_delay_call_count = 0;

// Local mock types for utilities testing
typedef struct {
    bool created;
    bool theme_applied;
    bool deleted;
} local_mock_lv_obj_t;

// Local mock functions for utilities testing
static local_mock_lv_obj_t* local_mock_lv_obj_create(local_mock_lv_obj_t* parent) {
    static local_mock_lv_obj_t obj = {true, false, false};
    obj.created = true;
    obj.deleted = false;
    obj.theme_applied = false;
    return &obj;
}

static void local_mock_lv_obj_del(local_mock_lv_obj_t* obj) {
    if (obj) {
        obj->deleted = true;
        obj->created = false;
    }
}

static void local_mock_delay(uint32_t ms) {
    mock_delay_called_with = ms;
    mock_delay_call_count++;
    mock_current_time += ms; // Simulate time passage
}

static uint32_t local_mock_millis() {
    return mock_current_time;
}

static void local_lv_tick_inc(uint32_t tick_period) {
    mock_tick_increment += tick_period;
}

static uint32_t local_lv_timer_handler() {
    mock_timer_handler_called = true;
    return 5; // Simulate some work time
}

// Local MockStyleManager for utilities testing
class LocalMockStyleManager {
public:
    static LocalMockStyleManager& GetInstance() {
        static LocalMockStyleManager instance;
        return instance;
    }
    
    bool theme_applied_to_screen = false;
    
    void apply_theme_to_screen(local_mock_lv_obj_t* screen) {
        if (screen) {
            screen->theme_applied = true;
            theme_applied_to_screen = true;
        }
    }
};

// Local MockTicker for utilities testing
class LocalMockTicker {
private:
    static uint32_t last_timestamp;
    static uint32_t last_tick_increment;
    static uint32_t last_task_run;
    
public:
    static void reset() {
        last_timestamp = 0;
        last_tick_increment = 0;
        last_task_run = 0;
    }
    
    static uint32_t get_elapsed_millis() {
        uint32_t current_timestamp = mock_current_time;
        uint32_t elapsed = current_timestamp - last_timestamp;
        last_timestamp = current_timestamp;
        
        return elapsed;
    }
    
    static void handle_dynamic_delay(uint32_t start_time) {
        uint32_t elapsed_time = mock_current_time - start_time;
        uint32_t target_frame_time = 16; // 60fps = ~16.7ms per frame
        
        if (elapsed_time < target_frame_time) {
            local_mock_delay(target_frame_time - elapsed_time);
        } else {
            local_mock_delay(1); // Yield to other tasks
        }
    }
    
    static void handle_lv_tasks() {
        uint32_t current_time = mock_current_time;
        
        // Increment tick counter with elapsed time
        uint32_t elapsed = current_time - last_tick_increment;
        if (elapsed > 0) {
            local_lv_tick_inc(elapsed);
            last_tick_increment = current_time;
        }
        
        // Process all pending tasks
        local_lv_timer_handler();
        last_task_run = current_time;
    }
};

// Define static members
uint32_t LocalMockTicker::last_timestamp = 0;
uint32_t LocalMockTicker::last_tick_increment = 0;
uint32_t LocalMockTicker::last_task_run = 0;

class LocalMockLvTools {
public:
    static local_mock_lv_obj_t* create_blank_screen() {
        local_mock_lv_obj_t* screen = local_mock_lv_obj_create(nullptr);
        reset_screen(screen);
        return screen;
    }
    
    static void reset_screen(local_mock_lv_obj_t* screen) {
        LocalMockStyleManager::GetInstance().apply_theme_to_screen(screen);
    }
};

// Note: setUp() and tearDown() are defined in test_main.cpp

void resetMockUtilitiesState() {
    mock_current_time = 0;
    mock_delay_called_with = 0;
    mock_tick_increment = 0;
    mock_timer_handler_called = false;
    mock_delay_call_count = 0;
    LocalMockStyleManager::GetInstance().theme_applied_to_screen = false;
    LocalMockTicker::reset();
}

// =================================================================
// TICKER UTILITY TESTS
// =================================================================

void test_ticker_get_elapsed_millis_initial(void) {
    resetMockUtilitiesState();
    
    mock_current_time = 1000;
    uint32_t elapsed = LocalMockTicker::get_elapsed_millis();
    
    // First call should return the time difference from start
    TEST_ASSERT_EQUAL_UINT32(1000, elapsed);
}

void test_ticker_get_elapsed_millis_subsequent_calls(void) {
    resetMockUtilitiesState();
    
    mock_current_time = 1000;
    LocalMockTicker::get_elapsed_millis(); // First call
    
    mock_current_time = 1500;
    uint32_t elapsed = LocalMockTicker::get_elapsed_millis(); // Second call
    
    TEST_ASSERT_EQUAL_UINT32(500, elapsed);
}

void test_ticker_get_elapsed_millis_multiple_calls(void) {
    resetMockUtilitiesState();
    
    uint32_t times[] = {0, 100, 350, 500, 1000};
    uint32_t expected_elapsed[] = {0, 100, 250, 150, 500};
    
    for (size_t i = 0; i < 5; i++) {
        mock_current_time = times[i];
        uint32_t elapsed = LocalMockTicker::get_elapsed_millis();
        TEST_ASSERT_EQUAL_UINT32(expected_elapsed[i], elapsed);
    }
}

void test_ticker_handle_dynamic_delay_fast_processing(void) {
    resetMockUtilitiesState();
    
    uint32_t start_time = 100;
    mock_current_time = 105; // Only 5ms elapsed
    
    LocalMockTicker::handle_dynamic_delay(start_time);
    
    // Should delay by (16 - 5) = 11ms to reach 60fps target
    TEST_ASSERT_EQUAL_UINT32(11, mock_delay_called_with);
}

void test_ticker_handle_dynamic_delay_slow_processing(void) {
    resetMockUtilitiesState();
    
    uint32_t start_time = 100;
    mock_current_time = 120; // 20ms elapsed (over 16ms target)
    
    LocalMockTicker::handle_dynamic_delay(start_time);
    
    // Should only delay 1ms to yield to other tasks
    TEST_ASSERT_EQUAL_UINT32(1, mock_delay_called_with);
}

void test_ticker_handle_dynamic_delay_exact_timing(void) {
    resetMockUtilitiesState();
    
    uint32_t start_time = 100;
    mock_current_time = 116; // Exactly 16ms elapsed
    
    LocalMockTicker::handle_dynamic_delay(start_time);
    
    // Should delay 1ms (no additional delay needed)
    TEST_ASSERT_EQUAL_UINT32(1, mock_delay_called_with);
}

void test_ticker_handle_lv_tasks_tick_increment(void) {
    resetMockUtilitiesState();
    
    mock_current_time = 50;
    LocalMockTicker::handle_lv_tasks();
    
    TEST_ASSERT_EQUAL_UINT32(50, mock_tick_increment);
    TEST_ASSERT_TRUE(mock_timer_handler_called);
}

void test_ticker_handle_lv_tasks_multiple_calls(void) {
    resetMockUtilitiesState();
    
    // First call
    mock_current_time = 50;
    LocalMockTicker::handle_lv_tasks();
    TEST_ASSERT_EQUAL_UINT32(50, mock_tick_increment);
    
    // Second call - should add the difference
    mock_current_time = 80;
    mock_timer_handler_called = false; // Reset for second call
    LocalMockTicker::handle_lv_tasks();
    TEST_ASSERT_EQUAL_UINT32(80, mock_tick_increment); // 50 + 30
    TEST_ASSERT_TRUE(mock_timer_handler_called);
}

void test_ticker_handle_lv_tasks_no_time_elapsed(void) {
    resetMockUtilitiesState();
    
    mock_current_time = 100;
    LocalMockTicker::handle_lv_tasks();
    
    // Same time - no additional increment
    LocalMockTicker::handle_lv_tasks();
    
    TEST_ASSERT_EQUAL_UINT32(100, mock_tick_increment); // Should remain the same
}

// =================================================================
// LVTOOLS UTILITY TESTS
// =================================================================

void test_lvtools_create_blank_screen(void) {
    resetMockUtilitiesState();
    
    local_mock_lv_obj_t* screen = LocalMockLvTools::create_blank_screen();
    
    TEST_ASSERT_NOT_NULL(screen);
    TEST_ASSERT_TRUE(screen->created);
    TEST_ASSERT_TRUE(screen->theme_applied);
    TEST_ASSERT_TRUE(LocalMockStyleManager::GetInstance().theme_applied_to_screen);
}

void test_lvtools_create_blank_screen_multiple(void) {
    resetMockUtilitiesState();
    
    // Create multiple screens
    local_mock_lv_obj_t* screen1 = LocalMockLvTools::create_blank_screen();
    LocalMockStyleManager::GetInstance().theme_applied_to_screen = false; // Reset
    local_mock_lv_obj_t* screen2 = LocalMockLvTools::create_blank_screen();
    
    TEST_ASSERT_NOT_NULL(screen1);
    TEST_ASSERT_NOT_NULL(screen2);
    TEST_ASSERT_TRUE(screen1->created);
    TEST_ASSERT_TRUE(screen2->created);
    TEST_ASSERT_TRUE(screen1->theme_applied);
    TEST_ASSERT_TRUE(screen2->theme_applied);
}

void test_lvtools_reset_screen(void) {
    resetMockUtilitiesState();
    
    // Create a screen manually
    local_mock_lv_obj_t* screen = local_mock_lv_obj_create(nullptr);
    TEST_ASSERT_FALSE(screen->theme_applied);
    
    LocalMockLvTools::reset_screen(screen);
    
    TEST_ASSERT_TRUE(screen->theme_applied);
    TEST_ASSERT_TRUE(LocalMockStyleManager::GetInstance().theme_applied_to_screen);
}

void test_lvtools_reset_screen_null_handling(void) {
    resetMockUtilitiesState();
    
    // Should handle null gracefully
    LocalMockLvTools::reset_screen(nullptr);
    
    // Should not crash and not apply theme
    TEST_ASSERT_FALSE(LocalMockStyleManager::GetInstance().theme_applied_to_screen);
}

void test_lvtools_screen_lifecycle(void) {
    resetMockUtilitiesState();
    
    // Create screen
    local_mock_lv_obj_t* screen = LocalMockLvTools::create_blank_screen();
    TEST_ASSERT_TRUE(screen->created);
    TEST_ASSERT_FALSE(screen->deleted);
    
    // Reset screen (should maintain creation state)
    LocalMockLvTools::reset_screen(screen);
    TEST_ASSERT_TRUE(screen->created);
    TEST_ASSERT_TRUE(screen->theme_applied);
    
    // Delete screen
    local_mock_lv_obj_del(screen);
    TEST_ASSERT_TRUE(screen->deleted);
    TEST_ASSERT_FALSE(screen->created);
}

// Note: PlatformIO will automatically discover and run test_ functions