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
#include <QObject>

#include "xxhash64.h"

const size_t READ_BLOCK = 4096 * 4;
const size_t MAX_OPEN_FILE = 196;

struct duplicate {
    duplicate (std::vector<std::string> const & paths, uint64_t const & size): paths(paths), size(size){}
    std::vector<std::string> paths;
    uint64_t size;
};

struct duplicates {
    std::vector<duplicate> duplicates;
    std::vector<std::string> pd_paths;
};

class duplicate_search : public QObject {
    Q_OBJECT
public:
    duplicate_search(const std::string &path);
    duplicate_search(const std::vector<std::string> &paths);
    ~duplicate_search();
public slots:
    void get_dublicate();
signals:
    void display_duplicates(duplicates dp);
    void display_progress(duplicates dp);
    void finished();
    void error(QString err);
private:
    std::vector<std::string> start_paths;
    std::map<uint64_t, std::vector<std::string>> mp;
    std::vector<std::string> pd_paths;
    void clear();
    void add_tomp(std::string const &p, uint64_t const &fs);
    void update(const std::string &path);
};

#endif // DUPLICATE_SEARCH_H
