#include "BiasingOperator.hh"

namespace lircst {
    BiasingOperator::BiasingOperator()
    : G4VBiasingOperator("lircstDataBiasingOperator"),
    fSplittingFactor(2),
    fApplyProbability(1.0) {
        fSplitAndKillOperation = new BiasingOperation("splitAndkill");
        
        // -- Define messengers:
        fSplittingFactorMessenger = 
            new G4GenericMessenger(this, "/lircstData/biasing/","Biasing control" );
        
        G4GenericMessenger::Command& splittingFactorCmd = 
            fSplittingFactorMessenger->DeclareProperty("setSplittingFactor", fSplittingFactor,
                                        "Define the splitting factor." );
        splittingFactorCmd.SetStates(G4State_Idle);
        
        fApplyProbabilityMessenger = 
            new G4GenericMessenger(this, "/lircstData/biasing/","Biasing control" );
        
        G4GenericMessenger::Command& applyProbCmd = 
            fApplyProbabilityMessenger->DeclareProperty("setApplyProbability", fApplyProbability,
                                        "Define the probability to apply the splitting/killing." );
        applyProbCmd.SetStates(G4State_Idle);
    }

    BiasingOperator::~BiasingOperator() {
        delete fSplitAndKillOperation;
    }

    void BiasingOperator::StartRun() {
        fSplitAndKillOperation->SetSplittingFactor ( fSplittingFactor  );
        fSplitAndKillOperation->SetApplyProbability( fApplyProbability );
        G4cout << GetName() << " : starting run with splitting factor = " << fSplittingFactor
                << ", and probability for applying the technique " << fApplyProbability
                << " . " << G4endl;
    }

    G4VBiasingOperation* BiasingOperator::ProposeNonPhysicsBiasingOperation( const G4Track* track,
                                        const G4BiasingProcessInterface* callingProcess) {
        // TODO: copilot generated

        // TODO: implement solid angle dependent biasing

        // -- We apply the biasing operation only to primary tracks:
        if ( track->GetParentID() == 0 ) return fSplitAndKillOperation;
        else return 0;
    }
}