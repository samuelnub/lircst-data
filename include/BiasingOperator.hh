#ifndef BIASEDOPERATOR_HH
#define BIASEDOPERATOR_HH

#include "G4VBiasingOperator.hh"

#include "BiasingOperation.hh"

#include "G4ParticleDefinition.hh"
#include "G4ParallelGeometriesLimiterProcess.hh"
#include "G4BiasingProcessSharedData.hh"
#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"
#include "G4VPhysicalVolume.hh"

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

        void SetParallelWorld( G4VPhysicalVolume* parallelWorld )
        { fParallelWorld = parallelWorld; }

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

        // What the hell is this
        const G4ParticleDefinition*                      fParticleToBias;
        G4VPhysicalVolume*                                fParallelWorld;
        G4int                                        fParallelWorldIndex;
        const G4BiasingProcessSharedData*             fBiasingSharedData;
        const G4ParallelGeometriesLimiterProcess* fBiasingLimiterProcess;
    };
}

#endif // BIASEDOPERATOR_HH