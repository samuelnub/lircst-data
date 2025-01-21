#ifndef BIASEDOPERATOR_HH
#define BIASEDOPERATOR_HH

#include "G4VBiasingOperator.hh"

#include "BiasingOperation.hh"

#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"

namespace lircst {
    class BiasingOperator : public G4VBiasingOperator {
    public:
        BiasingOperator();
        virtual ~BiasingOperator();

        // ------------------------------
        // Method added for this example:
        // ------------------------------
        BiasingOperation* GetSplitAndKillOperation() const
        { return fSplitAndKillOperation; }

        // -------------------------
        // Optional from base class:
        // -------------------------
        void StartRun();
        
    private:
        // --------------------------
        // Mandatory from base class:
        // --------------------------
        // Used for splitting/killing:
        virtual G4VBiasingOperation*
        ProposeNonPhysicsBiasingOperation( const G4Track* track,
                                            const G4BiasingProcessInterface* callingProcess );

        // Not used here:
        virtual G4VBiasingOperation* 
        ProposeOccurenceBiasingOperation( const G4Track*,
                                            const G4BiasingProcessInterface* ) { return 0; }
        // Not used here:
        virtual G4VBiasingOperation*
        ProposeFinalStateBiasingOperation( const G4Track*,
                                            const G4BiasingProcessInterface* ) { return 0; }

        private:
        BiasingOperation* fSplitAndKillOperation;
        G4int    fSplittingFactor;
        G4double fApplyProbability;
        // Messengers to change the 
        G4GenericMessenger*  fSplittingFactorMessenger;
        G4GenericMessenger* fApplyProbabilityMessenger;

        G4ThreeVector fCollPosition;
        G4double fCollTolerance;
    };
}

#endif // BIASEDOPERATOR_HH