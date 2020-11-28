#ifndef HTANALYZER_ANALYZER_H
#define HTANALYZER_ANALYZER_H

#include <Analyzer.h>
#include "HTAnalyzerAnalyzerResults.h"
#include "HTAnalyzerSimulationDataGenerator.h"

class HTAnalyzerAnalyzerSettings;
class ANALYZER_EXPORT HTAnalyzerAnalyzer : public Analyzer2
{
public:
	HTAnalyzerAnalyzer();
	virtual ~HTAnalyzerAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::auto_ptr< HTAnalyzerAnalyzerSettings > mSettings;
	std::auto_ptr< HTAnalyzerAnalyzerResults > mResults;
	AnalyzerChannelData* mSerial;

	HTAnalyzerSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //HTANALYZER_ANALYZER_H