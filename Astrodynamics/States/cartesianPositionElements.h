/*! \file cartesianPositionElements.h
 *    This header file contains the Cartesian position elements class included
 *    in Tudat.
 *
 *    Path              : /Astrodynamics/States/
 *    Version           : 2
 *    Check status      : Checked
 *
 *    Checker           : K. Kumar
 *    Affiliation       : Delft University of Technology
 *    E-mail address    : K.Kumar@tudelft.nl
 *
 *    Checker           : D. Dirkx
 *    Affiliation       : Delft University of Technology
 *    E-mail address    : D.Dirkx@tudelft.nl
 *
 *    Date created      : 31 January, 2011
 *    Last modified     : 4 February, 2011
 *
 *    References
 *
 *    Notes
 *
 *    Copyright (c) 2010 Delft University of Technology.
 *
 *    This software is protected by national and international copyright.
 *    Any unauthorized use, reproduction or modification is unlawful and
 *    will be prosecuted. Commercial and non-private application of the
 *    software in any form is strictly prohibited unless otherwise granted
 *    by the authors.
 *
 *    The code is provided without any warranty; without even the implied
 *    warranty of merchantibility or fitness for a particular purpose.
 *
 *    Changelog
 *      YYMMDD    Author            Comment
 *      110131    K. Kumar          First creation of code.
 *      110204    K. Kumar          Removed positionVector_, velocityVector_.
 */

#ifndef CARTESIANPOSITIONELEMENTS_H
#define CARTESIANPOSITIONELEMENTS_H

// Include statements.
#include "orbitalElements.h"

//! Cartesian position elements class.
/*!
 * This class contains the Cartesian position elements.
 */
class CartesianPositionElements : public OrbitalElements
{
public:

    //! Default constructor.
    /*!
     * Default constructor.
     */
    CartesianPositionElements( );

    //! Default destructor.
    /*!
     * Default destructor.
     */
    ~CartesianPositionElements( );

    //! Set Cartesian element: x.
    /*!
     * Sets the Cartesian element: x.
     * \param cartesianElementX Cartesian element: x.
     */
    void setCartesianElementX( const double& cartesianElementX );

    //! Set Cartesian element: y.
    /*!
     * Sets the Cartesian element: y.
     * \param cartesianElementY Cartesian element: y.
     */
    void setCartesianElementY( const double& cartesianElementY );

    //! Set Cartesian element: z.
    /*!
     * Sets the Cartesian element: z.
     * \param cartesianElementZ Cartesian element: z.
     */
    void setCartesianElementZ( const double& cartesianElementZ );

    //! Get Cartesian element: x.
    /*!
     * Returns the Cartesian element: x.
     * \return Cartesian element: x.
     */
    double& getCartesianElementX( );

    //! Get Cartesian element: y.
    /*!
     * Returns the Cartesian element: y.
     * \return Cartesian element: y.
     */
    double& getCartesianElementY( );

    //! Get Cartesian element: z.
    /*!
     * Returns the Cartesian element: z.
     * \return Cartesian element: z.
     */
    double& getCartesianElementZ( );

    //! Overload ostream to print class information.
    /*!
     * Overloads ostream to print class information.
     * \param stream Stream object.
     * \param cartesianPositionElements CartesianPositionElements object.
     * \return Stream object.
     */
    friend std::ostream& operator<<( std::ostream& stream,
                                     CartesianPositionElements&
                                     cartesianPositionElements );

protected:

private:
};

#endif // CARTESIANPOSITIONELEMENTS_H

// End of file.