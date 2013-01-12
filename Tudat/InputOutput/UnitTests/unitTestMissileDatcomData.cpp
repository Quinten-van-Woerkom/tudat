/*    Copyright (c) 2010-2012, Delft University of Technology
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without modification, are
 *    permitted provided that the following conditions are met:
 *      - Redistributions of source code must retain the above copyright notice, this list of
 *        conditions and the following disclaimer.
 *      - Redistributions in binary form must reproduce the above copyright notice, this list of
 *        conditions and the following disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *      - Neither the name of the Delft University of Technology nor the names of its contributors
 *        may be used to endorse or promote products derived from this software without specific
 *        prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
 *    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *    COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *    OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *    Changelog
 *      YYMMDD    Author            Comment
 *      110530    F.M. Engelen      First creation of code.
 *      120326    D. Dirkx          Modified code to be boost unit test framework.
 *
 *    References
 *     Reference data generated by datcom script written by F. Engelen. Output has been
 *     checked manually.
 *
 *    Notes
 *
 */

#define BOOST_TEST_MAIN

#include <limits>
#include <string>

#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <TudatCore/Basics/testMacros.h>
#include <TudatCore/InputOutput/matrixTextFileReader.h>

#include "Tudat/InputOutput/basicInputOutput.h"
#include "Tudat/InputOutput/missileDatcomData.h"

namespace tudat
{
namespace unit_tests
{

BOOST_AUTO_TEST_SUITE( test_missile_datcom_data )

BOOST_AUTO_TEST_CASE( testMissileDatcomData )
{
    using namespace tudat::input_output;
    using std::string;

    // Read and process data file.
    string fileLocation = getTudatRootPath( )
            + "InputOutput/UnitTests/inputForBenchMarkMissileDatcomData.dat";
    MissileDatcomData myMissileDatcomData( fileLocation );

    // Set name of file and path to write to and from.
    string outputFileName = getTudatRootPath( ) + "InputOutput/UnitTests/bodyTailConfiguration";

    // Write files for coefficients at each angle of attack.
    myMissileDatcomData.writeCoefficientsToFile( outputFileName );

    // Read benchmark files and check if all entries are equal to those written by DatcomData
    string totalPath;
    Eigen::MatrixXd benchmarkFile, tudatFile;

    for ( int i = 0; i < 5; i++ )
    {
        // Read file just written
        totalPath = outputFileName + "_" + boost::lexical_cast< string >( i );
        tudatFile = readMatrixFromFile( totalPath );

        // Read benchmark file
        totalPath = getTudatRootPath( ) + "InputOutput/UnitTests/benchmark_"
                + boost::lexical_cast< string >( i );
        benchmarkFile = readMatrixFromFile( totalPath );

        // Iterate over all entries and check values.
        for ( int j = 0; j < tudatFile.rows( ); j++ )
        {
            for ( int k = 1; k < tudatFile.cols( ); k++ )
            {
                if ( std::fabs( benchmarkFile( j, k ) ) >
                        std::numeric_limits< double >::epsilon( ) )
                {
                    BOOST_CHECK_CLOSE_FRACTION( tudatFile( j, k ),
                                                benchmarkFile( j, k ),
                                                std::numeric_limits< double >::epsilon( ) );
                }

                else
                {
                    BOOST_CHECK_SMALL( tudatFile( j, k ) - benchmarkFile( j, k ),
                                       std::numeric_limits< double >::epsilon( )  );
                }
            }
        }
    }
}

} // namespace unit_tests
} // namespace tudat

BOOST_AUTO_TEST_SUITE_END( )
