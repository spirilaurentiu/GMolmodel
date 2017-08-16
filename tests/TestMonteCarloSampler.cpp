#include "Robo.hpp"
#include "MonteCarloSampler.hpp"


int main (int argc, char **argv)
{

    SimTK::DuMMForceFieldSubsystem duMM;
    bSpecificAtom *p_SpecificAtom = new bSpecificAtom;
    bBond *p_Bond = new bBond;
    double **pp_D1;
    double **pp_D2;
    double *p_D1;
    double *p_D2;
   
    Topology *p_residue = new Topology();
    p_residue->init(duMM, 0, p_SpecificAtom, 0, p_Bond, pp_D1, pp_D2, p_D1, p_D2, true, "RES");

    //Sampler *p_sampler = new Sampler(p_residue, p_currentState);
    //MonteCarloSampler MCSampler(p_residue, p_currentState);

    return 0;
}