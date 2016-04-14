#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <vector>
#include <cmath>
#include <climits>
#include "RandomMersenne.h"
#include <string>
#include <iostream>
#include <sstream>


//GLOBAL CONSTANTS
#define PI 3.141592653589793238462
#define RRR 6378.388
#define RANDSEED 100



//////////// MAXMIN SETTINGS //////////////////////
#define MMPBEST 0.05

///////// ANTENNATION TYPES ////////////
#define	AFBOOL 0
#define AFLIN  1
#define AFSQR  2
#define AFSQRT 3
#define AFTANH 4
#define AFAMTS 5
#define AF1ONX 6
#define CURRENT_AF_TYPES 7

///////// ANTENNATION DIRECTIONS //////////
#define BIAFDIR   	0
#define UNIAFDIR  	1
#define CURRENT_AFDIR_TYPES 2

/////////// LOCAL SEARCH TYPES ////////////
#define LSNONE  0
#define LSEXP 	1

////////////// ACO TYPES ////////////////
#define ACOAS  	0
#define ACOACS 	1
#define ACOAMTS	2
#define ACOMMAS   	3

/////////////// MAP TYPES ///////////////
#define CARTESIAN 	0
#define LATITUDE	1

double StrToDouble(const std::string & s);
int StrToInt(const std::string & s);



struct TBatchCfg{ //stuff needed for running the batch file
	int nAB, nAS, nAE;     	//number of ants
	double pAB, pAS, pAE;   //pheromone laid
	double pDB, pDS, pDE;   //pheromone decayed
	double pIB, pIS, pIE;   //initial pheromone level
	
	double gPB, gPS, gPE;   //greedy probability ACS
	double igB;             //percentage iteration best vs global best
	int mAB, mAS, mAE;    	//maximum age of ants AMTS
	
	double aFB, aFS, aFE;   //antennation factor
	int gen, iter; 	//generations til stop, iterations per setting
	std::vector<bool> aFType;   //antennation Type -- can do multiple types
	std::vector<bool> aFDir;	//Antennation direction -- can do multiple
	int aLType;				//algorithm type
};


struct TPherCfg{ //stuff needed to run pheromone stuff
	int nC;	//number of cities
	int alpha, beta;	//parameters for calculating city attractiveness
	double pD, pI, pA;	//pheromone parameters
	int aLType;			//algorithm type
	double pMax, pMin;	//max/min values of pheromone (only applicable for MMAS)
	int mType;  //map type - cartesian or latitude
};

struct TLocalSearchCfg{
	int lSTimes;
	int lSType;
};

struct TAntCfg{
	int nC;		//number of cities
	int iD; 	//identity of ant
	int aLType; //algorithm type
	int aFType;	//antennation type
	int aFDir;	//antennation direction (uni/bi)
	double aF;	//antennation amount
	double gP;	//greed probability (only applicable for ACS)	
	int mA;		//max age (only applicable for AMTS	
};



#endif /*DEFINITIONS_H_*/

     