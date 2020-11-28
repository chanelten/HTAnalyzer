#ifndef HTANALYZER_ANALYZER_SETTINGS
#define HTANALYZER_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class HTAnalyzerAnalyzerSettings : public AnalyzerSettings
{
public:
	HTAnalyzerAnalyzerSettings();
	virtual ~HTAnalyzerAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	U32 mBitRate;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
};

#endif //HTANALYZER_ANALYZER_SETTINGS
