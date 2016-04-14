#include "PherMap.h"

PherMap::PherMap()
{
	//default constructor - not used
}

PherMap::PherMap(TPherCfg thePCfg, RandomMersenne *thePointer)
{
	
	pCfg = thePCfg; 
	randGen = thePointer;
	
	//initialise s array in both dimensions - using topdown triangle approach
	s.resize(pCfg.nC);
	for(std::vector<std::vector<TSegment> >::reverse_iterator pIt = s.rbegin(); pIt != s.rend(); ++pIt){
		pIt->resize(s.rend() - pIt);
	}
		
	std::cout << "PherMap::PherMap() finished" << std::endl;
}

void PherMap::SetCfg(TPherCfg thePCfg)
{
	pCfg = thePCfg;
}


void PherMap::ReducePher()
{
	for(int i = pCfg.nC -1; i >= 0; i--){
		for(int j = i -1; j >=0; j--){
			s[i][j].p = s[i][j].p * (1-pCfg.pD);
			if((pCfg.aLType == ACOMMAS) && (s[i][j].p < pCfg.pMin)) 
				      s[i][j].p = pCfg.pMin;
		}
	}	   
}


void PherMap::InitPher()
{

	for(int i = pCfg.nC - 1; i >= 0; i--){
		for(int j = i - 1; j >= 0;j--){
			s[i][j].p = pCfg.pI;
		}
	}
	
	if (pCfg.aLType == ACOMMAS){
		pCfg.pMax = pCfg.pI;
	    pCfg.pMin = 0;
	}
	
	
}


void PherMap::LayACSEdge(int cC, int nC)
{
	double addP = (1 - pCfg.pA) * GetSegP(cC, nC) + (pCfg.pA * pCfg.pI);
	SetSegP(cC, nC, addP);
}

void PherMap::LayPherPath(const std::vector<int> & path, int dist)
{
	double add;
	switch (pCfg.aLType) {
	case ACOAS: 
		for(int i = 0; i < pCfg.nC-1; i++){
			add = pCfg.pA / dist + GetSegP(i,i+1,path);
			SetSegP(i,i+1,path,add); //(pu*dist of seg)
		}
		//loop back to start - set s[path.0][path.end] pheromone
		add = pCfg.pA / dist + GetSegP(0, pCfg.nC -1,path);
		SetSegP(0,pCfg.nC-1,path, add);
		break;         //END AS CASE
	case ACOAMTS:
		for(int i = 0; i < pCfg.nC-1; i++){	
			add = pCfg.pA / dist + GetSegP(i,i+1,path);
			SetSegP(i,i+1,path,add); //(pu*dist of seg)
		}	
		//loop back to start - set s[path.0][path.end] pheromone
		add = pCfg.pA / dist + GetSegP(0, pCfg.nC -1,path);
		SetSegP(0,pCfg.nC-1,path, add);
		break;
	}
}

void PherMap::LayACSBestPath(const std::vector<int> & path, int dist)
{
    double add;
  	for(int i = 0; i< pCfg.nC-1; i++) //from path[n] to [n+1], lay pheromone
  	{	
  		add = pCfg.pA / dist + GetSegP(i,i+1,path);
  		SetSegP(i,i+1,path,add); //(pu*dist of seg + (1-pd*pher)
  	}
    //loop back to start - set s[path.0][path.end] pheromone
  	add = pCfg.pA / dist + GetSegP(0, pCfg.nC - 1,path);
  	SetSegP(0,pCfg.nC-1,path, add); //(pu*dist of seg + (1-pd*pher)	
}

void PherMap::LayMMBestPath(const std::vector<int> & path, int dist)
{
	static double pMaxFactor = (1-pow(MMPBEST, 1/pCfg.nC))/((pCfg.nC/2 -1)*pow(MMPBEST, 1/pCfg.nC));
	double add; 
	if (1/(pCfg.pD*dist) > pCfg.pMax)   //reassign
		pCfg.pMax = 1/(pCfg.pD*dist);
	
	pCfg.pMin = pCfg.pMax*pMaxFactor;

	for(int i = 0; i < pCfg.nC-1; i++){//from path[n] to [n+1], lay pheromone
		add = (pCfg.pA / dist) + GetSegP(i,i+1,path);
		if(add > pCfg.pMax) 
			add = pCfg.pMax;

		SetSegP(i,i+1,path,add); //(pu*dist of seg + (1-pd*pher)
	}
		
    //loop back to start - set s[path.0][path.end] pheromone
	add = pCfg.pA / dist + GetSegP(0,pCfg.nC -1,path);
	if(add > pCfg.pMax)
    	add = pCfg.pMax;
	
    SetSegP(0,pCfg.nC-1,path, add); //(pu*dist of seg + (1-pd*pher)	
}





int PherMap::WorldDistance(double x1, double y1, double x2, double y2)
{
	double q1, q2, q3, long1, long2, lat1, lat2;
		
	lat1  = PI * (int(x1)+(5.0 * (x1 - int(x1))/3))/180;
	long1 = PI * (int(y1)+(5.0 * (y1 - int(y1))/3))/180;
	lat2  = PI * (int(x2)+(5.0 * (x2 - int(x2))/3))/180;
	long2 = PI * (int(y2)+(5.0 * (y2 - int(y2))/3))/180;
	
	q1 = cos(long1 - long2);
	q2 = cos(lat1 - lat2);
	q3 = cos(lat1 + lat2);

	
	return int(RRR*acos(0.5*((1.0+q1)*q2-(1.0-q1)*q3))+1.0);	
			
}




int PherMap::GetSegD(int i,int j)
{
	if(i > j) 
		return s[i][j].d;
	else
		return s[j][i].d;	
}

int PherMap::GetSegD(int i,int j, const std::vector<int> & path)
{
	if(path[i] > path[j])
		return s[path[i]][path[j]].d;
	else
		return s[path[j]][path[i]].d;
}


double PherMap::GetSegP(int i,int j)
{
	if(i > j) 
		return s[i][j].p;
	else
		return s[j][i].p;	
}

double PherMap::GetSegP(int i,int j, const std::vector<int> & path)
{
	if(path[i] > path[j])
		return s[path[i]][path[j]].p;
	else
		return s[path[j]][path[i]].p;
}

double PherMap::GetSegA(int i, int j)
{
	if(i > j) 
		return s[i][j].a;
	else
		return s[j][i].a;	
}

double PherMap::GetSegA(int i, int j, const std::vector<int> & path)
{
	if(path[i] > path[j])
		return s[path[i]][path[j]].p;
	else
		return s[path[j]][path[i]].p;
}


void PherMap::SetSegP(int i, int j, const std::vector<int> & path, double p)
{
	if(path[i] > path[j]) 
		s[path[i]][path[j]].p = p;
	else
		s[path[j]][path[i]].p = p;	
}


void PherMap::SetSegP(int i, int j, double p)
{
	if(i > j) 
		s[i][j].p = p;
	else
	    s[j][i].p = p;	
	
}


int PherMap::GetPathDistance(const std::vector<int> & thePath)
{
	int length = 0;
	for(int i= 0; i< pCfg.nC -1; i++){
		length = length + GetSegD(i,i+1,thePath);	  
	}
	return length + GetSegD(pCfg.nC-1,0,thePath);	
}



void PherMap::CalcMapAttract()
{
	for(int i = pCfg.nC -1; i >= 0; i--){
		for(int j = i -1; j >=0; j--){
			s[i][j].a = pow(s[i][j].p,pCfg.alpha) / pow(s[i][j].d,pCfg.beta); //calculate relative attractiveness levels for each segment
		}
	}	 
}

//This function is passed a vector of strings corresponding to the cities in the map.txt
//don't need to worry about getting the number of cities or the map type, that is determined by Main
void PherMap::LoadCities(const std::vector<std::string> & myList) 
{
	std::vector<double> xCoord, yCoord;
	std::istringstream convertor;
	std::string xWrd, yWrd;
	double x1, y1, x2, y2;
	
	
	xCoord.reserve(pCfg.nC);
	yCoord.reserve(pCfg.nC);
	
	//load coordinates into an array in order
	
	for(int i = 0; i< pCfg.nC; i++){
		xWrd = myList[i].substr(0,myList[i].find(" ")); //x word
		yWrd = myList[i].substr(myList[i].find(" ")+1,myList[i].length()); //y word		

		xCoord.push_back(StrToDouble(xWrd));
		yCoord.push_back(StrToDouble(yWrd));
		
	}
	for(int i = pCfg.nC -1; i >= 0; i--){
		for(int j = i-1; j >=0; j--){

			x1 = xCoord[i]; y1 = yCoord[i];
			x2 = xCoord[j]; y2 = yCoord[j];
			switch(pCfg.mType) {
			case CARTESIAN:
				s[i][j].d = int(sqrt(pow(x1 - x2,2) + pow(y1-y2,2))+0.5);
				break;
			case LATITUDE: 
				s[i][j].d = WorldDistance(x1,y1,x2,y2);
				break;
			}
		}
	}
	std::cout << "PherMap::LoadCities() finished" << std::endl;
}





