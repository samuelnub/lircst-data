#include "ActionInitialisation.hh"

#include "PrimaryGeneratorAction.hh"
#include "EventAction.hh"
#include "RunAction.hh"

#include "CeleritasGlobals.hh"

namespace lircst {
    void ActionInitialisation::BuildForMaster() const {
        // Celeritas
        CeleritasGlobals::simple_offload.BuildForMaster(&CeleritasGlobals::setup_options, &CeleritasGlobals::shared_params);

        SetUserAction(new RunAction);
    }

    void ActionInitialisation::Build() const {
        // Celeritas
        CeleritasGlobals::simple_offload.Build(&CeleritasGlobals::setup_options, &CeleritasGlobals::shared_params, &CeleritasGlobals::local_transporter);

        SetUserAction(new RunAction);
        SetUserAction(new EventAction);
        SetUserAction(new PrimaryGeneratorAction);
    }
}