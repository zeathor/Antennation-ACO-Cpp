#ifndef COLONY_H_
#define COLONY_H_
#include "Definitions.h"
#include "Ant.h"
#include "PherMap.h"
#include <fstream>

class Colony
{
public:
	//default constructor
	Colony(); 
	//constructor - acts as initialiser
	Colony(int RandSeed, TBatchCfg theBCfg, TLocalSearchCfg theLSCfg, TPherCfg thePCfg, TAntCfg theACfg); 
	virtual ~Colony();
	void SetFile(std::string fileName);
	
	void InitMap(const std::vector<std::string> & myList);
	void RunBatch();
	
private:
    void ACSBatch(int totalIter);
	void ASBatch(int totalIter);
    void AMTSBatch(int totalIter);
    void MMBatch(int totalIter);
    void SetRandGen(int RandSeed);	

    bool OneTour();
        
    std::string GetLSType(int theType);
    std::string GetAntType(int theType);
    std::string GetAntDir(int theType);
    int Antennation();
    
    bool UpdatePheromone();
    
    std::vector<Ant> aL; //ant vector
    std::vector<int> bPath; //best path vector
    std::vector< std::vector<int> > aCL; //stores where any ant is at a given time - antCityList
    
    int bDist; //the best distance thus far
    int aMeet; //number of meetings in a tour -- TERRIBLE OO
    
    TPherCfg 		pCfg;
    TAntCfg 		aCfg;
    TLocalSearchCfg lSCfg;
    TBatchCfg		bCfg;
    std::ofstream 	saveFile;
    
	RandomMersenne randGen; //Random Number Generator, passed to PherMap and Ant
    PherMap myMap;			//pheromone map, passed to Ant
    
};

#endif /*COLONY_H_*/




