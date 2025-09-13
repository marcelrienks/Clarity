# Development Guidelines

## String Memory Management

### The Problem
During Step 5 optimization, we replaced `std::string` with `const char*` for panel names. This broke splash screen transitions and restoration mechanisms because:

1. `config.panelName.c_str()` returns a temporary pointer
2. `splashTargetPanel_ = panelName` stored the pointer (not the data)
3. By the time splash completed (~1500ms), the source string was deallocated
4. System tried to load panel with garbage data → crash
5. **Same issue affected restoration**: `restorationPanel = currentPanel` copied pointer, not data

### Cost-Benefit Analysis of String Optimizations

**Performance Impact:**
- `const char*`: 8 bytes (pointer only)
- `std::string`: ~24 bytes + string data (~32 bytes total)
- **Memory savings**: ~24 bytes per variable

**Complexity Cost:**
- Hybrid approach requires double storage per variable
- Complex lifetime management logic
- Developer guidelines and training overhead
- Ongoing maintenance burden and bug risk
- **Total complexity cost**: High

**ESP32 Context:**
- 320KB RAM available
- Panel variables: 2-3 total
- **Total memory impact**: ~50 bytes (0.016% of RAM)
- **Verdict**: Negligible performance gain, high complexity cost

### The Revised Rule
**Use `std::string` for all panel names and dynamic strings - safety over micro-optimization**
**Use `const char*` only for true static constants (PanelNames::*, ActionIds::*, etc.)**

### Examples

```cpp
// ✅ GOOD - Simple and safe (REVISED APPROACH)
std::string currentPanel = PanelNames::OIL;        // Safe for all use cases
std::string restorationPanel = PanelNames::OIL;    // Safe for all use cases
ProcessPanel(currentPanel.c_str());                // Minimal overhead

// ✅ GOOD - Static constants
const char* PANEL_NAME = PanelNames::OIL;          // True constants only
ProcessStaticPanel(PANEL_NAME);

// ❌ BAD - Premature optimization
const char* currentPanel = panelName;              // Unsafe pointer storage
const char* restorationPanel = currentPanel;       // Pointer aliasing issues
```

### Code Review Checklist
- [ ] Is this a true static constant? → `const char*` OK
- [ ] Is this a dynamic/runtime value? → Use `std::string`
- [ ] Does micro-optimization justify complexity? → Usually NO on ESP32

---

*This guideline was created after the Step 5 optimization bug to prevent similar issues.*