#include "duplicate_search.h"
#include "QThread"

bool is_equal(std::string const &first_file, const std::string &second_file, std::vector<std::string> &bad_file) {
    std::ifstream fin1(first_file, std::ios::in | std::ios::binary);
    std::ifstream fin2(second_file, std::ios::in | std::ios::binary);
    if (!fin1) {
        bad_file.push_back(first_file);
    }
    if (!fin2) {
        bad_file.push_back(second_file);
    }
    std::vector<char> buffer1(READ_BLOCK), buffer2(READ_BLOCK);
    while (!fin1.eof() && !fin2.eof()) {
        fin1.read(buffer1.data(), READ_BLOCK);
        fin2.read(buffer2.data(), READ_BLOCK);
        if (buffer1 != buffer2) {
            fin1.close();
            fin2.close();
            return false;
        }
    }
    bool ans = fin1.eof() && fin2.eof();
    fin1.close();
    fin2.close();
    return ans;
}

std::vector<std::vector<std::string>> find_equals(std::vector<std::string> const &file_names, std::vector<std::string> &bad_file) {
    struct file_read {
        std::string name;
        std::ifstream fin;
        std::vector<char> buffer;

        file_read() {}

        file_read(std::string const &name) : name(name), fin(name, std::ios::in | std::ios::binary),
                                             buffer(READ_BLOCK) {}

        void read() {
            if (!fin.eof()) {
                fin.read(buffer.data(), READ_BLOCK);
                buffer.resize(static_cast<uint64_t>(fin.gcount()));
            }
        }
    };

    std::vector<std::vector<file_read>> files(1);
    size_t files_count = 0;
    for (auto &s: file_names) {
        files.back().emplace_back(s);
        if(!files.back().back().fin){
            bad_file.push_back(s);
        } else {
            ++files_count;
        }
    }

    bool flag = true;
    while (files.size() < files_count && flag) {
        auto n = files.size();
        for (size_t i = 0; i < n; ++i) {
            if (files[i].size() > 1) {
                for (auto &j:files[i]) {
                    j.read();
                }
                std::sort(files[i].begin(), files[i].end(),
                          [](file_read const &a, file_read const &b) { return a.buffer < b.buffer; });
                while (files[i].front().buffer != files[i].back().buffer) {
                    if (files[i].back().buffer != files.back().back().buffer || i == files.size() - 1) {
                        files.emplace_back(0);
                    }
                    do {
                        files.back().emplace_back();
                        std::swap(files[i].back(), files.back().back());
                        files[i].pop_back();
                    } while (files[i].back().buffer == files.back().back().buffer);
                }
            }
        }
        flag = false;
        for (auto &k:files) {
            if (k.size() > 1) {
                flag = true;
                for (auto &j:k) {
                    flag = false;
                    if (!j.fin.eof()) {
                        flag = true;
                        break;
                    }
                }
                if (flag) {
                    break;
                }
            }
        }
    }

    std::vector<std::vector<std::string>> ans(0);
    for (auto &i:files) {
        ans.emplace_back(0);
        for (auto &j:i) {
            ans.back().push_back(j.name);
        }
    }
    return ans;
}

void duplicate_search::clear()  {
    start_paths = std::vector<std::string>(0);
    mp = std::map<uint64_t, std::vector<std::string>>();
    pd_paths = std::vector<std::string>(0);
}

void duplicate_search::add_tomp(std::string const &p, uint64_t const &fs) {
    auto it = mp.find(fs);
    if (it == mp.end()) {
        mp[fs] = std::vector<std::string>(1, p);
    } else {
        it->second.push_back(p);
    }
}

void duplicate_search::update(const std::string &path){
    QDirIterator it(path.c_str(), QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks | QDir::AllEntries, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }
        QFileInfo fi(it.next());
        if(fi.permission(QFile::ReadUser)) {
            if(fi.isFile()) {
                add_tomp(fi.absoluteFilePath().toStdString(), static_cast<uint64_t>(fi.size()));
            }
        } else {
            pd_paths.push_back(fi.absoluteFilePath().toStdString());
        }
    }
}

duplicate_search::duplicate_search(const std::string &path) {
    QObject();
    clear();
    start_paths.push_back(path);
}

duplicate_search::duplicate_search(const std::vector<std::string> &paths) {
    clear();
    start_paths = paths;
}

duplicate_search::~duplicate_search() {
}

void duplicate_search::get_dublicate() {
    int max_progress = 0;
    emit set_max_progress(0);
    emit set_progress(0);
    for(auto const& i: start_paths) {
        if (QThread::currentThread()->isInterruptionRequested()){
            break;
        }
        update(i);
    }
    duplicates ans;
    max_progress = static_cast<int>(mp.size() + (mp.size()/20) + 1);
    emit set_max_progress(max_progress);
    int t = 0;
    for (auto &i:mp) {
        ++t;
        emit set_progress(t);
        if (ans.duplicates.size() > 512){
            if (QThread::currentThread()->isInterruptionRequested()){
                break;
            }
            emit display_duplicates(ans);
            ans = duplicates();
        }
        if (i.second.size() > MAX_OPEN_FILE) {
            if(i.first < READ_BLOCK && i.second.size()*i.first < 2097152) {
                std::map<std::vector<char>, std::vector<std::string>> lmap;
                std::vector<char> buffer;
                for(auto const &p:i.second){
                    buffer.resize(READ_BLOCK);
                    std::ifstream fin(p, std::ios::in | std::ios::binary);
                    fin.read(buffer.data(), READ_BLOCK);
                    buffer.resize(static_cast<uint64_t>(fin.gcount()));
                    auto it = lmap.find(buffer);
                    if (it == lmap.end()) {
                        lmap[buffer] = std::vector<std::string>(1, p);
                    } else {
                        it->second.push_back(p);
                    }
                }
                for (auto &j:lmap) {
                    if (j.second.size() > 1) {
                        ans.duplicates.emplace_back(j.second, i.first);
                    }
                }
            } else {
                std::map<uint64_t, std::vector<std::string>> lmap;
                for (auto const &p: i.second) {
                    std::ifstream fin(p, std::ios::in | std::ios::binary);
                    if (!fin) {
                        throw std::runtime_error("file not can be open: " + p);
                    }
                    std::vector<char> buffer(READ_BLOCK);
                    auto h_state = XXHash64(32184721937120798);
                    fin.read(buffer.data(), READ_BLOCK);
                    h_state.add(buffer.data(), static_cast<uint64_t>(fin.gcount()));
                    fin.close();
                    uint64_t hs = h_state.hash();
                    auto it = lmap.find(hs);
                    if (it == lmap.end()) {
                        lmap[hs] = std::vector<std::string>(1, p);
                    } else {
                        it->second.push_back(p);
                    }
                }

                for (auto &t:lmap) {
                    std::vector<std::vector<std::string>> arr(0);
                    if (t.second.size() > MAX_OPEN_FILE) {
                        std::vector<std::string> last_part(0);
                        for (size_t v = 0; v < MAX_OPEN_FILE; ++v) {
                            last_part.push_back(t.second.back());
                            t.second.pop_back();
                        }
                        arr = find_equals(last_part, read_error_files);
                        std::sort(arr.begin(), arr.end(),
                                  [](std::vector<std::string> const &a, std::vector<std::string> const &b) {
                                      return a.size() > b.size();
                                  });
                        for (auto const &j : t.second) {
                            bool flag = true;
                            for (auto &k:arr) {
                                if (is_equal(k.front(), j, read_error_files)) {
                                    k.push_back(j);
                                    flag = false;
                                    break;
                                }
                            }
                            if (flag) {
                                arr.emplace_back(1, j);
                            }
                        }
                    } else if (t.second.size() > 1) {
                        arr = find_equals(t.second, read_error_files);
                    }
                    for (auto &j:arr) {
                        if (j.size() > 1) {
                            ans.duplicates.emplace_back(j, i.first);
                        }
                    }
                }
            }
        } else if (i.second.size() > 2) {
            auto arr = find_equals(i.second, read_error_files);
            for (auto &j:arr) {
                if (j.size() > 1) {
                    ans.duplicates.emplace_back(j, i.first);
                }
            }
        } else if (i.second.size() > 1) {
            if (is_equal(i.second.front(), i.second.back(), read_error_files)) {
                ans.duplicates.emplace_back(std::vector<std::string>({i.second.front(), i.second.back()}), i.first);
            }
        }
    }
    if (!QThread::currentThread()->isInterruptionRequested()){
        ans.pd_paths = pd_paths;
        emit display_duplicates(ans);
    }
    emit set_progress(max_progress);
    emit finished();
}
