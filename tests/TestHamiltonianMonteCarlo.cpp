/** @file
 * This file contains a native Gmolmodel test. A 2-butanol molecule
 * is simulated.
 */

#include <string>
#include <iostream>
#include <sstream>
#include "simmain.hpp"

#include "HamiltonianMonteCarloSampler.hpp"

int main(int argc, char **argv)
{
    // Declare a shared memory pointer used by all the classes in the library

    TARGET_TYPE *shm;
    TARGET_TYPE **coords; // convenient pointer to coordinates in shm
    int SHMSZ; // size
    int sweep;

    //  Declare variables for simulation parameters

    int natoms;
    int nosteps;
    int ntrials;
    int steps_per_trial;
    double temperature;
    double delta_t;

    // Set simulation parameters

    temperature = 300.0;
    delta_t = 0.0015; // ps
    nosteps = 10; // RESTORE DEL
    ntrials = 10; // RESTORE DEL
    steps_per_trial = nosteps / ntrials;
    std::cout<<"main ntrials: "<<ntrials<<std::endl;
    std::cout<<"main nosteps: "<<nosteps<<std::endl;

    // Set input filenames
 
    std::string mol2FN = "2but/ligand.mol2";
    std::string rbFN = "2but/ligand.rb";
    std::string gaffFN = "gaff.dat";
    std::string frcmodFN = "2but/ligand.frcmod";

    // Simulation type:
    // IC: Internal Coordinates Dynamics
    // TD: Torsional Dynamics
    // RR: Rigid Rings Torsional Dynamics

    std::string ictd = "TD";

    std::cout<<"mol2FN "<<mol2FN<<std::endl<<std::flush;
    std::cout<<"rbFN "<<rbFN<<std::endl<<std::flush;
    std::cout<<"gaffFN "<<gaffFN<<std::endl<<std::flush;
    std::cout<<"frcmodFN "<<frcmodFN<<std::endl<<std::flush;
    std::cout<<"ictd "<<ictd<<std::endl<<std::flush;

    // Read number of atoms from mol2 file

    std::string line;
    std::string column;

    std::ifstream mol2ifstream(mol2FN);
    while(std::getline(mol2ifstream, line)){
        std::istringstream iss(line);
        iss >> column;
        if(column == "MOL") break;
    }
    std::getline(mol2ifstream, line);
    std::istringstream iss(line);
    iss >> natoms;
    mol2ifstream.close();
    std::cout<<"natoms read from mol2: "<<natoms<<std::endl;

    // Read atom ordering from mol2
 
    int order[natoms+2]; // prmtop ORDER previously read from MMTK
    int acceptance;
    TARGET_TYPE **indexMap = NULL;
    TARGET_TYPE *PrmToAx_po = NULL;
    TARGET_TYPE *MMTkToPrm_po = NULL;
    int _indexMap[natoms][3];
    indexMap = new TARGET_TYPE*[(natoms)];
    PrmToAx_po = new TARGET_TYPE[natoms];
    MMTkToPrm_po = new TARGET_TYPE[natoms];

    for(int i=0; i<natoms; i++){
      order[i] = i; // instead of MMTK
    }
    for(int i=0; i<natoms; i++){
        _indexMap[i][2] = order[i];
    }
    order[natoms] = 1;
    order[natoms+1] = 1945;
    acceptance = order[natoms];

    // Set the shared memory size (SHMSZ)

    int natoms3 = 3*(natoms);
    int arrays_cut = 2 + 4*natoms3;

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


    World *p_world = new World(
        mol2FN, rbFN, gaffFN, frcmodFN,
        ictd, 
        PrmToAx_po, MMTkToPrm_po,
        shm
    );

    // Test Context
    Sampler *p_genericSampler = new Sampler(p_world->system, p_world->matter, p_world->lig1, p_world->ts);
    HamiltonianMonteCarloSampler *HMCsampler = new HamiltonianMonteCarloSampler(p_world->system, p_world->matter, p_world->lig1, p_world->ts);
    Context *context = new Context(p_world, p_genericSampler);
    World *world = context->getWorld();

    // Memory alloc for convinient arrays 

    coords = new TARGET_TYPE*[world->mr->natms];
    TARGET_TYPE **vels = new TARGET_TYPE*[world->mr->natms];
    TARGET_TYPE **inivels = new TARGET_TYPE*[world->mr->natms];
    TARGET_TYPE **grads = new TARGET_TYPE*[world->mr->natms];

    // Seed the random number generator 

    srand (time(NULL));

    // Load initial shared memory values

    float mysweep = 0;
    shm[0] = CNT_START;

    int cli;
    sweep = 0;
    bool world_initialized = false;
    int big_loop_i;

    shm[1] = CLIENT_FLAG;
    int a;
    int start, start_1, stop;
    int i = natoms*2*3 + 2;

    // Assign convenient pointers for order

    start = 2 + natoms3 + natoms3; 
    for(a=0; a<natoms; a++){
        indexMap[a] = &(shm[a*3 + start]);
    }

    // Assign order in shm[][2] 

    start = 2 + natoms3 + natoms3;
    cli = start;
    for(a=0; a<natoms; a++){ // Order
        cli += 2;
        shm[cli++] = order[a];
    }

    cli = 2;

    // Get coordinates in the same order as in mol2

    int mol2atomlineno = -1;
    int mol2atom_no;
    std::string mol2atom_name;
    SimTK::Real mol2x, mol2y, mol2z;

    mol2ifstream.open(mol2FN);
    while(std::getline(mol2ifstream, line)){
        std::istringstream iss(line);
        iss >> column;
        if(column == "@<TRIPOS>ATOM") break;
    }
    while(std::getline(mol2ifstream, line)){ // WATCHOUT to read only natoms lines
        ++mol2atomlineno;
        std::istringstream iss(line);
        column = line.substr(0, 13);
        if((column == "@<TRIPOS>BOND") || (mol2atomlineno == natoms)) break;
        iss >> mol2atom_no;
        iss >> mol2atom_name;
        iss >> mol2x;
        iss >> mol2y;
        iss >> mol2z;
        std::cout<<"mol2 info "<<mol2atom_no<<" "<< mol2atom_name
            <<" "<<mol2x<<" "<<mol2y<<" "<<mol2z<<std::endl;
        shm[cli++] = mol2x/10.0; shm[cli++] = mol2y/10.0; shm[cli++] = mol2z/10.0; // convert A to nm
    }
    mol2ifstream.close();

    // Get velocities

    for(a=0; a<natoms; a++){
        shm[cli++] = .0; 
        shm[cli++] = .0; 
        shm[cli++] = .0; 
    }

    // Get order    

    for(a=0; a<natoms; a++){
        cli += 2;
        shm[cli++] = order[a];
    }

    // Get forces   

    for(a=0; a<natoms; a++){
        shm[cli++] = .0;
        shm[cli++] = .0;
        shm[cli++] = .0;
    }

    // Get simulation parameters

    shm[cli++] = big_loop_i;          // Step
    shm[cli++] = (TARGET_TYPE)nosteps;    // Number of steps
    shm[cli++] = temperature; // Temeperature
    shm[cli++] = delta_t;
    shm[cli++] = 0.0; // Trouble flag
    shm[cli++] = 0.0; // p_energy_po->energy from MMTK
    cli++; // DAE set only by the server
    shm[cli] = acceptance + 0.0;

    // Set the client flag

    shm[1] = CLIENT_FLAG;
    shm[arrays_cut + 6] = 13.0; // set DAE to done

    // Assign convenient pointers for coordinates

    int shm_coords_sup = (natoms3)+2;
    int j=-1, k=0;
    start = 2; 
    for(j=0; j<natoms; j++){
        coords[j] = &(shm[j*3 + start]);
    }

    mysweep = shm[0] - CNT_START;

    // Assign convenient pointers for velocities

    start = 2 + natoms3;
    start_1 = start - 1;
    stop = 2 + natoms3 + natoms3;
    for(j=0; j<natoms; j++){
        vels[j]    = &(shm[j*3 + start]);
        inivels[j] = &(shm[j*3 + start]);
    }

    // Assign convenient pointers for gradients

    start = 2 + natoms3 + natoms3 + natoms3;
    start_1 = start - 1;
    stop = 2 + natoms3 + natoms3 + natoms3 + natoms3;
    for(j=0; j<natoms; j++){
        grads[j] = &(shm[j*3 + start]);
    }

    // Rewrite indexMap to memory

    start = 2 + natoms3 + natoms3;
    start_1 = start - 1;
    stop = 2 + natoms3 + natoms3 + natoms3;

    // Initialize Simulation

    TARGET_TYPE timestep, mytimestep;
    mytimestep = shm[arrays_cut + 3];

    // Build Topology and fill indexMap

    world->InitSimulation(coords, vels, inivels, indexMap, grads, mytimestep, true);
    world_initialized = true;

    // Options for mass matrix, Lennard Jones

    TARGET_TYPE temp_arg;
    TARGET_TYPE ts;
    int pyseed;
    int _massMatNumOpt = 1; // EU
    int _metroFixmanOpt = 1; // EU
    double _lj14sf = 1; //--

    #ifdef DEBUG_TIME
            //boost::timer boost_timer;
    #endif

    // Alloc memory for saving configurations WATCHOUT TOO BIG

    double **retConfsPois = new double* [ntrials];
    for(int r=0; r<ntrials; r++){
        retConfsPois[r] = new double[3 * world->mr->natms]; // WATCHOUT
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

    arrays_cut = 2 + 4*3*(world->mr->natms);
    shm[arrays_cut + 0] = 0.0; // step (will be incremented in MidVV
    shm[arrays_cut + 1] = (TARGET_TYPE)(nosteps);
    shm[arrays_cut + 2] = temperature;
    shm[arrays_cut + 3] = delta_t; // picosec
    shm[arrays_cut + 7] = 1.0;     // acceptance always 1 !!
    shm[arrays_cut + 9] = (TARGET_TYPE)steps_per_trial; // steps_per_trial
    shm[arrays_cut + 10] = 0.0;    // initialize trial

    // Print shm
    std::cout<<"shm contents:"<<std::endl;
    for(i=0; i<SHMSZ; i++){
        std::cout<<shm[i]<<" ";
        if(i%3 == 0) std::cout<<std::endl;
    }
    std::cout<<std::endl;

   // Simulate

    std::cout << std::fixed;
    std::cout << std::setprecision(4);
    SimTK::Real timeToReach = 0.0;
    SimTK::State& integAdvancedState = world->integ->updAdvancedState();
    const SimTK::State& tsState = world->ts->getState(); // less than or equal to integ advanced state
    for(int i = 0; i<30; i++){
        // -- STEPTO -- 
        timeToReach += (nosteps * delta_t);

        std::cout << "trying to make integrator to step to " << timeToReach 
                  << std::endl;
        std::cout << "Time before stepping: " << world->ts->getTime()  
                  //<< "; integAdvancedState Stage before stepping: " 
                  //<< (((SimTK::Subsystem *)(world->matter))->getStage(integAdvancedState)).getName() 
                  //<< "; tsState Stage before stepping: " 
                  //<< (((SimTK::Subsystem *)(world->matter))->getStage(tsState)).getName() 
                  << std::endl;

        //world->ts->initialize(tsState);
        //world->ts->stepTo(timeToReach);
        //world->integ->reinitialize(SimTK::Stage::Instance, true);

        std::cout << "Time after  stepping: " << world->ts->getTime()  
                  //<< "; integAdvancedState Stage after stepping: " 
                  //<< (((SimTK::Subsystem *)(world->matter))->getStage(integAdvancedState)).getName() 
                  //<< "; tsState Stage before stepping: " 
                  //<< (((SimTK::Subsystem *)(world->matter))->getStage(tsState)).getName() 
                  << std::endl;

        // -- UPDATE --
        std::cout << "before update integAdvancedState.getQ()" 
                  << integAdvancedState.getQ() << std::endl;

        HMCsampler->update((world->ts->updIntegrator()).updAdvancedState());

        std::cout << "after update integAdvancedState.getQ()" 
                  << integAdvancedState.getQ() << std::endl;
        std::cout << "Time after  update: " << world->ts->getTime()  
                  << "; integAdvancedState Stage after HMCsampler: " 
                  << (((SimTK::Subsystem *)(world->matter))->getStage(integAdvancedState)).getName() 
                  << "; tsState Stage before stepping: " 
                  << (((SimTK::Subsystem *)(world->matter))->getStage(tsState)).getName() 
                  << std::endl;
        writePdb(*((SimTK::Compound *)(world->lig1)), integAdvancedState, "pdbs", "sb_", 8, "MCs", i);
    }


    //delete HMCsampler;

}


