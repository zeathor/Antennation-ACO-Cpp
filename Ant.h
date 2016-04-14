#ifndef ANT_H_
#define ANT_H_
#include "Definitions.h"
#include "PherMap.h"

class Ant
{
public:
	Ant(); //blank constructor
	Ant(const Ant& theAnt); //copy constructor
	virtual ~Ant();
	const std::vector<int> & GetPath();
	const std::vector< std::vector<int> > & GetTree();
	int	GetDistance(); 
	int Move();  //returns what city it moved to, -1 if finished , antMeet is the number of meetings between ants thus far
	int MoveFinal();
	
	void Init(TAntCfg theACfg, TLocalSearchCfg theLSCfg, int a, int b, int theID);
	void ChangeCfg(TAntCfg theACfg);
	void SetMap(PherMap *thePher);
	void SetRandGen(RandomMersenne *theRand);

	void SetStartCity(int startCity);
	void Consolidate(const std::vector< std::vector<int> > & antTree, int theMeet);
	void TwoOpt(); //perform 2 opt local search
		
private:
	std::vector<int> tL;
	std::vector< std::vector<int> > AMTSTree;   //arcs travelled by ant in previous tours AMTS
	std::vector< std::vector<int> > antenMTree;//arcs travelled by other ants, stored  ANTENNATION
	std::vector< std::vector<int> > antenOTree;//used to consolidate only, temporary storage ANTENNATION
	
	
	
	int meet;
	int tD;   //total distance travelled thus far
	int cC;   //currentCity
	int cCnt;  //city counter
	int age;  //Age of ant AMTS
	int iD;
	
	//lets get rid of mCfg
	int alpha;
	int beta;
	
	TAntCfg aCfg; //ant config
	TLocalSearchCfg lSCfg; //local search config
	
	//random generator object. a pointer as the same randgen should be used for all classes.
	RandomMersenne *randGen;
	
	//pheromone map object. is actually a pointer
	PherMap *myMap;
	
	bool	IsNotTabooList(int city);
	double  AntennationFactor(int nextCity);
	double  AMTSFactor(int nextCity);

	
	void   ResetList();        //resets taboo list
	void   ResetAntenTree();   //resets antennation tree
	void ResetAMTSTree();    //resets AMTS tree	
};

#endif /*ANT_H_*/
