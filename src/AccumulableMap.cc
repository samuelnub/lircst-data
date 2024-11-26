#include "AccumulableMap.hh"

namespace lircst {
    AccumulableMap::AccumulableMap(const G4String& name) : G4VAccumulable(name), fMap() {}

    void AccumulableMap::AddValue(G4int key, G4double value) {
        fMap[key] += value;
    }

    G4double AccumulableMap::GetValue(G4int key) const {
        return fMap.at(key);
    }

    void AccumulableMap::Merge(const G4VAccumulable& other) {
        G4cout << "In AccumulableMap::Merge()" << G4endl;
        const auto& otherMap = static_cast<const AccumulableMap&>(other).fMap;
        for (const auto& entry : otherMap) {
            G4int key = entry.first;
            G4double value = entry.second;
            fMap[key] += value;
        }
    }

    void AccumulableMap::Reset() {
        fMap.clear();
    }
}