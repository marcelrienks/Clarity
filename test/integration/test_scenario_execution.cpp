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

// Enhanced Phase 2 scenarios for better edge case coverage

void test_scenario_power_cycle_recovery() {
    // Test system recovery after simulated power cycle
    fixture->clearScenario();
    
    fixture->addScenarioStep("System running with custom config",
        [&]() {
            fixture->setTheme(Theme::NIGHT);
            fixture->getPanelService()->show_panel(PanelType::KEY);
            fixture->triggerKeyPresent(true);
        },
        [&]() {
            return fixture->verifyPanelShown(PanelType::KEY) && 
                   fixture->verifyTheme(Theme::NIGHT);
        });
    
    fixture->addScenarioStep("Simulate power cycle - reset all state",
        [&]() {
            fixture->simulatePowerCycle();
        },
        [&]() {
            return fixture->verifySystemReset();
        });
    
    fixture->addScenarioStep("System restarts and recovers preferences",
        [&]() {
            fixture->initializeSystem();
        },
        [&]() {
            return fixture->verifyPanelShown(PanelType::SPLASH) &&
                   fixture->verifyTheme(Theme::DAY); // Should restore defaults
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_memory_pressure_handling() {
    // Test behavior under simulated memory pressure
    fixture->clearScenario();
    
    fixture->addScenarioStep("Create memory pressure",
        [&]() {
            fixture->simulateMemoryPressure(80); // 80% memory usage
        },
        [&]() {
            return fixture->verifyMemoryUsage() < 90; // Should not exceed 90%
        });
    
    fixture->addScenarioStep("System continues to function under pressure",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::OEM_OIL);
            fixture->triggerLights(true);
        },
        [&]() {
            return fixture->verifyPanelShown(PanelType::OEM_OIL) &&
                   fixture->verifyTheme(Theme::NIGHT);
        });
    
    fixture->addScenarioStep("Memory pressure relieved",
        [&]() {
            fixture->simulateMemoryPressure(30); // Release pressure
        },
        [&]() {
            return fixture->verifyMemoryUsage() < 50;
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_invalid_state_recovery() {
    // Test recovery from invalid hardware states
    fixture->clearScenario();
    
    fixture->addScenarioStep("Start in valid state",
        [&]() {
            fixture->getPanelService()->show_panel(PanelType::KEY);
            fixture->triggerKeyPresent(true);
        },
        [&]() {
            return fixture->verifyPanelShown(PanelType::KEY);
        });
    
    fixture->addScenarioStep("Force invalid key state (both present and not present)",
        [&]() {
            fixture->triggerKeyPresent(true);
            fixture->triggerKeyNotPresent(true); // Invalid state
        },
        [&]() {
            // System should handle gracefully - either stay on KEY or go to safe state
            PanelType currentPanel = fixture->getPanelService()->get_current_panel();
            return currentPanel == PanelType::KEY || 
                   currentPanel == PanelType::OEM_OIL ||
                   currentPanel == PanelType::SPLASH;
        });
    
    fixture->addScenarioStep("Return to valid state",
        [&]() {
            fixture->triggerKeyNotPresent(false);
        },
        [&]() {
            return fixture->verifyTriggerState(TriggerType::KEY_PRESENT, true) &&
                   fixture->verifyTriggerState(TriggerType::KEY_NOT_PRESENT, false);
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_concurrent_trigger_bursts() {
    // Test handling of rapid concurrent trigger changes
    fixture->clearScenario();
    
    fixture->addScenarioStep("Burst of concurrent triggers",
        [&]() {
            for (int i = 0; i < 20; i++) {
                // Simulate realistic scenarios where multiple triggers change simultaneously
                if (i % 4 == 0) {
                    // Car startup: Key present + lights off + lock off
                    fixture->triggerKeyPresent(true);
                    fixture->triggerKeyNotPresent(false);
                    fixture->triggerLights(false);
                    fixture->triggerLock(false);
                } else if (i % 4 == 1) {
                    // Driving: lights on (night)
                    fixture->triggerLights(true);
                } else if (i % 4 == 2) {
                    // Parking: lock on
                    fixture->triggerLock(true);
                } else {
                    // Leaving car: key removed
                    fixture->triggerKeyPresent(false);
                    fixture->triggerKeyNotPresent(true);
                }
                fixture->waitForTime(5); // 5ms between bursts
            }
        },
        [&]() {
            // Final state should be consistent
            return fixture->verifyTriggerState(TriggerType::KEY_NOT_PRESENT, true) &&
                   fixture->verifyPanelShown(PanelType::KEY);
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void test_scenario_theme_persistence_stress() {
    // Test theme persistence under various state changes
    fixture->clearScenario();
    
    fixture->addScenarioStep("Start with day theme",
        [&]() {
            fixture->setTheme(Theme::DAY);
            fixture->getPanelService()->show_panel(PanelType::OEM_OIL);
        },
        [&]() {
            return fixture->verifyTheme(Theme::DAY);
        });
    
    fixture->addScenarioStep("Rapid theme switching with panel changes",
        [&]() {
            for (int i = 0; i < 50; i++) {
                Theme theme = (i % 2 == 0) ? Theme::NIGHT : Theme::DAY;
                fixture->setTheme(theme);
                
                // Change panels during theme switching
                if (i % 5 == 0) fixture->getPanelService()->show_panel(PanelType::KEY);
                else if (i % 5 == 1) fixture->getPanelService()->show_panel(PanelType::LOCK);
                else if (i % 5 == 2) fixture->getPanelService()->show_panel(PanelType::OEM_OIL);
                else if (i % 5 == 3) fixture->getPanelService()->show_panel(PanelType::SPLASH);
                
                fixture->waitForTime(2); // 2ms between changes
            }
        },
        [&]() {
            // Final theme should be DAY (last iteration with i=49, 49%2==1)
            return fixture->verifyTheme(Theme::DAY);
        });
    
    bool result = fixture->executeScenario();
    TEST_ASSERT_TRUE(result);
}

void runScenarioExecutionTests() {
    setUp_scenario_execution();
    
    // Original tests
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
    
    // Enhanced Phase 2 scenarios
    RUN_TEST(test_scenario_power_cycle_recovery);
    RUN_TEST(test_scenario_memory_pressure_handling);
    RUN_TEST(test_scenario_invalid_state_recovery);
    RUN_TEST(test_scenario_concurrent_trigger_bursts);
    RUN_TEST(test_scenario_theme_persistence_stress);
    
    tearDown_scenario_execution();
}

#endif