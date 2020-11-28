#ifndef HT_ANALYZER_SETTINGS
#define HT_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class HTAnalyzerSettings : public AnalyzerSettings
{
public:
	HTAnalyzerSettings();
	virtual ~HTAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mMosiChannel;
	Channel mMisoChannel;
	U32 mBitRate;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mMosiChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mMisoChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
};

#endif //HT_ANALYZER_SETTINGS
