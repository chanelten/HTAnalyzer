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

private:
	virtual U8 ReadByte(AnalyzerChannelData *serial, bool advance_serial, Channel& channel, U32 samples_per_bit, U32 samples_to_first_center_of_first_data_bit, U64 *starting_sample);
	virtual bool DetectComReq(bool no_peek);
	virtual void SyncSerials();
	virtual AnalyzerChannelData* NextChannelEdge();
	virtual void AddFrame(const Frame &f);
	virtual U8 *GetChannelFrame(int *frame_length, AnalyzerChannelData* channel, U32 samples_per_bit, U32 samples_to_first_center_of_first_data_bit, U8 *keys);
	virtual void AddGenericDataFrame(const U8 data, const U64 starting_sample, const U64 ending_sample);
	virtual bool FrameToCredentials(U8 *frame, int frame_len, U8 *credentials, U8 *key);
	virtual void AddACKFrame(const U8 data, const U64 starting_sample, const U64 ending_sample);
	virtual bool GetAckFromChannel(AnalyzerChannelData *channel, Channel& channel_settings, U32 samples_per_bit, U32 samples_to_first_center_of_first_data_bit);

protected: //vars
	std::auto_ptr< HTAnalyzerSettings > mSettings;
	std::auto_ptr< HTAnalyzerResults > mResults;
	AnalyzerChannelData* mMosiSerial;
	AnalyzerChannelData* mMisoSerial;

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
