#ifndef __HAMMONTECARLOSAMPLER_HPP__
#define __HAMMONTECARLOSAMPLER_HPP__

#include "MonteCarloSampler.hpp"

class Topology;
class IState;

class HamiltonianMonteCarloSampler : public MonteCarloSampler
{
public:

    // Constructor
    HamiltonianMonteCarloSampler(SimTK::CompoundSystem *argCompoundSystem,
                                 SimTK::SimbodyMatterSubsystem *argMatter,
                                 SimTK::Compound *argResidue,
                                 SimTK::TimeStepper *argTimeStepper);

    // Destructor
    virtual ~HamiltonianMonteCarloSampler();

    // Initialize variables (like TVector)
    void initialize(SimTK::State& advanced, SimTK::Real timestep, int nosteps); 

    // Assign a random conformation. Time measured in picoseconds
    void propose(SimTK::State& someState, SimTK::Real timestep, int nosteps);

    // Performs the acception-rejection step and sets the state of the compound
    // to the appropriate conformation
    void update(SimTK::State& someState, SimTK::Real timestep, int nosteps);

    // Set kinetic energies    
    void setOldKE(SimTK::Real);

protected:
       SimTK::Real ke_o; // The kinetic energy retained for acc-rej step

};

#endif // __HAMMONTECARLOSAMPLER_HPP__

