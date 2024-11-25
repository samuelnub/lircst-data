#ifndef ACCUMULABLEMAP_HH
#define ACCUMULABLEMAP_HH

#include <map>
#include "G4Types.hh"

namespace lircst {
    class AccumulableMap {
    public:
        AccumulableMap() = default;
        ~AccumulableMap() = default;

        // Add values from another AccumulableMap
        AccumulableMap& operator+=(const AccumulableMap& other) {
            for (const auto& pair : other.fMap) {
                fMap[pair.first] += pair.second;  // Accumulate values
            }
            return *this;
        }

        AccumulableMap& operator+(const AccumulableMap& other) {
            for (const auto& pair : other.fMap) {
                fMap[pair.first] += pair.second;  // Accumulate values
            }
            return *this;
        }

        AccumulableMap& operator*(const AccumulableMap& other) {
            for (const auto& pair : other.fMap) {
                fMap[pair.first] *= pair.second;  // Accumulate values
            }
            return *this;
        }

        // Reset the map
        void Reset() {
            fMap.clear();
        }

        // Access the internal map
        const std::map<G4int, G4double>& GetMap() const {
            return fMap;
        }

        // Add a value to a specific key
        void AddValue(G4int key, G4double value) {
            fMap[key] += value;
        }

    private:
        std::map<G4int, G4double> fMap;
    };
}

#endif // ACCUMULABLEMAP_HH