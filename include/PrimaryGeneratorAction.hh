#ifndef PRIMARYGENERATORACTION_HH
#define PRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"

namespace lircst {
    class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
    public:
        PrimaryGeneratorAction();
        ~PrimaryGeneratorAction() override = default;

        void GeneratePrimaries(G4Event*) override;

    private:
        G4ParticleGun* fParticleGun;
    };
}

#endif // PRIMARYGENERATORACTION_HH