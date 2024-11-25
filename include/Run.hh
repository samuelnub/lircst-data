#ifndef RUN_HH
#define RUN_HH

#include "G4Run.hh"

namespace lircst {
    class Run : public G4Run {
    public:
        Run() = default;
        ~Run() override = default;

        void RecordEvent(const G4Event* event) override;
        void Merge(const G4Run* aRun) override;
    };
}


#endif // RUN_HH