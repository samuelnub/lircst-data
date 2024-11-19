#ifndef PHYSICSLIST_HH
#define PHYSICSLIST_HH

#include "G4VUserPhysicsList.hh"

namespace lircst {
    class PhysicsList : public G4VUserPhysicsList {
    public:
        PhysicsList() = default;
        ~PhysicsList() override = default;

        void ConstructParticle() override;
        void ConstructProcess() override;
    };
}

#endif // PHYSICSLIST_HH