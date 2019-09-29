#include <stdlib.h>
#include <iostream>
#include <libconfig.h++>
#include "elements.h"


std::vector<compound_t> load_compounds(const std::string &filepath) {
    libconfig::Config cfg;

    std::vector<compound_t> compounds;

    try {
        cfg.readFile(filepath.c_str());
    } catch( const libconfig::FileIOException& ex ) {
        std::cerr << "exception while read config file: " << ex.what();
        abort();
    } catch( const libconfig::ParseException& ex ) {
        std::cerr << "ParseException while parse config file: " << ex.what() << " line:" << ex.getLine();
        abort();
    }

    try {
        const auto& css = cfg.lookup("compounds");

        for(int i=0; i < css.getLength(); ++i) {
            auto& cs = css[i];
            auto& ess = cs.lookup("elements");

            compound_t c;
            c.formula = cs.lookup("formula").c_str();
            for(int j=0; j < ess.getLength();++j) {
                auto& es = ess[j];
                element_part_t e;
                e.formula = es.lookup("formula").c_str();
                e.weight = (float)es.lookup("weight");
                e.ignore = es.lookup("ignore");

                c.add_element(e);
            }
            compounds.push_back(c);
        }
    } catch( const libconfig::SettingNotFoundException& ex ) {
        std::cerr << "SettingNotFoundException while read config: " << ex.what() << ": " << ex.getPath();
        abort();
    } catch( const libconfig::SettingTypeException& ex ) {
        std::cerr << "SettingTypeException while read config: " << ex.what() << ": " << ex.getPath();
        abort();
    }

    return std::move(compounds);
}


std::vector<dosing_method_t> load_methods(const std::string &filepath) {
    libconfig::Config cfg;

    std::vector<dosing_method_t> methods;

    try {
        cfg.readFile(filepath.c_str());
    } catch( const libconfig::FileIOException& ex ) {
        std::cerr << "exception while read config file: " << ex.what();
        abort();
    } catch( const libconfig::ParseException& ex ) {
        std::cerr << "exception while parse config file: " << ex.what();
        abort();
    }

    try {
        const auto& mss = cfg.lookup("methods");

        for(int i=0; i < mss.getLength(); ++i) {
            auto& ms = mss[i];
            auto& ess = ms.lookup("elements");

            dosing_method_t m;
            m.name = ms.lookup("name").c_str();
            for(int j=0; j < ess.getLength();++j) {
                auto& es = ess[j];
                element_ppm_t e;
                e.formula = es.lookup("formula").c_str();
                e.ppm = es.lookup("ppm");

                m.elements.push_back(e);
            }
            methods.push_back(m);
        }
    } catch( const libconfig::SettingNotFoundException& ex ) {
        std::cerr << "Exception while parse config file: " << ex.what() << ": " << ex.getPath();
        abort();
    } catch( const libconfig::SettingTypeException& ex ) {
        std::cerr << "Exception while parse config file: " << ex.what() << ": " << ex.getPath();
        abort();
    }

    return std::move(methods);
}
