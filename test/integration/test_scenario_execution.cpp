#ifdef UNIT_TESTING

#include <unity.h>
#include "../utilities/test_fixtures.h"
#include "../../include/utilities/types.h"

std::unique_ptr<IntegrationTestFixture> fixture;

void setUp_scenario_execution() {
    fixture = std::make_unique<IntegrationTestFixture>();
    fixture->SetUp();
}

void tearDown_scenario_execution() {
    fixture->TearDown();
    fixture.reset();
}

void test_major_scenario_full_system() {
    // Major Scenario (Full System Test) from todo.md
    // > App starts with pressure and temperature values set to halfway  
    // > → Splash animates with day theme (white text)  
    // > → Oil panel loads with day theme (white scale ticks and icon)  
    // > → Oil panel needles animate  
    // > → Lights trigger high  
    // > → Oil panel does NOT reload, theme changes to night (red scale ticks and icon)  
    // > → Lock trigger high  ... (etc)
    
    fixture->clearScenario();
    
    // Step 1: App starts with day theme
    fixture->addScenarioStep("App starts with day theme", 
        [&]() {
            fixture->setTheme(Theme::DAY);
        },
        [&]() {
            return fixture->verifyTheme(Theme::DAY);
        });
    
    // Step 2: Splash animates 
    fixture->addScenarioStep("Splash panel loads",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::SPLASH);
        },
        [&]() {
            return fixture->verifyPanelShown(PanelType::SPLASH);
        });
    
    // Step 3: Oil panel loads with day theme
    fixture->addScenarioStep("Oil panel loads with day theme",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::OEM_OIL);
        },
        [&]() {
            return fixture->verifyPanelShown(PanelType::OEM_OIL) && 
                   fixture->verifyTheme(Theme::DAY);
        });
    
    // Step 4: Lights trigger high (theme should change to night)
    fixture->addScenarioStep("Lights trigger high - theme changes to night",
        [&]() {
            fixture->triggerLights(true);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::LIGHTS, true) &&
                   fixture->verifyTheme(Theme::NIGHT);
        });
    
    // Step 5: Lock trigger high
    fixture->addScenarioStep("Lock trigger high - Lock panel loads",
        [&]() {
            fixture->triggerLock(true);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::LOCK, true) &&
                   fixture->verifyPanelShown(PanelType::LOCK);
        });
    
    // Step 6: Key not present trigger high
    fixture->addScenarioStep("Key not present trigger high - Key panel loads",
        [&]() {
            fixture->triggerKeyNotPresent(true);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::KEY_NOT_PRESENT, true) &&
                   fixture->verifyPanelShown(PanelType::KEY);
        });
    
    // Step 7: Key present trigger high (invalid state)
    fixture->addScenarioStep("Key present trigger high - Lock panel loads (invalid state handling)",
        [&]() {
            fixture->triggerKeyPresent(true);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::KEY_PRESENT, true) &&
                   fixture->verifyPanelShown(PanelType::LOCK);
        });
    
    // Step 8: Key not present trigger low
    fixture->addScenarioStep("Key not present trigger low - Key panel loads (present=true)",
        [&]() {
            fixture->triggerKeyNotPresent(false);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::KEY_NOT_PRESENT, false) &&
                   fixture->verifyPanelShown(PanelType::KEY);
        });
    
    // Step 9: Key present trigger low
    fixture->addScenarioStep("Key present trigger low - Lock panel loads",
        [&]() {
            fixture->triggerKeyPresent(false);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::KEY_PRESENT, false) &&
                   fixture->verifyPanelShown(PanelType::LOCK);
        });
    
    // Step 10: Lock trigger low
    fixture->addScenarioStep("Lock trigger low - Oil panel loads with night theme",
        [&]() {
            fixture->triggerLock(false);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::LOCK, false) &&
                   fixture->verifyPanelShown(PanelType::OEM_OIL) &&
                   fixture->verifyTheme(Theme::NIGHT);
        });
    
    // Step 11: Lights trigger low
    fixture->addScenarioStep("Lights trigger low - theme changes to day",
        [&]() {
            fixture->triggerLights(false);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::LIGHTS, false) &&
                   fixture->verifyTheme(Theme::DAY);
        });
    
    // Execute the complete scenario
    bool scenarioResult = fixture->executeScenario();
    TEST_ASSERT_TRUE(scenarioResult);
}

void test_scenario_app_start_splash() {
    // Individual Test Scenario: App starts → Splash animates with day theme
    fixture->clearScenario();
    
    fixture->addScenarioStep("App starts with day theme",
        [&]() {
            fixture->setTheme(Theme::DAY);
        },
        [&]() {
            return fixture->verifyTheme(Theme::DAY);
        });
    
    fixture->addScenarioStep("Splash panel loads and animates",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::SPLASH);
            fixture->waitForTime(1000); // Wait for animation
        },
        [&]() {
            return fixture->verifyPanelShown(PanelType::SPLASH);
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_oil_panel_basic() {
    // App starts with pressure and temperature values → Oil panel loads
    fixture->clearScenario();
    
    fixture->addScenarioStep("App starts",
        [&]() {
            fixture->setTheme(Theme::DAY);
        });
    
    fixture->addScenarioStep("Splash loads",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::SPLASH);
        });
    
    fixture->addScenarioStep("Oil panel loads with day theme",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::OEM_OIL);
            fixture->waitForTime(500); // Wait for needle animation
        },
        [&]() {
            return fixture->verifyPanelShown(PanelType::OEM_OIL) &&
                   fixture->verifyTheme(Theme::DAY);
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_key_present_workflow() {
    // Key present trigger workflow
    fixture->clearScenario();
    
    fixture->addScenarioStep("Start with oil panel",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::OEM_OIL);
        });
    
    fixture->addScenarioStep("Key present trigger high → Key panel loads (green icon)",
        [&]() {
            fixture->triggerKeyPresent(true);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::KEY_PRESENT, true) &&
                   fixture->verifyPanelShown(PanelType::KEY);
        });
    
    fixture->addScenarioStep("Key present trigger low → Oil panel loads",
        [&]() {
            fixture->triggerKeyPresent(false);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::KEY_PRESENT, false) &&
                   fixture->verifyPanelShown(PanelType::OEM_OIL);
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_key_not_present_workflow() {
    // Key not present trigger workflow
    fixture->clearScenario();
    
    fixture->addScenarioStep("Start with oil panel",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::OEM_OIL);
        });
    
    fixture->addScenarioStep("Key not present trigger high → Key panel loads (red icon)",
        [&]() {
            fixture->triggerKeyNotPresent(true);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::KEY_NOT_PRESENT, true) &&
                   fixture->verifyPanelShown(PanelType::KEY);
        });
    
    fixture->addScenarioStep("Key not present trigger low → Oil panel loads",
        [&]() {
            fixture->triggerKeyNotPresent(false);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::KEY_NOT_PRESENT, false) &&
                   fixture->verifyPanelShown(PanelType::OEM_OIL);
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_lock_trigger_workflow() {
    // Lock trigger workflow
    fixture->clearScenario();
    
    fixture->addScenarioStep("Start with oil panel",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::OEM_OIL);
        });
    
    fixture->addScenarioStep("Lock trigger high → Lock panel loads",
        [&]() {
            fixture->triggerLock(true);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::LOCK, true) &&
                   fixture->verifyPanelShown(PanelType::LOCK);
        });
    
    fixture->addScenarioStep("Lock trigger low → Oil panel loads",
        [&]() {
            fixture->triggerLock(false);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::LOCK, false) &&
                   fixture->verifyPanelShown(PanelType::OEM_OIL);
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_lights_theme_change() {
    // Lights trigger theme change (no panel reload)
    fixture->clearScenario();
    
    fixture->addScenarioStep("Start with oil panel in day theme",
        [&]() {
            fixture->setTheme(Theme::DAY);
            fixture->getPanelService()->show_panel(PanelType::OEM_OIL);
        },
        [&]() {
            return fixture->verifyTheme(Theme::DAY) &&
                   fixture->verifyPanelShown(PanelType::OEM_OIL);
        });
    
    fixture->addScenarioStep("Lights trigger high → Theme changes to night (no reload)",
        [&]() {
            fixture->triggerLights(true);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::LIGHTS, true) &&
                   fixture->verifyTheme(Theme::NIGHT) &&
                   fixture->verifyPanelShown(PanelType::OEM_OIL); // Same panel, no reload
        });
    
    fixture->addScenarioStep("Lights trigger low → Theme changes to day (no reload)",
        [&]() {
            fixture->triggerLights(false);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::LIGHTS, false) &&
                   fixture->verifyTheme(Theme::DAY) &&
                   fixture->verifyPanelShown(PanelType::OEM_OIL); // Same panel, no reload
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_startup_with_active_triggers() {
    // App starts with triggers already active
    fixture->clearScenario();
    
    fixture->addScenarioStep("App starts with key present trigger already high",
        [&]() {
            fixture->triggerKeyPresent(true); // Set before showing panels
            fixture->setTheme(Theme::DAY);
        });
    
    fixture->addScenarioStep("Splash animates",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::SPLASH);
            fixture->waitForTime(1000);
        });
    
    fixture->addScenarioStep("Oil panel does NOT load, Key panel loads instead",
        [&]() {
            // In normal flow, oil panel would load, but key trigger should override
            fixture->waitForTime(100);
        },
        [&]() {
            return fixture->verifyPanelShown(PanelType::KEY) &&
                   fixture->verifyTriggerState(TriggerType::KEY_PRESENT, true);
        });
    
    fixture->addScenarioStep("Key present trigger low → Oil panel loads",
        [&]() {
            fixture->triggerKeyPresent(false);
        },
        [&]() {
            return fixture->verifyPanelShown(PanelType::OEM_OIL);
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_complex_trigger_interactions() {
    // Test complex interactions between multiple triggers
    fixture->clearScenario();
    
    fixture->addScenarioStep("Start with oil panel",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::OEM_OIL);
        });
    
    fixture->addScenarioStep("Multiple triggers activate simultaneously",
        [&]() {
            fixture->triggerLights(true);    // Should change theme
            fixture->triggerKeyPresent(true); // Should change panel
            fixture->triggerLock(true);       // Should override key panel
        },
        [&]() {
            // Lock should have highest priority
            return fixture->verifyTheme(Theme::NIGHT) &&
                   fixture->verifyPanelShown(PanelType::LOCK) &&
                   fixture->verifyTriggerState(TriggerType::LIGHTS, true) &&
                   fixture->verifyTriggerState(TriggerType::KEY_PRESENT, true) &&
                   fixture->verifyTriggerState(TriggerType::LOCK, true);
        });
    
    fixture->addScenarioStep("Deactivate lock trigger",
        [&]() {
            fixture->triggerLock(false);
        },
        [&]() {
            // Key panel should now be visible
            return fixture->verifyPanelShown(PanelType::KEY) &&
                   fixture->verifyTriggerState(TriggerType::KEY_PRESENT, true);
        });
    
    fixture->addScenarioStep("Deactivate all triggers",
        [&]() {
            fixture->triggerKeyPresent(false);
            fixture->triggerLights(false);
        },
        [&]() {
            // Should return to oil panel with day theme
            return fixture->verifyPanelShown(PanelType::OEM_OIL) &&
                   fixture->verifyTheme(Theme::DAY);
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_performance_stress() {
    // Stress test with rapid trigger changes
    fixture->clearScenario();
    
    fixture->addScenarioStep("Rapid trigger changes",
        [&]() {
            for (int i = 0; i < 100; i++) {
                fixture->triggerKeyPresent(i % 2);
                fixture->triggerKeyNotPresent((i + 1) % 2);
                fixture->triggerLock((i + 2) % 2);
                fixture->triggerLights((i + 3) % 2);
                fixture->waitForTime(1); // 1ms between changes
            }
        },
        [&]() {
            // System should still be responsive and in a valid state
            PanelType currentPanel = fixture->getPanelService()->get_current_panel();
            return currentPanel == PanelType::SPLASH ||
                   currentPanel == PanelType::OEM_OIL ||
                   currentPanel == PanelType::KEY ||
                   currentPanel == PanelType::LOCK;
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void runScenarioExecutionTests() {
    setUp_scenario_execution();
    
    RUN_TEST(test_major_scenario_full_system);
    RUN_TEST(test_scenario_app_start_splash);
    RUN_TEST(test_scenario_oil_panel_basic);
    RUN_TEST(test_scenario_key_present_workflow);
    RUN_TEST(test_scenario_key_not_present_workflow);
    RUN_TEST(test_scenario_lock_trigger_workflow);
    RUN_TEST(test_scenario_lights_theme_change);
    RUN_TEST(test_scenario_startup_with_active_triggers);
    RUN_TEST(test_scenario_complex_trigger_interactions);
    RUN_TEST(test_scenario_performance_stress);
    
    tearDown_scenario_execution();
}

#endif