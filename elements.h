#ifndef ELEMENTS_H
#define ELEMENTS_H

#include <stdint.h>
#include <string>
#include <vector>

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


#endif // ELEMENTS_H
