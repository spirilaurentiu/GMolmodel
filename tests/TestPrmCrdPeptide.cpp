/** @file
 * This file contains a native Gmolmodel test. A 2-butanol molecule
 * is simulated.
 */

#include <string>
#include <iostream>
#include <sstream>
#include "simmain.hpp"

#include "HamiltonianMonteCarloSampler.hpp"
#include "readAmberInput.hpp"
#include "SetupReader.hpp"

int main(int argc, char **argv)
{
    // Initialize setup reader
    SetupReader setupReader(argv[1]);

    // Build Gmolmodel simulation world
    World *p_world;
    if(setupReader.getValues("VISUAL")[0] == "TRUE"){
        p_world = new World(true, std::stod(setupReader.getValues("FREE_TIMESTEP")[0]));
    }else{
        p_world = new World(false, std::stod(setupReader.getValues("FREE_TIMESTEP")[0]));
    }

    // Get input filenames
    std::vector<std::string> argValues;
    std::vector<std::string>::iterator argValuesIt;
    argValues = setupReader.getValues("MOLECULES");
    for(argValuesIt = argValues.begin(); argValuesIt != argValues.end(); ++argValuesIt){
        // Get molecule files
        std::string prmtopFN = *argValuesIt + std::string("/ligand.prmtop");
        std::string inpcrdFN = *argValuesIt + std::string("/ligand.inpcrd");
        std::string rbFN = *argValuesIt + std::string("/ligand.rb");
        std::string flexFN = *argValuesIt + std::string("/ligand.flex");

        // Get regimen
        std::string ictd = setupReader.getValues("REGIMEN")[0];

        // Read Amber prmtop and inpcrd
        readAmberInput *amberReader = new readAmberInput();
        amberReader->readAmberFiles(inpcrdFN, prmtopFN);

        // Add molecules
        p_world->AddMolecule(amberReader, rbFN, flexFN, ictd);

        delete amberReader;
    }

    p_world->Init();


    // Initialize sampler
    srand (time(NULL));
    HamiltonianMonteCarloSampler *p_HMCsampler = new HamiltonianMonteCarloSampler(p_world->system, p_world->matter, p_world->lig1, p_world->forceField, p_world->forces, p_world->ts);
    Context *context = new Context(p_world, p_HMCsampler);
    World *world = context->getWorld();
    world->forceField->setTracing(true);

   // Simulate
    std::cout << std::fixed << std::setprecision(4);
    const SimTK::State& constRefState = world->integ->getState();
    SimTK::State& integAdvancedState = world->integ->updAdvancedState();
    p_HMCsampler->initialize( integAdvancedState, 
                              std::stod(setupReader.getValues("FREE_TIMESTEP")[0]),
                              std::stoi(setupReader.getValues("STEPS")[0]),
                              SimTK::Real( std::stod(setupReader.getValues("TEMPERATURE")[0]) ) );
    //world->forceField->dump();
    //world->forceField->dumpCForceFieldParameters(std::cout);
    //world->system->realize(integAdvancedState, SimTK::Stage::Dynamics);
    std::cout << "Initial const state PE: " << std::setprecision(20)
        << world->forces->getMultibodySystem().calcPotentialEnergy(constRefState)
        << " integ advanced state PE: "
        << world->forces->getMultibodySystem().calcPotentialEnergy(integAdvancedState) 
        << " sampler temperature: "
        << p_HMCsampler->getTemperature()
        << std::endl;
    
    for(int i = 0; i < std::stoi(setupReader.getValues("STEPS")[0]); i++){

        p_HMCsampler->update(integAdvancedState, 
            std::stod(setupReader.getValues("FREE_TIMESTEP")[0]),
            std::stoi(setupReader.getValues("FREE_MDSTEPS")[0]) );

        writePdb( ((SimTK::Compound)(world->getTopology(0, 0))), integAdvancedState, "pdbs", "sb_", 8, "HMC0s", i);
        writePdb( ((SimTK::Compound)(world->getTopology(1, 0))), integAdvancedState, "pdbs", "sb_", 8, "HMC1s", i);
        //writePdb( ((SimTK::Compound)(world->getTopology(2, 0))), integAdvancedState, "pdbs", "sb_", 8, "HMC2s", i);
        //writePdb( ((SimTK::Compound)(world->getTopology(3, 0))), integAdvancedState, "pdbs", "sb_", 8, "HMC3s", i);
    }

}



