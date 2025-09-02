# Clarity Documentation Index

This is the central documentation index for the Clarity ESP32 automotive gauge system. Documents are organized hierarchically to avoid circular references and provide clear navigation paths.

## 📋 Quick Reference

- **Current Implementation Status**: [implementation-status.md](implementation-status.md) - What's built vs planned
- **Architecture Overview**: [architecture.md](architecture.md) - Current v3.0 system architecture
- **Hardware Configuration**: [hardware.md](hardware.md) - GPIO mappings and display setup
- **Testing Strategy**: [testing-strategy.md](testing-strategy.md) - Realistic testing approach

## 📚 Document Hierarchy

### Level 1: Overview & Status
Start here to understand the current system:

1. **[Implementation Status](implementation-status.md)** - ⭐ **READ FIRST**
   - What's actually implemented vs documented plans  
   - Current v3.0 vs proposed v4.0 architecture
   - Status of all major components

### Level 2: Core Architecture  
Current implementation details:

2. **[Architecture](architecture.md)** - Current v3.0 Implementation
   - MVP pattern structure
   - PolledHandler/QueuedHandler interrupt system
   - Sensor ownership model
   - Memory architecture

3. **[Hardware Configuration](hardware.md)**
   - GPIO pin mappings
   - Display specifications  
   - Wokwi emulator setup
   - Power requirements

### Level 3: Implementation Guidance
Development guidelines and requirements:

4. **[Requirements](requirements.md)**
   - Functional requirements
   - Non-functional requirements
   - Memory analysis and constraints
   - Performance targets

5. **[Standards](standards.md)**
   - Coding conventions
   - File organization
   - Naming patterns

6. **[Patterns](patterns.md)** 
   - Architectural patterns in use
   - Design principles
   - Best practices

### Level 4: Technical Details
Detailed technical specifications:

7. **[Sensor Implementation](sensor.md)**
   - BaseSensor pattern
   - Change detection
   - GPIO vs ADC sensors

8. **[Error Handling](error.md)**
   - Error classification
   - Error reporting
   - Recovery procedures

9. **[Input System](input.md)**
   - Button handling
   - Timing detection
   - Universal actions

10. **[Testing Strategy](testing-strategy.md)**
    - Platform limitations
    - Testing approaches
    - Memory validation

### Level 5: Reference & Planning
Additional reference materials:

11. **[Development Scenarios](scenarios.md)**
    - Use case scenarios
    - User workflows
    - System interactions

12. **[TODO List](todo/todo.md)**
    - Current development tasks
    - Feature backlog

### Level 6: Visual Documentation
Detailed architectural diagrams:

13. **[Architecture Overview](diagrams/architecture-overview.md)**
    - Component relationship diagram
    - System boundaries

14. **[Application Flow](diagrams/application-flow.md)**
    - Startup sequence
    - Runtime processing flow

15. **[Interrupt Handling Flow](diagrams/interrupt-handling-flow.md)**
    - Detailed interrupt processing
    - Handler coordination

### Level 7: Future Planning
Proposed future enhancements:

16. **[Proposed v4.0 Architecture](plans/proposed-interrupt-architecture.md)**
    - Trigger/Action separation proposal
    - Migration planning
    - Advanced features

## 🎯 Reading Paths by Use Case

### New to the Project
1. [Implementation Status](implementation-status.md) - Understand current vs planned
2. [Architecture](architecture.md) - Learn the system structure  
3. [Hardware Configuration](hardware.md) - Understand the hardware setup

### Contributing to Development  
1. [Standards](standards.md) - Coding conventions
2. [Architecture](architecture.md) - System structure
3. [Testing Strategy](testing-strategy.md) - How to test changes
4. [TODO List](todo/todo.md) - Available tasks

### Adding New Features
1. [Architecture](architecture.md) - Understand existing patterns
2. [Patterns](patterns.md) - Follow established practices
3. [Requirements](requirements.md) - Understand constraints
4. [Sensor Implementation](sensor.md) or [Error Handling](error.md) - Relevant subsystem

### Debugging Issues
1. [Implementation Status](implementation-status.md) - Verify what's actually implemented
2. [Hardware Configuration](hardware.md) - Check GPIO mappings
3. [Error Handling](error.md) - Error system capabilities
4. [Testing Strategy](testing-strategy.md) - Validation approaches

### Understanding Performance
1. [Requirements](requirements.md) - Memory analysis and performance targets
2. [Architecture](architecture.md) - System efficiency design
3. [Testing Strategy](testing-strategy.md) - Memory validation approaches

## ⚠️ Important Notes

### Architecture Versions
- **v3.0**: Current implementation (PolledHandler/QueuedHandler)
- **v4.0**: Proposed future architecture (TriggerHandler/ActionHandler)
- **Documents marked "v3.0" describe actual implementation**
- **Documents marked "v4.0" describe proposed future work**

### Deprecated Documents
These documents are kept for reference but may contain outdated information:
- `architecture-original.md` - Original architecture document (pre-correction)

### Document Dependencies
This hierarchy minimizes circular references:
- Higher level documents may reference lower level documents
- Lower level documents should not extensively reference higher level documents
- Cross-references at the same level are acceptable but should be minimal

## 📝 Document Maintenance

### Updating Documentation
When making changes:
1. Update [Implementation Status](implementation-status.md) if implementation status changes
2. Update [Architecture](architecture.md) if system structure changes
3. Update this index if new documents are added or organization changes

### Avoiding Circular References
- Each document should have a clear primary purpose
- Extensive cross-referencing should be avoided
- Use this index for navigation rather than embedding navigation in each document

### Version Control
- All documents should indicate their applicable system version (v3.0 vs v4.0)
- Major architectural changes should be reflected in [Implementation Status](implementation-status.md)
- Proposed changes go in the `plans/` directory until implemented

---

**Last Updated**: January 2025  
**System Version**: v3.0 (current implementation)