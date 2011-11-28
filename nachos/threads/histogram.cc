#include "histogram.h"

#include <fstream>
#include <iomanip>
#include <limits>



// Bucket class
Bucket::Bucket()
{
  Reset();
}

void
Bucket::Deposit( const HistoDatumT &datum )
{
  // keep count until it overflows
  if ( ! overflowed )
    {
      size_t prev = count;
      ++ count;
      
      if ( prev > count )
	  overflowed = true;
    }

  // maintain extrema
  if ( datum < v_min )
    v_min = datum;

  if ( datum > v_max )
    v_max = datum;
}

void
Bucket::Reset()
{
  overflowed = false;
  count = 0;
  v_min = std::numeric_limits<HistoDatumT>::max();
  v_max = std::numeric_limits<HistoDatumT>::min();
}



// default values for histogram bounds
size_t Histogram::HistoN1 = 20;
size_t Histogram::HistoWidth1 = 10;
int Histogram::HistoMin1 = 0;
size_t Histogram::HistoN2 = 20;
size_t Histogram::HistoWidth2 = 10;
int Histogram::HistoMin2 = 0;

// Histogram class
Histogram::Histogram( size_t _num_buckets , size_t _bucket_width ,
		      HistoDatumT _range_min  )
  : num_buckets( _num_buckets), bucket_width( _bucket_width ),
    range_min( _range_min ), range_max( _range_min + _num_buckets * _bucket_width ),
    buckets( NULL ),
    underflow_bucket(), overflow_bucket(),
    sum( 0 ), sum_overflowed( false )
{
  // allocate buckets
  buckets = new Bucket[ num_buckets ];
}

Histogram::~Histogram()
{
  // deallocate buckets
  delete [] buckets;
  buckets = NULL;
}

// member used to add a value to the histogram
void
Histogram::RecordDatum( const HistoDatumT &datum )
{
  if ( datum < range_min )
    { // --> datum goes in the underflow bucket
      underflow_bucket.Deposit( datum );
    }
  else if ( datum >= range_max )
    { // --> datum goes in the overflow bucket
      overflow_bucket.Deposit( datum );
    }
  else
    { // --> put the datum in its bucket
      size_t which = datum - range_min;
      which /= bucket_width;

      buckets[ which ].Deposit( datum );
    }

  // maintain a sum value until it overflows
  if ( ! sum_overflowed )
    {
      HistoDatumT prev = sum;
      sum += datum;

      if ( prev > sum )
	sum_overflowed = true;
    }
}

void
Histogram::Write( const char *fname , OWorA openmode ) const
{
  using namespace std;

  // field widths
  const size_t whichw = 5;
  const size_t countw = 10;
  const size_t minw = 10;
  const size_t maxw = 10;
  const char delim = '\t';

  // open output file
  ios_base::openmode flags;
  if ( OVERWRITE == openmode )
    flags = ios_base::ate;
  else
    flags = ios_base::trunc;

  ofstream fout( fname , ios_base::out | flags );
  if ( ! fout.good() )
    {
      fprintf( stderr , "Could not open '%s' for writing.\n" , fname );

      return;
    }

  // output histogram sizing data
  fout << "#HSPEC:";
  fout << num_buckets << ',';
  fout << bucket_width << ',';
  fout << range_min << endl;
  fout << endl;

  // output header row
  fout << '#';
  fout << setw( whichw ) << "WHICH";
  fout << delim << setw( countw ) << "COUNT";
  fout << delim << setw( minw ) << "MIN";
  fout << delim << setw( maxw ) << "MAX";
  fout << endl;

  fout << endl;

  // output underflow bucket's data
  fout << setw( whichw ) << 0;
  fout << delim << setw( countw );
  if ( underflow_bucket.overflowed )
      fout << "OVER";
  else
      fout << underflow_bucket.count;
  fout << delim << setw( minw ) << underflow_bucket.v_min;
  fout << delim << setw( maxw ) << underflow_bucket.v_max;
  fout << endl;

  fout << endl;

  // output data for each bucket
  for ( size_t which = 0 ;
	which < num_buckets ;
	++ which )
    {
      fout << setw( whichw ) << (which + 1);
      fout << delim << setw( countw );
      if ( buckets[which].overflowed )
	fout << "OVER";
      else
	fout << buckets[which].count;
      fout << delim << setw( minw ) << buckets[which].v_min;
      fout << delim << setw( maxw ) << buckets[which].v_max;
      fout << endl;
    }

  fout << endl;

  // output overflow bucket's data
  fout << setw( whichw ) << (num_buckets + 1);
  fout << delim << setw( countw );
  if ( overflow_bucket.overflowed )
      fout << "OVER";
  else
      fout << overflow_bucket.count;
  fout << delim << setw( minw ) << overflow_bucket.v_min;
  fout << delim << setw( maxw ) << overflow_bucket.v_max;
  fout << endl;


  fout.close();
}

void Histogram::Reset()
{
  for ( size_t which = 0 ;
	which < num_buckets ;
	++ which )
    buckets[which].Reset();

  underflow_bucket.Reset();
  overflow_bucket.Reset();
  sum = 0;
  sum_overflowed = false;
}

// if representible by a size_t value, return the number of values
// that have been recorded in this histogram
size_t Histogram::Count( bool &overflowed ) const
{
  // check *flow overflows
  if ( underflow_bucket.overflowed ||
       overflow_bucket.overflowed )
    {
      overflowed = true;
      return 0;
    }

  // add under
  size_t count = underflow_bucket.count;
  size_t prev = count;

  // add over
  count += overflow_bucket.count;
  if ( prev > count )
    {
      overflowed = true;
      return 0;
    }

  for ( size_t which = 0 ;
	which < num_buckets ;
	++ which )
    {
      // if any bucket overflowed, count overflows
      if ( buckets[which].overflowed )
	{
	  overflowed = true;
	  return 0;
	}

      // add each bucket
      prev = count;
      count += buckets[which].count;
      if ( prev > count )
	{
	  overflowed = true;
	  return 0;
	}
    }

  // return count
  return count;
}

// return the sum, if it hasn't overflowed
HistoDatumT Histogram::Sum( bool &overflowed ) const
{
  overflowed = sum_overflowed;
  return sum;
}

bool Histogram::Minimum( HistoDatumT &min ) const
{
  // least bucket to...
  if ( 0 < underflow_bucket.count )
    {
      min = underflow_bucket.v_min;
      return true;
    }

  for ( size_t which = 0 ;
	which < num_buckets ;
	++ which )
    {
      if ( 0 < buckets[which].count )
	{
	  min = buckets[which].v_min;
	  return true;
	}
    }

  // greatest bucket
  if ( 0 < underflow_bucket.count )
    {
      min = underflow_bucket.v_min;
      return true;
    }

  return false;
}

bool Histogram::Maximum( HistoDatumT &max ) const
{
  // greatest bucket to...
  if ( 0 < overflow_bucket.count )
    {
      max = overflow_bucket.v_max;
      return true;
    }

  for ( size_t which = num_buckets ;
	0 < which ;
	-- which )
    {
      if ( 0 < buckets[which - 1].count )
	{
	  max = buckets[which - 1].v_max;
	  return true;
	}
    }

  // least bucket
  if ( 0 < underflow_bucket.count )
    {
      max = underflow_bucket.v_max;
      return true;
    }

  return false;
}
