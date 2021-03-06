#include "settings.h"
#include "Shlwapi.h"
#include <fstream>
#include <string>

// CONSTANTS
const float PI = 3.14159265358979323846f;
const char* CL_FILENAME = "resample.cl";

// SETTINGS
int SAMPLE_SIZE = 1024;
bool PRINT_TO_FILE = false;
std::string RESULTS_FILE = "results.txt";
unsigned int RUN_COUNT = 1;
float MAX_DIFF = 0.001f;

namespace
{
	const char* PREF_APP = "Resample";
	const char* PREF_FILE = "resample.ini";
	
	const char* PREF_SAMPLE_SIZE = "SampleSize";
	const char* PREF_PRINT_TO_FILE = "PrintToFile";
	const char* PREF_RESULTS_FILE = "ResultsFile";
	const char* PREF_RUN_COUNT = "RunCount";
	const char* PREF_MAX_DIFF = "MaxDiff";
	const char* PREF_OUTPUT_SIGNALIN = "OutputFile_SignalIn";
	const char* PREF_OUTPUT_TIMEIN = "OutputFile_TimeIn";
	const char* PREF_OUTPUT_SIGNALOUT = "OutputFile_SignalOut";
	const char* PREF_OUTPUT_TIMEOUT = "OutputFile_TimeOut";

	// Data File Locations
	const char* OCTAVE_APP = "Octave";
	const char* OCTAVE_PREF_FILE = "octave.ini";
	const char* PREF_SIGNAL_DATA = "SignalData";
	const char* PREF_COEFFS_DATA = "CoeffsData";
	const char* PREF_OUTPUT_DATA = "OutputData";
	const char* PREF_SAMPLE_INPUT_RATE = "SampleInputRate";
	const char* PREF_SAMPLE_OUTPUT_RATE = "SampleOutputRate";
	const char* PREF_POLYNOMIAL_ORDER = "PolynomialOrder";
}

namespace settings
{
	std::string GetStringFromPrefs(const std::string& prefFile, const std::string& section, const std::string& key, const std::string& defVal)
	{
		char buf[255];
		GetPrivateProfileStringA(section.c_str(), key.c_str(), defVal.c_str(), buf, 255, prefFile.c_str());
		return std::string(buf);
	}
	int GetIntFromPrefs(const std::string& prefFile, const std::string& section, const std::string& key, int defVal)
	{
		return GetPrivateProfileIntA(section.c_str(), key.c_str(), defVal, prefFile.c_str());
	}
	bool GetBoolFromPrefs(const std::string& prefFile, const std::string& section, const std::string& key, bool defVal)
	{
		const int val = GetIntFromPrefs(prefFile, section, key, defVal ? 1 : 0);
		return (val == 1);
	}
	float GetFloatFromPrefs(const std::string& prefFile, const std::string& section, const std::string& key, float defVal)
	{
		const std::string defValString = std::to_string(defVal);
		const std::string str = GetStringFromPrefs(prefFile, section, key, defValString);
		return (float)std::atof(str.c_str());
	}

	std::string PrefFilePath(const std::string& pref)
	{
		char buf[255];
		GetFullPathNameA(pref.c_str(), 255, buf, NULL);
		return std::string(buf);
	}

	bool PrefFileExists(const std::string& prefFile)
	{
		if (!PathFileExistsA(prefFile.c_str()))
			return false;

		return true;
	}

	bool CreatePrefFile(const std::string& prefFile)
	{
		std::ofstream prefsFile;
		prefsFile.open(prefFile.c_str(), std::ofstream::out);
		if (!prefsFile.is_open())
			return false;
		
		if (prefFile == OCTAVE_PREF_FILE)
			WriteOctavePrefsDefaults();
		else if(prefFile == PREF_FILE)
			WriteResamplePrefs();
		return true;
	}

	void WriteResamplePrefs()
	{
		const std::string prefFilePath = PrefFilePath(PREF_FILE);

		std::string output;

		output = std::to_string(SAMPLE_SIZE);
		WritePrivateProfileStringA(PREF_APP, PREF_SAMPLE_SIZE, output.c_str(), prefFilePath.c_str());

		output = (PRINT_TO_FILE ? "1" : "0");
		WritePrivateProfileStringA(PREF_APP, PREF_PRINT_TO_FILE, output.c_str(), prefFilePath.c_str());

		WritePrivateProfileStringA(PREF_APP, PREF_RESULTS_FILE, RESULTS_FILE.c_str(), prefFilePath.c_str());

		output = std::to_string(RUN_COUNT);
		WritePrivateProfileStringA(PREF_APP, PREF_RUN_COUNT, output.c_str(), prefFilePath.c_str());

		output = std::to_string(MAX_DIFF);
		WritePrivateProfileStringA(PREF_APP, PREF_MAX_DIFF, output.c_str(), prefFilePath.c_str());

		// Output Data
		WritePrivateProfileStringA(PREF_APP, PREF_OUTPUT_TIMEIN, "..\\data\\ResampleOutput_TimeIn.csv", prefFilePath.c_str());
		WritePrivateProfileStringA(PREF_APP, PREF_OUTPUT_SIGNALIN, "..\\data\\ResampleOutput_SignalIn.csv", prefFilePath.c_str());
		WritePrivateProfileStringA(PREF_APP, PREF_OUTPUT_TIMEOUT, "..\\data\\ResampleOutput_TimeOut.csv", prefFilePath.c_str());
		WritePrivateProfileStringA(PREF_APP, PREF_OUTPUT_SIGNALOUT, "..\\data\\ResampleOutput_SignalOut.csv", prefFilePath.c_str());
	}

	// loads settings from PREF_FILE. If file does not exist, creates file with default values
	void ReadResamplePrefs()
	{
		if (!PrefFileExists(PREF_FILE))
		{
			(void)CreatePrefFile(PREF_FILE);
			return;
		}
		SAMPLE_SIZE = GetIntFromPrefs(PrefFilePath(PREF_FILE), PREF_APP, PREF_SAMPLE_SIZE, SAMPLE_SIZE);
		PRINT_TO_FILE = GetBoolFromPrefs(PrefFilePath(PREF_FILE), PREF_APP, PREF_PRINT_TO_FILE, PRINT_TO_FILE);
		RESULTS_FILE = GetStringFromPrefs(PrefFilePath(PREF_FILE), PREF_APP, PREF_RESULTS_FILE, RESULTS_FILE);
		RUN_COUNT = GetIntFromPrefs(PrefFilePath(PREF_FILE), PREF_APP, PREF_RUN_COUNT, RUN_COUNT);
		MAX_DIFF = GetFloatFromPrefs(PrefFilePath(PREF_FILE), PREF_APP, PREF_MAX_DIFF, MAX_DIFF);
	}
	// Output Data Files
	std::string GetResampleOuputFile_TimeIn()
	{
		return GetStringFromPrefs(PrefFilePath(PREF_FILE), PREF_APP, PREF_OUTPUT_TIMEIN, RESULTS_FILE);
	}

	std::string GetResampleOuputFile_SignalIn()
	{
		return GetStringFromPrefs(PrefFilePath(PREF_FILE), PREF_APP, PREF_OUTPUT_SIGNALIN, RESULTS_FILE);
	}

	std::string GetResampleOuputFile_TimeOut()
	{
		return GetStringFromPrefs(PrefFilePath(PREF_FILE), PREF_APP, PREF_OUTPUT_TIMEOUT, RESULTS_FILE);
	}

	std::string GetResampleOuputFile_SignalOut()
	{
		return GetStringFromPrefs(PrefFilePath(PREF_FILE), PREF_APP, PREF_OUTPUT_SIGNALOUT, RESULTS_FILE);
	}


	//////////////////// DATA SETTINGS ////////////////////////
	void WriteOctavePrefsDefaults()
	{
		(void)CreatePrefFile(OCTAVE_PREF_FILE);
		const std::string prefFilePath = PrefFilePath(OCTAVE_PREF_FILE);
		WritePrivateProfileStringA(OCTAVE_APP, PREF_SIGNAL_DATA, "..\\data\\test_resample_input_signal_FULL.csv", prefFilePath.c_str());
		WritePrivateProfileStringA(OCTAVE_APP, PREF_COEFFS_DATA, "", prefFilePath.c_str());
		WritePrivateProfileStringA(OCTAVE_APP, PREF_OUTPUT_DATA, "", prefFilePath.c_str());
		WritePrivateProfileStringA(OCTAVE_APP, PREF_SAMPLE_INPUT_RATE, std::to_string(256).c_str(), prefFilePath.c_str());
		WritePrivateProfileStringA(OCTAVE_APP, PREF_SAMPLE_OUTPUT_RATE, std::to_string(128).c_str(), prefFilePath.c_str());
		WritePrivateProfileStringA(OCTAVE_APP, PREF_POLYNOMIAL_ORDER, std::to_string(6).c_str(), prefFilePath.c_str());
	}
	
	// Reads from Octave.ini
	std::string GetTestDataPath(const std::string& dataKey)
	{
		if (!PrefFileExists(OCTAVE_PREF_FILE))
		{
			CreatePrefFile(OCTAVE_PREF_FILE);
			WriteOctavePrefsDefaults();
		}
		return GetStringFromPrefs(PrefFilePath(OCTAVE_PREF_FILE), OCTAVE_APP, dataKey, "");
	}

	std::string GetSignalTestDataPath()
	{
		return GetTestDataPath(PREF_SIGNAL_DATA);
	}

	std::string GetCoeffsTestDataPath()
	{
		return GetTestDataPath(PREF_COEFFS_DATA);
	}

	std::string GetOutputTestDataPath()
	{
		return GetTestDataPath(PREF_OUTPUT_DATA);
	}

	int GetTestSampleInputRate()
	{
		return GetIntFromPrefs(PrefFilePath(OCTAVE_PREF_FILE), OCTAVE_APP, PREF_SAMPLE_INPUT_RATE, 100);
	}

	int GetTestSampleOutputRate()
	{
		return GetIntFromPrefs(PrefFilePath(OCTAVE_PREF_FILE), OCTAVE_APP, PREF_SAMPLE_OUTPUT_RATE, 50);
	}

	int GetTestPolynomialOrder()
	{
		return GetIntFromPrefs(PrefFilePath(OCTAVE_PREF_FILE), OCTAVE_APP, PREF_POLYNOMIAL_ORDER, 7);
	}

}