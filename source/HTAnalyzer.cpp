#include "HTAnalyzer.h"
#include "HTAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

HTAnalyzer::HTAnalyzer()
:	Analyzer2(),  
	mSettings( new HTAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

HTAnalyzer::~HTAnalyzer()
{
	KillThread();
}

void HTAnalyzer::SetupResults()
{
	mResults.reset( new HTAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mMosiChannel );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mMisoChannel );
}

U8 HTAnalyzer::ReadByte(AnalyzerChannelData *serial, bool advance_serial, Channel& channel, U32 samples_per_bit, U32 samples_to_first_center_of_first_data_bit, U64 *starting_sample)
{
	U8 data = 0;
	U8 mask = 1;

	if(advance_serial)
	{
		serial->AdvanceToNextEdge();
	}

	mResults->AddMarker( serial->GetSampleNumber(), AnalyzerResults::Start, channel);
	if(starting_sample != NULL)
	{
		*starting_sample = serial->GetSampleNumber();
	}

	serial->Advance( samples_to_first_center_of_first_data_bit );

	for( U32 i=0; i<8; i++ )
	{
		//let's put a dot exactly where we sample this bit:
		mResults->AddMarker( serial->GetSampleNumber(), AnalyzerResults::Dot, channel );

		if( serial->GetBitState() == BIT_HIGH )
			data |= mask;

		serial->Advance( samples_per_bit );

		mask = mask << 1;
	}
	mResults->AddMarker( serial->GetSampleNumber(), AnalyzerResults::Stop, channel );
	serial->Advance( samples_per_bit );

	return data;
}

void HTAnalyzer::SyncSerials()
{
	// Sync mosi to miso
	U64 miso_sample_number = mMisoSerial->GetSampleNumber();
	mMosiSerial->AdvanceToAbsPosition(miso_sample_number);
}

bool HTAnalyzer::DetectComReq(bool no_peek)
{
	if(!no_peek && NextChannelEdge() == mMosiSerial)
	{
		return false;
	}
	if(no_peek)
	{
		mMisoSerial->Advance(1);
		if(mMisoSerial->GetBitState() == BIT_HIGH)
		{
			mMisoSerial->AdvanceToNextEdge();
		}
	}
	SyncSerials();
	U32 min_samples_for_com_req = U32(0.18 * double(mSampleRateHz)); // 180ms
	U32 max_samples_for_com_req = U32(0.22 * double(mSampleRateHz)); // 220ms
	U32 measure_width = mMisoSerial->GetSampleOfNextEdge() - mMisoSerial->GetSampleNumber(); // end - start
	if(measure_width > min_samples_for_com_req && measure_width < max_samples_for_com_req)
	{
		mResults->AddMarker( mMisoSerial->GetSampleNumber(), AnalyzerResults::DownArrow, mSettings->mMisoChannel );
		// also go to the next sample
		mMisoSerial->AdvanceToNextEdge();
		mResults->AddMarker( mMisoSerial->GetSampleNumber(), AnalyzerResults::UpArrow, mSettings->mMisoChannel );
		SyncSerials();
		return true;
	}
	return false;
}

AnalyzerChannelData* HTAnalyzer::NextChannelEdge()
{
	U64 next_miso_edge = mMisoSerial->GetSampleOfNextEdge();
	U64 next_mosi_edge = mMosiSerial->GetSampleOfNextEdge();
	if((next_miso_edge < next_mosi_edge) && (next_miso_edge > 0))
	{
		return mMisoSerial;
	}
	else if(next_mosi_edge > 0)
	{
		return mMosiSerial;
	}
	return NULL;
}

void HTAnalyzer::AddFrame(const Frame &f)
{
	mResults->AddFrame( f );
	mResults->CommitResults();
	ReportProgress( f.mEndingSampleInclusive );
}

void HTAnalyzer::AddGenericDataFrame(const U8 data, const U64 starting_sample, const U64 ending_sample)
{
	Frame frame;
	frame.mStartingSampleInclusive = starting_sample;
	frame.mEndingSampleInclusive = ending_sample;
	frame.mData1 = data;
	AddFrame(frame);
}


#define ACK (U8)(0x11)
#define NAK (U8)(0xEE)
#define STX (U8)(0xF2)

void HTAnalyzer::AddACKFrame(const U8 data, const U64 starting_sample, const U64 ending_sample)
{
	Frame frame;
	frame.mStartingSampleInclusive = starting_sample;
	frame.mEndingSampleInclusive = ending_sample;
	frame.mData1 = data;
	AddFrame(frame);
}

#define HT_CREDENTIALS_LEN 20
#define HT_KEYS_LEN 5
#define TEMP_BUFFER_LEN 1024
#define bcdToDecimal(x) (U8)((x) - (((x) >> 4) * 6))
U8 *HTAnalyzer::GetChannelFrame(int *frame_length, AnalyzerChannelData* channel, U32 samples_per_bit, U32 samples_to_first_center_of_first_data_bit, U8 *keys)
{
	U8 temp[TEMP_BUFFER_LEN]; // temp buffer
	U8 data;
	int len = 0;

	enum {
		PHASE_STX,
		PHASE_LEN_MSB,
		PHASE_LEN_LSB,
		PHASE_DATA,
		PHASE_CHECKSUM,
		PHASE_END
	} phase = PHASE_STX;
	U16 remaining_frame_length = 0;
	U8 checksum = 0;

	while(phase != PHASE_END)
	{
		if(NextChannelEdge() != channel)
		{
			mResults->AddMarker( channel->GetSampleNumber(), AnalyzerResults::ErrorX, (channel == mMisoSerial) ? (mSettings->mMisoChannel) : (mSettings->mMosiChannel) );
			break;
		}
		U64 starting_sample;
		data = ReadByte(channel, true, (channel == mMisoSerial) ? (mSettings->mMisoChannel) : (mSettings->mMosiChannel) , samples_per_bit, samples_to_first_center_of_first_data_bit, &starting_sample);
		if(phase == PHASE_STX)
		{
			if(data == 0xF2)
			{
				phase = PHASE_LEN_MSB;
				continue;
			}
			break;
		}
		if(phase == PHASE_LEN_MSB)
		{
			remaining_frame_length = bcdToDecimal(data) * 100;
			phase = PHASE_LEN_LSB;
			checksum = 0;
			checksum ^= data;
			continue;
		}
		if(phase == PHASE_LEN_LSB)
		{
			remaining_frame_length += bcdToDecimal(data);
			if(remaining_frame_length > TEMP_BUFFER_LEN)
			{
				mResults->AddMarker( channel->GetSampleNumber(), AnalyzerResults::ErrorX, (channel == mMisoSerial) ? (mSettings->mMisoChannel) : (mSettings->mMosiChannel) );
				break;
			}
			phase = PHASE_DATA;
			checksum ^= data;
			continue;
		}
		if(phase == PHASE_DATA)
		{
			temp[len] = data;
			len++;
			remaining_frame_length--;
			checksum ^= data;
			if(remaining_frame_length == 0)
			{
				phase = PHASE_CHECKSUM;
			}
			if(keys != NULL)
			{
				if((len > 1) && (remaining_frame_length > 0))
				{
					int idx = len - 2;
					data = data ^ keys[idx % HT_KEYS_LEN];
					AddGenericDataFrame(data, starting_sample, channel->GetSampleNumber());
				}
			} else
			{
				AddGenericDataFrame(data, starting_sample, channel->GetSampleNumber());
			}
			continue;
		}
		if(phase == PHASE_CHECKSUM)
		{
			if(checksum != data)
			{
				mResults->AddMarker( channel->GetSampleNumber(), AnalyzerResults::ErrorX, (channel == mMisoSerial) ? (mSettings->mMisoChannel) : (mSettings->mMosiChannel) );
				break;
			}
			phase = PHASE_END;
			//AddGenericDataFrame(data, starting_sample, channel->GetSampleNumber());
		}
	}

	if(phase != PHASE_END)
	{
		return NULL;
	}

	U8 *result = (U8 *)malloc(len);
	memcpy(result, temp, len);
	*frame_length = len;
	return result;
}

bool HTAnalyzer::FrameToCredentials(U8 *frame, int frame_len, U8 *credentials, U8 *key)
{
	static const U8 key_mask[5] = { 0x0d, 0x0b, 0x00, 0x06, 0x10 };
	static const U8 key_check[5] = { 0x0d, 0x0d, 0x00, 0x06, 0x10 };
	if(frame_len != HT_CREDENTIALS_LEN)
	{
		return false;
	}
	for(int i = 0; i < HT_CREDENTIALS_LEN; i++)
	{
		if(i < HT_KEYS_LEN)
		{
			key[i] = (frame)[i] ^ key_mask[i];
		}
		credentials[i] = (frame)[i] ^ key[i % HT_KEYS_LEN];
	}
	if (memcmp(credentials + 10, key_check, 5) == 0)
	{
		return true;
	}
	return false;
}

bool HTAnalyzer::GetAckFromChannel(AnalyzerChannelData *channel, Channel& channel_settings, U32 samples_per_bit, U32 samples_to_first_center_of_first_data_bit)
{
#if 0
	if(NextChannelEdge() != channel)
	{
		mResults->AddMarker( channel->GetSampleNumber(), AnalyzerResults::ErrorX, channel_settings );
		return false;
	}
#endif
	U64 starting_sample;
	U8 data = ReadByte(channel, true, channel_settings, samples_per_bit, samples_to_first_center_of_first_data_bit, &starting_sample);
	AddACKFrame(data, starting_sample, channel->GetSampleNumber());
	if(data != ACK)
	{
		mResults->AddMarker( channel->GetSampleNumber(), AnalyzerResults::ErrorX, channel_settings);
		return false;
	}
	return true;
}

void HTAnalyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();
	U8 credentials[HT_CREDENTIALS_LEN];
	U8 keys[HT_KEYS_LEN];

	mMosiSerial = GetAnalyzerChannelData( mSettings->mMosiChannel );
	mMisoSerial = GetAnalyzerChannelData( mSettings->mMisoChannel );

	mMisoSerial->AdvanceToNextEdge();
	if( mMisoSerial->GetBitState() == BIT_HIGH )
	{
		mMisoSerial->AdvanceToNextEdge();
	}
	SyncSerials();

	U32 samples_per_bit = mSampleRateHz / mSettings->mBitRate;
	U32 samples_to_first_center_of_first_data_bit = U32( 1.5 * double( mSampleRateHz ) / double( mSettings->mBitRate ) );

	for( ; ; )
	{

		while(!DetectComReq(true))
		{
			mMisoSerial->AdvanceToNextEdge();
			mMisoSerial->AdvanceToNextEdge();
		}

		U8 data;
		U64 starting_sample;
		int cred_frame_len;
		U8 *cred_frame = NULL;
		int req_frame_len;
		U8 *req_frame = NULL;
		// looking now for an ACK. (MOSI) ---------------
		if(!GetAckFromChannel(mMosiSerial, mSettings->mMosiChannel, samples_per_bit, samples_to_first_center_of_first_data_bit))
		{
			continue;
		}

		// looking now for a credentials frame! (MISO) --------

		if(NextChannelEdge() != mMisoSerial)
		{
			mResults->AddMarker( mMosiSerial->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mMosiChannel );
			continue;
		}

		cred_frame_len = 0;
		cred_frame = GetChannelFrame(&cred_frame_len, mMisoSerial, samples_per_bit, samples_to_first_center_of_first_data_bit, NULL);
		if(cred_frame == NULL)
		{
			continue;
		}
		bool has_credentials = FrameToCredentials(cred_frame + 1, cred_frame_len - 2, credentials, keys);
		free(cred_frame);
		cred_frame = NULL;
		if(!has_credentials)
		{
			mResults->AddMarker( mMisoSerial->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mMisoChannel );
			continue;
		}

		// looking now for an ACK. (MOSI) ---------------
		if(!GetAckFromChannel(mMosiSerial, mSettings->mMosiChannel, samples_per_bit, samples_to_first_center_of_first_data_bit))
		{
			continue;
		}

		// looking for the IR Request now. (MISO) --------------------
		if(NextChannelEdge() != mMisoSerial)
		{
			mResults->AddMarker( mMosiSerial->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mMosiChannel );
			continue;
		}
		req_frame_len = 0;
		req_frame = GetChannelFrame(&req_frame_len, mMisoSerial, samples_per_bit, samples_to_first_center_of_first_data_bit, (U8 *)keys);
		if(req_frame == NULL)
		{
			mResults->AddMarker( mMisoSerial->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mMisoChannel );
			continue;
		}

		// looking now for an ACK. (MOSI) ---------------
		if(!GetAckFromChannel(mMosiSerial, mSettings->mMosiChannel, samples_per_bit, samples_to_first_center_of_first_data_bit))
		{
			continue;
		}

		// 
		if(NextChannelEdge() == mMisoSerial)
		{
			req_frame_len = 0;
			req_frame = GetChannelFrame(&req_frame_len, mMisoSerial, samples_per_bit, samples_to_first_center_of_first_data_bit, (U8 *)keys);
			if(req_frame == NULL)
			{
				mResults->AddMarker( mMisoSerial->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mMisoChannel );
				continue;
			}
			free(req_frame);
			req_frame = NULL;
			// looking now for an ACK. (MOSI) ---------------
			if(!GetAckFromChannel(mMosiSerial, mSettings->mMosiChannel, samples_per_bit, samples_to_first_center_of_first_data_bit))
			{
				continue;
			}
		}
		else
		{
			req_frame_len = 0;
			req_frame = GetChannelFrame(&req_frame_len, mMosiSerial, samples_per_bit, samples_to_first_center_of_first_data_bit, (U8 *)keys);
			if(req_frame == NULL)
			{
				mResults->AddMarker( mMosiSerial->GetSampleNumber(), AnalyzerResults::ErrorX, mSettings->mMosiChannel );
				continue;
			}
			free(req_frame);
			req_frame = NULL;
			mResults->AddMarker( mMisoSerial->GetSampleNumber(), AnalyzerResults::Start, mSettings->mMisoChannel );
			// looking now for an ACK. (MISO) ---------------
			if(!GetAckFromChannel(mMisoSerial, mSettings->mMisoChannel, samples_per_bit, samples_to_first_center_of_first_data_bit))
			{
				continue;
			}
		}
		

	}
}

bool HTAnalyzer::NeedsRerun()
{
	return false;
}

U32 HTAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 HTAnalyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 4;
}

const char* HTAnalyzer::GetAnalyzerName() const
{
	return "Handy Terminal Analyzer";
}

const char* GetAnalyzerName()
{
	return "Handy Terminal Analyzer";
}

Analyzer* CreateAnalyzer()
{
	return new HTAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}