#include "PrimaryGeneratorAction.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

namespace lircst {
    PrimaryGeneratorAction::PrimaryGeneratorAction() 
        : G4VUserPrimaryGeneratorAction(), 
          fParticleGun(0) {
        
        G4int noOfParticles = 1;
        this->fParticleGun = new G4ParticleGun(noOfParticles);

        // Set particle properties
        G4double particleEnergy = 6.0 * MeV;

        this->fParticleGun->SetParticleDefinition(G4Gamma::GammaDefinition());
        this->fParticleGun->SetParticleEnergy(particleEnergy);

        // We'll set physical location / direction per event
    }

    void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
        // Set particle pos / dir
        // We'll have a parallel beam
        G4double beamWidth = 5.0 * cm;
        G4double beamHeight = 5.0 * cm;
        G4double x = G4UniformRand() * beamWidth - beamWidth / 2;
        G4double y = G4UniformRand() * beamHeight - beamHeight / 2;

        this->fParticleGun->SetParticlePosition(G4ThreeVector(x, y, -10.0 * cm));
        this->fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));

        this->fParticleGun->GeneratePrimaryVertex(anEvent);
    }
}