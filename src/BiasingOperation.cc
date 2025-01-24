#include "BiasingOperation.hh"

#include "Randomize.hh"

#include "Util.hh"

// Primarily taken from GB03 example in Geant4 examples/extended/biasing

namespace lircst {
    BiasingOperation::BiasingOperation(G4String name) 
    :   G4VBiasingOperation(name), 
        fParticleChange(),
        fParticleChangeForNothing()
    {
        G4ThreeVector SDPosition = G4ThreeVector(0, Util::GetWorldSize() * Util::GetWorldGunSDRatio(), 0);        
        G4ThreeVector incidentIntersection = G4ThreeVector(0,0,0); // With our setup, the intersection point is the centre of our world
        fCollPosition = Util::GetCollSDToIncidentRatio() * (SDPosition + incidentIntersection);

        // We widen the tolerance to give more leeway. The actual SD collimator will have the narrow tolerance
        G4double widenCollFactor = 10;
        fCollTolerance = std::cos(Util::GetCollCosAcceptanceDeg() * widenCollFactor * deg);

        // Set split factor and application probability
        G4double preciseSplittingFactor = 2.0 / (1.0 - fCollTolerance);
        fSplittingFactor = static_cast<G4int>(std::round(preciseSplittingFactor));
        fApplyProbability = 1.0;

        G4cout << "Splitting factor: " << fSplittingFactor << G4endl;
    }

    G4double BiasingOperation::DistanceToApplyOperation( const G4Track* ,
                                    G4double ,
                                    G4ForceCondition* condition) {
        // -- return "infinite" distance for interaction, but asks for GenerateBiasingFinalState
        // -- being called anyway at the end of the step, by returning the "Forced" condition
        // -- flag.
        *condition = Forced;
        return DBL_MAX;
    }

    G4VParticleChange* BiasingOperation::GenerateBiasingFinalState( const G4Track* track,
                                      const G4Step* step) {

        G4cout << "GenerateBiasingFinalState Current step number with step status: " << track->GetCurrentStepNumber() << " " << step->GetPostStepPoint()->GetStepStatus() << G4endl;

        // Check if step is limited by the geometry: as we attached the biasing operator
        // to the absorber layer, this volume boundary is the one of the absorber.
        // (check of current step # of track is inelegant, but is to fix a "feature"
        // that a cloned track can wrongly be seen in the wrong volume, because of numerical
        // precision issue. In this case it makes a tiny step, which should be disregarded).
        if (( step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary ) &&
            ( track->GetCurrentStepNumber() != 1 ) &&
            ( true ) ) { // TODO: ignore if we have just entered the paraphantom geometry

            // -- Before deciding for killing or splitting, we make decision on applying
            // -- the technique or not:
            G4double trial = G4UniformRand(); // -- Note: G4UniformRand() is thread-safe
                                                // -- engine for random numbers

            if ( trial <= fApplyProbability ) {
                // -- Now, we decide
                // Solid angle biasing.

                auto step = track->GetStep();

                // Get pre-step momentum direction
                G4ThreeVector momentumDirection = step->GetPreStepPoint()->GetMomentumDirection();

                // Compute dot product with collimator position and see if it's within acceptable tolerance
                G4ThreeVector expectedDirection = (fCollPosition - step->GetPreStepPoint()->GetPosition()).unit();
                G4double alignment = expectedDirection.dot(momentumDirection); // No std::abs here because we only want those travelling towards collimator

                if (alignment > fCollTolerance) {
                    // -------------------------------------------------
                    // Here, We do "splitting":
                    // -------------------------------------------------

                    // Get track weight:
                    G4double initialWeight = track->GetWeight();
                    // Define the tracks weight:
                    G4double weightOfTrack = initialWeight/fSplittingFactor;
                    
                    // The "particle change" is the object to be used to communicate to
                    // the tracking the update of the primary state and/or creation
                    // secondary tracks.
                    fParticleChange.Initialize(*track);
                    
                    // ask currect track weight to be changed to new value:
                    fParticleChange.ProposeParentWeight( weightOfTrack );
                    
                    // Now make clones of this track (this is the actual splitting):
                    // we will then have the primary and N-1 clones of it, hence the
                    // splitting by a factor N:
                    fParticleChange.SetNumberOfSecondaries( fSplittingFactor-1 );
                    for ( G4int iSplit = 1 ; iSplit <  fSplittingFactor ; iSplit++ ) {
                        G4Track* clone = new G4Track( *track );
                        clone->SetWeight( weightOfTrack );
                        fParticleChange.AddSecondary( clone );
                    }
                    fParticleChange.SetSecondaryWeightByProcess(true); // -- tricky
                    // -- take it as is ;) [though not necessary here, put for safety]

                    // this new final state is returned to the tracking;
                    return &fParticleChange;
                    
                }
                else {
                    // --------------------------------------------------------------
                    // Here, We do killing, playing a russian
                    // roulette, killing 1/fSplittingFactor of the tracks in average:
                    // --------------------------------------------------------------

                    // Get track weight:
                    G4double initialWeight = track->GetWeight();
                    
                    // The "particle change" is the object to be used to communicate to
                    // the tracking the update of the primary state and/or creation
                    // secondary tracks.
                    fParticleChange.Initialize(*track);
                    
                    // Shoot a random number (in ]0,1[ segment):
                    G4double random = G4UniformRand();
                    
                    // Decide to kill, keeping 1/fSplittingFactor of tracks:
                    G4double survivingProbability = 1.0/fSplittingFactor;
                    if ( random > survivingProbability ) {
                        // We ask for the the track to be killed:
                        fParticleChange.ProposeTrackStatus(fStopAndKill);
                    }
                    else {
                        // In this case, the track survives. We change its weight
                        // to conserve weight among killed and survival tracks:
                        fParticleChange.ProposeParentWeight( initialWeight*fSplittingFactor );
                    }
                    
                    // this new final state is returned to the tracking;
                    return &fParticleChange;
                }
            }  // -- end of : if ( trial > probaForApplying )
        }      // -- end of : if ( ( step->GetPostStepPoint()->GetStepStatus() ==
                //                                                       fGeomBoundary ) ...
        

        // Here, the step was not limited by the geometry (but certainly by a physics
        // process). We do nothing: ie we make no change to the current track.
        fParticleChangeForNothing.Initialize(*track);
        return &fParticleChangeForNothing;
    }
}