#ifndef ACCUMULABLEMAP_HH
#define ACCUMULABLEMAP_HH

#include <map>
#include "G4Types.hh"
#include "G4VAccumulable.hh"

namespace lircst {
    template <typename T>
    class AccumulableMap : public G4VAccumulable {
    public:
        AccumulableMap<T>(const G4String& name);
        virtual ~AccumulableMap() {};

        void AddValue(G4int key, T value);
        T GetValue(G4int key) const;
        const std::map<G4int, T>& GetMap() const { return fMap; }
        
        virtual void Merge(const G4VAccumulable& other);
        virtual void Reset();

    private:
        std::map<G4int, T> fMap;
    };
}

#endif // ACCUMULABLEMAP_HH