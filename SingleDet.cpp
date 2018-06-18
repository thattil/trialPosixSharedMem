#include "SingleDet.h"
#include "SharedMemory.h"
#include "sls_detector_exceptions.h"
#include "ansi.h"

#include <unistd.h>
#include <iostream>
#include <cstring> //strcpy

SingleDet::SingleDet(int type, int multiId, int id, bool verify, MultiDet* m):
	sharedMemory(0),
	thisSingleDet(0),
	singleId(id),
	multiDet(m),
	dacs(0),
	adcs(0)
{
	InitSharedMemory(multiId, type, verify);
	InitializeDetectorStructure(type, verify);
}

SingleDet::SingleDet(int multiId, int id, bool verify, MultiDet* m):
	sharedMemory(0),
	thisSingleDet(0),
	singleId(id),
	multiDet(m),
	dacs(0),
	adcs(0)
{
	int type = GetDetectorTypeFromShm(multiId, id);
	InitSharedMemory(multiId, type, verify);
}

SingleDet::~SingleDet(){
	if (sharedMemory) {
		sharedMemory->UnmapSharedMemory(thisSingleDet);
		delete sharedMemory;
	}
}


void SingleDet::FreeSharedMemory(int multiId, int singleId) {
	SharedMemory* shm = new SharedMemory(multiId, singleId);
	shm->RemoveSharedMemory();
	delete shm;
	//thisMultiDet->numberofdetectorts = 0;
}


int SingleDet::GetDetectorTypeFromShm(int multiId, int singleId) {
	SharedMemory* shm = new SharedMemory(multiId, singleId);
	std::string shmname = shm->GetName();

	// shm not created before
	if (!SharedMemory::IsExisting(shmname)) {
		cprintf(RED,"Shared memory %s does not exist.\n"
				"Corrupted Multi Shared memory. Please free shared memory.\n",
				shmname.c_str());
		throw SharedMemoryException();
	}
	sharedSingleDet* sdet = 0;
	size_t sz = sizeof(sharedSingleDet);
	sdet = (sharedSingleDet*)shm->OpenSharedMemory(sz, false);

	int type = sdet->type;
	shm->UnmapSharedMemory(sdet);
	delete shm;

	return type;
}


int SingleDet::GetDetectorTypeFromDetector(std::string s, int port) {
	return 1; // connect etc
}


void SingleDet::FreeSharedMemory() {
	sharedMemory->RemoveSharedMemory();
}

bool SingleDet::InitSharedMemory(int multiId, int type, bool verify) {
	// clear
	if (sharedMemory)
		delete sharedMemory;

	thisSingleDet = 0;

	// calculate shared memory size
	int sz = CalculateSharedMemorySize(type);

	bool created = false;
	sharedMemory = new SharedMemory(multiId, singleId);
	// open existing shm
	if (SharedMemory::IsExisting(sharedMemory->GetName())) {
		thisSingleDet = (sharedSingleDet*)sharedMemory->OpenSharedMemory(sz, verify);
		if (verify && thisSingleDet->shmversion != SINGLE_SHMVERSION) {
			cprintf(RED, "Single (%d) shared memory version mismatch "
					"(expected 0x%x but got 0x%x)\n",
					singleId, SINGLE_SHMVERSION, thisSingleDet->shmversion);
			throw SharedMemoryException();
		}
	}

	// create new shm
	else {
		try {
			thisSingleDet = (sharedSingleDet*)sharedMemory->CreateSharedMemory(sz);
			created = true;
		} catch(...) {
			sharedMemory->RemoveSharedMemory();
			thisSingleDet = 0;
			throw;
		}
	}

	return created;
}


void SingleDet::InitializeDetectorStructure(bool created, int type, bool verify) {
	// set up new structure
		char* goff = (char*)thisSingleDet;

	if (created) {
		strcpy(thisSingleDet->hostname, "localhost");
		thisSingleDet->shmversion = SINGLE_SHMVERSION;
		thisSingleDet->onlineFlag = 0; // or 1
		thisSingleDet->type = type;

		switch(type){
		case 0:
			thisSingleDet->ndacs= 5;
			thisSingleDet->nadcs = 1;
			break;
		case 1:
			thisSingleDet->ndacs= 10;
			thisSingleDet->nadcs = 2;
			break;
		default:
			thisSingleDet->ndacs= 15;
			thisSingleDet->nadcs = 3;
			break;
		}
		thisSingleDet->dacoff = sizeof(sharedSingleDet);
		thisSingleDet->adcoff = thisSingleDet->dacoff + (thisSingleDet->ndacs * sizeof(int));
	}

	dacs = (int*) (goff + thisSingleDet->dacoff);
	adcs = (int*) (goff + thisSingleDet->adcoff);


	if (created) {
		InitializeDetectorStructurePointers();
	}

	thisSingleDet->lastPID = getpid();
}

int SingleDet::CalculateSharedMemorySize(int type) {
	int nd = 0, na = 0;
	switch(type) {
	case 0: nd = 5; na = 1; break;
	case 1: nd = 10; na = 2; break;
	default: nd = 15; na = 3; break;
	}
	return (sizeof(sharedSingleDet) + (nd * sizeof(int)) + (na * (sizeof(int))));
}

void SingleDet::InitializeDetectorStructurePointers() {
	for (int idac=0; idac < thisSingleDet->ndacs; ++idac)
		*(dacs+idac)=0;
	for (int iadc=0; iadc < thisSingleDet->nadcs; ++iadc)
		*(adcs+iadc)=0;
}

void SingleDet::SetHostname(std::string s) {
	strcpy(thisSingleDet->hostname, s.c_str());
}

std::string SingleDet::GetHostname() {
	return string(thisSingleDet->hostname);
}


void SingleDet::SetOnline(int online) {
	thisSingleDet->onlineFlag = online;
}

int SingleDet::GetOnline() {
	return thisSingleDet->onlineFlag;
}

int SingleDet::GetType() {
	return thisSingleDet->type;
}
