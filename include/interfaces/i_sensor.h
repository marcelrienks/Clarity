#ifndef I_SENSOR_H
#define I_SENSOR_H

class ISensor
{
public:    
    virtual void init();
    virtual int get_reading();
};

#endif // I_SENSOR_H

//TODO: Generics
// Base class for all reading types
// class ReadingBase {
//     public:
//         virtual ~ReadingBase() = default;
//     };
    
//     // Interface for all sensors
//     class ISensor {
//     public:
//         // Pure virtual method returning the base type
//         virtual ReadingBase* get_reading() = 0;
        
//         // Helper template method for type-safe casting
//         template <typename T>
//         T* get_reading_as() {
//             return dynamic_cast<T*>(get_reading());
//         }
        
//         virtual void init();
//         virtual ~ISensor() = default;
//     };
    
//     // Template for specific reading types
//     template <typename T>
//     class Reading : public ReadingBase {
//     private:
//         T value;
        
//     public:
//         explicit Reading(T val) : value(val) {}
        
//         T getValue() const { return value; }
//     };