/* -------------------------------------------------------------------------- *
 *                 SimTK Molmodel Example: Simple Protein                     *
 * -------------------------------------------------------------------------- *
 * This is the first example from the Molmodel User's Guide. It creates a     *
 * small protein (a five-residue peptide), simulates it and generates a live  *
 * animation while it is running.                                             *
 *                                                                            *
 * Authors: Christopher Bruns, Michael Sherman                                *
 * -------------------------------------------------------------------------- */

#include "Molmodel.h"
#include <iostream>
#include <exception>
using namespace SimTK;

int main() {
try {
    
    // SIMBODY CONSTRUCTORS WORK:
    System someSystem;
    MultibodySystem someMultibodySystem; // Extends System (Simbody) // adoptSystemGuts(new MultibodySystemRep())
    SimbodyMatterSubsystem matter(someMultibodySystem); // Extends Subsystem (Simbody)
    DecorationSubsystem decorations(someMultibodySystem); // Extends Subsystem (Simbody)
    // =====================

    // MOLMODEL CONSTRUCTORS DON'T WORK:
    MolecularMechanicsSystem someMolecularMechanicsSystem; // Extends MultibodySystem (Simbody)
    //CompoundSystem system; // Extends MolecularMechanicsSystem (Molmodel)
    //SimbodyMatterSubsystem matter(system);
    //DecorationSubsystem decorations(system);
    //DuMMForceFieldSubsystem forceField(system);
    //forceField.loadAmber99Parameters();
    //Protein protein("SIMTK");
    //protein.assignBiotypes();
    //system.adoptCompound(protein);
    //system.modelCompounds(); 
    //system.addEventReporter(new Visualizer::Reporter(system, 0.020));
    //system.addEventHandler(new VelocityRescalingThermostat(	   system,  293.15, 0.1));
    //State state = system.realizeTopology();
    //LocalEnergyMinimizer::minimizeEnergy(system, state, 15.0);
    //VerletIntegrator integ(system);
    //TimeStepper ts(system, integ);
    // =====================

    //ts.initialize(state);
    //ts.stepTo(20.0); // 20ps

    return 0;
} 
catch(const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
}
catch(...) {
    std::cerr << "ERROR: An unknown exception was raised" 
              << std::endl;
    return 1;
}

}


