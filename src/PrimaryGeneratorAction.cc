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

        // Rotating gantry around Z axis
        G4double radius = Util::GetSourceDistIsocenter();
        G4ThreeVector pos(
            radius * std::cos(fGantryAngle),
            radius * std::sin(fGantryAngle),
            0
        );

        G4ThreeVector axis = -pos.unit(); // Point towards the centre of the world

        // We want a cone beam with the same angular spread as the one subtended by the phantom at the source, so we randomise the momentum direction within that cone
        G4double alpha = std::atan(Util::GetPhantomSize() / Util::GetSourceDistIsocenter());

        // Sample by Solid angle, not Angle!
        G4double xi = G4UniformRand();
        G4double cosTheta = 1.0 - xi * (1.0 - std::cos(alpha));
        G4double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);
        G4double phi = G4UniformRand() * 2.0 * CLHEP::pi;

        G4ThreeVector dir_local(
            sinTheta * std::cos(phi),
            sinTheta * std::sin(phi),
            cosTheta
        );

        G4ThreeVector dir = dir_local.rotateUz(axis);
        this->fParticleGun->SetParticlePosition(pos);
        this->fParticleGun->SetParticleMomentumDirection(dir);

        this->fParticleGun->GeneratePrimaryVertex(anEvent);
    }
}