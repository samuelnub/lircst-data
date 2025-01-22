#include "PhysicsList.hh"

#include "G4PhysicsListHelper.hh"
#include "G4Gamma.hh"
#include "G4Positron.hh"
#include "G4Electron.hh"
#include "G4PhotoElectricEffect.hh"
#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4RayleighScattering.hh"

#include "G4BiasingHelper.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4ProcessManager.hh"

namespace lircst {
    void PhysicsList::ConstructParticle() {
        G4Gamma::GammaDefinition();
        G4Positron::PositronDefinition();
        G4Electron::ElectronDefinition();
    }

    void PhysicsList::ConstructProcess() {
        this->AddTransportation();

        auto physicsHelper = G4PhysicsListHelper::GetPhysicsListHelper();
        
        auto particle = G4Gamma::GammaDefinition();
        physicsHelper->RegisterProcess(new G4PhotoElectricEffect, particle);
        physicsHelper->RegisterProcess(new G4ComptonScattering, particle);
        physicsHelper->RegisterProcess(new G4GammaConversion, particle);
        physicsHelper->RegisterProcess(new G4RayleighScattering, particle);

        // For biasing
        G4BiasingHelper::ActivateNonPhysicsBiasing(particle->GetProcessManager());
        G4BiasingHelper::AddLimiterProcess(particle->GetProcessManager());
    }
}