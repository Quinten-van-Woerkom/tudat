/*    Copyright (c) 2010-2016, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#ifndef TUDAT_RADIATIONPRESSUREACCELERATIONPARTIAL_H
#define TUDAT_RADIATIONPRESSUREACCELERATIONPARTIAL_H

#include <boost/shared_ptr.hpp>

#include "Tudat/Astrodynamics/OrbitDetermination/AccelerationPartials/accelerationPartial.h"
#include "Tudat/Astrodynamics/ElectroMagnetism/radiationPressureInterface.h"

namespace tudat
{

namespace orbit_determination
{

namespace partial_derivatives
{

//! Calculates partial derivative of cannon ball radiation pressure acceleration wrt radiation pressure coefficient.
/*!
 * Calculates partial derivative of cannon ball radiation pressure acceleration wrt radiation pressure coefficient.
 * \param radiationPressure Current radiation pressure (in N/m^2)
 * \param area (Reference) area for radiation pressure acceleration.
 * \param bodyMass Mass of body undergoing acceleration.
 * \param vectorToSource Vector from body undergoing acceleration to source of radiation.
 * \return Partial derivative of cannon ball radiation pressure acceleration wrt radiation pressure coefficient.
 */
Eigen::Vector3d computePartialOfCannonBallRadiationPressureAccelerationWrtRadiationPressureCoefficient(
        const double radiationPressure,
        const double area,
        const double bodyMass,
        const Eigen::Vector3d& vectorToSource );

//! Class to calculate the partials of the cannnonball radiation pressure acceleration w.r.t. parameters and states.
class CannonBallRadiationPressurePartial: public AccelerationPartial
{
public:

    //! Constructor.
    /*!
     * Constructor.
     * \param radiationPressureInterface Interface object for properties of radiation pressure computation (i.e. reference
     * area, pressure magnitude, etc.)
     * \param massFunction Function returning the mass of the body undergoing the acceleration.
     * \param acceleratedBody Name of the body undergoing acceleration.
     * \param acceleratingBody Name of the body exerting acceleration.
     */
    CannonBallRadiationPressurePartial(
            const boost::shared_ptr< electro_magnetism::RadiationPressureInterface > radiationPressureInterface,
            const boost::function< double( ) > massFunction,
            const std::string& acceleratedBody, const std::string& acceleratingBody ):
        AccelerationPartial( acceleratedBody, acceleratingBody,
                             basic_astrodynamics::cannon_ball_radiation_pressure ),
        sourceBodyState_( radiationPressureInterface->getSourcePositionFunction( ) ),
        acceleratedBodyState_( radiationPressureInterface->getTargetPositionFunction( ) ),
        areaFunction_( boost::bind( &electro_magnetism::RadiationPressureInterface::getArea, radiationPressureInterface ) ),
        radiationPressureCoefficientFunction_(
            boost::bind( &electro_magnetism::RadiationPressureInterface::getRadiationPressureCoefficient,
                         radiationPressureInterface ) ),
        radiationPressureFunction_( boost::bind( &electro_magnetism::RadiationPressureInterface::getCurrentRadiationPressure,
                                                 radiationPressureInterface ) ),
        acceleratedBodyMassFunction_( massFunction ){ }

    //! Destructor.
    ~CannonBallRadiationPressurePartial( ){ }

    //! Function for calculating the partial of the acceleration w.r.t. the position of body undergoing acceleration..
    /*!
     *  Function for calculating the partial of the acceleration w.r.t. the position of body undergoing acceleration
     *  and adding it to exting partial block.
     *  Update( ) function must have been called during current time step before calling this function.
     *  \param partialMatrix Block of partial derivatives of acceleration w.r.t. Cartesian position of body
     *  undergoing acceleration where current partial is to be added.
     *  \param addContribution Variable denoting whether to return the partial itself (true) or the negative partial (false).
     *  \param startRow First row in partialMatrix block where the computed partial is to be added.
     *  \param startColumn First column in partialMatrix block where the computed partial is to be added.
     */
    void wrtPositionOfAcceleratedBody( Eigen::Block< Eigen::MatrixXd > partialMatrix,
                                       const bool addContribution = 1, const int startRow = 0, const int startColumn = 0 )
    {
        if( addContribution )
        {
            partialMatrix.block( startRow, startColumn, 3, 3 ) += currentPartialWrtPosition_;
        }
        else
        {
            partialMatrix.block( startRow, startColumn, 3, 3 ) -= currentPartialWrtPosition_;
        }
    }

    //! Function for calculating the partial of the acceleration w.r.t. the velocity of body undergoing acceleration..
    /*!
     *  Function for calculating the partial of the acceleration w.r.t. the velocity of body undergoing acceleration and
     *  adding it to exting partial block.
     *  The update( ) function must have been called during current time step before calling this function.
     *  \param partialMatrix Block of partial derivatives of acceleration w.r.t. Cartesian position of body
     *  exerting acceleration where current partial is to be added.
     *  \param addContribution Variable denoting whether to return the partial itself (true) or the negative partial (false).
     *  \param startRow First row in partialMatrix block where the computed partial is to be added.
     *  \param startColumn First column in partialMatrix block where the computed partial is to be added.
     */
    void wrtPositionOfAcceleratingBody( Eigen::Block< Eigen::MatrixXd > partialMatrix,
                                        const bool addContribution = 1, const int startRow = 0, const int startColumn = 0 )
    {
        if( addContribution )
        {
            partialMatrix.block( startRow, startColumn, 3, 3 ) -= currentPartialWrtPosition_;
        }
        else
        {
            partialMatrix.block( startRow, startColumn, 3, 3 ) += currentPartialWrtPosition_;
        }
    }

    //! Function for setting up and retrieving a function returning a partial w.r.t. a double parameter.
    /*!
     *  Function for setting up and retrieving a function returning a partial w.r.t. a double parameter.
     *  Function returns empty function and zero size indicator for parameters with no dependency for current acceleration.
     *  \param parameter Parameter w.r.t. which partial is to be taken.
     *  \return Pair of parameter partial function and number of columns in partial (0 for no dependency, 1 otherwise).
     */
    std::pair< boost::function< void( Eigen::MatrixXd& ) >, int >
    getParameterPartialFunction(
            boost::shared_ptr< estimatable_parameters::EstimatableParameter< double > > parameter );

    //! Function for setting up and retrieving a function returning a partial w.r.t. a vector parameter.
    /*!
     *  Function for setting up and retrieving a function returning a partial w.r.t. a vector parameter.
     *  Function returns empty function and zero size indicator for parameters with no dependency for current acceleration.
     *  \param parameter Parameter w.r.t. which partial is to be taken.
     *  \return Pair of parameter partial function and number of columns in partial (0 for no dependency).
     */
    std::pair< boost::function< void( Eigen::MatrixXd& ) >, int > getParameterPartialFunction(
            boost::shared_ptr< estimatable_parameters::EstimatableParameter< Eigen::VectorXd > > parameter );

    void wrtRadiationPressureCoefficient( Eigen::MatrixXd& partial )
    {
        partial = computePartialOfCannonBallRadiationPressureAccelerationWrtRadiationPressureCoefficient(
                    radiationPressureFunction_( ), areaFunction_( ), acceleratedBodyMassFunction_( ),
                    ( sourceBodyState_( ) - acceleratedBodyState_( ) ).normalized( ) );
    }

    //! Function for updating partial w.r.t. the bodies' positions
    /*!
     *  Function for updating common blocks of partial to current state. For the radiation pressure acceleration,
     *  position partial is computed and set.
     *  \param currentTime Time at which partials are to be calculated
     */
    void update( const double currentTime = 0.0 )
    {
        if( !( currentTime_ == currentTime ) )
        {
            // Compute helper quantities.
            Eigen::Vector3d rangeVector = ( acceleratedBodyState_( ) - sourceBodyState_( ) );
            double range = rangeVector.norm( );
            double rangeInverse = 1.0 / ( range );

            // Compute position partial.
            currentPartialWrtPosition_ =
                    ( radiationPressureCoefficientFunction_( ) * areaFunction_( ) * radiationPressureFunction_( ) /
                      acceleratedBodyMassFunction_( ) ) * ( Eigen::Matrix3d::Identity( ) * rangeInverse - 3.0 *
                                                            rangeVector * rangeVector.transpose( ) * rangeInverse / (
                                                                range * range ) );
            currentTime_ = currentTime;
        }
    }

private:

    //! Function returning position of radiation source.
    boost::function< Eigen::Vector3d( ) > sourceBodyState_;

    //! Function returning position of body undergoing acceleration.
    boost::function< Eigen::Vector3d( )> acceleratedBodyState_;

    //! Function returning reflecting (or reference) area of radiation pressure on acceleratedBody_
    boost::function< double( ) > areaFunction_;

    //! Function returning current radiation pressure coefficient (usually denoted C_{r}).
    boost::function< double( ) > radiationPressureCoefficientFunction_;

    //! Function returning current radiation pressure (in N/m^{2})
    boost::function< double( ) > radiationPressureFunction_;

    //! Function returning the mass of the body undergoing the acceleration.
    boost::function< double( ) > acceleratedBodyMassFunction_;

    //! Current partial of acceleration w.r.t. position of body undergoing acceleration (equal to minus partial w.r.t.
    //! position of body exerting acceleration).
    Eigen::Matrix3d currentPartialWrtPosition_;
};

}

}

}

#endif // TUDAT_RADIATIONPRESSUREACCELERATIONPARTIAL_H
