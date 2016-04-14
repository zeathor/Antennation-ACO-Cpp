#include "Definitions.h"


double StrToDouble(const std::string & s){
	std::istringstream i(s);
	double x;
	i >> x;
	return x;
};

int StrToInt(const std::string & s){
 	std::istringstream i(s);
 	int x;
 	i >> x;
 	return x;
};