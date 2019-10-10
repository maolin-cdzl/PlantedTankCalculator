#ifndef ELEMENTS_H
#define ELEMENTS_H

#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include <QString>

struct element_part_t {
    std::string     formula;
    float           weight      = 0;
    bool            ignore      = false;
};

struct compound_t {
    float                                 weight = 0;
    std::string                         formula;
    std::vector<element_part_t>         elements;

    inline void add_element(const element_part_t& e) {
        weight += e.weight;
        if( !e.ignore ) {
            elements.push_back(e);
        }
    }

    inline float get_weight() const {
        return weight;
    }

    inline bool has_element(const std::string& element) const {
        for(auto& e : elements) {
            if( e.formula.compare(element) == 0 ) {
                return true;
            }
        }
        return false;
    }

    inline float get_percent(const std::string& element) const {
        for(auto& e: elements) {
            if( e.formula.compare(element) == 0 ) {
                return e.weight / weight;
            }
        }
        return 0.0f;
    }
};

struct element_ppm_t {
    std::string         formula;
    float               ppm;
};

struct dosing_method_t {
    std::string                     name;
    std::vector<element_ppm_t>      elements;
};

std::vector<compound_t> load_compounds(const std::string& filepath);
std::vector<dosing_method_t> load_methods(const std::string& filepath);

std::vector<compound_t> load_compounds_from_string(const QString& content);
std::vector<dosing_method_t> load_methods_from_string(const QString& content);

struct solution_fertilizer_t {
    std::string         formula;
    float               gram;
};

struct solution_t {
    std::string         name;
    int                 tank_l;
    int                 container_ml;
    float               dosing_ml;
    std::vector<solution_fertilizer_t>     fertilizers;
};

QStringList get_solution_list();
std::shared_ptr<solution_t> get_solution(const std::string& name);
void save_solution(const solution_t& solution);

QString get_ptc_directory();
QString get_ptc_sub_directory(const QString& sub);
QString get_ptc_configure();
bool reset_ptc_configure();

float roundf3(float x);

#endif // ELEMENTS_H
