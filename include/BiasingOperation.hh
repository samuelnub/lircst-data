#ifndef BIASINGOPERATION_HH
#define BIASINGOPERATION_HH

#include "G4VBiasingOperation.hh"
#include "G4ParticleChange.hh"
#include "G4ParticleChangeForNothing.hh"

namespace lircst {
    class BiasingOperation : public G4VBiasingOperation {
    public:
        BiasingOperation(G4String name);
        ~BiasingOperation() override = default;

        // Unused in our case
        virtual const G4VBiasingInteractionLaw* ProvideOccurenceBiasingInteractionLaw( const G4BiasingProcessInterface*,
                                                G4ForceCondition&                 ) {return 0;}
        virtual G4VParticleChange* ApplyFinalStateBiasing( const G4BiasingProcessInterface*,
                                                const G4Track*,
                                                const G4Step*,
                                                G4bool&                           ) {return 0;}

        // Used in our case
        virtual G4double DistanceToApplyOperation( const G4Track*,
                                        G4double,
                                        G4ForceCondition* condition );
        virtual G4VParticleChange* GenerateBiasingFinalState( const G4Track*,
                                          const G4Step* );


        void SetSplittingFactor( G4int splittingFactor ) { fSplittingFactor = splittingFactor; }
        void SetApplyProbability( G4double proba ) { fApplyProbability = proba; }

        G4int    GetSplittingFactor()  const { return fSplittingFactor;  }
        G4double GetApplyProbability() const { return fApplyProbability; }

    private:
        G4ParticleChange            fParticleChange;
        G4ParticleChangeForNothing  fParticleChangeForNothing;
        G4int    fSplittingFactor;
        G4double fApplyProbability;
    };
}


#endif // BIASINGOPERATION_HH