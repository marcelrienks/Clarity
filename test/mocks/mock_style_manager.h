#include "test_utilities.h"
#include <map>

class MockStyleManager {
public:
    struct MockThemeColors {
        local_style_lv_color_t keyPresent;
        local_style_lv_color_t keyNotPresent;
        local_style_lv_color_t gaugeNormal;
        local_style_lv_color_t gaugeDanger;
    };

    static MockStyleManager& GetInstance() {
        static MockStyleManager instance;
        return instance;
    }

    mock_lv_style_t textStyle;
    mock_lv_style_t gaugeStyle;
    mock_lv_style_t needleStyle;
    mock_lv_style_t iconStyle;

    MockThemeColors get_colours(const std::string& theme) {
        static MockThemeColors colors = {
            .keyPresent = mock_lv_color_hex(0x00FF00),
            .keyNotPresent = mock_lv_color_hex(0xFF0000),
            .gaugeNormal = mock_lv_color_hex(0xFFFFFF),
            .gaugeDanger = mock_lv_color_hex(0xFF0000)
        };
        return colors;
    }

private:
    MockStyleManager() {
        // Initialize all styles
        mock_lv_style_init(&textStyle);
        mock_lv_style_init(&gaugeStyle);
        mock_lv_style_init(&needleStyle);
        mock_lv_style_init(&iconStyle);
    }
};
