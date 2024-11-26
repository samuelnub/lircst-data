#ifndef ACCUMULABLEMAP_HH
#define ACCUMULABLEMAP_HH

#include <map>
#include "G4Types.hh"
#include "G4VAccumulable.hh"

namespace lircst {
    class AccumulableMap : public G4VAccumulable {
    public:
        AccumulableMap(const G4String& name);
        virtual ~AccumulableMap() {};

        void AddValue(G4int key, G4double value);
        G4double GetValue(G4int key) const;
        const std::map<G4int, G4double>& GetMap() const { return fMap; }
        
        virtual void Merge(const G4VAccumulable& other);
        virtual void Reset();

    private:
        std::map<G4int, G4double> fMap;
    };
}

#endif // ACCUMULABLEMAP_HH