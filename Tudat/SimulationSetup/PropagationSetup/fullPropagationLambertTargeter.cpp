/*    Copyright (c) 2010-2018, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#include "Tudat/SimulationSetup/PropagationSetup/createStateDerivativeModel.h"
#include "Tudat/Astrodynamics/MissionSegments/lambertTargeter.h"
#include "Tudat/Astrodynamics/MissionSegments/lambertTargeterIzzo.h"
#include "Tudat/Astrodynamics/MissionSegments/lambertRoutines.h"
#include "Tudat/SimulationSetup/EnvironmentSetup/defaultBodies.h"
#include "Tudat/SimulationSetup/PropagationSetup/createAccelerationModels.h"
#include <Tudat/SimulationSetup/tudatEstimationHeader.h>

#include "Tudat/Astrodynamics/Gravitation/librationPoint.h"
#include "Tudat/Astrodynamics/BasicAstrodynamics/celestialBodyConstants.h"
#include "Tudat/Astrodynamics/BasicAstrodynamics/missionGeometry.h"

// required for the MGA
#include "Tudat/Astrodynamics/Ephemerides/approximatePlanetPositions.h"
#include "Tudat/Astrodynamics/TrajectoryDesign/trajectory.h"
#include "Tudat/Astrodynamics/TrajectoryDesign/exportTrajectory.h"
#include "Tudat/Astrodynamics/TrajectoryDesign/planetTrajectory.h"

#include "Tudat/External/SpiceInterface/spiceInterface.h"

namespace tudat
{

namespace propagators
{

//! Function to directly setup a body map corresponding to the assumptions of the Lambert targeter.
simulation_setup::NamedBodyMap setupBodyMapLambertTargeter(
        const std::string& nameCentralBody,
        const std::string& nameBodyToPropagate,
        const std::vector< std::string >& departureAndArrivalBodies,
        const Eigen::Vector3d& cartesianPositionAtDeparture,
        const Eigen::Vector3d& cartesianPositionAtArrival,
        const bool departureAndArrivalInitialisationFromEphemerides = false )
{

//    Eigen::Vector6d cartesianStateAtDeparture;
//    Eigen::Vector6d cartesianStateAtArrival;

    spice_interface::loadStandardSpiceKernels( );


    // Create central body object.
    std::vector< std::string > bodiesToCreate;
    bodiesToCreate.push_back( nameCentralBody );

    // If we use direct ephemerides for the departure and arrival bodies
    if ( departureAndArrivalInitialisationFromEphemerides == true ){
        bodiesToCreate.push_back( departureAndArrivalBodies[0] );
        bodiesToCreate.push_back( departureAndArrivalBodies[1] );
    }


    std::map< std::string, std::shared_ptr< simulation_setup::BodySettings > > bodySettings =
                    simulation_setup::getDefaultBodySettings( bodiesToCreate );

    // Define central body ephemeris settings.
    std::string frameOrigin = "SSB";
    std::string frameOrientation = "ECLIPJ2000";
    bodySettings[ nameCentralBody ]->ephemerisSettings = std::make_shared< simulation_setup::ConstantEphemerisSettings >(
            ( Eigen::Vector6d( ) << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 ).finished( ), frameOrigin, frameOrientation );

    bodySettings[ nameCentralBody ]->ephemerisSettings->resetFrameOrientation( frameOrientation );
    bodySettings[ nameCentralBody ]->rotationModelSettings->resetOriginalFrame( frameOrientation );








    // Create body map.
    simulation_setup::NamedBodyMap bodyMap = createBodies( bodySettings );

    std::cout << "detection location error after body map " << "\n\n";

    bodyMap[ nameBodyToPropagate ] = std::make_shared< simulation_setup::Body >( );
    bodyMap[ nameBodyToPropagate ]->setEphemeris( std::make_shared< ephemerides::TabulatedCartesianEphemeris< > >(
                    std::shared_ptr< interpolators::OneDimensionalInterpolator
                    < double, Eigen::Vector6d > >( ), frameOrigin, frameOrientation ) );


    // constant ephemerides for departure and arrival bodies
    if (departureAndArrivalInitialisationFromEphemerides == false){ // (and if direct ephemerides not available?)
        bodyMap[ departureAndArrivalBodies[0] ] = std::make_shared< simulation_setup::Body >( );
        Eigen::Vector6d cartesianStateAtDeparture;
        cartesianStateAtDeparture.segment(0,3) = cartesianPositionAtDeparture;
        cartesianStateAtDeparture.segment(3,3) = Eigen::Vector3d::Zero();
        bodyMap[ departureAndArrivalBodies[0] ]->setEphemeris( std::make_shared< ephemerides::ConstantEphemeris >( cartesianStateAtDeparture,
                                                                                                                  frameOrigin, frameOrientation ));

        bodyMap[ departureAndArrivalBodies[1] ] = std::make_shared< simulation_setup::Body >( );
        Eigen::Vector6d cartesianStateAtArrival;
        cartesianStateAtArrival.segment(0,3) = cartesianPositionAtArrival;
        cartesianStateAtArrival.segment(3,3) = Eigen::Vector3d::Zero();
        bodyMap[ departureAndArrivalBodies[1] ]->setEphemeris( std::make_shared< ephemerides::ConstantEphemeris >( cartesianStateAtArrival,
                                                                                                                  frameOrigin, frameOrientation ));
    }



    setGlobalFrameBodyEphemerides( bodyMap, frameOrigin, frameOrientation );



    return bodyMap;


}

//! Function to directly setup an acceleration map for the Lambert targeter.
basic_astrodynamics::AccelerationMap setupAccelerationMapLambertTargeter(
        const std::string& nameCentralBody,
        const std::string& nameBodyToPropagate,
        const simulation_setup::NamedBodyMap& bodyMap )
{

    std::vector< std::string > bodiesToPropagate;
    bodiesToPropagate.push_back( nameBodyToPropagate );
    std::vector< std::string > centralBodies;
    centralBodies.push_back( nameCentralBody );

    std::map< std::string, std::vector< std::shared_ptr< simulation_setup::AccelerationSettings > > > bodyToPropagateAccelerations;
    bodyToPropagateAccelerations[nameCentralBody].push_back(std::make_shared< simulation_setup::AccelerationSettings >(
                                                          basic_astrodynamics::central_gravity ) );

    simulation_setup::SelectedAccelerationMap accelerationMap;
    accelerationMap[ nameBodyToPropagate ] = bodyToPropagateAccelerations;

    basic_astrodynamics::AccelerationMap accelerationModelMap = createAccelerationModelsMap(
                        bodyMap, accelerationMap, bodiesToPropagate, centralBodies );


    return accelerationModelMap;

}


//! Function to propagate the Lambert targeter solution from the associated environment




//! Function to determine the cartesian state at a given time for a keplerian orbit, based on the initial state.
Eigen::Vector6d propagateLambertTargeterSolution(
        const Eigen::Vector6d& initialState,
        const double finalPropagationTime,
        const double gravitationalParameter)
{

    Eigen::Vector6d keplerianInitialState = orbital_element_conversions::convertCartesianToKeplerianElements(initialState,
                                                                                                             gravitationalParameter);
    // Retrieve the semi-major axis and eccentricty of the keplerian orbit.
    double semiMajorAxis = keplerianInitialState[orbital_element_conversions::semiMajorAxisIndex];
    double eccentricity = keplerianInitialState[orbital_element_conversions::eccentricityIndex];

    // Calculate the initial mean anomaly.
    double initialTrueAnomaly = keplerianInitialState[orbital_element_conversions::trueAnomalyIndex];
    double initialMeanAnomaly = orbital_element_conversions::convertEccentricAnomalyToMeanAnomaly(
                orbital_element_conversions::convertTrueAnomalyToEccentricAnomaly(initialTrueAnomaly, eccentricity), eccentricity);

    // Calculate the mean anomaly at the final time.
    double meanAnomalyEndPropagation = initialMeanAnomaly + orbital_element_conversions::convertElapsedTimeToMeanAnomalyChange(
                finalPropagationTime, gravitationalParameter, semiMajorAxis);

    // Determine the final
    Eigen::Vector6d finalKeplerianState = keplerianInitialState;
    finalKeplerianState[orbital_element_conversions::trueAnomalyIndex] =
            orbital_element_conversions::convertEccentricAnomalyToTrueAnomaly(
                orbital_element_conversions::convertMeanAnomalyToEccentricAnomaly(eccentricity, meanAnomalyEndPropagation), eccentricity);

    Eigen::Vector6d cartesianStateLambertSolution = orbital_element_conversions::convertKeplerianToCartesianElements(
                finalKeplerianState, gravitationalParameter);

    return cartesianStateLambertSolution;
}



//! Function to propagate the full dynamics problem and the Lambert targeter solution.
void propagateLambertTargeterAndFullProblem( Eigen::Vector3d cartesianPositionAtDeparture,
        Eigen::Vector3d cartesianPositionAtArrival,
        const double timeOfFlight,
        const double initialTime,
        simulation_setup::NamedBodyMap& bodyMap,
        const basic_astrodynamics::AccelerationMap& accelerationModelMap,
        const std::vector<std::string>& bodiesToPropagate,
        const std::vector<std::string>& centralBody,
        const std::shared_ptr< numerical_integrators::IntegratorSettings< double > > integratorSettings,
        std::map< double, Eigen::Vector6d >& lambertTargeterResult,
        std::map< double, Eigen::Vector6d >& fullProblemResult,
        const std::vector<std::string>& departureAndArrivalBodies,
        const bool arrivalAndDepartureInitialisationFromEphemerides = false,
        const bool terminationSphereOfInfluence = false,
        const double departureBodyGravitationalParameter = TUDAT_NAN,
        const double arrivalBodyGravitationalParameter = TUDAT_NAN,
        const double centralBodyGravitationalParameter = TUDAT_NAN
        )
{

    lambertTargeterResult.clear( );
    fullProblemResult.clear( );

    // Retrieve the gravitational parameter of the main body.
    double gravitationalParameterCentralBody = ( centralBodyGravitationalParameter == centralBodyGravitationalParameter ) ?
                    centralBodyGravitationalParameter :
                    bodyMap[ centralBody[0] ]->getGravityFieldModel()->getGravitationalParameter();


//    double gravitationalParameter = bodyMap[centralBody[0]]->getGravityFieldModel()->getGravitationalParameter();

    // Get halved value of the time of flight, used as initial time for the propagation.
    double halvedTimeOfFlight = timeOfFlight / 2.0;

    // Final time at the end of the transfer
    double finalTime = initialTime + timeOfFlight;

    // Retrieve positions from ephemerides
    double radiusSphereOfInfluenceDeparture;
    double radiusSphereOfInfluenceArrival;

    if (arrivalAndDepartureInitialisationFromEphemerides == true)
    {

        // Cartesian state at departure
        Eigen::Vector6d cartesianStateDepartureBody =
                bodyMap.at( departureAndArrivalBodies.at( 0 ) )->getEphemeris( )->getCartesianState( initialTime);
        cartesianPositionAtDeparture = cartesianStateDepartureBody.segment(0,3);

        // Cartesian state at arrival
        Eigen::Vector6d cartesianStateArrivalBody =
                bodyMap.at( departureAndArrivalBodies.at( 1 ) )->getEphemeris()->getCartesianState(finalTime);
        cartesianPositionAtArrival =  cartesianStateArrivalBody.segment(0,3);

    }


    if (terminationSphereOfInfluence == true) {

        double distanceDepartureToCentralBodies = ( bodyMap[ centralBody[0] ]->getState().segment(0,3) -
                cartesianPositionAtDeparture.segment(0,3) ).norm();
        double distanceArrivalToCentralBodies = ( bodyMap[ centralBody[0] ]->getState().segment(0,3) -
                cartesianPositionAtArrival.segment(0,3) ).norm();


        double gravitationalParameterDepartureBody;
        std::cout << "print before comparison between input and TUDAT_NAN" << "\n\n";
        if ( departureBodyGravitationalParameter != TUDAT_NAN){
            std::cout << "gravitational parameter provided as input" << "\n\n";
            gravitationalParameterDepartureBody = departureBodyGravitationalParameter;
        }
        else {
            std::cout << "gravitational parameter departure body not provided" << "\n\n";
            gravitationalParameterDepartureBody = bodyMap[departureAndArrivalBodies[0]]->getGravityFieldModel()->getGravitationalParameter();
        }


        double gravitationalParameterArrivalBody;
        if ( arrivalBodyGravitationalParameter != TUDAT_NAN ){
            gravitationalParameterArrivalBody = arrivalBodyGravitationalParameter;
        }
        else {
            gravitationalParameterArrivalBody = bodyMap[departureAndArrivalBodies[1]]->getGravityFieldModel()->getGravitationalParameter();
        }


        radiusSphereOfInfluenceDeparture = tudat::mission_geometry::computeSphereOfInfluence(
                    distanceDepartureToCentralBodies, gravitationalParameterDepartureBody, gravitationalParameterCentralBody);
        radiusSphereOfInfluenceArrival = tudat::mission_geometry::computeSphereOfInfluence(
                    distanceArrivalToCentralBodies, gravitationalParameterArrivalBody, gravitationalParameterCentralBody);

    }




    // Run the Lambert targeter.
    mission_segments::LambertTargeterIzzo LambertTargeter(
                cartesianPositionAtDeparture, cartesianPositionAtArrival, timeOfFlight, gravitationalParameterCentralBody );

    // Retrieve cartesian state at departure.
    Eigen::Vector3d cartesianVelocityAtDeparture = LambertTargeter.getInertialVelocityAtDeparture();
    Eigen::Vector6d cartesianStateAtDeparture;
    cartesianStateAtDeparture.segment(0,3) = cartesianPositionAtDeparture;
    cartesianStateAtDeparture.segment(3,3) = cartesianVelocityAtDeparture;

    // Keplerian state at departure.
    Eigen::Vector6d keplerianElementsAtDeparture = tudat::orbital_element_conversions::convertCartesianToKeplerianElements(
                cartesianStateAtDeparture, gravitationalParameterCentralBody);

    double semiMajorAxis = LambertTargeter.getSemiMajorAxis();
    double eccentricity = keplerianElementsAtDeparture( orbital_element_conversions::eccentricityIndex );

    double trueAnomalyAtDeparture = keplerianElementsAtDeparture(orbital_element_conversions::trueAnomalyIndex);
    double meanAnomalyAtDeparture = orbital_element_conversions::convertEccentricAnomalyToMeanAnomaly(
                orbital_element_conversions::convertTrueAnomalyToEccentricAnomaly(trueAnomalyAtDeparture, eccentricity),
                eccentricity);


    // Calculate the true anomaly at half the time of flight.
    double meanAnomalyChangeHalfTimeOfFlight = orbital_element_conversions::convertElapsedTimeToMeanAnomalyChange(halvedTimeOfFlight,
                                                                                      gravitationalParameterCentralBody, semiMajorAxis);

    double meanAnomalyHalfTimeOfFlight = meanAnomalyChangeHalfTimeOfFlight + meanAnomalyAtDeparture;
    double trueAnomalyHalfTimeOfFlight = orbital_element_conversions::convertEccentricAnomalyToTrueAnomaly(
                orbital_element_conversions::convertMeanAnomalyToEccentricAnomaly( eccentricity, meanAnomalyHalfTimeOfFlight ), eccentricity);


    // Define the state at half of the time of flight (initial state for the propagation).
    Eigen::Vector6d initialStatePropagationKeplerianElements;
    initialStatePropagationKeplerianElements.segment(0,5) = keplerianElementsAtDeparture.segment(0,5);
    initialStatePropagationKeplerianElements[orbital_element_conversions::trueAnomalyIndex] = trueAnomalyHalfTimeOfFlight;

    Eigen::Vector6d initialStatePropagationCartesianElements = orbital_element_conversions::convertKeplerianToCartesianElements(
                initialStatePropagationKeplerianElements, gravitationalParameterCentralBody);



    // Initialise variables for propagatation.
    std::vector< std::string > centralBodiesPropagation;
    centralBodiesPropagation.push_back( "SSB" );

    Eigen::Vector6d cartesianStateLambertSolution;



    // Define forward propagator settings variables.
    integratorSettings->initialTime_ = initialTime + halvedTimeOfFlight;

    // Define forward propagation settings
    std::shared_ptr< propagators::TranslationalStatePropagatorSettings< double > > propagatorSettingsForwardPropagation;

    if ( terminationSphereOfInfluence == false ){

        propagatorSettingsForwardPropagation = std::make_shared< propagators::TranslationalStatePropagatorSettings< double > >
                   ( centralBodiesPropagation, accelerationModelMap, bodiesToPropagate, initialStatePropagationCartesianElements,
                                                                                                            initialTime + timeOfFlight );
    }

    else {

        // Termination settings if the forward propagation stops at the sphere of influence of the arrival body
        std::shared_ptr< SingleDependentVariableSaveSettings > terminationDependentVariableAtArrival =
                std::make_shared< SingleDependentVariableSaveSettings >(
              relative_distance_dependent_variable, bodiesToPropagate[0], departureAndArrivalBodies[1] );

        std::shared_ptr< PropagationTerminationSettings > forwardPropagationTerminationSettings =
            std::make_shared< PropagationDependentVariableTerminationSettings >( terminationDependentVariableAtArrival,
                                                                                 radiusSphereOfInfluenceArrival, false);

        propagatorSettingsForwardPropagation = std::make_shared< propagators::TranslationalStatePropagatorSettings< double > >
                    ( centralBodiesPropagation, accelerationModelMap, bodiesToPropagate, initialStatePropagationCartesianElements,
                      forwardPropagationTerminationSettings );

     }



    // Perform forward propagation.
    propagators::SingleArcDynamicsSimulator< > dynamicsSimulatorIntegrationForwards(bodyMap, integratorSettings,
                                                                                    propagatorSettingsForwardPropagation );
    std::map< double, Eigen::VectorXd > stateHistoryFullProblemForwardPropagation = dynamicsSimulatorIntegrationForwards.getEquationsOfMotionNumericalSolution( );

    // Calculate the difference between the full problem and the Lambert targeter solution along the forward propagation branch of the orbit.
    for( std::map< double, Eigen::VectorXd >::iterator itr = stateHistoryFullProblemForwardPropagation.begin( );
         itr != stateHistoryFullProblemForwardPropagation.end( ); itr++ )
    {

        cartesianStateLambertSolution = propagateLambertTargeterSolution(initialStatePropagationCartesianElements, itr->first - (initialTime + halvedTimeOfFlight),
                                                                         gravitationalParameterCentralBody);

        lambertTargeterResult[ itr->first ] = cartesianStateLambertSolution;
        fullProblemResult[ itr->first ] = itr->second;

    }





    // Define backward propagator settings variables.
    integratorSettings->initialTimeStep_ = -1 * integratorSettings->initialTimeStep_;
    integratorSettings->initialTime_ = initialTime + halvedTimeOfFlight;


    // Define backward propagation settings.
    std::shared_ptr< propagators::TranslationalStatePropagatorSettings< double > > propagatorSettingsBackwardPropagation;

    if (terminationSphereOfInfluence == false){

        propagatorSettingsBackwardPropagation = std::make_shared< propagators::TranslationalStatePropagatorSettings< double > >
                ( centralBodiesPropagation, accelerationModelMap, bodiesToPropagate, initialStatePropagationCartesianElements,
                                                                                                  initialTime );
    }
    else {

        // Termination settings if the backward propagation stops at the sphere of influence of the departure body
        std::shared_ptr< SingleDependentVariableSaveSettings > terminationDependentVariableAtDeparture =
                std::make_shared< SingleDependentVariableSaveSettings >(
                      relative_distance_dependent_variable, bodiesToPropagate[0], departureAndArrivalBodies[0] );
        std::shared_ptr< PropagationTerminationSettings > backwardPropagationTerminationSettings =
                std::make_shared< PropagationDependentVariableTerminationSettings >( terminationDependentVariableAtDeparture,
                                                                                         radiusSphereOfInfluenceDeparture, false);

        propagatorSettingsBackwardPropagation = std::make_shared< propagators::TranslationalStatePropagatorSettings< double > >
                ( centralBodiesPropagation, accelerationModelMap, bodiesToPropagate, initialStatePropagationCartesianElements,
                  backwardPropagationTerminationSettings );
    }


    // Perform the backward propagation.
    propagators::SingleArcDynamicsSimulator< > dynamicsSimulatorIntegrationBackwards(bodyMap, integratorSettings,
                                                                                     propagatorSettingsBackwardPropagation );

    std::map< double, Eigen::VectorXd > stateHistoryFullProblemBackwardPropagation = dynamicsSimulatorIntegrationBackwards.getEquationsOfMotionNumericalSolution( );

    // Calculate the difference between the full problem and the Lambert targeter solution along the forward propagation branch of the orbit.
    for( std::map< double, Eigen::VectorXd >::iterator itr = stateHistoryFullProblemBackwardPropagation.begin( );
         itr != stateHistoryFullProblemBackwardPropagation.end( ); itr++ )
    {

        cartesianStateLambertSolution = propagateLambertTargeterSolution(initialStatePropagationCartesianElements,  - (initialTime + halvedTimeOfFlight) + itr->first,
                                                                         gravitationalParameterCentralBody);

        lambertTargeterResult[ itr->first ] = cartesianStateLambertSolution;
        fullProblemResult[ itr->first ] = itr->second;

    }

}


//! Function to compute the difference in cartesian state between Lambert targeter solution and full dynamics problem, both at departure
//! and at arrival.
std::pair< Eigen::Vector6d, Eigen::Vector6d > getDifferenceFullPropagationWrtLambertTargeterAtDepartureAndArrival(
        const Eigen::Vector3d& cartesianPositionAtDeparture,
        const Eigen::Vector3d& cartesianPositionAtArrival,
        const double timeOfFlight,
        const double initialTime,
        simulation_setup::NamedBodyMap& bodyMap,
        const basic_astrodynamics::AccelerationMap& accelerationModelMap,
        const std::vector< std::string >& bodiesToPropagate,
        const std::vector< std::string >& centralBodies,
        const std::shared_ptr< numerical_integrators::IntegratorSettings< double > > integratorSettings,
        const std::vector< std::string >& departureAndArrivalBodies,
        const bool arrivalAndDepartureInitialisationFromEphemerides = false,
        const bool terminationSphereOfInfluence = false)

{
    std::map< double, Eigen::Vector6d > lambertTargeterResult;
    std::map< double, Eigen::Vector6d > fullProblemResult;

    // compute full problem and Lambert targeter solution both at departure and arrival.
    propagateLambertTargeterAndFullProblem(cartesianPositionAtDeparture, cartesianPositionAtArrival, timeOfFlight, initialTime,
                                           bodyMap, accelerationModelMap, bodiesToPropagate, centralBodies, integratorSettings,
                                           lambertTargeterResult, fullProblemResult, departureAndArrivalBodies, arrivalAndDepartureInitialisationFromEphemerides,
                                           terminationSphereOfInfluence);

    Eigen::Vector6d stateLambertTargeterAtDeparture = lambertTargeterResult.begin( )->second;
    Eigen::Vector6d propagatedStateFullProblemAtDeparture = fullProblemResult.begin( )->second;
    Eigen::Vector6d stateLambertTargeterAtArrival = lambertTargeterResult.rbegin( )->second;
    Eigen::Vector6d propagatedStateFullProblemAtArrival = fullProblemResult.rbegin( )->second;

    // Difference between the two propagated states at departure and arrival.
    return std::make_pair( stateLambertTargeterAtDeparture - propagatedStateFullProblemAtDeparture,
                           stateLambertTargeterAtArrival - propagatedStateFullProblemAtArrival);
}




////std::map< double, std::pair<Eigen::Vector6d, Eigen::Vector6d> >
//void fullPropagationMGA(
//        const int numberOfLegs,
//        const std::vector< std::string >& nameBodiesTrajectory,
//        const std::vector< std::string >& centralBody,
//        const std::vector< std::string >& bodyToPropagate,
//        const std::vector< int >& legTypeVector,
//        const std::vector< ephemerides::EphemerisPointer >& ephemerisVector,
//        const Eigen::VectorXd& gravitationalParameterVector,
//        const Eigen::VectorXd& trajectoryVariableVector,
//        const double centralBodyGravitationalParameter,
//        const Eigen::VectorXd& minimumPericenterRadiiVector,
//        const Eigen::VectorXd& semiMajorAxesVector,
//        const Eigen::VectorXd& eccentricitiesVector,
//        const std::shared_ptr< numerical_integrators::IntegratorSettings< double > > integratorSettings,
//        std::map< int, std::map< double, Eigen::Vector6d > >& lambertTargeterResultForEachLeg,
//        std::map< int, std::map< double, Eigen::Vector6d > >& fullProblemResultForEachLeg){


//    // Calculate the MGA trajectory
//    tudat::transfer_trajectories::Trajectory trajectory( numberOfLegs, legTypeVector, ephemerisVector,
//                                                         gravitationalParameterVector, trajectoryVariableVector,
//                                                         centralBodyGravitationalParameter, minimumPericenterRadiiVector,
//                                                         semiMajorAxesVector, eccentricitiesVector );

//    int numberLegsIncludingDSM = ((trajectoryVariableVector.size()-1-numberOfLegs)/4.0) + numberOfLegs ;

//    std::vector< Eigen::Vector3d > positionVector;
//    std::vector< double > timeVector;
//    std::vector< double > deltaVVector;
//    double totalDeltaV;

//    std::map< double, std::pair<Eigen::Vector6d, Eigen::Vector6d> > stateDifferenceAtDepartureAndArrival;

//    // Calculate the orbits
//    trajectory.calculateTrajectory( totalDeltaV );
//    trajectory.maneuvers( positionVector, timeVector, deltaVVector );

//    std::map< int, Eigen::Vector3d > cartesianPositionAtDepartureLambertTargeter;
//    std::map< int, Eigen::Vector3d > cartesianPositionAtArrivalLambertTargeter;


//    integratorSettings->initialTimeStep_ = 1000.0;

//    simulation_setup::NamedBodyMap bodyMap = setupBodyMapLambertTargeter(centralBody[0], bodyToPropagate[0]);
//    basic_astrodynamics::AccelerationMap accelerationMap = setupAccelerationMapLambertTargeter(centralBody[0],
//                                                                                               bodyToPropagate[0], bodyMap);
//    double timeOfFlight;
//    int counterLegTotal = 0;
//    int counterLegWithDSM = 0;
//    std::vector< double > timeOfFlightVector;



//    // Calculate the time of flight for each leg (one leg with a deep-space manoeuvre is divided into two sub-legs)

//    for (int i = 0 ; i < numberOfLegs - 1 ; i ++){

//        if (legTypeVector[i] == transfer_trajectories::mga_Departure ||
//                legTypeVector[i] == transfer_trajectories::mga_Swingby ){

//            timeOfFlight = trajectoryVariableVector[1 + counterLegTotal];
//            timeOfFlightVector.push_back( timeOfFlight );
//            counterLegTotal++;

//        }

//        else {

//                timeOfFlight = trajectoryVariableVector[numberOfLegs + 1 + counterLegWithDSM * 4]
//                        * trajectoryVariableVector[counterLegTotal + 1];
//                timeOfFlightVector.push_back( timeOfFlight );

//                timeOfFlight = (1 - trajectoryVariableVector[numberOfLegs + 1 + counterLegWithDSM * 4])
//                        * trajectoryVariableVector[counterLegTotal + 1];
//                timeOfFlightVector.push_back( timeOfFlight );
//                counterLegWithDSM++;
//                counterLegTotal++;
//        }

//    }



//    for (int i = 0; i<numberLegsIncludingDSM-1 ; i++)
//    {

//        cartesianPositionAtDepartureLambertTargeter[ i ] = positionVector[i];
//        cartesianPositionAtArrivalLambertTargeter[ i ] = positionVector[i+1];

//        std::vector< std::string > departureAndArrivalBodies;
//        departureAndArrivalBodies.push_back( nameBodiesTrajectory[i] );
//        departureAndArrivalBodies.push_back( nameBodiesTrajectory[1 + i]);

//        Eigen::Vector3d cartesianPositionAtDeparture = cartesianPositionAtDepartureLambertTargeter[i];
//        Eigen::Vector3d cartesianPositionAtArrival = cartesianPositionAtArrivalLambertTargeter[i];


//       // Compute the difference in state between the full problem and the Lambert targeter solution at departure and at arrival
//        std::map< double, Eigen::Vector6d > lambertTargeterResultForOneLeg;
//        std::map< double, Eigen::Vector6d > fullProblemResultForOneLeg;
//        propagateLambertTargeterAndFullProblem( cartesianPositionAtDeparture, cartesianPositionAtArrival,
//                timeOfFlightVector[i], 0.0, bodyMap, accelerationMap, bodyToPropagate, centralBody,
//                integratorSettings, lambertTargeterResultForOneLeg, fullProblemResultForOneLeg,
//                departureAndArrivalBodies, true, true);


//        lambertTargeterResultForEachLeg[i] = lambertTargeterResultForOneLeg;
//        fullProblemResultForEachLeg[i] = fullProblemResultForOneLeg;


//    }


//}




//std::map< int, std::pair< Eigen::Vector6d, Eigen::Vector6d > > getDifferenceFullPropagationWrtLambertTargeterMGA(
//        const int numberOfLegs,
//        const std::vector< std::string >& nameBodiesTrajectory,
//        const std::vector< std::string >& centralBody,
//        const std::vector< std::string >& bodyToPropagate,
//        const std::vector< int >& legTypeVector,
//        const std::vector< ephemerides::EphemerisPointer >& ephemerisVector,
//        const Eigen::VectorXd& gravitationalParameterVector,
//        const Eigen::VectorXd& trajectoryVariableVector,
//        const double centralBodyGravitationalParameter,
//        const Eigen::VectorXd& minimumPericenterRadiiVector,
//        const Eigen::VectorXd& semiMajorAxesVector,
//        const Eigen::VectorXd& eccentricitiesVector,
//        const std::shared_ptr< numerical_integrators::IntegratorSettings< double > > integratorSettings)
//{

//    int numberLegsIncludingDSM = ((trajectoryVariableVector.size()-1-numberOfLegs)/4.0) + numberOfLegs ;


//    std::map< int, std::map< double, Eigen::Vector6d > > lambertTargeterResultForEachLeg;
//    std::map< int, std::map< double, Eigen::Vector6d > > fullProblemResultForEachLeg;

//    // compute full problem and Lambert targeter solution both at departure and arrival.

//      fullPropagationMGA(numberOfLegs, nameBodiesTrajectory, centralBody, bodyToPropagate, legTypeVector,
//                       ephemerisVector, gravitationalParameterVector, trajectoryVariableVector,
//                       centralBodyGravitationalParameter, minimumPericenterRadiiVector, semiMajorAxesVector,
//                       eccentricitiesVector, integratorSettings, lambertTargeterResultForEachLeg,
//                       fullProblemResultForEachLeg);


//    std::map< int, std::pair< Eigen::Vector6d, Eigen::Vector6d > > stateDifferenceAtArrivalAndDepartureForEachLeg;

//    for (int i = 0 ; i< numberLegsIncludingDSM-1 ; i++){

//        std::map< double, Eigen::Vector6d > lambertTargeterResultCurrentLeg = lambertTargeterResultForEachLeg[i];
//        std::map< double, Eigen::Vector6d > fullProblemResultCurrentLeg = fullProblemResultForEachLeg[i];

//        Eigen::Vector6d stateLambertTargeterAtDepartureForOneLeg = lambertTargeterResultCurrentLeg.begin( )->second;
//        Eigen::Vector6d stateFullProblemAtDepartureForOneLeg = fullProblemResultCurrentLeg.begin( )->second;
//        Eigen::Vector6d stateLambertTargeterAtArrivalForOneLeg = lambertTargeterResultCurrentLeg.rbegin( )->second;
//        Eigen::Vector6d stateFullProblemAtArrivalForOneLeg = fullProblemResultCurrentLeg.rbegin( )->second;

//        stateDifferenceAtArrivalAndDepartureForEachLeg[i] = std::make_pair( stateLambertTargeterAtDepartureForOneLeg -
//                                                                            stateFullProblemAtDepartureForOneLeg,
//                                                                            stateLambertTargeterAtArrivalForOneLeg -
//                                                                            stateFullProblemAtArrivalForOneLeg);



//    }


//    return stateDifferenceAtArrivalAndDepartureForEachLeg;

//}








}

}