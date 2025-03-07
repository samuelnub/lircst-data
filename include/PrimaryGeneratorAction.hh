#ifndef PRIMARYGENERATORACTION_HH
#define PRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"

#include "BetaSpectrum.hh"

#include <random>

namespace lircst {
    enum SourceType {
        Pencil = 0,
        Parallel2D = 1,
        Parallel3D = 2,
        Cone = 3,
    };

    enum SpectrumType {
        Mono = 0,
        Poly = 1,
    };

    class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
    public:
        PrimaryGeneratorAction();
        ~PrimaryGeneratorAction() override = default;

        void GeneratePrimaries(G4Event*) override;

    private:
        SourceType fSourceType = SourceType::Cone;
        SpectrumType fSpectrumType = SpectrumType::Poly;

        beta_distribution<> fBetaDist = beta_distribution<>(2.0, 4.0); // According to Analytical parameters
        G4float fBetaMaxEnergy = 0.120 * MeV; // According to Analytical parameters

        std::random_device rd;
        std::mt19937 gen;

        G4ParticleGun* fParticleGun;
    };
}

#endif // PRIMARYGENERATORACTION_HH