
#include <QSurfaceFormat>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <cstdlib>

#include "global.h"
#include "config.h"
#include "logger.h"
#include "configurationmanager.h"

#include "atsdb.h"
#include "client.h"
#include "mainwindow.h"
#include "files.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include <osgDB/Registry>
#include "cpl_conv.h"
#endif

using namespace Utils;

namespace po = boost::program_options;

int main (int argc, char **argv)
{
    bool fuse = false;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        //("compression", po::value<int>(), "set compression level")
        ("fuse,f", po::bool_switch(&fuse), "assume fuse environment")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }


    // check if basic configuration works
    try
    {
        std::cout << "ATSDBClient: setting directory paths" << std::endl;

        std::string system_install_path = SYSTEM_INSTALL_PATH;

#if USE_EXPERIMENTAL_SOURCE == true
        std::cout <<"ATSDBClient: includes experimental features" << std::endl;

        if (fuse)
        {
            const char* appdir = std::getenv("APPDIR");
            assert (appdir);

            //std::cout << "ATSDBClient: assuming fuse environment in " << appdir << std::endl;

            system_install_path = std::string(appdir) + "/appdir/atsdb";

            std::cout << "ATSDBClient: set install path to '" << system_install_path << "'" << std::endl;
            assert (Files::directoryExists(system_install_path));


            osgDB::FilePathList path_list;

            path_list.push_back("$ORIGIN/appdir/lib");
            path_list.push_back("$ORIGIN/lib");
            path_list.push_back("appdir/lib");

            osgDB::Registry::instance()->setLibraryFilePathList(std::string(appdir) + "/appdir/lib");

            CPLSetConfigOption( "GDAL_DATA", "appdir/atsdb/data/gdal");
        }
#endif

        std::cout << "ATSDBClient: checking if local configuration exists ... ";

        if (!Files::directoryExists(HOME_SUBDIRECTORY))
        {
            std::cout << " no" << std::endl;

            if (!Files::directoryExists(system_install_path))
            {
                std::cerr << "ATSDBClient: unable to locate system installation files at '" << system_install_path
                          << "'" << std::endl;
                return -1;
            }
            std::cout << "ATSDBClient: copying files from system installation from '" << system_install_path
                      << "' to '" << HOME_SUBDIRECTORY <<  "' ... ";
            if (!Files::copyRecursively(system_install_path, HOME_SUBDIRECTORY))
            {
                std::cout << " failed" << std::endl;
                return -1;
            }
            std::cout << " done" << std::endl;
        }
        else
        {
            std::cout << " yes" << std::endl;
        }

        std::cout << "ATSDBClient: opening simple config file at '" << HOME_CONF_DIRECTORY+"main.conf'" << std::endl;

        SimpleConfig config ("main.conf");
        assert (config.existsId("version"));
        assert (config.existsId("configuration_path"));
        assert (config.existsId("save_config_on_exit"));
        assert (config.existsId("log_properties_file"));
        assert (config.existsId("save_config_on_exit"));

        CURRENT_CONF_DIRECTORY = HOME_CONF_DIRECTORY+config.getString("configuration_path")+"/";

        std::cout << "ATSDBClient: current configuration path is '" << CURRENT_CONF_DIRECTORY+"'" << std::endl;

        std::string log_config_path = HOME_CONF_DIRECTORY+config.getString("log_properties_file");
        Files::verifyFileExists(log_config_path);

        std::cout << "ATSDBClient: initializing logger using '" << log_config_path << "'" << std::endl;
        Logger::getInstance().init(log_config_path);

        loginf << "ATSDBClient: startup version " << VERSION;
        std::string config_version = config.getString("version");
        loginf << "ATSDBClient: configuration version " << config_version;

        if (config.existsId("enable_multisampling") && config.getBool("enable_multisampling")
                && config.existsId("multisampling"))
        {
            unsigned int num_samples = config.getUnsignedInt("multisampling");
            loginf << "ATSDBClient: enabling multisampling with " << num_samples << " samples";
            //TODO
//            QSurfaceFormat fmt;
//            fmt.setSamples(num_samples);
//            QSurfaceFormat::setDefaultFormat(fmt);
        }

        ConfigurationManager::getInstance().init (config.getString("main_configuration_file"));
    }
    catch (std::exception &ex)
    {
        logerr  << "Main: Caught Exception '" << ex.what() << "'";
        logerr.flush();

        return -1;
    }
    catch(...)
    {
        logerr  << "Main: Caught Exception";
        logerr.flush();

        return -1;
    }

    // real atsdb stuff
    try
    {
        ATSDB::instance().initialize();

        Client mf(argc, argv);

        MainWindow window;

        window.show();

        return mf.exec();

    }
    catch (std::exception &ex)
    {
        logerr  << "Main: Caught Exception '" << ex.what() << "'";
        logerr.flush();

        if (ATSDB::instance().ready())
            ATSDB::instance().shutdown();

        return -1;
    }
    catch(...)
    {
        logerr  << "Main: Caught Exception";
        logerr.flush();

        if (ATSDB::instance().ready())
            ATSDB::instance().shutdown();

        return -1;
    }
    loginf << "Main: Shutdown";
    return 0;
}
