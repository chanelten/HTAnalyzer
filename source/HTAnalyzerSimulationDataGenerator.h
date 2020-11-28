#ifndef HTANALYZER_SIMULATION_DATA_GENERATOR
#define HTANALYZER_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class HTAnalyzerAnalyzerSettings;

class HTAnalyzerSimulationDataGenerator
{
public:
	HTAnalyzerSimulationDataGenerator();
	~HTAnalyzerSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, HTAnalyzerAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	HTAnalyzerAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateSerialByte();
	std::string mSerialText;
	U32 mStringIndex;

	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //HTANALYZER_SIMULATION_DATA_GENERATOR