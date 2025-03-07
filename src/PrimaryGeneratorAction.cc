#include "PrimaryGeneratorAction.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include "Util.hh"

#include "Randomize.hh"

namespace lircst {
    PrimaryGeneratorAction::PrimaryGeneratorAction() : G4VUserPrimaryGeneratorAction() {
        
        this->gen = std::mt19937(rd());

        G4int noOfParticles = 1;
        this->fParticleGun = new G4ParticleGun(noOfParticles);

        // Set particle properties
        G4double particleEnergy = Util::GetGunEnergy();
        this->fParticleGun->SetParticleDefinition(G4Gamma::GammaDefinition());

        if (fSpectrumType == SpectrumType::Mono) {
            this->fParticleGun->SetParticleEnergy(particleEnergy);
        }
    }

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
        // Set particle pos / dir
        if (fSpectrumType == SpectrumType::Poly) {
            G4double particleEnergy = fBetaDist(gen) * fBetaMaxEnergy;
            this->fParticleGun->SetParticleEnergy(particleEnergy);
        }

        G4double x = 0;
        G4double y = 0;
        if (fSourceType == SourceType::Parallel2D || fSourceType == SourceType::Parallel3D || fSourceType == SourceType::Cone) {
            G4double beamWidth = Util::GetPhantomSize() * 2;
            G4double beamHeight = (fSourceType == SourceType::Parallel2D) ? 0 : Util::GetPhantomSize() * 2;
            x = G4UniformRand() * beamWidth - beamWidth / 2;
            y = G4UniformRand() * beamHeight - beamHeight / 2;
        }

        if (fSourceType == SourceType::Pencil || fSourceType == SourceType::Parallel2D || fSourceType == SourceType::Parallel3D) {
            this->fParticleGun->SetParticlePosition(G4ThreeVector(x, y, -(Util::GetWorldSize() * Util::GetWorldGunSDRatio())));
            this->fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));
        }
        else if (fSourceType == SourceType::Cone) {
            G4ThreeVector pos = G4ThreeVector(0, 0, -(Util::GetWorldSize() * Util::GetWorldGunSDRatio()));
            this->fParticleGun->SetParticlePosition(pos);
            this->fParticleGun->SetParticleMomentumDirection(G4ThreeVector(x, y, -(Util::GetPhantomSize())) - pos);
        }

        this->fParticleGun->GeneratePrimaryVertex(anEvent);
    }
}