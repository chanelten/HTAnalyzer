#ifndef HT_SIMULATION_DATA_GENERATOR
#define HT_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class HTAnalyzerSettings;

class HTSimulationDataGenerator
{
public:
	HTSimulationDataGenerator();
	~HTSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, HTAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	HTAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateSerialByte();
	std::string mSerialText;
	U32 mStringIndex;

	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //HT_SIMULATION_DATA_GENERATOR