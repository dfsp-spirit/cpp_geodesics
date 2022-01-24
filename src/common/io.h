#pragma once

#include <string>
#include <sstream>

inline bool file_exists (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}


// Get a readable duration strings for a number of seconds.
inline std::string secduration(const double secs, const bool full = false) {
    std::stringstream ss;    

    long lsecs = std::round(secs);
    long mins = lsecs / 60;
    long hours = mins / 60;
    long days = hours / 24;
    if(full || days > 0) {
        ss << days << "d " << hours % 24 << "h " << mins % 60 << "m " << lsecs % 60 << "s";
        return ss.str();
    }
    if(hours > 0) {
        ss << hours << "h " << mins % 60 << "m " << lsecs % 60 << "s";
        return ss.str();
    }
    if(mins > 0) {
        ss << mins << "m " << lsecs % 60 << "s";
        return ss.str();
    }    
    ss << lsecs << "s";
    return ss.str();
}