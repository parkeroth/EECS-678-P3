/*
 * This file is to be used to test the Histogram class.
 *
 * % g++ <this file> histogram.o -o histo_test
 * % histo_test <num buckets> <divisor: e.g. 10> <num samples>
 *
 * generates the 'test.histo' file and prints some metrics to cout
 */
#include "histogram.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace std;

int
main( int argc , char *argv[] )
{
  srand( time( NULL ) );

  size_t n = atoi( argv[1] );
  size_t d = atoi( argv[2] );
  size_t k = atoi( argv[3] );
  Histogram h ( n , RAND_MAX / d / n , 0 );

  for ( size_t i = 0 ;
	i < k ;
	++ i )
    {
      h.RecordDatum( rand() / d );
    }

  h.Write( "test.histo" );

  bool over;

  cout << boolalpha;

  over = false;
  cout << "Count: " << h.Count(over) << '\t' << "Overflowed? " << over << endl;
  over = false;
  cout << "Sum: " << h.Sum(over) << '\t' << "Overflowed? " << over << endl;
  cout << "Min: " << h.Minimum() << endl;
  cout << "Max: " << h.Maximum() << endl;

  return 0;
}
