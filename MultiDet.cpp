#include "MultiDet.h"
#include "SharedMemory.h"
#include "SingleDet.h"
#include "sls_detector_exceptions.h"
#include "ansi.h"


#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string.h>

MultiDet::MultiDet(int id, bool verify, bool update):
	sharedMemory(0),
	thisMultiDet(0),
	detId(id)
{
	bool created = InitSharedMemory(verify, update);
	InitializeDetectorStructure(created, verify, update);
}

MultiDet::~MultiDet(){
	for (vector<SingleDet*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		delete(*it);
	}
	detectors.clear();

	if (sharedMemory) {
		sharedMemory->UnmapSharedMemory(thisMultiDet);
		delete sharedMemory;
	}
}


void MultiDet::FreeSharedMemory(int multiId) {
	// get number of detectors
	int numDetectors = 0;
	SharedMemory* shm = new SharedMemory(multiId, -1);
	std::string shmname = shm->GetName();

	// shm not created before
	if (SharedMemory::IsExisting(shmname)) {
		sharedMultiDet* mdet = (sharedMultiDet*)shm->OpenSharedMemory(sizeof(sharedMultiDet), false);
		numDetectors = mdet->numberOfDetectors;
		shm->UnmapSharedMemory(mdet);
		shm->RemoveSharedMemory();
	}
	delete shm;

	for (int i = 0; i < numDetectors; ++i) {
		SharedMemory* shm = new SharedMemory(multiId, i);
		shm->RemoveSharedMemory();
		delete shm;
	}
}


void MultiDet::FreeSharedMemory() {

	// single detector vector
	for (vector<SingleDet*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		(*it)->FreeSharedMemory();
		delete (*it);
	}
	detectors.clear();

	// multi detector
	if (sharedMemory) {
		sharedMemory->RemoveSharedMemory();
		delete sharedMemory;
	}
	thisMultiDet = 0;
}

bool MultiDet::InitSharedMemory(bool verify, bool update) {

	// clear
	if (sharedMemory)
		delete sharedMemory;
	thisMultiDet = 0;

	for (vector<SingleDet*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		delete(*it);
	}
	detectors.clear();

	// create/open shm and map to structure
	size_t sz = sizeof(sharedMultiDet);

	bool created = false;
	sharedMemory = new SharedMemory(detId, -1);
	if (SharedMemory::IsExisting(sharedMemory->GetName())) {
		thisMultiDet = (sharedMultiDet*)sharedMemory->OpenSharedMemory(sz, verify);
		if (verify && thisMultiDet->shmversion != MULTI_SHMVERSION) {
			cprintf(RED, "Multi shared memory version mismatch "
					"(expected 0x%x but got 0x%x)\n",
					MULTI_SHMVERSION, thisMultiDet->shmversion);
			throw SharedMemoryException();
		}
	} else {
		try {
			thisMultiDet = (sharedMultiDet*)sharedMemory->CreateSharedMemory(sz);
			created = true;
		} catch(...) {
			sharedMemory->RemoveSharedMemory();
			thisMultiDet = 0;
			throw;
		}
	}

	return created;
}


void MultiDet::InitializeDetectorStructure(bool created, bool verify, bool update) {
	// set up new structure
	if (created) {
		thisMultiDet->numberOfDetectors = 0;
		thisMultiDet->shmversion = MULTI_SHMVERSION;
		thisMultiDet->onlineFlag = 1;
	}

	// get objects from single det shared memory (open)
	for (int i = 0; i < thisMultiDet->numberOfDetectors; i++) {
		SingleDet* sdet = new SingleDet(detId, i, verify, this); // crash here prevents complete loading of detectors array
		detectors.push_back(sdet);
	}


	//update user details
	if (update) {
		thisMultiDet->lastPID = getpid();
		memset(thisMultiDet->lastUser, 0, 50);
		memset(thisMultiDet->lastDate, 0, 50);
		try {
			strncpy(thisMultiDet->lastUser, exec("whoami").c_str(), 50);
			strncpy(thisMultiDet->lastDate, exec("date").c_str(), 29);//size of date
		} catch(...) {
			strncpy(thisMultiDet->lastUser, exec("errorreading").c_str(), 50);
			strncpy(thisMultiDet->lastDate, exec("errorreading").c_str(), 50);
		}
	}
}



void MultiDet::AddSingleDetector (std::string s) {

	cout << "Adding detector " << s << endl;

	for (vector<SingleDet*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		if ((*it)->GetHostname() == s) {
            cout << "Detector " << s << "already part of the multiDetector!" << endl
                 << "Remove it before adding it back in a new position!" << endl;
            return;
		}
	}

    //check entire shared memory if it doesnt exist?? needed?
	//could be that detectors not loaded completely cuz of crash in new slsdetector in initsharedmemory

	// get type by connecting
	int type = SingleDet::GetDetectorTypeFromDetector(s, 1952);
	if (type == 0) {
		cout << "Detector " << s << "does not exist in shared memory "
				"and could not connect to it to determine the type!" << endl;
		//setErrorMask(getErrorMask() | MULTI_DETECTORS_NOT_ADDED);
		// appendNotAddedList(name);
		return;
	}



	int pos = detectors.size();
	SingleDet* sdet = new SingleDet(type, detId, pos, false, this);
	detectors.push_back(sdet);
	detectors[pos]->SetHostname(s);
	detectors[pos]->SetOnline(1);
	++thisMultiDet->numberOfDetectors;
}


SingleDet* MultiDet::GetSingleDetector(unsigned int pos) {
	if (pos >= 0 && pos < detectors.size()) {
		return detectors[pos];
	}
	return 0;
}


std::string MultiDet::GetUserDetails() {
	std::ostringstream sstream;

	if (!detectors.size()) {
		return std::string("not in use");
	}

	//hostname
	sstream << "\nHostname: " << GetHostname();
	//type
	sstream<< "\nType: ";

	for (int i = 0; i < thisMultiDet->numberOfDetectors; ++i)
		sstream<< detectors[i]->GetType() << "+";
	//PID
	sstream << "\nPID: " << thisMultiDet->lastPID
	//user
	<< "\nUser: " << thisMultiDet->lastUser
	<< "\nDate: " << thisMultiDet->lastDate << endl;

	string s = sstream.str();
	return s;
}

std::string MultiDet::exec(const char* cmd) {
	int bufsize = 128;
	char buffer[bufsize];
	std::string result = "";
	FILE* pipe = popen(cmd, "r");
	if (!pipe) throw std::exception();
	try {
		while (!feof(pipe)) {
			if (fgets(buffer, bufsize, pipe) != NULL)
				result += buffer;
		}
	} catch (...) {
		pclose(pipe);
		throw;
	}
	pclose(pipe);
	result.erase(result.find_last_not_of(" \t\n\r")+1);
	return result;

}


void MultiDet::SetHostname(std::string s) {
	 size_t p1 = 0;
	 std::string temp = string(s);
	 size_t p2 = temp.find('+', p1);
	 //single
	 if (p2 == string::npos) {
		 AddSingleDetector(s);
	 }
	 // multi
	 else {
		 while(p2 != string::npos) {
			 AddSingleDetector(temp.substr(p1, p2-p1));
			 temp = temp.substr(p2 + 1);
			 p2 = temp.find('+');
		 }
	 }
}

std::string MultiDet::GetHostname() {
	std::string s;
	for (vector<SingleDet*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
		s += (*it)->GetHostname() + "+";
	}
	return s;
}


void MultiDet::SetOnline(int online) {
	if (online != -1) {
		for (vector<SingleDet*>::const_iterator it = detectors.begin(); it != detectors.end(); ++it) {
			(*it)->SetOnline(online);
		}
	}
}

int MultiDet::GetOnline() {
	return thisMultiDet->onlineFlag;
}
