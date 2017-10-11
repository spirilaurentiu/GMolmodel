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

int main(int argc, char **argv)
{
    // Declare a shared memory pointer used by all the classes in the library

    TARGET_TYPE *shm;
    int SHMSZ; // size of vestigial pipe memory

    //  Declare variables for simulation parameters

    int natoms;
    int nosteps;
    int ntrials;

    // Set simulation parameters

    nosteps = 10; // RESTORE DEL
    ntrials = 10; // RESTORE DEL
    std::cout<<"main ntrials: "<<ntrials<<std::endl;
    std::cout<<"main nosteps: "<<nosteps<<std::endl;

    // Set input filenames
 
    std::string mol2FN = std::string(argv[1]) + std::string("/ligand.mol2");
    std::string rbFN = std::string(argv[1]) + std::string("/ligand.rb");
    std::string gaffFN = "gaff.dat";
    std::string frcmodFN = std::string(argv[1]) + std::string("/ligand.frcmod");
    std::string flexFN = std::string(argv[1]) + std::string("/ligand.flex");

    // Simulation type:
    // IC: Internal Coordinates Dynamics
    // TD: Torsional Dynamics
    // RR: Rigid Rings Torsional Dynamics
    // RB: Rigid Bodies

    std::string ictd = "IC";

    std::cout<<"mol2FN "<<mol2FN<<std::endl<<std::flush;
    std::cout<<"rbFN "<<rbFN<<std::endl<<std::flush;
    std::cout<<"gaffFN "<<gaffFN<<std::endl<<std::flush;
    std::cout<<"frcmodFN "<<frcmodFN<<std::endl<<std::flush;
    std::cout<<"flexFN "<<flexFN<<std::endl<<std::flush;
    std::cout<<"ictd "<<ictd<<std::endl<<std::flush;

    // Read number of atoms from mol2 file

    std::string line;
    std::string column;

    // Read Amber prmtop and inpcrd

    readAmberInput *amberReader = new readAmberInput();
    amberReader->readAmberFiles(std::string(argv[1]) + std::string("/ligand.inpcrd"), 
        std::string(argv[1]) + std::string("/ligand.prmtop"));
    natoms = amberReader->getNumberAtoms();
    std::cout << "natoms " << natoms << std::endl << std::flush;

    // Read atom ordering from mol2
 
    int order[natoms+2]; // prmtop ORDER previously read from MMTK
    TARGET_TYPE **indexMap = NULL;
    TARGET_TYPE *PrmToAx_po = NULL;
    TARGET_TYPE *MMTkToPrm_po = NULL;
    indexMap = new TARGET_TYPE*[(natoms)];
    PrmToAx_po = new TARGET_TYPE[natoms];
    MMTkToPrm_po = new TARGET_TYPE[natoms];

    for(int i=0; i<natoms; i++){
      order[i] = i; // instead of MMTK
    }
    order[natoms] = 1;
    order[natoms+1] = 1945;

    // Set the shared memory size (SHMSZ)

    int natoms3 = 3*(natoms);

    SHMSZ = (
        2*sizeof(TARGET_TYPE) +       // Counter and flag
        natoms3*sizeof(TARGET_TYPE) + // Positions
        natoms3*sizeof(TARGET_TYPE) + // Velocities
        natoms3*sizeof(TARGET_TYPE) + // indexMap
        natoms3*sizeof(TARGET_TYPE) + // Gradients
        5*sizeof(TARGET_TYPE) +       // ac + 0 -> 4  // step, nosteps, temperature, timestep, trouble
        1*sizeof(TARGET_TYPE) +       // ac + 5       // potential energy
        2*sizeof(TARGET_TYPE) +       // ac + 6->7    // DAE step done; Move accepted
        1*sizeof(TARGET_TYPE) +       // ac + 8       // KE
        1*sizeof(TARGET_TYPE) +       // ac + 9       // steps_per_trial
        1*sizeof(TARGET_TYPE) //+     // ac + 10      // trial
    );
    shm = new TARGET_TYPE[SHMSZ];

    // Build Gmolmodel simulation world

    World *p_world = new World(amberReader, rbFN, flexFN, ictd, //PrmToAx_po, MMTkToPrm_po,
        shm); 

    // Seed the random number generator 

    srand (time(NULL));

    // Load initial shared memory values

    //bool world_initialized = false;

    // Assign convenient pointers for order

    // Assign order in shm[][2] 

    // Get coordinates in the same order as in mol2

    // Get velocities

    // Get order    

    // Get forces   

    // Get simulation parameters

    // Set the client flag

    // Assign convenient pointers for coordinates

    // Assign convenient pointers for velocities

    // Assign convenient pointers for gradients

    // Rewrite indexMap to memory

    // Initialize Simulation

    TARGET_TYPE mytimestep;
    mytimestep = 0.0015;

    // Aloc necessary memory for InitSimulation - temporary

    SimTK::Real **coords;
    coords = new SimTK::Real*[p_world->mr->natoms]; // ELIZA
    for(int j=0; j<natoms; j++){coords[j] = new SimTK::Real[3];}
    SimTK::Real **vels;
    vels = new SimTK::Real*[p_world->mr->natoms]; // ELIZA
    for(int j=0; j<natoms; j++){vels[j] = new SimTK::Real[3];}
    SimTK::Real **inivels;
    inivels = new SimTK::Real*[p_world->mr->natoms]; // ELIZA
    for(int j=0; j<natoms; j++){inivels[j] = new SimTK::Real[3];}
    //SimTK::Real **indexMap;
    indexMap = new SimTK::Real*[p_world->mr->natoms]; // ELIZA
    for(int j=0; j<natoms; j++){indexMap[j] = new SimTK::Real[3];}
    SimTK::Real **grads;
    grads = new SimTK::Real*[p_world->mr->natoms]; // ELIZA
    for(int j=0; j<natoms; j++){grads[j] = new SimTK::Real[3];}


    p_world->InitSimulation(/*coords, vels, inivels, indexMap, grads,*/ mytimestep, true);

    // Initialize sampler
    HamiltonianMonteCarloSampler *p_HMCsampler = new HamiltonianMonteCarloSampler(p_world->system, p_world->matter, p_world->lig1, p_world->ts);
    Context *context = new Context(p_world, p_HMCsampler);
    World *world = context->getWorld();

    // Options for mass matrix, Lennard Jones

    int pyseed = 0;
    int _massMatNumOpt = 1; // EU
    int _metroFixmanOpt = 1; // EU
    double _lj14sf = 1; //--

    // Alloc memory for saving configurations WATCHOUT TOO BIG

    double **retConfsPois = new double* [ntrials];
    for(int r=0; r<ntrials; r++){
        retConfsPois[r] = new double[3 * world->mr->natoms]; // WATCHOUT
    }
    double *retPotEsPoi = new double[ntrials];
    double *accs = new double;

    *world->pyseed = pyseed;
    world->massMatNumOpt = _massMatNumOpt;
    world->metroFixmanOpt = _metroFixmanOpt;
    world->lj14sf = _lj14sf; //--
    world->sysRetConfsPois = retConfsPois;
    world->sysRetPotEsPoi = retPotEsPoi;
    world->sysAccs = accs;

   // Simulate

    std::cout << std::fixed;
    std::cout << std::setprecision(4);
    SimTK::State& integAdvancedState = world->integ->updAdvancedState();
    p_HMCsampler->initialize(integAdvancedState, atof(argv[3]), atoi(argv[4]));
    for(int i = 0; i<atoi(argv[2]); i++){
        // -- STEPTO -- 

        //std::cout << "Time before stepping: " << world->ts->getTime()
                  //<< "; integAdvancedState Stage before stepping: " 
                  //<< (((SimTK::Subsystem *)(world->matter))->getStage(integAdvancedState)).getName() 
                  //<< "; integAdvancedState Stage before stepping: " 
                  //<< (((SimTK::Subsystem *)(world->matter))->getStage(integAdvancedState)).getName() 
                  //<< std::endl;

        //world->ts->initialize(integAdvancedState);
        //world->ts->stepTo(timeToReach);
        //world->integ->reinitialize(SimTK::Stage::Instance, true);

        //std::cout << "Time after  stepping: " << world->ts->getTime()
                  //<< "; integAdvancedState Stage after stepping: " 
                  //<< (((SimTK::Subsystem *)(world->matter))->getStage(integAdvancedState)).getName() 
                  //<< "; integAdvancedState Stage before stepping: " 
                  //<< (((SimTK::Subsystem *)(world->matter))->getStage(integAdvancedState)).getName() 
                  //<< std::endl;


        // -- UPDATE --
        std::cout << "=========================================" << std::endl;
        std::cout << "Q before update integAdvancedState " 
                  << integAdvancedState.getQ() << std::endl;
        std::cout << "U before update integAdvancedState " 
                  << integAdvancedState.getU() << std::endl;
        std::cout << "Time before update: " << world->ts->getTime() << std::endl;

        //p_HMCsampler->update((world->ts->updIntegrator()).updAdvancedState());
        p_HMCsampler->update(integAdvancedState, atof(argv[3]), atoi(argv[4]));

        std::cout << "Q after update integAdvancedState " 
                  << integAdvancedState.getQ() << std::endl;
        std::cout << "U after update integAdvancedState " 
                  << integAdvancedState.getU() << std::endl;
        std::cout << "Time after update: " << world->ts->getTime()  
                  << "; integAdvancedState Stage after p_HMCsampler: " 
                  << (((SimTK::Subsystem *)(world->matter))->getStage(integAdvancedState)).getName() 
                  << "; integAdvancedState Stage before stepping: " 
                  << (((SimTK::Subsystem *)(world->matter))->getStage(integAdvancedState)).getName() 
                  << std::endl;
        writePdb(*((SimTK::Compound *)(world->lig1)), integAdvancedState, "pdbs", "sb_", 8, "HMCs", i);
    }


}



