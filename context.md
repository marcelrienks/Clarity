BREAKTHROUGH! The crash is still happening even without component rendering! This proves that the issue is NOT in the component Render calls or how we're using displayProvider_.

  The crash is happening during component creation itself. We can see:
  - Component creation happens: [5494] TESTING: Component rendering disabled, but creation enabled
  - Then crash during lv_screen_load(): [5504] loading...

  This means the UIFactory::create...() calls are corrupting memory.

  Since this works in the main branch but fails in our branch, and we know the issue isn't in:
  - ❌ Component rendering
  - ❌ Display provider usage
  - ❌ StyleManager
  - ❌ Screen creation
  - ❌ Sensor initialization

  The issue must be in how components are created - specifically how the styleService is being passed to the UIFactory methods.

  In our changes, we store styleService_ as a member and pass it to:
  UIFactory::createOemOilPressureComponent(styleService_);
  UIFactory::createOemOilTemperatureComponent(styleService_);

  Let me check if there's an issue with the styleService lifetime or validity. The styleService is created in main.cpp and passed to panels, but maybe there's a dependency ordering issue.

  Looking at the memory addresses:
  - screen: 0x3ffb3aac
  - backgroundStyle: 0x3ffb841c
  - Crash at: 0x3ffb3e5c and 0x3ffb3dfc

  The crash addresses are between the screen and backgroundStyle addresses, suggesting memory corruption in the heap area between these objects.

  This points to UIFactory component creation allocating memory that corrupts adjacent heap blocks.