#ifndef PRIMARYGENERATORACTION_HH
#define PRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"

#include "BetaSpectrum.hh"

#include <random>

namespace lircst {

    enum SpectrumType {
        Mono = 0,
        Poly = 1,
    };

    class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
    public:
        PrimaryGeneratorAction();
        ~PrimaryGeneratorAction() override = default;

        void SetGantryAngle(G4double angle) { fGantryAngle = angle; }
        void GeneratePrimaries(G4Event*) override;

    private:
        SpectrumType fSpectrumType = SpectrumType::Mono;

        G4double fGantryAngle = 45 * deg; //0.0 * radian;

        beta_distribution<> fBetaDist = beta_distribution<>(2.0, 4.0); // According to Analytical parameters
        G4float fBetaMaxEnergy = 0.120 * MeV; // According to Analytical parameters

        std::random_device rd;
        std::mt19937 gen;

        G4ParticleGun* fParticleGun;
    };
}

#endif // PRIMARYGENERATORACTION_HH