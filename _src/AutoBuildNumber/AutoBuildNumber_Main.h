#include <vector>
#include <string>
#include <fstream>
#include <iostream>


using namespace std;

typedef vector<string>     LineList;
typedef LineList::iterator LineIter;

void ProcessLine(LineList & theList, char * theData);