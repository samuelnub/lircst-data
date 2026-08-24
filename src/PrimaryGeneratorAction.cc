#include "PrimaryGeneratorAction.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4MTRunManager.hh"

#include "RunManager.hh"
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

        //if (fSpectrumType == SpectrumType::Poly) {
        //    G4double particleEnergy = fBetaDist(gen) * fBetaMaxEnergy;
        //    this->fParticleGun->SetParticleEnergy(particleEnergy);
        //}

        auto runManager = static_cast<RunManager*>(G4MTRunManager::GetMasterRunManager());
        G4double gantryAngle = runManager->GetCurrentGantryAngleRad();

        gantryAngle -= Util::GetGunAngleOffset(); // Mainly for debugging purposes, for our CST system, it should subtract 0.0f

        // Rotating gantry around Z axis
        G4double radius = Util::GetSourceDistIsocenter();
        G4ThreeVector pos(
            radius * std::cos(gantryAngle),
            radius * std::sin(gantryAngle),
            0
        );

        G4ThreeVector axis = -pos.unit(); // Point towards the centre of the world
        G4double alpha;
        G4ThreeVector dir_local;
        G4ThreeVector dir;

        switch (fBeamShape) {
            case BeamShape::Cone: {
                    // We want a cone beam with the same angular spread as the one subtended by the phantom at the source, so we randomise the momentum direction within that cone
                    // Sample by Solid angle, not Angle!
                    alpha = std::atan(Util::GetPhantomSize() / Util::GetSourceDistIsocenter());
                    G4double xi = G4UniformRand();
                    G4double cosTheta = 1.0 - xi * (1.0 - std::cos(alpha));
                    G4double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);
                    G4double phi = G4UniformRand() * 2.0 * CLHEP::pi;

                    dir_local = G4ThreeVector(
                        sinTheta * std::cos(phi),
                        sinTheta * std::sin(phi),
                        cosTheta
                    );
                }
                break;
            case BeamShape::Pyramid: {
                    alpha = std::atan(Util::GetPhantomSize() / Util::GetSourceDistIsocenter());
                    G4double x = Util::GenRandomDouble(-std::tan(alpha), std::tan(alpha));
                    G4double y = Util::GenRandomDouble(-std::tan(alpha), std::tan(alpha));
                    dir_local = G4ThreeVector(x, y, 1.0).unit();
                }
                break;
            default:
                G4Exception("PrimaryGeneratorAction::GeneratePrimaries", "BEAMSHAPE001", FatalException, "Unknown beam shape");
                break;
        }

        dir = dir_local.rotateUz(axis);
        this->fParticleGun->SetParticlePosition(pos);
        this->fParticleGun->SetParticleMomentumDirection(dir);
        this->fParticleGun->GeneratePrimaryVertex(anEvent);
    }
}