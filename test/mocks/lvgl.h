#pragma once

// Mock LVGL definitions for unit testing
#ifdef UNIT_TESTING

#include <cstdint>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <algorithm>

// Complete struct definitions for mocking
struct _lv_obj_t {
    void* user_data = nullptr;
    // Add minimal required fields
    int dummy = 0;
};

struct _lv_style_t {
    int dummy = 0;
};

struct _lv_group_t {
    int dummy = 0;
};

struct _lv_disp_t {
    int dummy = 0;
};

// Basic LVGL types
typedef struct _lv_obj_t lv_obj_t;
typedef struct _lv_style_t lv_style_t;
typedef struct _lv_group_t lv_group_t;
typedef struct _lv_disp_t lv_disp_t;
typedef int16_t lv_coord_t;
typedef uint8_t lv_align_t;
typedef uint8_t lv_dir_t;
typedef uint8_t lv_part_t;
typedef uint8_t lv_state_t;
typedef uint32_t lv_style_prop_t;

// Mock color type
typedef struct {
    uint16_t full;
} lv_color_t;

// Point and area types
typedef struct {
    lv_coord_t x;
    lv_coord_t y;
} lv_point_t;

typedef struct {
    lv_coord_t x1;
    lv_coord_t y1;
    lv_coord_t x2;
    lv_coord_t y2;
} lv_area_t;

// LVGL constants
#define LV_ALIGN_CENTER             0
#define LV_ALIGN_TOP_LEFT           1
#define LV_ALIGN_TOP_MID            2
#define LV_ALIGN_TOP_RIGHT          3
#define LV_ALIGN_BOTTOM_LEFT        4
#define LV_ALIGN_BOTTOM_MID         5
#define LV_ALIGN_BOTTOM_RIGHT       6
#define LV_ALIGN_LEFT_MID           7
#define LV_ALIGN_RIGHT_MID          8

#define LV_DIR_NONE                 0
#define LV_DIR_LEFT                 1
#define LV_DIR_RIGHT                2
#define LV_DIR_TOP                  4
#define LV_DIR_BOTTOM               8
#define LV_DIR_HOR                  (LV_DIR_LEFT | LV_DIR_RIGHT)
#define LV_DIR_VER                  (LV_DIR_TOP | LV_DIR_BOTTOM)
#define LV_DIR_ALL                  (LV_DIR_HOR | LV_DIR_VER)

#define LV_PART_MAIN                0
#define LV_PART_SCROLLBAR           1
#define LV_PART_INDICATOR           2
#define LV_PART_KNOB                3
#define LV_PART_SELECTED            4
#define LV_PART_ITEMS               5
#define LV_PART_TICKS               6
#define LV_PART_CURSOR              7

#define LV_STATE_DEFAULT            0
#define LV_STATE_CHECKED            1
#define LV_STATE_FOCUSED            2
#define LV_STATE_FOCUS_KEY          4
#define LV_STATE_EDITED             8
#define LV_STATE_HOVERED            16
#define LV_STATE_PRESSED            32
#define LV_STATE_SCROLLED           64
#define LV_STATE_DISABLED           128

// Style properties
#define LV_STYLE_WIDTH              0
#define LV_STYLE_HEIGHT             1
#define LV_STYLE_X                  2
#define LV_STYLE_Y                  3
#define LV_STYLE_ALIGN              4
#define LV_STYLE_RADIUS             5
#define LV_STYLE_PAD_TOP            6
#define LV_STYLE_PAD_BOTTOM         7
#define LV_STYLE_PAD_LEFT           8
#define LV_STYLE_PAD_RIGHT          9
#define LV_STYLE_MARGIN_TOP         10
#define LV_STYLE_MARGIN_BOTTOM      11
#define LV_STYLE_MARGIN_LEFT        12
#define LV_STYLE_MARGIN_RIGHT       13
#define LV_STYLE_BG_COLOR           14
#define LV_STYLE_BG_OPA             15
#define LV_STYLE_BORDER_COLOR       16
#define LV_STYLE_BORDER_WIDTH       17
#define LV_STYLE_BORDER_OPA         18
#define LV_STYLE_OUTLINE_COLOR      19
#define LV_STYLE_OUTLINE_WIDTH      20
#define LV_STYLE_OUTLINE_OPA        21
#define LV_STYLE_SHADOW_COLOR       22
#define LV_STYLE_SHADOW_WIDTH       23
#define LV_STYLE_SHADOW_OPA         24
#define LV_STYLE_IMG_OPA            25
#define LV_STYLE_IMG_RECOLOR        26
#define LV_STYLE_IMG_RECOLOR_OPA    27
#define LV_STYLE_LINE_COLOR         28
#define LV_STYLE_LINE_WIDTH         29
#define LV_STYLE_LINE_OPA           30
#define LV_STYLE_ARC_COLOR          31
#define LV_STYLE_ARC_WIDTH          32
#define LV_STYLE_ARC_OPA            33
#define LV_STYLE_TEXT_COLOR         34
#define LV_STYLE_TEXT_OPA           35
#define LV_STYLE_TEXT_FONT          36

// Opacity constants
#define LV_OPA_TRANSP               0
#define LV_OPA_0                    0
#define LV_OPA_25                   64
#define LV_OPA_50                   127
#define LV_OPA_75                   191
#define LV_OPA_100                  255
#define LV_OPA_COVER                255

// Mock LVGL state management
class MockLVGLState {
public:
    static MockLVGLState& instance() {
        static MockLVGLState inst;
        return inst;
    }
    
    struct MockObject {
        lv_coord_t x = 0, y = 0, width = 0, height = 0;
        lv_obj_t* parent = nullptr;
        std::vector<lv_obj_t*> children;
        std::map<lv_style_prop_t, int32_t> styles;
        bool visible = true;
        void* user_data = nullptr;
        std::function<void(lv_obj_t*)> event_callback;
    };
    
    lv_obj_t* createObject(lv_obj_t* parent = nullptr) {
        auto obj = new lv_obj_t();
        objects[obj] = std::make_unique<MockObject>();
        if (parent) {
            objects[obj]->parent = parent;
            if (objects.count(parent)) {
                objects[parent]->children.push_back(obj);
            }
        }
        return obj;
    }
    
    void deleteObject(lv_obj_t* obj) {
        if (objects.count(obj)) {
            // Remove from parent's children
            if (objects[obj]->parent && objects.count(objects[obj]->parent)) {
                auto& children = objects[objects[obj]->parent]->children;
                children.erase(std::remove(children.begin(), children.end(), obj), children.end());
            }
            // Delete children recursively
            for (auto child : objects[obj]->children) {
                deleteObject(child);
            }
            objects.erase(obj);
        }
        delete obj;
    }
    
    MockObject* getObject(lv_obj_t* obj) {
        return objects.count(obj) ? objects[obj].get() : nullptr;
    }
    
    void reset() {
        for (auto& [obj, mock] : objects) {
            delete obj;
        }
        objects.clear();
        screen = nullptr;
        activeScreen = nullptr;
    }
    
    void setScreen(lv_obj_t* scr) { screen = scr; }
    lv_obj_t* getScreen() { return screen; }
    void setActiveScreen(lv_obj_t* scr) { activeScreen = scr; }
    lv_obj_t* getActiveScreen() { return activeScreen; }
    
private:
    std::map<lv_obj_t*, std::unique_ptr<MockObject>> objects;
    lv_obj_t* screen = nullptr;
    lv_obj_t* activeScreen = nullptr;
};

// Object functions
inline lv_obj_t* lv_obj_create(lv_obj_t* parent) {
    return MockLVGLState::instance().createObject(parent);
}

inline void lv_obj_del(lv_obj_t* obj) {
    MockLVGLState::instance().deleteObject(obj);
}

inline void lv_obj_set_pos(lv_obj_t* obj, lv_coord_t x, lv_coord_t y) {
    auto mock = MockLVGLState::instance().getObject(obj);
    if (mock) {
        mock->x = x;
        mock->y = y;
    }
}

inline void lv_obj_set_size(lv_obj_t* obj, lv_coord_t w, lv_coord_t h) {
    auto mock = MockLVGLState::instance().getObject(obj);
    if (mock) {
        mock->width = w;
        mock->height = h;
    }
}

inline void lv_obj_set_width(lv_obj_t* obj, lv_coord_t w) {
    auto mock = MockLVGLState::instance().getObject(obj);
    if (mock) mock->width = w;
}

inline void lv_obj_set_height(lv_obj_t* obj, lv_coord_t h) {
    auto mock = MockLVGLState::instance().getObject(obj);
    if (mock) mock->height = h;
}

inline void lv_obj_align(lv_obj_t* obj, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs) {
    auto mock = MockLVGLState::instance().getObject(obj);
    if (mock) {
        mock->x = x_ofs;
        mock->y = y_ofs;
    }
}

inline void lv_obj_add_style(lv_obj_t* obj, lv_style_t* style, lv_part_t part) {
    // Mock implementation
}

inline void lv_obj_set_style_prop(lv_obj_t* obj, lv_style_prop_t prop, int32_t value, lv_part_t part) {
    auto mock = MockLVGLState::instance().getObject(obj);
    if (mock) {
        mock->styles[prop] = value;
    }
}

inline void lv_obj_set_user_data(lv_obj_t* obj, void* user_data) {
    auto mock = MockLVGLState::instance().getObject(obj);
    if (mock) mock->user_data = user_data;
}

inline void* lv_obj_get_user_data(lv_obj_t* obj) {
    auto mock = MockLVGLState::instance().getObject(obj);
    return mock ? mock->user_data : nullptr;
}

// Screen functions
inline lv_obj_t* lv_scr_act() {
    return MockLVGLState::instance().getActiveScreen();
}

inline void lv_scr_load(lv_obj_t* scr) {
    MockLVGLState::instance().setActiveScreen(scr);
}

// Style functions
inline void lv_style_init(lv_style_t* style) {}
inline void lv_style_reset(lv_style_t* style) {}
inline void lv_style_set_prop(lv_style_t* style, lv_style_prop_t prop, int32_t value) {}
inline void lv_style_set_bg_color(lv_style_t* style, lv_color_t color) {}
inline void lv_style_set_bg_opa(lv_style_t* style, uint8_t opa) {}
inline void lv_style_set_text_color(lv_style_t* style, lv_color_t color) {}
inline void lv_style_set_text_opa(lv_style_t* style, uint8_t opa) {}
inline void lv_style_set_line_color(lv_style_t* style, lv_color_t color) {}
inline void lv_style_set_line_width(lv_style_t* style, lv_coord_t width) {}
inline void lv_style_set_length(lv_style_t* style, lv_coord_t length) {}
inline void lv_style_set_arc_width(lv_style_t* style, lv_coord_t width) {}
inline void lv_obj_invalidate(lv_obj_t* obj) {}

// Color functions
inline lv_color_t lv_color_hex(uint32_t c) { 
    lv_color_t color = {static_cast<uint16_t>(c & 0xFFFF)}; 
    return color; 
}

inline lv_color_t lv_color_white() { 
    lv_color_t color = {0xFFFF}; 
    return color; 
}

inline lv_color_t lv_color_black() { 
    lv_color_t color = {0}; 
    return color; 
}

inline lv_color_t lv_color_red() {
    lv_color_t color = {0xF800};
    return color;
}

inline lv_color_t lv_color_green() {
    lv_color_t color = {0x07E0};
    return color;
}

inline lv_color_t lv_color_blue() {
    lv_color_t color = {0x001F};
    return color;
}

// Event system
struct lv_event_t {
    lv_obj_t* target;
    uint8_t code;
    void* user_data;
    void* param;
};

typedef void (*lv_event_cb_t)(lv_event_t*);
typedef uint8_t lv_event_code_t;

#define LV_EVENT_CLICKED        1
#define LV_EVENT_PRESSED        2
#define LV_EVENT_RELEASED       3
#define LV_EVENT_VALUE_CHANGED  4
#define LV_EVENT_FOCUSED        5
#define LV_EVENT_DEFOCUSED      6

inline void lv_obj_add_event_cb(lv_obj_t* obj, lv_event_cb_t event_cb, lv_event_code_t filter, void* user_data) {
    auto mock = MockLVGLState::instance().getObject(obj);
    if (mock) {
        mock->event_callback = [event_cb, filter, user_data](lv_obj_t* target) {
            lv_event_t event;
            event.target = target;
            event.code = filter;
            event.user_data = user_data;
            event.param = nullptr;
            event_cb(&event);
        };
    }
}

// Timer and tick functions
inline void lv_tick_inc(uint32_t tick_period) {}
inline uint32_t lv_timer_handler() { return 1; }

// Arc widget (for gauges)
inline lv_obj_t* lv_arc_create(lv_obj_t* parent) {
    return MockLVGLState::instance().createObject(parent);
}

inline void lv_arc_set_value(lv_obj_t* arc, int16_t value) {}
inline void lv_arc_set_range(lv_obj_t* arc, int16_t min, int16_t max) {}
inline void lv_arc_set_angles(lv_obj_t* arc, uint16_t start, uint16_t end) {}

// Label widget
inline lv_obj_t* lv_label_create(lv_obj_t* parent) {
    return MockLVGLState::instance().createObject(parent);
}

inline void lv_label_set_text(lv_obj_t* label, const char* text) {}
inline void lv_label_set_text_fmt(lv_obj_t* label, const char* fmt, ...) {}

// Image widget
inline lv_obj_t* lv_img_create(lv_obj_t* parent) {
    return MockLVGLState::instance().createObject(parent);
}

inline void lv_img_set_src(lv_obj_t* img, const void* src_img) {}

#endif // UNIT_TESTING