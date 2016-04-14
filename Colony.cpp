#include "Colony.h"

Colony::Colony(int RandSeed, TBatchCfg theBCfg, TLocalSearchCfg theLSCfg, TPherCfg thePCfg, TAntCfg theACfg)
{
	randGen = RandomMersenne(RandSeed);
	bCfg = theBCfg;
	lSCfg = theLSCfg;
	pCfg = thePCfg;
	aCfg = theACfg;

	
	bPath.resize(pCfg.nC);
	bDist = INT_MAX; 
	//stuff for antennation
	
    aCL.resize(pCfg.nC); //1st dimension corresponds to each city, 2nd correspond to the ant IDs
    for(std::vector<std::vector<int> >::iterator antIt = aCL.begin(); antIt != aCL.end(); ++antIt){
    	antIt->reserve(bCfg.nAE);
    }
    
	std::cout << "Colony::Colony() finished" << std::endl;
}


Colony::~Colony()
{
}

void Colony::SetFile(std::string fileName)
{
	if(saveFile.is_open()){
		saveFile.flush();
		saveFile.close();
	}
			
	saveFile.open(fileName.c_str(),std::ios::in|std::ios::app);
	std::cout << "Colony::SetFile() finished" << std::endl;
	
}

void Colony::SetRandGen(int RandSeed)
{
	randGen.RandomInit(RandSeed);
}

void Colony::InitMap(const std::vector<std::string> & myList)
{
	//first need to initialise the map
	myMap = PherMap(pCfg, &randGen);
	
	myMap.LoadCities(myList);
	std::cout << "Colony::InitMap() finished" << std::endl;
}


bool Colony::OneTour()
{
	aMeet = 0;
	
	myMap.CalcMapAttract(); //precalculate attractiveness
	
	
	for(std::vector<Ant>::iterator antIt = aL.begin(); antIt != aL.end(); ++antIt){//set initial city
		antIt->SetStartCity(randGen.IRandomX(0, pCfg.nC-1));	
	}
	//move each ant until all cities have been visited	

	for(int i = 0; i < pCfg.nC -1; i++){
		for(std::vector<Ant>::iterator antIt = aL.begin(); antIt != aL.end(); ++antIt){//move each ant
			//add ant id to the appropriate city index

			aCL[antIt->Move()].push_back(antIt-aL.begin());
		}
	//	aMeet = Antennation()+ aMeet;
	}
	
	for(std::vector<Ant>::iterator antIt = aL.begin(); antIt != aL.end(); ++antIt){
	    antIt->MoveFinal();
	}

	//lets do our local search technique
	for(std::vector<Ant>::iterator antIt = aL.begin(); antIt != aL.end(); ++antIt)
	{
		antIt->TwoOpt();
	}

	//update the pheromone map
	return UpdatePheromone();

}

int Colony::Antennation()
{
	int total = 0;
	static std::vector<std::vector<int> > aT; //antTree. Reserve the size so we only have to declare it once
	aT.reserve(pCfg.nC);
	for(std::vector<std::vector<int> >::iterator tIt = aT.begin(); tIt != aT.end(); ++tIt){
		tIt->reserve(pCfg.nC);
	}
	
	//first go through each city
	for(std::vector<std::vector<int> >::iterator cityIt = aCL.begin(); cityIt != aCL.end(); ++cityIt){
		//check if there is more than 1 ant on that city
		if(cityIt->size() > 1){ 
			//first find out how many meetings
			total +=  cityIt->size() * (cityIt->size()-1) / 2; 
			std::vector<std::vector<int> > cT; //combined tree
			cT.resize(pCfg.nC);
			for(std::vector<std::vector<int> >::iterator tIt = cT.begin(); tIt != cT.end(); ++tIt){
				tIt->resize(pCfg.nC);
			}
			
			//then go through each ant - add their antennation tree to the combined tree
			for(std::vector<int>::iterator antIt = cityIt->begin(); antIt != cityIt->end(); ++antIt){
				aT = aL[*antIt].GetTree();
				for(int i = 0; i < pCfg.nC; i++){
					for(int j = 0; j < pCfg.nC; j++){
						if((i != j) && (aT[i][j] > 0)){
							cT[i][j]++;
						}
					}
				}
			}
				
				
			for(std::vector<int>::iterator antIt = cityIt->begin(); antIt != cityIt->end(); ++antIt){
				aL[*antIt].Consolidate(cT, cityIt->size());
			}
		}	
		
		//clear that city in the vector
		while(cityIt->size() > 0){
			cityIt->pop_back();
		}
		
	}

	return total;
}

//returns whether a new best path has been found
bool Colony::UpdatePheromone()
{
	static std::vector<int> iPath;
	int iDist = INT_MAX;
	bool iterGlobal = false, newDist = false;
	
	iPath.resize(pCfg.nC);
	
	myMap.ReducePher();
	
	//if so, we need to determine whether to use global or iterative best path when updating
	//boolean value is IterGlobal, true if iteration best, false if global best
	if((pCfg.aLType == ACOMMAS) && (randGen.Random() < bCfg.igB)){
		iterGlobal = true;
	}
	
	//quick check to see if we have a new best path
	//Strictly only required for MM and ACS
	//used in mapping best path for all algorithms
	for(std::vector<Ant>::iterator antIt = aL.begin(); antIt != aL.end(); ++antIt){
		//ant has a better distance than the global best
		if(antIt->GetDistance() < bDist) {
			newDist = true;
			bPath = antIt->GetPath();
			bDist = antIt->GetDistance();	
						
		}
		if((antIt->GetDistance() < iDist) && (!iterGlobal)){
			iDist = antIt->GetDistance();
			iPath = antIt->GetPath();
		}
		if(pCfg.aLType != ACOMMAS){
			if(pCfg.aLType == ACOACS)
				myMap.LayACSBestPath(bPath, bDist);
				
			myMap.LayPherPath(antIt->GetPath(), antIt->GetDistance());
		}
	}
	
	if(pCfg.aLType == ACOMMAS){
		if(iterGlobal)
			myMap.LayMMBestPath(iPath, iDist);
		else
			myMap.LayMMBestPath(bPath, bDist);	 
	}	
	
	return newDist;
}

void Colony::RunBatch()
{
	int aFCount = 0, aDCount = 0;

	for(std::vector<bool>::iterator aFIt = bCfg.aFType.begin(); aFIt != bCfg.aFType.end(); ++aFIt){
		if(*aFIt) 
			aFCount++;
	}
	
	for(std::vector<bool>::iterator aDIt = bCfg.aFDir.begin(); aDIt != bCfg.aFDir.end(); ++aDIt){
		if(*aDIt) 
			aDCount++;
	}
		
	int totalIter = bCfg.iter * aFCount * aDCount
		      * floor((bCfg.nAE+bCfg.nAS*1.00001-bCfg.nAB)/bCfg.nAS)
		      * floor((bCfg.pAE+bCfg.pAS*1.00001-bCfg.pAB)/bCfg.pAS)
		      * floor((bCfg.pIE+bCfg.pIS*1.00001-bCfg.pIB)/bCfg.pIS)
		      * floor((bCfg.pDE+bCfg.pDS*1.00001-bCfg.pDB)/bCfg.pDS)
		      * floor((bCfg.aFE+bCfg.aFS*1.00001-bCfg.aFB)/bCfg.aFS);
	
	switch(bCfg.aLType){
	case ACOACS:
		totalIter *= floor((bCfg.gPE+bCfg.gPS*1.00001-bCfg.gPB)/bCfg.gPS);
		ACSBatch(totalIter);
		break;
	case ACOAS:
		ASBatch(totalIter);
		break;
	case ACOAMTS:
		totalIter *= floor((bCfg.mAE+bCfg.mAS*1.00001-bCfg.mAB)/bCfg.mAS);
		AMTSBatch(totalIter);
		break;
	case ACOMMAS:
		MMBatch(totalIter);
		break;
	default: break;
	}

}


void Colony::ACSBatch(int totalIter)
{
	
	std::cout << "Colony::ACSBatch() begin" << std::endl;
	
	int curProgress, iterL, curGen, gen;
	double sigDist, sigGen;
	
	int antL; //batch iteration counters - (parameter)
	double pAL, pDL, aFL, pIL, gPL; //batch iteration counters - (parameter)L
	
	double totalAMeetCount, runAMeetCount, avgGen, avgDist; //values that are stored (averages)
	std::vector<int> distArray, genArray; //for calculating std deviation
	
	distArray.reserve(bCfg.iter);
	genArray.reserve(bCfg.iter);
	//first things first, instantiate ant array with initial size and set the various ants
	aL.reserve(bCfg.nAE); //reserve the maximum room needed
	aL.resize(bCfg.nAB); // create the number of ants needed
	//go through each ant and set them up
	for(std::vector<Ant>::iterator antIter = aL.begin(); antIter != aL.end(); ++antIter){
		antIter->SetMap(&myMap);
		antIter->SetRandGen(&randGen);
		antIter->Init(aCfg, lSCfg, pCfg.alpha, pCfg.beta, antIter - aL.begin());
	}
	
	std::cout << "Colony::ACSBatch() - iter " << bCfg.iter << " gen" << bCfg.gen <<  std::endl;
	
//	saveFile << "BatchFile test, number of cities =" << pCfg.nC << std::endl;
//	saveFile << "ACS Algorithm - LSType " << GetLSType(lSCfg.lSType) << " Times = " << lSCfg.lSTimes << std::endl;
//	saveFile << "Format:" << std::endl;
//	saveFile << "meanGen sigGen meanDist sigDist DP AP IP NA AF GP ANTMEET" << std::endl;
	
	curProgress = 0;
	
	// 8 loops- AFTYPE, DIRTYPE,  number of ants, add P, decay P, init P, AntF, Iterations	  
	//Start off with AFTYPE - using an iterator to go through the types
	for(std::vector<bool>::iterator aFIter = bCfg.aFType.begin(); aFIter!=bCfg.aFType.end(); ++aFIter){
	if(*aFIter){ //if the Antennation type is true, do a batch run
				
		aCfg.aFType = aFIter - bCfg.aFType.begin();
		saveFile << "AntType = " << GetAntType(aCfg.aFType) << std::endl;
	
		//Now AFDIR - using an iterator to go through the types	
		for(std::vector<bool>::iterator aDIter = bCfg.aFDir.begin(); aDIter!=bCfg.aFDir.end(); ++aDIter){
		if(*aDIter){ //if the Antennation Direction is true
			aCfg.aFDir = aDIter - bCfg.aFDir.begin();
			saveFile << "AntDir = " << GetAntDir(aCfg.aFDir) << std::endl;
					
			// now the number of ants
			antL = bCfg.nAB;
	//		RandGen.RandomInit(RANDSEED); //reset random seed
					
			while(antL <= bCfg.nAE){//ANT LOOP
				while(aL.size() < antL){
					aL.push_back(Ant()); //add an ant
					//do the normal initialisation stuff
					aL.back().SetMap(&myMap);
					aL.back().SetRandGen(&randGen);
					aL.back().Init(aCfg, lSCfg, pCfg.alpha, pCfg.beta, aL.size());
				}
				//REST OF LOOPS DON'T REQUIRE ANY CHANGE TO THE PROBLEM, JUST THE SETTINGS
				gPL = bCfg.gPB;
				while(gPL <= bCfg.gPE) {//GREED PROB LOOP
					aCfg.gP = gPL;
					pAL = bCfg.pAB; 
					while(pAL <= bCfg.pAE) { //PHER ADD LOOP
						pCfg.pA = pAL;
						pDL = bCfg.pDB;
						while(pDL <= bCfg.pDE) { //PHER DECAY LOOP
							pCfg.pD = pDL;
							pIL = bCfg.pIB;
							while(pIL <= bCfg.pIE) {//PHER INIT LOOP
								pCfg.pI = pIL;
								aFL = bCfg.aFB;
								while(aFL <= bCfg.aFE) { //ANTENNATION LOOP
									aCfg.aF = aFL;
/*-----------------------------------------------------------------------------
-----------------------Starting Batch Section----------------------------------
-----------------------------------------------------------------------------*/								

	iterL = avgGen = avgDist = totalAMeetCount = 0;

	
	
	///STARTING AN ITERATION
	
	//TIME TO SET THE CHANGED CFG VALUES
	myMap.SetCfg(pCfg);
	for(std::vector<Ant>::iterator antIter = aL.begin(); antIter != aL.end(); ++antIter){
		antIter->ChangeCfg(aCfg);
	}
	
	
	while(iterL < bCfg.iter) {
		curGen = runAMeetCount = gen = 0;
		bDist = INT_MAX;
		myMap.InitPher();
		while(curGen < bCfg.gen) {
			//if the tour does return a better path, reset curGen to zero else inc
			if(OneTour()){
				curGen = 0; 
			}
			else {
				curGen++;
			}
			gen++;
		    runAMeetCount += aMeet;
		}
		totalAMeetCount += runAMeetCount/gen; //average meetings per generation
		
		for(std::vector<int>::iterator pIt = bPath.begin(); pIt != bPath.end();++pIt){
			std::cout << "["<<*pIt << "]";
			saveFile << *pIt << " ";
		}
		std::cout << std::endl;
		saveFile << std::endl;
		avgGen += gen - curGen;
		avgDist += bDist;
		distArray.push_back(bDist);
		genArray.push_back(gen-curGen);
		
		std::cout << "Iter Done - gen " << gen-curGen << " dist " << bDist <<  std::endl;
		std::cout << "avgGen " << avgGen << " avgDist " << avgDist <<  std::endl;
		saveFile << "dist = " << bDist << std::endl;
		 
		iterL++;
	}

	
	///FINISHING AN ITERATION
	avgGen /= bCfg.iter;
	avgDist /= bCfg.iter;
	totalAMeetCount /= bCfg.iter;
	
	curProgress += bCfg.iter;
	std::cout << "Batch percentage complete = " << 100*curProgress/totalIter << "%" << std::endl;
	
	
	
	// TIME TO CALCULATE STANDARD DEVIATION
	sigGen = sigDist = 0;
	
	//This is a bad check, I guess somehow they could get out of synch. but fuck it
	while(genArray.size() > 0){
		sigDist += pow(avgDist - distArray.back(),2);
		distArray.pop_back();		
		
		sigGen += pow(avgGen - genArray.back(),2);
		genArray.pop_back();
	}
	
	sigDist /= bCfg.iter;
	sigGen /= bCfg.iter;
	
	//saveFile << "meanGen sigGen meanDist sigDist DP AP IP NA AF GP ANTMEET" << std::endl;
	//time to write out to the file
//	saveFile << avgGen << "  " << sigGen << " " << avgDist << " " << sigDist << " "
//		<< pDL << " " << pAL << " " << pIL << " " << antL << " " << aFL << " " 
//		<< gPL << " " << totalAMeetCount << std::endl;
											
		
																											
/*-------------------------------------------------------------------------------
-----------------------Finished batch section ----------------------------------
-------------------------------------------------------------------------------*/

									aFL += bCfg.aFS;
								} //ENDING ANTENNATION LOOP
								pIL += pIL + bCfg.pIS;
							}  //ENDING PHER INITIAL LOOP
							pDL += bCfg.pDS;
						} //ENDING PHER DECAY LOOP
						pAL += bCfg.pAS;	
					} //ENDING PHER ADD LOOP
					gPL += bCfg.gPS;
				} //ENDING GREED PROB LOOP
				antL += bCfg.nAS;
				saveFile << std::endl;
			} //ENDING ANT LOOP
		} //ENDING AF DIRECTION 'TRUE' LOOP
		} //ENDING AF DIRECTION VECTOR LOOP
	} //ENDING AF TYPE 'TRUE' LOOP	
	} //ENDING AF TYPE VECTOR LOOP
	saveFile.close();
}



void Colony::ASBatch(int totalIter)
{
	
	std::cout << "Colony::ASBatch() begin" << std::endl;
	
	int curProgress, iterL, curGen, gen;
	double sigDist, sigGen;
	
	int antL; //batch iteration counters - (parameter)
	double pAL, pDL, aFL, pIL; //batch iteration counters - (parameter)L
	
	double totalAMeetCount, runAMeetCount, avgGen, avgDist; //values that are stored (averages)
	std::vector<int> distArray, genArray; //for calculating std deviation
	
	distArray.reserve(bCfg.iter);
	genArray.reserve(bCfg.iter);
	//first things first, instantiate ant array with initial size and set the various ants
	aL.reserve(bCfg.nAE); //reserve the maximum room needed
	aL.resize(bCfg.nAB); // create the number of ants needed
	//go through each ant and set them up
	for(std::vector<Ant>::iterator antIter = aL.begin(); antIter != aL.end(); ++antIter){
		antIter->SetMap(&myMap);
		antIter->SetRandGen(&randGen);
		antIter->Init(aCfg, lSCfg, pCfg.alpha, pCfg.beta, antIter - aL.begin());
	}
	
	
	saveFile << "BatchFile test, number of cities =" << pCfg.nC << std::endl;
	saveFile << "AS Algorithm - LSType " << GetLSType(lSCfg.lSType) << " Times = " << lSCfg.lSTimes << std::endl;
	saveFile << "Format:" << std::endl;
	saveFile << "meanGen sigGen meanDist sigDist DP AP IP NA AF ANTMEET" << std::endl;
	
	curProgress = 0;
	
	// 8 loops- AFTYPE, DIRTYPE,  number of ants, add P, decay P, init P, AntF, Iterations	  
	//Start off with AFTYPE - using an iterator to go through the types
	for(std::vector<bool>::iterator aFIter = bCfg.aFType.begin(); aFIter!=bCfg.aFType.end(); ++aFIter){
	if(*aFIter){ //if the Antennation type is true, do a batch run
				
		aCfg.aFType = aFIter - bCfg.aFType.begin();
		saveFile << "AntType = " << GetAntType(aCfg.aFType) << std::endl;
	
		//Now AFDIR - using an iterator to go through the types	
		for(std::vector<bool>::iterator aDIter = bCfg.aFDir.begin(); aDIter!=bCfg.aFDir.end(); ++aDIter){
		if(*aDIter){ //if the Antennation Direction is true
			aCfg.aFDir = aDIter - bCfg.aFDir.begin();
			saveFile << "AntDir = " << GetAntDir(aCfg.aFDir) << std::endl;
					
			// now the number of ants
			antL = bCfg.nAB;
	//		RandGen.RandomInit(RANDSEED); //reset random seed
					
			while(antL <= bCfg.nAE){//ANT LOOP
				while(aL.size() < antL){
					aL.push_back(Ant()); //add an ant
					//do the normal initialisation stuff
					aL.back().SetMap(&myMap);
					aL.back().SetRandGen(&randGen);
					aL.back().Init(aCfg, lSCfg, pCfg.alpha, pCfg.beta, aL.size());
				}
				//REST OF LOOPS DON'T REQUIRE ANY CHANGE TO THE PROBLEM, JUST THE SETTINGS
				pAL = bCfg.pAB; 
				while(pAL <= bCfg.pAE) { //PHER ADD LOOP
					pCfg.pA = pAL;
					pDL = bCfg.pDB;
					while(pDL <= bCfg.pDE) { //PHER DECAY LOOP
						pCfg.pD = pDL;
						pIL = bCfg.pIB;
						while(pIL <= bCfg.pIE) {//PHER INIT LOOP
							pCfg.pI = pIL;
							aFL = bCfg.aFB;
							while(aFL <= bCfg.aFE) { //ANTENNATION LOOP
								aCfg.aF = aFL;
/*-----------------------------------------------------------------------------
-----------------------Starting Batch Section----------------------------------
-----------------------------------------------------------------------------*/								

	iterL = 0;
	avgGen = 0;
	avgDist = 0;
	totalAMeetCount = 0;

	
	
	///STARTING AN ITERATION
	
	//TIME TO SET THE CHANGED CFG VALUES
	myMap.SetCfg(pCfg);
	for(std::vector<Ant>::iterator antIter = aL.begin(); antIter != aL.end(); ++antIter){
		antIter->ChangeCfg(aCfg);
	}
	
	
	while(iterL < bCfg.iter) {
		curGen = runAMeetCount = gen = 0;
		bDist = INT_MAX;
		myMap.InitPher();
		while(curGen < bCfg.gen) {
			//if the tour does return a better path, reset curGen to zero else inc
			if(OneTour()){
				curGen = 0; 
			}
			else {
				curGen++;
			}
			gen++;
		    runAMeetCount += aMeet;
		}
		totalAMeetCount += runAMeetCount/gen; //average meetings per generation
		
		avgGen += gen - curGen;
		avgDist += bDist;
		distArray.push_back(bDist);
		genArray.push_back(gen-curGen);
		 
		iterL++;
		std::cout << "Iter Done - gen " << gen-curGen << " dist " << bDist <<  std::endl;
		std::cout << "avgGen " << avgGen << " avgDist " << avgDist <<  std::endl;
	}

	
	///FINISHING AN ITERATION
	avgGen /= bCfg.iter;
	avgDist /= bCfg.iter;
	totalAMeetCount /= bCfg.iter;
	
	curProgress += bCfg.iter;
	std::cout << "Batch percentage complete = " << 100*curProgress/totalIter << "%" << std::endl;
	
	
	
	// TIME TO CALCULATE STANDARD DEVIATION
	sigGen = sigDist = 0;

	//This is a bad check, I guess somehow they could get out of synch. but fuck it
	while(genArray.size() > 0){

		sigDist += pow(avgDist - distArray.back(),2);
		distArray.pop_back();		

	
		sigGen += pow(avgGen - genArray.back(),2);
		genArray.pop_back();
	}

	
	sigDist /= bCfg.iter;
	sigGen /= bCfg.iter;
	
	
	//saveFile << "meanGen sigGen meanDist sigDist DP AP IP NA AF ANTMEET" << std::endl;
	//time to write out to the file
	saveFile << avgGen << "  " << sqrt(sigGen) << " " << avgDist << " " << sqrt(sigDist) << " "
		<< pDL << " " << pAL << " " << pIL << " " << antL << " " << aFL << " " 
		<< totalAMeetCount << std::endl;
											
		
																											
/*-------------------------------------------------------------------------------
-----------------------Finished batch section ----------------------------------
-------------------------------------------------------------------------------*/
								aFL += bCfg.aFS;
							} //ENDING ANTENNATION LOOP
							pIL += pIL + bCfg.pIS;
						}  //ENDING PHER INITIAL LOOP
						pDL += bCfg.pDS;
					} //ENDING PHER DECAY LOOP
					pAL += bCfg.pAS;	
				} //ENDING PHER ADD LOOP
			antL += bCfg.nAS;
			saveFile << std::endl;
			} //ENDING ANT LOOP
		} //ENDING AF DIRECTION 'TRUE' LOOP
		} //ENDING AF DIRECTION VECTOR LOOP
	} //ENDING AF TYPE 'TRUE' LOOP	
	} //ENDING AF TYPE VECTOR LOOP
	saveFile.close();

}
  
void Colony::AMTSBatch(int totalIter)
{
	
	std::cout << "Colony::AMTSBatch() begin" << std::endl;
	
	int curProgress, iterL, curGen, gen;
	double sigDist, sigGen;
	
	int antL; //batch iteration counters - (parameter)
	double pAL, pDL, aFL, pIL, mAL; //batch iteration counters - (parameter)L
	
	double totalAMeetCount, runAMeetCount, avgGen, avgDist; //values that are stored (averages)
	std::vector<int> distArray, genArray; //for calculating std deviation
	
	distArray.reserve(bCfg.iter);
	genArray.reserve(bCfg.iter);
	//first things first, instantiate ant array with initial size and set the various ants
	aL.reserve(bCfg.nAE); //reserve the maximum room needed
	aL.resize(bCfg.nAB); // create the number of ants needed
	//go through each ant and set them up
	for(std::vector<Ant>::iterator antIter = aL.begin(); antIter != aL.end(); ++antIter){
		antIter->SetMap(&myMap);
		antIter->SetRandGen(&randGen);
		antIter->Init(aCfg, lSCfg, pCfg.alpha, pCfg.beta, antIter - aL.begin());
	}
	
	saveFile << "BatchFile test, number of cities =" << pCfg.nC << std::endl;
	saveFile << "AMTS Algorithm - LSType " << GetLSType(lSCfg.lSType) << " Times = " << lSCfg.lSTimes << std::endl;
	saveFile << "Format:" << std::endl;
	saveFile << "meanGen sigGen meanDist sigDist DP AP IP NA AF ANTMEET" << std::endl;
	
	curProgress = 0;
	
	// 8 loops- AFTYPE, DIRTYPE,  number of ants, add P, decay P, init P, AntF, Iterations	  
	//Start off with AFTYPE - using an iterator to go through the types
	for(std::vector<bool>::iterator aFIter = bCfg.aFType.begin(); aFIter!=bCfg.aFType.end(); ++aFIter){
	if(*aFIter){ //if the Antennation type is true, do a batch run
				
		aCfg.aFType = aFIter - bCfg.aFType.begin();
		saveFile << "AntType = " << GetAntType(aCfg.aFType) << std::endl;
	
		//Now AFDIR - using an iterator to go through the types	
		for(std::vector<bool>::iterator aDIter = bCfg.aFDir.begin(); aDIter!=bCfg.aFDir.end(); ++aDIter){
		if(*aDIter){ //if the Antennation Direction is true
			aCfg.aFDir = aDIter - bCfg.aFDir.begin();
			saveFile << "AntDir = " << GetAntDir(aCfg.aFDir) << std::endl;
					
			// now the number of ants
			antL = bCfg.nAB;
	//		RandGen.RandomInit(RANDSEED); //reset random seed
					
			while(antL <= bCfg.nAE){//ANT LOOP
				while(aL.size() < antL){
					aL.push_back(Ant()); //add an ant
					//do the normal initialisation stuff
					aL.back().SetMap(&myMap);
					aL.back().SetRandGen(&randGen);
					aL.back().Init(aCfg, lSCfg, pCfg.alpha, pCfg.beta, aL.size());
				}
				//REST OF LOOPS DON'T REQUIRE ANY CHANGE TO THE PROBLEM, JUST THE SETTINGS
				mAL = bCfg.mAB; 
				while(mAL <= bCfg.mAE) { //MAX AGE LOOP
					aCfg.mA = mAL;
					pAL = bCfg.pAB; 
					while(pAL <= bCfg.pAE) { //PHER ADD LOOP
						pCfg.pA = pAL;
						pDL = bCfg.pDB;
						while(pDL <= bCfg.pDE) { //PHER DECAY LOOP
							pCfg.pD = pDL;
							pIL = bCfg.pIB;
							while(pIL <= bCfg.pIE) {//PHER INIT LOOP
								pCfg.pI = pIL;
								aFL = bCfg.aFB;
								while(aFL <= bCfg.aFE) { //ANTENNATION LOOP
									aCfg.aF = aFL;
/*-----------------------------------------------------------------------------
-----------------------Starting Batch Section----------------------------------
-----------------------------------------------------------------------------*/								

	iterL = avgGen = avgDist = totalAMeetCount = 0;

	
	
	///STARTING AN ITERATION
	
	//TIME TO SET THE CHANGED CFG VALUES
	myMap.SetCfg(pCfg);
	for(std::vector<Ant>::iterator antIter = aL.begin(); antIter != aL.end(); ++antIter){
		antIter->ChangeCfg(aCfg);
	}
	
	
	while(iterL < bCfg.iter) {
		curGen = runAMeetCount = gen = 0;
		bDist = INT_MAX;
		myMap.InitPher();
		while(curGen < bCfg.gen) {
			//if the tour does return a better path, reset curGen to zero else inc
			if(OneTour()){
				curGen = 0; 
			}
			else {
				curGen++;
			}
			gen++;
		    runAMeetCount += aMeet;
		}
		totalAMeetCount += runAMeetCount/gen; //average meetings per generation
		
		avgGen += gen - curGen;
		avgDist += bDist;
		distArray.push_back(bDist);
		genArray.push_back(gen-curGen);
		
		std::cout << "Iter Done - gen " << gen-curGen << " dist " << bDist <<  std::endl;
		std::cout << "avgGen " << avgGen << " avgDist " << avgDist <<  std::endl;
		 
		iterL++;
	}

	
	///FINISHING AN ITERATION
	avgGen /= bCfg.iter;
	avgDist /= bCfg.iter;
	totalAMeetCount /= bCfg.iter;
	
	curProgress += bCfg.iter;
	std::cout << "Batch percentage complete = " << 100*curProgress/totalIter << "%" << std::endl;
	
	
	
	// TIME TO CALCULATE STANDARD DEVIATION
	sigGen = sigDist = 0;
	
	//This is a bad check, I guess somehow they could get out of synch. but fuck it
	while(genArray.size() > 0){
		sigDist += pow(avgDist - distArray.back(),2);
		distArray.pop_back();		
		
		sigGen += pow(avgGen - genArray.back(),2);
		genArray.pop_back();
	}
	
	sigDist /= bCfg.iter;
	sigGen /= bCfg.iter;
	
	//saveFile << "meanGen sigGen meanDist sigDist DP AP IP NA AF ANTMEET" << std::endl;
	//time to write out to the file
	saveFile << avgGen << "  " << sigGen << " " << avgDist << " " << sigDist << " "
		<< pDL << " " << pAL << " " << pIL << " " << antL << " " << aFL << " " 
		<< totalAMeetCount << std::endl;
											
		
																											
/*-------------------------------------------------------------------------------
-----------------------Finished batch section ----------------------------------
-------------------------------------------------------------------------------*/
									aFL += bCfg.aFS;
								} //ENDING ANTENNATION LOOP
								pIL += pIL + bCfg.pIS;
							}  //ENDING PHER INITIAL LOOP
							pDL += bCfg.pDS;
						} //ENDING PHER DECAY LOOP
						pAL += bCfg.pAS;	
					} //ENDING PHER ADD LOOP
					mAL += bCfg.mAS;	
				} //ENDING MAX AGE LOOP
			antL += bCfg.nAS;
			saveFile << std::endl;
			} //ENDING ANT LOOP
		} //ENDING AF DIRECTION 'TRUE' LOOP
		} //ENDING AF DIRECTION VECTOR LOOP
	} //ENDING AF TYPE 'TRUE' LOOP	
	} //ENDING AF TYPE VECTOR LOOP
	saveFile.close();

}
  
void Colony::MMBatch(int totalIter)
{
	
	std::cout << "Colony::MMBatch() begin" << std::endl;
	
	int curProgress, iterL, curGen, gen;
	double sigDist, sigGen;
	
	int antL; //batch iteration counters - (parameter)
	double pAL, pDL, aFL, pIL, mAL; //batch iteration counters - (parameter)L
	
	double totalAMeetCount, runAMeetCount, avgGen, avgDist; //values that are stored (averages)
	std::vector<int> distArray, genArray; //for calculating std deviation
	
	distArray.reserve(bCfg.iter);
	genArray.reserve(bCfg.iter);
	//first things first, instantiate ant array with initial size and set the various ants
	aL.reserve(bCfg.nAE); //reserve the maximum room needed
	aL.resize(bCfg.nAB); // create the number of ants needed
	//go through each ant and set them up
	for(std::vector<Ant>::iterator antIter = aL.begin(); antIter != aL.end(); ++antIter){
		antIter->SetMap(&myMap);
		antIter->SetRandGen(&randGen);
		antIter->Init(aCfg, lSCfg, pCfg.alpha, pCfg.beta, antIter - aL.begin());
	}
	
	std::cout << "Colony::ACSBatch() - iter " << bCfg.iter << " gen" << bCfg.gen <<  std::endl;
	
	saveFile << "BatchFile test, number of cities =" << pCfg.nC << std::endl;
	saveFile << "MM Algorithm - LSType " << GetLSType(lSCfg.lSType) << " Times = " << lSCfg.lSTimes << std::endl;
	saveFile << "Format:" << std::endl;
	saveFile << "meanGen sigGen meanDist sigDist DP AP IP NA AF ANTMEET" << std::endl;
	
	curProgress = 0;
	
	// 8 loops- AFTYPE, DIRTYPE,  number of ants, add P, decay P, init P, AntF, Iterations	  
	//Start off with AFTYPE - using an iterator to go through the types
	for(std::vector<bool>::iterator aFIter = bCfg.aFType.begin(); aFIter!=bCfg.aFType.end(); ++aFIter){
	if(*aFIter){ //if the Antennation type is true, do a batch run
				
		aCfg.aFType = aFIter - bCfg.aFType.begin();
		saveFile << "AntType = " << GetAntType(aCfg.aFType) << std::endl;
	
		//Now AFDIR - using an iterator to go through the types	
		for(std::vector<bool>::iterator aDIter = bCfg.aFDir.begin(); aDIter!=bCfg.aFDir.end(); ++aDIter){
		if(*aDIter){ //if the Antennation Direction is true
			aCfg.aFDir = aDIter - bCfg.aFDir.begin();
			saveFile << "AntDir = " << GetAntDir(aCfg.aFDir) << std::endl;
					
			// now the number of ants
			antL = bCfg.nAB;
	//		RandGen.RandomInit(RANDSEED); //reset random seed
					
			while(antL <= bCfg.nAE){//ANT LOOP
				while(aL.size() < antL){
					aL.push_back(Ant()); //add an ant
					//do the normal initialisation stuff
					aL.back().SetMap(&myMap);
					aL.back().SetRandGen(&randGen);
					aL.back().Init(aCfg, lSCfg, pCfg.alpha, pCfg.beta, aL.size());
				}
				//REST OF LOOPS DON'T REQUIRE ANY CHANGE TO THE PROBLEM, JUST THE SETTINGS
				mAL = bCfg.mAB; 
				while(mAL <= bCfg.mAE) { //MAX AGE LOOP
					aCfg.mA = mAL;
					pAL = bCfg.pAB; 
					while(pAL <= bCfg.pAE) { //PHER ADD LOOP
						pCfg.pA = pAL;
						pDL = bCfg.pDB;
						while(pDL <= bCfg.pDE) { //PHER DECAY LOOP
							pCfg.pD = pDL;
							pIL = bCfg.pIB;
							while(pIL <= bCfg.pIE) {//PHER INIT LOOP
								pCfg.pI = pIL;
								aFL = bCfg.aFB;
								while(aFL <= bCfg.aFE) { //ANTENNATION LOOP
									aCfg.aF = aFL;
/*-----------------------------------------------------------------------------
-----------------------Starting Batch Section----------------------------------
-----------------------------------------------------------------------------*/								

	iterL = avgGen = avgDist = totalAMeetCount = 0;

	
	
	///STARTING AN ITERATION
	
	//TIME TO SET THE CHANGED CFG VALUES
	myMap.SetCfg(pCfg);
	for(std::vector<Ant>::iterator antIter = aL.begin(); antIter != aL.end(); ++antIter){
		antIter->ChangeCfg(aCfg);
	}
	
	
	while(iterL < bCfg.iter) {
		curGen = runAMeetCount = gen = 0;
		bDist = INT_MAX;
		myMap.InitPher();
		while(curGen < bCfg.gen) {
			//if the tour does return a better path, reset curGen to zero else inc
			if(OneTour()){
				curGen = 0; 
			}
			else {
				curGen++;
			}
			gen++;
		    runAMeetCount += aMeet;
		}
		totalAMeetCount += runAMeetCount/gen; //average meetings per generation
		
		avgGen += gen - curGen;
		avgDist += bDist;
		distArray.push_back(bDist);
		genArray.push_back(gen-curGen);
		 
		iterL++;
	}

	
	///FINISHING AN ITERATION
	avgGen /= bCfg.iter;
	avgDist /= bCfg.iter;
	totalAMeetCount /= bCfg.iter;
	
	curProgress += bCfg.iter;
	std::cout << "Batch percentage complete = " << 100*curProgress/totalIter << "%" << std::endl;
	
	// TIME TO CALCULATE STANDARD DEVIATION
	sigGen = sigDist = 0;
	
	//This is a bad check, I guess somehow they could get out of synch. but fuck it
	while(genArray.size() > 0){
		sigDist += pow(avgDist - distArray.back(),2);
		distArray.pop_back();		
		
		sigGen += pow(avgGen - genArray.back(),2);
		genArray.pop_back();
	}

	
	sigDist /= bCfg.iter;
	sigGen /= bCfg.iter;
	
	//saveFile << "meanGen sigGen meanDist sigDist DP AP IP NA AF ANTMEET" << std::endl;
	//time to write out to the file
	saveFile << avgGen << "  " << sigGen << " " << avgDist << " " << sigDist << " "
		<< pDL << " " << pAL << " " << pIL << " " << antL << " " << aFL << " " 
		<< totalAMeetCount << std::endl;
											
		
																											
/*-------------------------------------------------------------------------------
-----------------------Finished batch section ----------------------------------
-------------------------------------------------------------------------------*/
									aFL += bCfg.aFS;
								} //ENDING ANTENNATION LOOP
								pIL += pIL + bCfg.pIS;
							}  //ENDING PHER INITIAL LOOP
							pDL += bCfg.pDS;
						} //ENDING PHER DECAY LOOP
						pAL += bCfg.pAS;	
					} //ENDING PHER ADD LOOP
					mAL += bCfg.mAS;	
				} //ENDING MAX AGE LOOP
			antL += bCfg.nAS;
			saveFile << std::endl;
			} //ENDING ANT LOOP
		} //ENDING AF DIRECTION 'TRUE' LOOP
		} //ENDING AF DIRECTION VECTOR LOOP
	} //ENDING AF TYPE 'TRUE' LOOP	
	} //ENDING AF TYPE VECTOR LOOP
	saveFile.close();
}



std::string Colony::GetLSType(int theType)
{
	std::string result; 
	switch(theType){
	case LSNONE: 
		result = "NONE";
		break;
	case LSEXP:
		result = "EXPONENTIAL";
		break;
	default: result = "ERROR";
	}
	return result;
}

std::string Colony::GetAntType(int theType)
{
	std::string result; 
	switch(theType){
	case AFBOOL: 
		result = "AFBOOL";
		break;
	case AFLIN:
		result = "AFLIN";
		break;
	case AFSQR:
		result = "AFSQR";
		break;
	case AFSQRT: 
		result = "AFSQRT";
		break;
	case AFTANH:
		result = "AFTANH";
		break;
	case AFAMTS:
		result = "AFAMTS";
		break;
	case AF1ONX:
		result = "AF1ONX";
		break;
	default: result = "ERROR";
	}
	return result;
}

std::string Colony::GetAntDir(int theType)
{
	std::string result; 
	switch(theType){
	case BIAFDIR: 
		result = "BIAFDIR";
		break;
	case UNIAFDIR:
		result = "UNIAFDIR";
		break;
	default: 
		result = "ERROR";
	}
	return result;
}


