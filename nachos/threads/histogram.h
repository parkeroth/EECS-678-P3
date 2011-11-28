#ifndef HISTO_H
#define HISTO_H

/*
 * A histogram class (and a bucket class)
 *
 * It is used by the METRICS code in the ThreadIP class found in thread_ip.{h,cc}
 *
 * histo_test.cc is a test driver.
 *
 * By: Nicolas Frisby. Mar 2006
 */

#include <stddef.h>

/*
 * HistoDatumT is the type of the value tracked by the histogram.
 * It must meet the following constraints:
 *   <, >, >=, +=, and (s << datum) (stream concat) must be defined
 *   a - b must yield a value coercible to a size_t
 */
typedef int HistoDatumT;


/*
 * buckets keep a:
 *  1) count,
 *  2) whether or not that count has overflowed, and
 *  3) extrema
 */
class Bucket
{
 public:
  Bucket();

  // add a datum to the bucket
  void Deposit( const HistoDatumT &datum );

  // resets all values
  void Reset();

  bool overflowed;
  size_t count;
  HistoDatumT v_min;
  HistoDatumT v_max;
};



// the histogram class
class Histogram
{
 public:
  enum OWorA { OVERWRITE , APPEND };

  Histogram( size_t _num_buckets , size_t _bucket_width , 
	     HistoDatumT range_min );
  ~Histogram();

  // member used to add a value to the histogram
  void RecordDatum( const HistoDatumT &datum );

  // writes bucket data to a file
  void Write( const char *fname , OWorA openmode = OVERWRITE ) const;

  // resets all histogram data
  void Reset();

  // accessors to observations of the data so far collected
  size_t Count( bool &overflowed ) const;
  HistoDatumT Sum( bool &overflowed ) const;
  bool Minimum( HistoDatumT& ) const;
  bool Maximum( HistoDatumT& ) const;


  // class data members: (N.B. NOT object members)

  // for inactivity histo
  static size_t HistoN1;
  static size_t HistoWidth1;
  static int HistoMin1;

  // for activity histo
  static size_t HistoN2;
  static size_t HistoWidth2;
  static int HistoMin2;

  // NOTE: default values for the above class members are initialized
  // in histogram.cc
 private:
  size_t num_buckets;
  size_t bucket_width;

  // values below range_min are placed in the underflow bucket
  HistoDatumT range_min;

  // values equal to or greater than range_max are placed in the
  // overflow bucket
  HistoDatumT range_max;

  // pointer to dynamic memory holding the buckets
  Bucket *buckets;

  Bucket underflow_bucket;
  Bucket overflow_bucket;

  // histogram attempts to track a sum
  // if that sum overflows, it stops tracking the sum
  HistoDatumT sum;
  bool sum_overflowed;
};

#endif /* HISTO_H */
