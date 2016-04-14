#ifndef PHERMAP_H_
#define PHERMAP_H_
#include "Definitions.h"
#include <string>


struct TSegment{
	double a;
	double p;
	int d;
};


class PherMap{
public:
	PherMap(); //default constructor - not used
	PherMap(TPherCfg thePCfg, RandomMersenne *thePointer);
	virtual ~PherMap(){};
	
	void SetCfg(TPherCfg thePCfg);
	void LoadCities(const std::vector<std::string> & myList);
	void ReducePher();
	void InitPher();
	void LayPherPath(const std::vector<int> & path, int dist);
	void LayACSBestPath(const std::vector<int> & path, int dist);
	void LayACSEdge(int cC, int nC);
	void LayMMBestPath(const std::vector<int> & path, int dist);	

	
	//these 4 functions return distance/pheromone from path segment using the cut-down triangular array
	//first function uses indexes, second function uses path and indexes
	int GetSegD(int i, int j);
	int GetSegD(int i, int j, const std::vector<int> & path);
	
	double GetSegP(int i, int j);
	double GetSegP(int i, int j, const std::vector<int> & path);
	
	double GetSegA(int i, int j);
	double GetSegA(int i, int j, const std::vector<int> & path);

	
	
	// sets the pheromone levels of an arc ensuring no out-of-bounds arrays
	//first function sets the pheromone using a given path and then the indices of 2 cities in that path
	//second function uses the absolute city numbers
	void SetSegP(int i, int j, const std::vector<int> & path, double p);
	void SetSegP(int i, int j, double p);

	
	int GetPathDistance(const std::vector<int> & thePath);

	
	void CalcMapAttract();
	
private:
	std::vector< std::vector<TSegment> > s;
	TPherCfg pCfg;
	RandomMersenne *randGen; //pointer to global Random nubmer generator
	int WorldDistance(double x1, double y1, double x2, double y2);
	

	
};


  
#endif /*PHERMAP_H_*/
