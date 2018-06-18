#pragma once

class SharedMemory;
class SingleDet;

#include <vector>
#include <string>


#define MULTI_SHMVERSION	0x180608

class MultiDet{

	  typedef  struct sharedMultiDet{

			/* FIXED PATTERN. Do not change this portion of structure, only append */
		  int shmversion;
		  pid_t lastPID;
		  char lastUser[50];
		  char lastDate[50];
		  int numberOfDetectors;
		  /** end of the fixed pattern portion */


		  int onlineFlag;
	  }sharedMultiDet;

public:

	  /**
	   * Constructor
	   * @param id multi detector id
	   * @param verify true to verify if shm size matches existing one
	   * @param update true to update last user pid, date etc
	   */
	MultiDet(int id = 0, bool verify = true, bool update = true);
	~MultiDet();

	/**
	 * Free shared memory from the command line
	 * avoiding creating the constructor classes and mapping
	 * @param multiId multi detector Id
	 */
	static void FreeSharedMemory(int multiId);

	/**
	 * Free shared memory and delete shared memory structure
	 */
	void FreeSharedMemory();

	/**
	 * Get single detector object from id
	 * @param pos position in detectors array
	 */
	SingleDet* GetSingleDetector(unsigned int pos);

	std::string GetUserDetails();

	void SetHostname(std::string s);
	std::string GetHostname();
	void SetOnline(int online);
	int GetOnline();

private:

	/**
	 * Initialize shared memory
	 * @param verify true to verify if shm size matches existing one
	 * @param update true to update last user pid, date etc
	 * @returns true if the shared memory was created now
	 */
	bool InitSharedMemory(bool verify = true, bool update = true);

	/**
	 * Initialize detector structure
	 * @param created true if shared memory was just created now
	 * @param verify true to verify if shm size matches existing one
	 * @param update true to update last user pid, date etc
	 */
	void InitializeDetectorStructure(bool created, bool verify = true, bool update = true);

	/**
	 * Add single detector
	 * @param s hostname of the single detector
	 */
	void AddSingleDetector (std::string s);

	std::string exec(const char* cmd);

	SharedMemory* sharedMemory;

	sharedMultiDet* thisMultiDet;

	std::vector <SingleDet*> detectors;

	int detId;
};



