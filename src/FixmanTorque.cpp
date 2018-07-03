#include "Robo.hpp"
#include "FixmanTorque.hpp"


////////////////////////////
////// GRID FORCE //////////
////////////////////////////
FixmanTorque::FixmanTorque(SimTK::CompoundSystem *compoundSystem, SimTK::SimbodyMatterSubsystem& matter
                    ) : matter(matter){
  this->compoundSystem = compoundSystem;
  scaleFactor = 1.0;
}

void FixmanTorque::calcForce(const SimTK::State& state, SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
                           SimTK::Vector_<SimTK::Vec3>& particleForces, SimTK::Vector& mobilityForces) const
{
    // Compute Fixman torque
    int nu = state.getNU();
    SimTK::Vector V3(nu);
    SimTK::Vector V4(nu);
    SimTK::Real* D0 = new SimTK::Real(1.0);
    matter.calcFixmanTorque(state, V3, V4, D0);
    delete D0;
    // end - Compute Fixman torque

    //std::cout << "Applied " ;
    int uslot = -1;
    for (SimTK::MobilizedBodyIndex mbx(0); mbx < matter.getNumBodies(); ++mbx){
        const SimTK::MobilizedBody& mobod = matter.getMobilizedBody(mbx);

        for(int k = 0; k < mobod.getNumU(state); k++){
            uslot++;
            mobod.applyOneMobilityForce(state, k, scaleFactor * V4[uslot], mobilityForces);
            //std::cout << " " << std::setprecision(10) << std::fixed << V4[uslot] << " to " << int(mbx) ;
            //std::cout << std::setprecision(5) << std::fixed << V4[uslot] << ' ';
        }

    }
    //std::cout << std::endl;

    //const SimTK::Real q = knee.getOneQ(state, 0);
    //const SimTK::Real x = q < low ? q-low : (q > high ? q-high : 0);
    //knee.applyOneMobilityForce(state, 0, -k*x, mobilityForces);
}

// This should be carefully analyzed. Intended to be taken from somewhere else.
SimTK::Real FixmanTorque::calcPotentialEnergy(const SimTK::State& state) const {
  SimTK::Real energy = 0.0;
  return energy;
}

bool FixmanTorque::dependsOnlyOnPositions() const {
  return true;
}

SimTK::Real FixmanTorque::getScaleFactor(void)
{
    return scaleFactor;
}

void FixmanTorque::setScaleFactor(SimTK::Real argScaleFactor)
{
    this->scaleFactor = argScaleFactor;
}

////////////////////////////
////// END GRID FORCE //////
////////////////////////////
