#include "AutoBuildNumber_Main.h"

int 
	MajorVersion = 0,
	MinorVersion = 0,
	SubVersion = 0,
	BuildNumber = 0;

int main(int theArgCount, char ** theArgs)
{
	if (theArgCount == 1)
	{
		cout << "File name must be specified\n";
		return 1;
	}

	ifstream aF( theArgs[1], ios::in );

	if ( !aF.is_open() )
	{
		cout << "File not found\n";
		return 1;
	}

	LineList aVect;

	while (true)
	{
		char aData[500];
		aF.getline(aData, sizeof aData);

		ProcessLine(aVect, aData);

		if ( aF.bad() || aF.eof() ) break;
	}

	aF.close();

	if (BuildNumber == 0)
	{
		cout << "#define BUILDNUMBER string not found\n";
		return 1;
	}

	ofstream aFO(theArgs[1], ios::out|ios::trunc);

	if ( !aFO.is_open() )
	{
		cout << "Could not write data back to file\n";
		return 1;
	}

	

	bool aNewLine = false;
	for (LineIter aIt = aVect.begin(); aIt != aVect.end(); aIt++)
	{
		if (aNewLine)
			aFO << endl;
		
		aFO << *aIt;

		aNewLine = true;
	}

	aFO.close();

	cout << "Success\n";
	return 0;
}



void ProcessLine(LineList & theList, char * theData)
{
	char a[100], *b = 0;
	if ( b=strstr(theData, "BUILDMAJORVER") )
		sscanf(b, "%s %d", a, &MajorVersion);

	else if ( b=strstr(theData, "BUILDMINORVER") )
		sscanf(b, "%s %d", a, &MinorVersion);

	else if ( b=strstr(theData, "BUILDSUBVER") )
		sscanf(b, "%s %d", a, &SubVersion);

	else if ( b=strstr(theData, "BUILDNUMBER") )
	{
		sscanf(b, "%s %d", a, &BuildNumber);
		sprintf(theData, "#define BUILDNUMBER %d", ++BuildNumber);
	}
	else if ( b=strstr(theData, "WORDVER") )
	{
		sprintf(theData, "#define WORDVER %d, %d, %d, %d",
			MajorVersion,
			MinorVersion,
			SubVersion,
			BuildNumber);
	}
	else if ( b=strstr(theData, "DOTTEDVER") )
	{
		sprintf(theData, "#define DOTTEDVER \"%d.%d.%d.%d\"",
			MajorVersion,
			MinorVersion,
			SubVersion,
			BuildNumber);
	}
	else if ( b=strstr(theData, "COMMAVER") )
	{
		sprintf(theData, "#define COMMAVER \"%d, %d, %d, %d\"",
			MajorVersion,
			MinorVersion,
			SubVersion,
			BuildNumber);
	}
	else if ( b=strstr(theData, "BUILDSTRING") )
	{
		sprintf(theData, "#define BUILDSTRING \"Build %d\"", BuildNumber);
	}

	

	theList.push_back(theData);
}