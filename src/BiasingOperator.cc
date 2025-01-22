#include "BiasingOperator.hh"

#include "G4Gamma.hh"
#include "G4ProcessManager.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4ParticleTable.hh"

#include "Util.hh"

namespace lircst {
    BiasingOperator::BiasingOperator()
    : G4VBiasingOperator("lircstDataBiasingOperator"),
    fSplittingFactor(10), // TODO: scale probabiltiy based on "cone of acceptance" proportion
    fApplyProbability(1.0) {
        fSplitAndKillOperation = new BiasingOperation("splitAndkill");

        fParticleToBias = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
        
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
        // ---------------
        // -- Setup stage:
        // ---------------
        // -- get the particle process manager...
        const G4ProcessManager* processManager = fParticleToBias->GetProcessManager();
        // -- ... to obtain the biasing information shared among this particle processes:
        fBiasingSharedData = G4BiasingProcessInterface::GetSharedData( processManager );

        fBiasingLimiterProcess = fBiasingSharedData->GetParallelGeometriesLimiterProcess();
        const_cast<G4ParallelGeometriesLimiterProcess*>(fBiasingLimiterProcess)->AddParallelWorld( "ParaWorld" );

        fSplitAndKillOperation->SetSplittingFactor ( fSplittingFactor  );
        fSplitAndKillOperation->SetApplyProbability( fApplyProbability );
        G4cout << GetName() << " : starting run with splitting factor = " << fSplittingFactor
                << ", and probability for applying the technique " << fApplyProbability
                << " . " << G4endl;

        G4ThreeVector SDPosition = G4ThreeVector(0, Util::GetWorldSize() * Util::GetWorldGunSDRatio(), 0);        
        G4ThreeVector incidentIntersection = G4ThreeVector(0,0,0); // With our setup, the intersection point is the centre of our world
        fCollPosition = Util::GetCollSDToIncidentRatio() * (SDPosition + incidentIntersection);

        fCollTolerance = std::cos(Util::GetCollCosAcceptanceDeg() * deg);
    }

    G4VBiasingOperation* BiasingOperator::ProposeNonPhysicsBiasingOperation( const G4Track* track,
                                        const G4BiasingProcessInterface* callingProcess) {

        // Check if particle is a gamma
        if ( track->GetDefinition() != G4Gamma::GammaDefinition() ) return 0;

        auto step = track->GetStep();

        // Get pre-step momentum direction
        G4ThreeVector momentumDirection = step->GetPreStepPoint()->GetMomentumDirection();

        // Compute dot product with collimator position and see if it's within acceptable tolerance
        G4ThreeVector expectedDirection = (fCollPosition - step->GetPreStepPoint()->GetPosition()).unit();
        G4double alignment = expectedDirection.dot(momentumDirection); // No std::abs here because we only want those travelling towards collimator
        if (alignment < fCollTolerance) return 0;

        G4cout << "Proposing biasing operation as we meet the criteria in our Biasing Operator" << G4endl;

        return fSplitAndKillOperation;
    }
}