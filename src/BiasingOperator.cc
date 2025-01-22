#include "BiasingOperator.hh"

#include "G4Gamma.hh"
#include "G4ProcessManager.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4ParticleTable.hh"

#include "Util.hh"

namespace lircst {
    BiasingOperator::BiasingOperator()
    : G4VBiasingOperator("lircstDataBiasingOperator") {
        fSplitAndKillOperation = new BiasingOperation("splitAndkill");

        fParticleToBias = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
        
    }

    BiasingOperator::~BiasingOperator() {
        delete fSplitAndKillOperation;
    }

    void BiasingOperator::StartRun() {
        // ---------------
        // -- Setup stage:
        // ---------------
        // -- get the particle process manager...
        const G4ProcessManager* processManager = fParticleToBias->GetProcessManager();
        // -- ... to obtain the biasing information shared among this particle processes:
        fBiasingSharedData = G4BiasingProcessInterface::GetSharedData( processManager );

        fBiasingLimiterProcess = fBiasingSharedData->GetParallelGeometriesLimiterProcess();
        const_cast<G4ParallelGeometriesLimiterProcess*>(fBiasingLimiterProcess)->AddParallelWorld( "ParaWorld" );

    }

    G4VBiasingOperation* BiasingOperator::ProposeNonPhysicsBiasingOperation( const G4Track* track,
                                        const G4BiasingProcessInterface* callingProcess) {

        // Check if particle is a gamma
        if ( track->GetDefinition() != G4Gamma::GammaDefinition() ) return 0;

        return fSplitAndKillOperation;
    }
}