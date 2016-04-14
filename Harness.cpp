#include <string>
#include <iostream>
#include <fstream>
#include <ostream>
#include <map>

#include "Definitions.h"
#include "Colony.h"



//purpose of main is to perform all the IO and pass the information onwards
//colony runs the ACO
//ant is each individual ant
//phermap is the pheromone map
enum ACOType{ StrACOndef, 
              	StrACOAS, 
              	StrACOACS, 
              	StrACOAMTS, 
              	StrACOMMAS};

enum CoordType{ StrCoordndef,
				StrCoordLatitude,
				StrCoordCartesian};

enum LSType{ StrLSndef,
				StrLSNone,
				StrLSExp};

enum AFDirType{ StrAFDirndef,
				StrAFDirBi,
				StrAFDirUni};

enum AFType{ StrAFndef,
				StrAFBool,
				StrAFLin,
				StrAFSqr,
				StrAFSqrt,
				StrAFTanh,
				StrAFAmts,
				StrAF1OnX};

static std::map<std::string, ACOType> 	mapACOType;
static std::map<std::string, CoordType> mapCoordType;
static std::map<std::string, LSType> 	mapLSType;
static std::map<std::string, AFDirType> mapAFDirType;
static std::map<std::string, AFType> 	mapAFType;

static void InitialiseSwitches();


int main(int argc, char** argv)
{
	
	//read-in config file, place into a vector of strings
	//parse out comments
	std::cout <<"WORK!!" << std::endl;
	std::string line, saveFile;
	std::vector<std::string> parsedCfg, cityList;
	int randSeed;
	int iter = 0; 
	int pA, pB; //" " indices in strings

	
	
	TPherCfg thePCfg;
	TAntCfg theACfg;
	TLocalSearchCfg theLSCfg;
	TBatchCfg theBCfg;
	
	theBCfg.aFDir.resize(CURRENT_AFDIR_TYPES);
	theBCfg.aFType.resize(CURRENT_AF_TYPES);
	
	InitialiseSwitches();
	

	std::ifstream cfgFile("config.txt");
	///////////////////TRIM COMMENTS FROM CONFIG FILE ///////////////////////////////////
	while(!cfgFile.eof()) { //while not at the end of the file
		getline(cfgFile,line);
		if(line.compare(0,2, "//") != 0){ //while line does not start with // i.e. comments
			parsedCfg.push_back(line);
		}
	}
	cfgFile.close(); //finished with the config file
	
	///////////////MAP FILE RUBBISH	/////////////////////////////////////////
	//time to parse the map file
	std::ifstream mapFile(parsedCfg[iter].c_str()); iter++; // ALWAYS ADD iter++ WHEN ACCESSING PARSEDCFG
	getline(mapFile,line); 
	
	thePCfg.nC = theACfg.nC = StrToDouble(line.substr(0,line.find(" ")));
	switch(mapCoordType[line.substr(line.find(" ")+1,line.length())]){
	case StrCoordLatitude: 
		thePCfg.mType = LATITUDE;
		break;
	case StrCoordCartesian:
		thePCfg.mType = CARTESIAN;
		break;
	default:
		thePCfg.mType = CARTESIAN;
	}	
	
	//time to get the different map locations
	while(!mapFile.eof()){
		getline(mapFile,line);
		cityList.push_back(line);
	}	
	mapFile.close();
	/////////////// FINISHED WITH THE MAP FILE///////////////////////////////
	
	////////////// STORE THE NEW SAVEFILE///////////////////////////////
	saveFile = parsedCfg[iter]; iter++;
		
	/////////////// TIME TO PARSE THE REST OF THE CONFIG FILE////////////////
	//////////////// PARSING NON BATCH VALUE ////////////////////////////////
	// Algorithm type - AS/ACS/AMTS/MMAS 	(string)	
	switch(mapACOType[parsedCfg[iter]]){
	case StrACOAS:
		theBCfg.aLType = thePCfg.aLType = theACfg.aLType = ACOAS;
		break;
	case StrACOACS:
		theBCfg.aLType = thePCfg.aLType = theACfg.aLType = ACOACS;
		break;
	case StrACOAMTS:
		theBCfg.aLType = thePCfg.aLType = theACfg.aLType = ACOAMTS;
		break;	
	case StrACOMMAS:
		theBCfg.aLType = thePCfg.aLType = theACfg.aLType = ACOMMAS;
		break;
	default:
		theBCfg.aLType = thePCfg.aLType = theACfg.aLType = ACOAS;
		break;
	} iter++;
	// Alpha value (int)
	// Beta values (int)
	thePCfg.alpha = StrToInt(parsedCfg[iter]); iter++;
	thePCfg.beta = StrToInt(parsedCfg[iter]); iter++;
	
	// Random Seed value
	randSeed = StrToInt(parsedCfg[iter]); iter++;
	
	// Local Search type - NONE (default), LSLIN, LSEXP	(string)
	switch(mapLSType[parsedCfg[iter]]){
	case StrLSNone:
		theLSCfg.lSType = LSNONE;
		break;
	case StrLSExp:
		theLSCfg.lSType = LSEXP;
		break;
	default: 
		theLSCfg.lSType = LSNONE;
		break;
	} iter++;	
	// Local Search times per ant per iteration 		(int)	
	theLSCfg.lSTimes = StrToInt(parsedCfg[iter]); iter++;
	// Antennation Direction - BIAFDIR UNIAFDIR 
	// Place each type on a separate line 
	// Continues checking for AFdirections until empty line
	bool dirSelected = false;

	while(parsedCfg[iter] != "")
	{
		switch(mapAFDirType[parsedCfg[iter]]){
		case StrAFDirBi:
			theBCfg.aFDir[BIAFDIR] = true;
			dirSelected = true;
			break;
		case StrAFDirUni:
			theBCfg.aFDir[UNIAFDIR] = true;
			dirSelected = true;
			break;
		default: //use Bi directional
			break;
		}
		iter++;
	}
	if(dirSelected == false)
		theBCfg.aFDir[BIAFDIR] = true;
	
	iter++;
	// Antennation type - AFBOOL AFLIN AFSQR AFSQRT AFTANH AFAMTS AF1ONX	(string)
	// Place each type on a separate line 
	// Continues checking for AFtypes until empty line	
	while(parsedCfg[iter] != "")
	{
		switch(mapAFType[parsedCfg[iter]]){
		case StrAFBool:
			theBCfg.aFType[AFBOOL] = true;
			break;
		case StrAFLin:
			theBCfg.aFType[AFLIN] = true;
			break;
		case StrAFSqr:
			theBCfg.aFType[AFSQR] = true;
			break;
		case StrAFSqrt:
			theBCfg.aFType[AFSQRT] = true;
			break;
		case StrAFTanh:
			theBCfg.aFType[AFTANH] = true;
			break;
		case StrAFAmts:
			theBCfg.aFType[AFAMTS] = true;
			break;	
		case StrAF1OnX:
			theBCfg.aFType[AF1ONX] = true;
			break;
		default: //use Boolean
			theBCfg.aFType[AFBOOL] = true; 
			break;
		}
		iter++;
	}
	iter++;
		

	// Batch variable settings - all settings are Begin Step End
	// Ants in simulation	(int)
	pA = parsedCfg[iter].find(" ");
	pB = parsedCfg[iter].find_last_of(" ");
	theBCfg.nAB = StrToInt(parsedCfg[iter].substr(0,pA));
	theBCfg.nAS = StrToInt(parsedCfg[iter].substr(pA+1,pB));
	theBCfg.nAE = StrToInt(parsedCfg[iter].substr(pB,parsedCfg[iter].length()));	
	iter++;

	// Pheromone Added 	 (double)
	pA = parsedCfg[iter].find(" ");
	pB = parsedCfg[iter].find_last_of(" ");
	theBCfg.pAB = StrToDouble(parsedCfg[iter].substr(0,pA));
	theBCfg.pAS = StrToDouble(parsedCfg[iter].substr(pA+1,pB));
	theBCfg.pAE = StrToDouble(parsedCfg[iter].substr(pB,parsedCfg[iter].length()));	
	iter++;

	// Pheromone Evaporation/decay NOT persistence 	(double)
	pA = parsedCfg[iter].find(" ");
	pB = parsedCfg[iter].find_last_of(" ");
	theBCfg.pDB = StrToDouble(parsedCfg[iter].substr(0,pA));
	theBCfg.pDS = StrToDouble(parsedCfg[iter].substr(pA+1,pB));
	theBCfg.pDE = StrToDouble(parsedCfg[iter].substr(pB,parsedCfg[iter].length()));	
	iter++;

	// Initial Pheromone 	(double)
	pA = parsedCfg[iter].find(" ");
	pB = parsedCfg[iter].find_last_of(" ");
	theBCfg.pIB = StrToDouble(parsedCfg[iter].substr(0,pA));
	theBCfg.pIS = StrToDouble(parsedCfg[iter].substr(pA+1,pB));
	theBCfg.pIE = StrToDouble(parsedCfg[iter].substr(pB,parsedCfg[iter].length()));	
	iter++;

	// Antennation Factor 						(double)
	pA = parsedCfg[iter].find(" ");
	pB = parsedCfg[iter].find_last_of(" ");
	theBCfg.aFB = StrToDouble(parsedCfg[iter].substr(0,pA));
	theBCfg.aFS = StrToDouble(parsedCfg[iter].substr(pA+1,pB));
	theBCfg.aFE = StrToDouble(parsedCfg[iter].substr(pB,parsedCfg[iter].length()));	
	iter++;

	// Number of generations with no change		(int)
	theBCfg.gen = StrToInt(parsedCfg[iter]); iter++;

	// Number of iterations for each setting	(int)
	theBCfg.iter = StrToInt(parsedCfg[iter]); iter++;

	//empty line
	iter++;
	

	switch(thePCfg.aLType){
	case ACOAS: //default, do nothing 
		break;
	case ACOACS:
		pA = parsedCfg[iter].find(" ");
		pB = parsedCfg[iter].find_last_of(" ");
		theBCfg.gPB = StrToDouble(parsedCfg[iter].substr(0,pA));
		theBCfg.gPS = StrToDouble(parsedCfg[iter].substr(pA+1,pB));
		theBCfg.gPE = StrToDouble(parsedCfg[iter].substr(pB,parsedCfg[iter].length()));	
		iter++;
		theBCfg.igB = StrToDouble(parsedCfg[iter]); iter++;
		break;
	case ACOAMTS:
		pA = parsedCfg[iter].find(" ");
		pB = parsedCfg[iter].find_last_of(" ");
		theBCfg.mAB = StrToDouble(parsedCfg[iter].substr(0,pA));
		theBCfg.mAS = StrToDouble(parsedCfg[iter].substr(pA+1,pB));
		theBCfg.mAE = StrToDouble(parsedCfg[iter].substr(pB,parsedCfg[iter].length()));	
		iter++;
		break;
	case ACOMMAS: //do nothing
		break;
	}

	
	
	////NOTE: THIS COULD ALL BE TAKEN CARE OF IN THE INITIALISER
	//FOR CLARITY, LETS DO EACH STEP
	std::cout << "finished reading file" << std::endl;
	
	//initialise Colony
	Colony theColony(randSeed, theBCfg, theLSCfg, thePCfg, theACfg);
	//set up the map
	theColony.InitMap(cityList);
	//set output file
	theColony.SetFile(saveFile);
	//run this bitch
	theColony.RunBatch();
	
	
	
	
	return 0;

	
}

//	std::cout << << std::endl;
static void InitialiseSwitches()
{
	mapACOType["AS"] 	= StrACOAS;
	mapACOType["ACS"] 	= StrACOACS;
	mapACOType["AMTS"] 	= StrACOAMTS;
	mapACOType["MMAS"] 	= StrACOMMAS;
	
	mapCoordType["LATITUDE"] 	= StrCoordLatitude;
	mapCoordType["CARTESIAN"] 	= StrCoordCartesian;
	
	mapLSType["NONE"] 	= StrLSNone;
	mapLSType["LSEXP"] 	= StrLSExp;
	
	mapAFDirType["BIAFDIR"] 	= StrAFDirBi;
	mapAFDirType["UNIAFDIR"] 	= StrAFDirUni;
	
	mapAFType["AFBOOL"] = StrAFBool;
	mapAFType["AFLIN"] 	= StrAFLin;
	mapAFType["AFSQR"] 	= StrAFSqr;
	mapAFType["AFSQRT"] = StrAFSqrt;
	mapAFType["AFTANH"] = StrAFTanh;
	mapAFType["AFAMTS"] = StrAFAmts;
	mapAFType["AF1ONX"] = StrAF1OnX;
	
	
}


// Configuration File Format - lines with >1 parameter are \t separated
// any lines starting with // are ignored
// Map filename	(string)
// Save batch filenam (string)
// Empty line 
// Algorithm type - AS/ACS/AMTS/MMAS 				(string)
// Alpha value (int)
// Beta values (int)								
// Local Search type - NONE (default), LSLIN, LSEXP	(string)
// Local Search times per ant per iteration 		(int)	
// Antennation Direction - BIAFDIR UNIAFDIR 
// Place each type on a separate line 
// Continues checking for AFdirections until empty line
// Antennation type - AFBOOL AFLIN AFSQR AFSQRT AFTANH AFAMTS AF1ONX	(string)
// Place each type on a separate line 
// Continues checking for AFtypes until empty line
// After empty line, batch variable settings - all settings are start step end
// Ants in simulation 						(int)
// Pheromone Added 							(double)
// Pheromone Evaporation NOT persistence 	(double)
// Initial Pheromone 						(double)
// Antennation Factor 						(double)
// Number of generations with no change		(int)
// Number of iterations for each setting	(int)
// empty line
// ADDITIONAL SETTINGS FOR EACH ALGORITHM
// IF ACS
// greedy probability 						(double)
// % iBest vs gBest - single value 			(double)
// IF AMTS
// maximum ant age 							(int)
// IF MMAS
// pBest value - check MMAS for details		(double)


