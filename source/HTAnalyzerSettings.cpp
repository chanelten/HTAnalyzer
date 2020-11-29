#include "HTAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


HTAnalyzerSettings::HTAnalyzerSettings()
:	mMosiChannel( UNDEFINED_CHANNEL ),
	mBitRate( 4800 )
{
	mMosiChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mMosiChannelInterface->SetTitleAndTooltip( "HT MOSI", "HT Main Controller output" );
	mMosiChannelInterface->SetChannel( mMosiChannel );
	mMisoChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mMisoChannelInterface->SetTitleAndTooltip( "HT MISO", "HT IR output" );
	mMisoChannelInterface->SetChannel( mMisoChannel );

	mBitRateInterface.reset( new AnalyzerSettingInterfaceInteger() );
	mBitRateInterface->SetTitleAndTooltip( "Bit Rate (Bits/S)",  "Specify the bit rate in bits per second." );
	mBitRateInterface->SetMax( 6000000 );
	mBitRateInterface->SetMin( 1 );
	mBitRateInterface->SetInteger( mBitRate );

	AddInterface( mMosiChannelInterface.get() );
	AddInterface( mMisoChannelInterface.get() );
	//AddInterface( mBitRateInterface.get() ); // 4800 forever :D

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mMosiChannel, "MOSI", false );
	AddChannel( mMisoChannel, "MISO", false );
}

HTAnalyzerSettings::~HTAnalyzerSettings()
{
}

bool HTAnalyzerSettings::SetSettingsFromInterfaces()
{
	mMosiChannel = mMosiChannelInterface->GetChannel();
	mMisoChannel = mMisoChannelInterface->GetChannel();
	mBitRate = mBitRateInterface->GetInteger();

	ClearChannels();
	AddChannel( mMosiChannel, "HT MOSI Channel", true );
	AddChannel( mMisoChannel, "HT MISO Channel", true );

	return true;
}

void HTAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mMosiChannelInterface->SetChannel( mMosiChannel );
	mMisoChannelInterface->SetChannel( mMisoChannel );
	mBitRateInterface->SetInteger( mBitRate );
}

void HTAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mMosiChannel;
	text_archive >> mMisoChannel;
	text_archive >> mBitRate;

	ClearChannels();
	AddChannel( mMosiChannel, "HT MOSI Channel", true );
	AddChannel( mMisoChannel, "HT MISO Channel", true );

	UpdateInterfacesFromSettings();
}

const char* HTAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mMosiChannel;
	text_archive << mMisoChannel;
	text_archive << mBitRate;

	return SetReturnString( text_archive.GetString() );
}
