#ifndef CELERITASGLOBALS_HH
#define CELERITASGLOBALS_HH

#include <accel/AlongStepFactory.hh>
#include <accel/FastSimulationOffload.hh>
#include <accel/LocalTransporter.hh>
#include <accel/SetupOptions.hh>
#include <accel/SharedParams.hh>
#include <accel/SimpleOffload.hh>
#include <corecel/Macros.hh>
#include <corecel/io/Logger.hh>

namespace lircst
{
    class CeleritasGlobals
    { 
    public:
        inline static celeritas::SetupOptions setup_options;
        inline static celeritas::SharedParams shared_params;
        inline static G4ThreadLocal celeritas::LocalTransporter local_transporter;
        inline static G4ThreadLocal celeritas::SimpleOffload simple_offload;
    };
}

#endif // CELERITASGLOBALS_HH