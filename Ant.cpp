#include "Ant.h"


Ant::Ant()
{
	//does nothing, not even resize space. doesn't need to. Everything is done by Init -- convenience
}

//constructor


Ant::~Ant()
{
//does nothing yet - doesn't need to ..... i think
}

Ant::Ant(const Ant& theAnt)//copy constructor
{
	
	
}

//sets the initial config values at the start of a run
void Ant::Init(TAntCfg theACfg, TLocalSearchCfg theLSCfg, int a, int b, int theID)
{
	aCfg = theACfg;
	lSCfg = theLSCfg;
	alpha = a;
	beta = b;
	iD = theID;
	
	tL.resize(aCfg.nC);
	antenMTree.resize(aCfg.nC);
	antenOTree.resize(aCfg.nC);
		    
	
	for(std::vector<std::vector<int> >::iterator rowIt = antenMTree.begin(); rowIt != antenMTree.end(); ++rowIt){
			rowIt->resize(aCfg.nC);	
	}
	
	for(std::vector<std::vector<int> >::iterator rowIt = antenOTree.begin(); rowIt != antenOTree.end(); ++rowIt){
			rowIt->resize(aCfg.nC);	
	}
	
	if (aCfg.aLType == ACOAMTS){
		AMTSTree.resize(aCfg.nC);
		for(std::vector<std::vector<int> >::iterator rowIt = AMTSTree.begin(); rowIt != AMTSTree.end(); ++rowIt){
				rowIt->resize(aCfg.nC);	
		}
	}
	
}


void Ant::SetMap(PherMap *thePher)
{
	myMap = thePher;
}

void Ant::SetRandGen(RandomMersenne *theRand)
{
	randGen = theRand;	
}

//only used to alter settings that change during a batch
void Ant::ChangeCfg(TAntCfg theACfg)
{
	aCfg = theACfg;
}


int Ant::GetDistance()
{
	return tD;
}

const std::vector<int> & Ant::GetPath()
{
	return tL;
}

const std::vector< std::vector<int> > & Ant::GetTree()
{
	return antenMTree;
}


bool Ant::IsNotTabooList(int city)
{
	int i = 0;
	while ((tL[i] != city) && (i < cCnt)) 
		i++;
	if (i == cCnt)
		return true; //haven't found string, ie not taboo
	else 
		return false; //found the string, taboo
}


void Ant::ResetAntenTree()
{
	for(std::vector<std::vector<int> >::iterator rowIt = antenMTree.begin(); rowIt != antenMTree.begin(); ++rowIt){
		for(std::vector<int>::iterator colIt = rowIt->begin(); colIt != rowIt->end(); ++colIt){
			*colIt = 0;
		}		
	}
}

void Ant::ResetAMTSTree()
{
	for(std::vector<std::vector<int> >::iterator rowIt = AMTSTree.begin(); rowIt != AMTSTree.begin(); ++rowIt){
		for(std::vector<int>::iterator colIt = rowIt->begin(); colIt != rowIt->end(); ++colIt){
			*colIt = 0;
		}		
	}
}

void Ant::ResetList()
{
	for(std::vector<int>::iterator rowIt = tL.begin(); rowIt != tL.end(); ++rowIt){
		*rowIt = -1;
	}		
}

void Ant::SetStartCity(int startCity)
{
	ResetList();
	ResetAntenTree();
	if(aCfg.aLType == ACOAMTS){	
		if(age > aCfg.mA){
			age = 0;
	    	ResetAMTSTree();
		} 	
	}
	meet = 0;
	tD = 0;
	cC = startCity;
	cCnt = 0;
	tL[cCnt] = cC;	
}


int Ant::MoveFinal()
{

	tD = tD + myMap->GetSegD(cC, tL[0]);
	age++;	
	return tL[0];
}


int Ant::Move()
{
	double probList[aCfg.nC];
	double sumProb =0.0, mostAttract = 0.0;
	int acsCity = -1;
	int nextCity;
	
	cCnt++;
	
	//completely fill probability list	
	for (int i = 0; i < aCfg.nC; i++) {
		if(IsNotTabooList(i)) {
			double prob = myMap->GetSegA(cC,i)*(1-AntennationFactor(i));								
	
			if(aCfg.aLType == ACOAMTS)
				prob = prob / AMTSFactor(i);
	
				//ACS ALGORITHM, storing most attractive city thus far
			if((aCfg.aLType == ACOACS) && (prob > mostAttract)) {

				mostAttract = prob;
				acsCity = i;
			}
	
			sumProb = sumProb + prob;
			probList[i] = prob;
		}
		else { //taboo, make probability zero, no chance of going there
			probList[i] = 0.0;	
		}
				
	}//finish creating roulette wheel
		
	
	//NEED TO CHECK IF IN ACS MODE AND ALSO RAND IS BELOW GREED THRESHOLD
	//IF SO, CHOOSE MOST ATTRACTIVE CITY IN ROULETTE WHEEL
	if((aCfg.aLType == ACOACS) && (randGen->Random() < aCfg.gP)) {
		nextCity = acsCity;
	}
	else {//DO NORMAL AS ALGORITHM	
		double nmlProb = 0.0;
		nextCity = 0;
		nmlProb = probList[nextCity];
		double prob = randGen->Random() * sumProb; //get a random number between 0 and the total sum of probabilities
		do{
			nextCity++;
			nmlProb += probList[nextCity];
		}
		while(nmlProb <= prob);
	}
	//if using AMTS, increment the segment corresponding to the travelled arc
	if(aCfg.aLType == ACOAMTS)
		AMTSTree[cC][nextCity]++;

	//add record to Antennation internal tree of path taken
	antenMTree[cC][nextCity]++;

	
	//if we are dealing with UniDirectional Antennation, arc ij AND ji are incremented
	if (aCfg.aFDir == UNIAFDIR) 
		antenMTree[nextCity][cC]++;
	
	//If we are dealing with ACS, have to update the arcs locally i.e. after every movement
	if (aCfg.aLType == ACOACS)
		myMap->LayACSEdge(cC, nextCity);
	
	tD = tD + myMap->GetSegD(cC, nextCity);
	tL[cCnt] = nextCity;
	cC = nextCity;
	return  cC; 
			
}

double Ant::AntennationFactor(int nextCity)
{
	double result = 0;
	if (meet != 0){
    	switch(aCfg.aFType){
    	case AFBOOL: 
    		if(antenMTree[cC][nextCity] > 0)
    			result = aCfg.aF; 
    		else 
    			result = 0;
    		break;
    	case AFLIN:  
    		result = aCfg.aF * (antenMTree[cC][nextCity]/meet);
    		break;
    	case AFSQRT: 
    		result = aCfg.aF * sqrt(antenMTree[cC][nextCity]/meet);
    		break;
    	case AFSQR:  
    		result = aCfg.aF * pow(antenMTree[cC][nextCity]/meet, 2.0);
    		break;
    	case AFTANH:
    		result = aCfg.aF * tanh(antenMTree[cC][nextCity]/sqrt(meet));
    		break;
    	case AFAMTS: 
    		result = (aCfg.aF * sqrt(antenMTree[cC][nextCity]))/(1+aCfg.aF * sqrt(antenMTree[cC][nextCity]));
    		break;
    	case AF1ONX: 
    		result = (aCfg.aF * antenMTree[cC][nextCity]) /(1+aCfg.aF * antenMTree[cC][nextCity]);
    		break;
    	}   	
    }
	return result;
}


void Ant::Consolidate(const std::vector< std::vector<int> > & antTree, int theMeet)
{
	meet += theMeet -1;
	for(int i= 0; i < aCfg.nC; i++){
		for(int j = 0; j < aCfg.nC; j++){
			antenMTree[i][j] += antTree[i][j];
		}
	}
}


double Ant::AMTSFactor(int nextCity)
{
	return 1 + sqrt(AMTSTree[cC][nextCity]);
}


//performs 2-Opt with Linear weighting.
//pA is chosen, all other path positions are ranked by closeness
//The 2 nearest positions have weighting ceil((nC-1)/2),  all other cities are
//weighted according to their rank i.e., proximity

void Ant::TwoOpt()
{
	int pA,pB,temp,newDist, offset, theRange;
	theRange =  aCfg.nC / 2;
	
	for(int i = 0; i < lSCfg.lSTimes; i++){
		offset = 0;
		
		pA = randGen->IRandomX(0,aCfg.nC-1); //starting point
		do{
			offset++;			
		} while((randGen->Random() > 0.25) && (offset < theRange));
					
		if (randGen->Random() > 0.5)
			pB = (pA + offset) % aCfg.nC;
		else  //even offset, we subtract
			pB = (pA - offset+ aCfg.nC) % aCfg.nC;
		
	  //swap the values (we'll swap back if we have to)
		temp = tL[pA];
		tL[pA] = tL[pB];
		tL[pB] = temp;

		newDist = myMap->GetPathDistance(tL);
		if (newDist < tD) 
			tD = newDist; //don't need to do anything else
	    else {
		    //need to swap back
		    temp = tL[pA];
		    tL[pA] = tL[pB];
		    tL[pB] = temp;	
	    }
	}
		

}

