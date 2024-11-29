#include "PrimaryGeneratorAction.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include "Util.hh"

namespace lircst {
    PrimaryGeneratorAction::PrimaryGeneratorAction() : G4VUserPrimaryGeneratorAction() {
        
        G4int noOfParticles = 1;
        this->fParticleGun = new G4ParticleGun(noOfParticles);

        // Set particle properties
        G4double particleEnergy = 60.0 * keV; // 60 keV seems reasonable to see the Compton effect take place

        this->fParticleGun->SetParticleDefinition(G4Gamma::GammaDefinition());
        this->fParticleGun->SetParticleEnergy(particleEnergy);

        // We'll set physical location / direction per event
    }

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
        // Set particle pos / dir
        // We'll have a parallel beam
        G4double beamWidth = Util::GetScorerSize() / 2;
        G4double beamHeight = Util::GetScorerSize() / 2;
        G4double x = 0; // G4UniformRand() * beamWidth - beamWidth / 2;
        G4double y = 0; // G4UniformRand() * beamHeight - beamHeight / 2;
        // ^^^ Pencil beam

        this->fParticleGun->SetParticlePosition(G4ThreeVector(x, y, -(Util::GetWorldSize() * Util::GetGunSDRatio())));
        this->fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));

        this->fParticleGun->GeneratePrimaryVertex(anEvent);
    }
}