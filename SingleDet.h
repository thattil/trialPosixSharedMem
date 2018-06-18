#pragma once

class SharedMemory;
class MultiDet;

#include <string>
#include <sys/types.h>

#define SINGLE_SHMVERSION	0x180608

class SingleDet{

	typedef  struct sharedSingleDet{

		/* FIXED PATTERN. Do not change this portion of structure, only append */
		int shmversion;
		pid_t lastPID;
		char hostname[1000];
		int type;
		/** end of the fixed pattern portion */

		int onlineFlag;
		int ndacs;
		int nadcs;
		int dacoff;
		int adcoff;
	}sharedSingleDet;

public:
	SingleDet(int type, int multiId, int id, bool verify = true, MultiDet* m = NULL);
	SingleDet(int multiId, int id, bool verify = true, MultiDet* m = NULL);
	~SingleDet();

	/**
	 * Free shared memory from the command line
	 * avoiding creating the constructor classes and mapping
	 * @param multiId multi detector Id
	 * @param singleId single detector Id
	 */
	static void FreeSharedMemory(int multiId, int singleId);

	/**
	 * Get Detector type directly from detector
	 * @param s hostname of detector
	 * @param port control port
	 */
	static int GetDetectorTypeFromDetector(std::string s, int port = 1952);

	/**
	 * Free shared memory and delete shared memory structure
	 * to be ONLY called from the multiSlsDetector object
	 * Otherwise, one has to update numberofdetectors, hostname etc in multi shm
	 */
	void FreeSharedMemory();

	void SetHostname(std::string s);
	std::string GetHostname();
	void SetOnline(int online);
	int GetOnline();
	int GetType();

private:

	/**
	 * Get Detector Type from Shared Memory (opening shm without verifying size)
	 * @param multiId multi detector Id
	 * @param singleId single detector Id
	 */
	static int GetDetectorTypeFromShm(int multiId, int singleId);

	/**
	 * Initialize shared memory
	 * @param multiId multi detector Id
	 * @param type type of detector
	 * @param verify true to verify if shm size matches existing one
	 * @returns true if the shared memory was created now
	 */
	bool InitSharedMemory(int multiId, int type, bool verify = true);

	/**
	 * Calculate shared memory size based on detector type
	 * @param type type of detector
	 */
	int CalculateSharedMemorySize(int type);

	/**
	 * Initialize detector structure
	 * @param created true if shared memory was just created now
	 * @param type type of detector
	 * @param verify true to verify if shm size matches existing one
	 */
	void InitializeDetectorStructure(bool created, int type, bool verify = true);

	/**
	 * Initialize detector structure pointers
	 */
	void InitializeDetectorStructurePointers();

	SharedMemory* sharedMemory;
	sharedSingleDet* thisSingleDet;

	int singleId;

	MultiDet* multiDet;

	int* dacs;
	int* adcs;
};
