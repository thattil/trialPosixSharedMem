#include "ansi.h"
#include "MultiDet.h"
#include "SingleDet.h"
#include "sls_detector_exceptions.h"

#include <iostream>
#include <stdio.h>	//sprintf
#include <stdlib.h> //atoi
using namespace std;



int main(int argc, char *argv[]) {
	int multiId=-1, singleId=-1;
	char cmd[1000]="";
	if(sscanf(argv[1],"%u-%u:%s", &multiId, &singleId, cmd) == 3);
	else if (sscanf(argv[1],"%u-%s", &multiId, cmd) == 2);
	else if (sscanf(argv[1],"%u:%s", &singleId, cmd) == 2);
	else if (sscanf(argv[1],"%s", cmd) == 1);
	else {
		cprintf(BLUE,"Usage:\n"
				"./trial [multiId-][singleId:]command "
				"Command options: phostname, ghostname, ponline, gonline, free\n\n");
		return EXIT_FAILURE;
	}

	if (multiId == -1)
		multiId = 0;

	string scmd = cmd;
	bool verify = true;
	bool update = true;

	if (scmd == "free") {
		if (singleId != -1) {
			SingleDet::FreeSharedMemory(multiId, singleId);
		} else
			MultiDet::FreeSharedMemory(multiId);
		return 0;
	} else if (scmd == "guserdetails") {
		verify = false;
		update = false;
	}


	if ((scmd != "phostname") &&
			(scmd != "ghostname") &&
			(scmd != "ponline") &&
			(scmd != "gonline") &&
			(scmd != "guserdetails")
	)	{
		cout<<"unknown command"<<endl;
		return -1;
	}

	MultiDet* mdet = 0;
	try {
		mdet = new MultiDet((multiId == -1) ? 0 : multiId, verify, update);
	}catch(const SharedMemoryException & e) {
		cout << e.GetMessage() << endl;
		return 0;
	}

	// specific single det
	if (singleId != -1) {
		SingleDet* sdet = mdet->GetSingleDetector(singleId);
		if (!sdet) {
			cout << "Single detector at position " << singleId << " does not exist" << endl;

		}
		else {
			if (scmd == "phostname")
				sdet->SetHostname(string(argv[2]));
			else if (scmd == "ghostname")
				cout << sdet->GetHostname() << endl;
			else if (scmd == "ponline")
				sdet->SetOnline(atoi(argv[2]));
			else if (scmd == "gonline")
				cout << sdet->GetOnline() << endl;
			else cout << "Unknown command" << endl;
		}

	}
	// all slsdetectors
	else  {
		if (singleId == -1)
			singleId = 0;
		if (scmd == "guserdetails")
			cout<<mdet->GetUserDetails()<<endl;
		else if (scmd == "phostname")
			mdet->SetHostname(string(argv[2]));
		else if (scmd == "ghostname")
			cout << mdet->GetHostname() << endl;
		else if (scmd == "ponline")
			mdet->SetOnline(atoi(argv[2]));
		else if (scmd == "gonline")
			cout << mdet->GetOnline() << endl;
		else cout << "Unknown command" << endl;
	}

	delete mdet;

	//cout << "Goodbye" << endl;
	return 0;

}
