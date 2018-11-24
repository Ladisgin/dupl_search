#ifndef DUPLICATE_SEARCH_H
#define DUPLICATE_SEARCH_H
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <limits>
#include <algorithm>

#include <QFileInfo>
#include <QDirIterator>
#include <QString>

#include "xxhash64.h"

const size_t READ_BLOCK = 4096 * 4;
const size_t MAX_OPEN_FILE = 200;

class duplicate_search {
private:
    std::map<uint64_t, std::vector<std::string>> mp;
    std::vector<std::string> denied_files;

    void clear();

    void add_tomp(std::string const &p, uint64_t const &fs);

public:
    void update(const std::string &path);

    duplicate_search() {
        clear();
    }

    explicit duplicate_search(const std::string &path);

    std::vector<std::pair<std::vector<std::string>, uint64_t>> get_dublicate();
    std::vector<std::pair<std::vector<std::string>, uint64_t>> get_dublicate(std::vector<std::string> &denied_files);

};

#endif // DUPLICATE_SEARCH_H
