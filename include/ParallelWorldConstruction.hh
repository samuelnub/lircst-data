#ifndef PARA_WORLD_CONSTRUCTION_HH
#define PARA_WORLD_CONSTRUCTION_HH

#include "G4VUserParallelWorld.hh"

namespace lircst {
    class ParallelWorldConstruction : public G4VUserParallelWorld {
    public:
        ParallelWorldConstruction(G4String worldName);
        ~ParallelWorldConstruction() override = default;

        virtual void Construct();
        virtual void ConstructSD();
    };
}

#endif // PARA_WORLD_CONSTRUCTION_HH