/*! \file unitTestUnitConversions.h
 *    This unit test will test the unit conversions that are
 *    defined in unitConversions.h.
 *
 *    Path              : /Astrodynamics/
 *    Version           : 1
 *    Check status      : Checked
 *
 *    Author            : J. Melman
 *    Affiliation       : Delft University of Technology
 *    E-mail address    : J.C.P.Melman@tudelft.nl
 *
 *    Checker           : D. Dirkx
 *    Affiliation       : Delft University of Technology
 *    E-mail address    : D.Dirkx@student.tudelft.nl
 *
 *    Date created      : 11 January, 2011
 *    Last modified     : 24 January, 2011
 *
 *    References
 *
 *    Notes
 *      Test runs code and verifies result against expected value.
 *      If the tested code is erroneous, the test function returns a boolean
 *      true; if the code is correct, the function returns a boolean false.
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
 *      110111    J. Melman         First creation of code.
 *      110124    J. Melman         Adapted to the offical Tudat standards.
 */

// Include statements.
#include <iostream>
#include "unitConversions.h"
#include "physicalConstants.h"
#include "basicMathematicsFunctions.h"

//! Namespace for all unit tests.
/*!
 * Namespace containing all unit tests.
 */
namespace unit_tests
{

//! Test unit conversions.
/*!
 * Tests unit conversions.
 * \return Boolean indicating success of test
 * ( false = successful; true = failed ).
 */
bool testUnitConversions( );

}

// End of file.