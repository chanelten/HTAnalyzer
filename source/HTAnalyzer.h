#ifndef HT_ANALYZER_H
#define HT_ANALYZER_H

#include <Analyzer.h>
#include "HTAnalyzerResults.h"
#include "HTSimulationDataGenerator.h"

class HTAnalyzerSettings;
class ANALYZER_EXPORT HTAnalyzer : public Analyzer2
{
public:
	HTAnalyzer();
	virtual ~HTAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::auto_ptr< HTAnalyzerSettings > mSettings;
	std::auto_ptr< HTAnalyzerResults > mResults;
	AnalyzerChannelData* mMosiSerial;

	HTSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //HT_ANALYZER_H
