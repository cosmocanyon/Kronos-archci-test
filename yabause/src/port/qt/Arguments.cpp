#include "Arguments.h"

#include "VolatileSettings.h"
#include "QtYabause.h"

#include <iostream>
#include <iomanip>

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QStringListIterator>
#include <QVector>
#include <QVectorIterator>
#include <QFile>

namespace Arguments
{

	void autoframeskip(const QString& param);
	void autoload(const QString& param);
	void autostart(const QString& param);
	void binary(const QString& param);
	void bios(const QString& param);
	void syslangid(const QString& param);
	void cdrom(const QString& param);
	void fullscreen(const QString& param);
	void help(const QString& param);
	void iso(const QString& param);
	void nobios(const QString& param);
	void nosound(const QString& param);
	void version(const QString& param);

	struct Option
	{
		const char * shortname;
		const char * longname;
		const char * parameter;
		const char * description;
		unsigned short priority;
		void (*callback)(const QString& param);
	};

	static std::vector<Option> const availableOptions =
	{
		{ NULL,  "--autoframeskip=", "0|1", "Enable or disable vertical synchronization.",  2, autoframeskip },
		{ NULL,  "--autoload=", "<SAVESTATE>", "Automatically start emulation and load a save state.",1, autoload },
		{ "-a",  "--autostart", NULL,       "Automatically start emulation.",                      1, autostart },
		{ NULL,  "--binary=", "<FILE>[:ADDRESS]", "Use a binary file.",                           1, binary },
		{ "-b",  "--bios=", "<BIOS>",       "Choose a bios file.",                                3, bios },
		{ "-l",  "--language=", "<language>","Choose the system language: english, deutsch, french, spanish, italian, japanese",                     7, syslangid },
		{ "-c",  "--cdrom=", "<CDROM>",     "Choose the cdrom device.",                           4, cdrom },
		{ "-f",  "--fullscreen", NULL,      "Start the emulator in fullscreen.",                  5, fullscreen },
		{ "-h",  "--help", NULL,            "Show this help and exit.",                           0, help },
		{ "-i",  "--iso=", "<ISO>",         "Choose a dump image file. supports i.e. .cue, .iso, .zip", 4, iso },
        { "-nb", "--no-bios", NULL,         "Use the emulated bios.",                              3, nobios },
        { "-ns", "--no-sound", NULL,        "Turns sound off.",                                   6, nosound },
		{ "-v",  "--version", NULL,         "Show version and exit.",                             0, version },
	};

	void parse()
	{
		QVector<Option const *> choosenOptions(static_cast<int>(availableOptions.size()));
		QVector<QString> params(static_cast<int>(availableOptions.size()));

		QStringList const arguments = QApplication::arguments();
		QStringListIterator argit(arguments);

		while(argit.hasNext())
		{
			bool isFirst = !argit.hasPrevious();
			QString const & argument = argit.next();

			//if its a file its probably a game file so if so use it as such
			if(QFile::exists(argument))
			{
				if (!isFirst)
				{
					Option const & autoStartOption = *std::find_if(availableOptions.begin(), availableOptions.end(), [](Option const & e)
					{
						return e.shortname && strcmp(e.shortname, "-a") == 0;
					});
					choosenOptions[autoStartOption.priority] = &autoStartOption;
					Option const & imageFileOption = *std::find_if(availableOptions.begin(), availableOptions.end(), [](Option const & e)
					{
						return e.shortname && strcmp(e.shortname, "-i") == 0;
					});
					choosenOptions[imageFileOption.priority] = &imageFileOption;
					params[imageFileOption.priority] = argument;
				}
			}

			for(Option const & option : availableOptions)
			{
				if (argument == option.shortname)
				{
					choosenOptions[option.priority] = &option;
					if (option.parameter)
						params[option.priority] = argit.next();
				}
				if (argument.startsWith(option.longname))
				{
					choosenOptions[option.priority] = &option;
					if (option.parameter)
						params[option.priority] = argument.mid((int)strlen(option.longname));
				}
			}
		}

		for(int i = 0; i < availableOptions.size(); i++)
		{
			Option const * option = choosenOptions[i];
			if (option)
				if (option->parameter)
					option->callback(params[option->priority]);
				else
					option->callback(QString());
		}
	}

	void autoframeskip(const QString& param)
	{
		VolatileSettings * vs = QtYabause::volatileSettings();
		vs->setValue("General/EnableFrameSkipLimiter", param);
	}

	void autoload(const QString& param)
	{
		VolatileSettings * vs = QtYabause::volatileSettings();
		vs->setValue("autorun", true);
		vs->setValue("autorun/load", true);
		vs->setValue("autorun/load/slot", param.toInt());
	}

	void autostart(const QString& param)
	{
		VolatileSettings * vs = QtYabause::volatileSettings();
		vs->setValue("autorun", true);
	}

	void binary(const QString& param)
	{
		QString filename;
		uint address = 0x06004000;
		QStringList parts = param.split(':');
		filename = parts[0];
		if (parts.size() > 1)
			address = parts[1].toUInt(NULL, 0);

		VolatileSettings * vs = QtYabause::volatileSettings();
		vs->setValue("autostart", true);
	}

	void bios(const QString& param)
	{
		VolatileSettings * vs = QtYabause::volatileSettings();
		vs->setValue("General/Bios", param);
	}

	void biosSettings(const QString &param)
	{
		VolatileSettings * vs = QtYabause::volatileSettings();
		vs->setValue("General/BiosSettings", param);
	}

	void syslangid(const QString& param)
	{
		VolatileSettings * vs = QtYabause::volatileSettings();
		if (param.toLower() == "english") { vs->setValue("General/SystemLanguageID", 0); }
		if (param.toLower() == "deutsch") { vs->setValue("General/SystemLanguageID", 1); }
		if (param.toLower() == "french") { vs->setValue("General/SystemLanguageID", 2); }
		if (param.toLower() == "spanish") { vs->setValue("General/SystemLanguageID", 3); }
		if (param.toLower() == "italian") { vs->setValue("General/SystemLanguageID", 4); }
		if (param.toLower() == "japanese") { vs->setValue("General/SystemLanguageID", 5); }
	}

	void cdrom(const QString& param)
	{
		VolatileSettings * vs = QtYabause::volatileSettings();
		vs->setValue("General/CdRom", CDCORE_ARCH);
		vs->setValue("General/CdRomISO", param);
		QtYabause::updateTitle();
	}

	void fullscreen(const QString& param)
	{
		VolatileSettings * vs = QtYabause::volatileSettings();
		vs->setValue("Video/Fullscreen", true);
	}

	void help(const QString& param)
	{
		std::cout << std::endl << "Kronos commands:" << std::endl;

		for(Option const & option : availableOptions)
		{
			QString longandparam(option.longname);
			if (option.parameter)
				longandparam.append(option.parameter);

			if (option.shortname)
			{
				std::cout << std::setw(5) << std::right << option.shortname << ", ";
			}
			else
			{
				std::cout << std::setw(7) << ' ';
			}
			std::cout << std::setw(27) << std::left << longandparam.toLocal8Bit().constData()
				<< option.description
				<< std::endl;
		}

		exit(0);
	}

	void iso(const QString& param)
	{
		VolatileSettings * vs = QtYabause::volatileSettings();
		vs->setValue( "General/CdRom", CDCORE_ISO );
		vs->setValue( "General/CdRomISO", param );
	}

	void nobios(const QString& param)
	{
		VolatileSettings * vs = QtYabause::volatileSettings();
		vs->setValue("General/EnableEmulatedBios", true);
	}

	void nosound(const QString& param)
	{
		VolatileSettings * vs = QtYabause::volatileSettings();
		vs->setValue("Sound/SoundCore", SNDCORE_DUMMY);
	}

	void version(const QString& param)
	{
		std::cout << VERSION << std::endl;

		exit(0);
	}

}
