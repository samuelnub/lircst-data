#include "StackingAction.hh"

#include "G4Track.hh"

namespace lircst {
    G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track* aTrack) {
        // For now, we don't want to kill any tracks, but we might want to do that in the future
        if (aTrack->GetParentID() > 0) {
            // We don't care about secondaries, kill em (this makes the entire system lose a bit of energy, but we don't care about that)
            return fKill;
        }
        return fUrgent;
    }
}