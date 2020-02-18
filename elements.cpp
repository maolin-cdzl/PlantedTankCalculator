#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <libconfig.h++>
#include "elements.h"
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

float roundf3(float x) {
    return std::round(x * 100000.0) / 100000.0;
}

std::vector<compound_t> load_compounds_from_string(const QString& content) {
    libconfig::Config cfg;

    std::vector<compound_t> compounds;

    do {
        try {
            cfg.readString(content.toStdString().c_str());
        } catch( const libconfig::ParseException& ex ) {
            std::cerr << "ParseException while parse config file: " << ex.what() << " line:" << ex.getLine();
            break;
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
            compounds.clear();
            break;
        } catch( const libconfig::SettingTypeException& ex ) {
            std::cerr << "SettingTypeException while read config: " << ex.what() << ": " << ex.getPath();
            compounds.clear();
            break;
        }
    } while(0);

    return std::move(compounds);
}



std::vector<compound_t> load_compounds(const std::string &filepath) {
    QFile file(filepath.c_str());
    if( file.open(QFile::ReadOnly | QFile::Text)) {
        QString content;
        while( !file.atEnd() ) {
            content.append( file.readLine() );
        }
        file.close();
        return load_compounds_from_string(content);
    }
    return std::vector<compound_t>();
}

std::vector<dosing_method_t> load_methods_from_string(const QString& content) {
    libconfig::Config cfg;

    std::vector<dosing_method_t> methods;

    do {
        try {
            cfg.readString(content.toStdString().c_str());
        } catch( const libconfig::ParseException& ex ) {
            std::cerr << "exception while parse config file: " << ex.what();
            break;
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
            methods.clear();
            break;
        } catch( const libconfig::SettingTypeException& ex ) {
            std::cerr << "Exception while parse config file: " << ex.what() << ": " << ex.getPath();
            methods.clear();
            break;
        }
    } while(0);
    return std::move(methods);
}

std::vector<dosing_method_t> load_methods(const std::string &filepath) {
    QFile file(filepath.c_str());
    if( file.open(QFile::ReadOnly | QFile::Text)) {
        QString content;
        while( !file.atEnd() ) {
            content.append( file.readLine() );
        }
        file.close();
        return load_methods_from_string(content);
    }
    return std::vector<dosing_method_t>();
}

//#ifdef WIN32

QString get_home_directory() {
    QString homedir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    qDebug() << "homedir: " << homedir.toLocal8Bit();
    return homedir;
}

/*
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

QString get_home_directory() {
    struct passwd *pw = getpwuid(getuid());

    QString homedir = pw->pw_dir;
    return std::move(homedir);
}
#endif
*/

QString get_ptc_directory() {
    QString ptc;
    do {
        QDir dir;
        if( ! dir.exists(get_home_directory()) ) break;
        ptc = get_home_directory() + QDir::separator() + ".plantedtankcalculator";
        qDebug() << "ptc: " << ptc;
        if( ! dir.exists(ptc) ) {
            qDebug() << "create ptc";
            if( ! dir.mkdir(ptc) ) {
                qDebug() << "create ptc directory failed";
                ptc.clear();
                break;
            }
        }
    } while(0);

    return ptc;
}

QString get_ptc_sub_directory(const QString& sub) {
    QString subdir;
    do {
        QString ptc = get_ptc_directory();
        if( ptc.isEmpty() ) break;
        subdir = ptc + QDir::separator() + sub;
        QDir dir;
        if( ! dir.exists(subdir) ) {
            if( ! dir.mkdir(subdir) ) {
                subdir.clear();
                break;
            }
        }
    } while(0);

    return subdir;
}

QString get_ptc_configure() {
    QString filepath;
    do {
        filepath = get_ptc_directory();
        if( filepath.isEmpty() ) break;
        filepath = filepath + QDir::separator() + "ptc.cfg";

        QFile file(filepath);
        if( !file.exists() ) {
            if( !reset_ptc_configure() ) {
                filepath.clear();
                break;
            }
        }
    } while(0);
    return filepath;
}

bool reset_ptc_configure() {
    do {
        QString filepath = get_ptc_directory();
        if( filepath.isEmpty() ) break;
        filepath = filepath + QDir::separator() + "ptc.cfg";

        QString content;
        QFile res(":/ptc.cfg");
        if( ! res.open( QFile::ReadOnly | QFile::Text ) ) break;
        while( ! res.atEnd() ) {
            content += res.readLine();
        }
        res.close();

        QFile file(filepath);
        if( !file.open(QFile::WriteOnly | QFile::Text ) ) break;
        file.write(content.toUtf8());
        file.close();

        return true;
    } while(0);
    return false;
}

QStringList get_solution_list() {
    QStringList solutions;
    do {
        QString solutiondir = get_ptc_sub_directory("solution");
        if( solutiondir.isEmpty() ) break;
        QDir dir(solutiondir);
        if( ! dir.exists() ) break;

        QStringList nf;
        nf << "*.solution";
        QFileInfoList fil = dir.entryInfoList(nf,QDir::Files);
        for(auto& fi : fil) {
            solutions.append( fi.baseName() );
        }
    } while(0);

    return std::move(solutions);
}

std::shared_ptr<solution_t> get_solution(const std::string& name) {
    do {
        QString filepath = get_ptc_sub_directory("solution") + QDir::separator() + QString::fromStdString(name) + ".solution";
        QFile file(filepath);

        if( !file.open( QFile::ReadOnly | QFile::Text ) ) break;
        QString content;
        while( !file.atEnd() ) {
            content.append( file.readLine() );
        }
        file.close();

        libconfig::Config cfg;
        try {
            cfg.readString(content.toStdString().c_str());
        } catch( const libconfig::ParseException& ex ) {
            std::cerr << "exception while parse config file: " << ex.what();
            break;
        }

        auto solution = std::make_shared<solution_t>();
        try {
            solution->name = name;
            solution->tank_l = cfg.lookup("tank");
            solution->container_ml = cfg.lookup("container");
            solution->dosing_ml = cfg.lookup("dosing");

            auto& fss = cfg.lookup("fertilizer");

            for(int j=0; j < fss.getLength();++j) {
                auto& fs = fss[j];
                solution_fertilizer_t f;
                f.formula = fs.lookup("formula").c_str();
                f.gram = fs.lookup("gram");

                solution->fertilizers.push_back(f);
            }
        } catch( const libconfig::SettingNotFoundException& ex ) {
            std::cerr << "Exception while parse config file: " << ex.what() << ": " << ex.getPath();
            break;
        } catch( const libconfig::SettingTypeException& ex ) {
            std::cerr << "Exception while parse config file: " << ex.what() << ": " << ex.getPath();
            break;
        }
        return solution;
    } while(0);

    return nullptr;
}

void save_solution(const solution_t& solution) {
    do {
        QString solution_dir = get_ptc_sub_directory("solution");
        QDir dir;
        if( ! dir.exists(solution_dir) ) break;
        QString solution_file = solution_dir + QDir::separator() + solution.name.c_str() + ".solution";

        libconfig::Config cfg;
        libconfig::Setting& root = cfg.getRoot();
        root.add("tank",libconfig::Setting::TypeInt) = solution.tank_l;
        root.add("container",libconfig::Setting::TypeInt) = solution.container_ml;
        root.add("dosing",libconfig::Setting::TypeFloat) = solution.dosing_ml;

        libconfig::Setting& fss = root.add("fertilizer",libconfig::Setting::TypeList);
        for(const solution_fertilizer_t& f : solution.fertilizers) {
            libconfig::Setting& fs = fss.add(libconfig::Setting::TypeGroup);
            fs.add("formula",libconfig::Setting::TypeString) = f.formula;
            fs.add("gram",libconfig::Setting::TypeFloat) = f.gram;
        }

        cfg.writeFile(solution_file.toStdString().c_str());
    } while(0);
}
