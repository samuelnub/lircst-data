#ifndef WORKERINITIALIZATION_HH
#define WORKERINITIALIZATION_HH

#include "G4UserWorkerInitialization.hh"
#include "G4MTRunManager.hh"

namespace lircst {
    class WorkerInitialization : public G4UserWorkerInitialization {
    public:
        WorkerInitialization() = default;
        ~WorkerInitialization() override = default;

        void WorkerRunStart() const override;
    };
}

#endif // WORKERINITIALIZATION_HH