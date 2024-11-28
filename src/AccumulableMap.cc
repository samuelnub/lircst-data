#include "AccumulableMap.hh"

namespace lircst {
    template <typename T>
    AccumulableMap<T>::AccumulableMap(const G4String& name) : G4VAccumulable(name), fMap() {}

    template <typename T>
    void AccumulableMap<T>::AddValue(G4int key, T value) {
        fMap[key] += value;
    }

    template <typename T>
    T AccumulableMap<T>::GetValue(G4int key) const {
        return fMap.at(key);
    }

    template <typename T>
    void AccumulableMap<T>::Merge(const G4VAccumulable& other) {
        G4cout << "In AccumulableMap::Merge()" << G4endl;
        const auto& otherMap = static_cast<const AccumulableMap<T>&>(other).fMap;
        for (const auto& entry : otherMap) {
            G4int key = entry.first;
            G4double value = entry.second;
            fMap[key] += value;
        }
    }

    template <typename T>
    void AccumulableMap<T>::Reset() {
        fMap.clear();
    }

    template class AccumulableMap<G4int>;
}