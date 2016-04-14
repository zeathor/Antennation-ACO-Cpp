#ifndef RANDOMMERSENNE_H_
#define RANDOMMERSENNE_H_

#if defined(__GNUC__)
  // Compilers supporting C99 or C++0x have inttypes.h defining these integer types
  #include <inttypes.h>
  #define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#elif defined(_WIN16) || defined(__MSDOS__) || defined(_MSDOS) 
   // 16 bit systems use long int for 32 bit integer
  typedef   signed long int int32_t;
  typedef unsigned long int uint32_t;
#elif defined(_MSC_VER)
  // Microsoft have their own definition
  typedef   signed __int32  int32_t;
  typedef unsigned __int32 uint32_t;
  typedef   signed __int64  int64_t;
  typedef unsigned __int64 uint64_t;
  #define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#else
  // This works with most compilers
  typedef signed int          int32_t;
  typedef unsigned int       uint32_t;
  typedef long long           int64_t;
  typedef unsigned long long uint64_t;
  #define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#endif

#if 0
#define MERS_N   351
#define MERS_M   175
#define MERS_R   19
#define MERS_U   11
#define MERS_S   7
#define MERS_T   15
#define MERS_L   17
#define MERS_A   0xE4BD75F5
#define MERS_B   0x655E5280
#define MERS_C   0xFFD58000
#else    
// or constants for type MT19937:
#define MERS_N   624
#define MERS_M   397
#define MERS_R   31
#define MERS_U   11
#define MERS_S   7
#define MERS_T   15
#define MERS_L   18
#define MERS_A   0x9908B0DF
#define MERS_B   0x9D2C5680
#define MERS_C   0xEFC60000
#endif


class RandomMersenne
{
	
public:
	RandomMersenne(){
		RandomInit(0); LastInterval = 0;} //default constructor
   RandomMersenne(int seed) {         // Constructor
      RandomInit(seed); LastInterval = 0;}
   void RandomInit(int seed);          // Re-seed
   void RandomInitByArray(int const seeds[], int NumSeeds); // Seed by more than 32 bits
   int IRandom (int min, int max);     // Output random integer
   int IRandomX(int min, int max);     // Output random integer, exact */
   double Random();                    // Output random float
   uint32_t BRandom();                 // Output random bits
private:
   void Init0(int seed);               // Basic initialization procedure
   uint32_t mt[MERS_N];                // State vector
   int mti;                            // Index into mt
   uint32_t LastInterval;              // Last interval length for IRandomX
   uint32_t RLimit;                    // Rejection limit used by IRandomX
};

#endif /*RANDOMMERSENNE_H_*/
