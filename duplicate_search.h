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

struct duplicate {
    duplicate (std::vector<std::string> const & paths, uint64_t const & size): paths(paths), size(size){}
    std::vector<std::string> paths;
    uint64_t size;
};

struct duplicates {
    std::vector<duplicate> duplicates;
    std::vector<std::string> pd_paths;
};

class duplicate_search {
private:
    std::map<uint64_t, std::vector<std::string>> mp;
    std::vector<std::string> pd_paths;

    void clear();

    void add_tomp(std::string const &p, uint64_t const &fs);

public:
    void update(const std::string &path);

    duplicate_search() {
        clear();
    }

    explicit duplicate_search(const std::string &path);

    duplicates get_dublicate();
};

#endif // DUPLICATE_SEARCH_H
