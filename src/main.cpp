#include <filesystem>
#include <fstream>
#include <vector>
#include <iostream>
#include <unordered_set>

inline bool files_are_identical(std::filesystem::path const& file1, std::filesystem::path const& file2) {
    std::error_code ec;
    auto size1 = std::filesystem::file_size(file1, ec);
    if(ec) return false;
    auto size2 = std::filesystem::file_size(file2, ec);
    if(ec) return false;

    if(size1 != size2) return false;

    std::ifstream f1(file1, std::ios::binary);
    std::ifstream f2(file2, std::ios::binary);
    if(!f1 || !f2) return false;

    constexpr size_t buf_size = 8192;
    std::vector<char> buf1(buf_size), buf2(buf_size);

    while(f1 && f2) {
        f1.read(buf1.data(), buf_size);
        f2.read(buf2.data(), buf_size);

        std::streamsize r1 = f1.gcount();
        std::streamsize r2 = f2.gcount();
        if(r1 != r2) return false;

        if(!std::equal(buf1.begin(), buf1.begin() + r1, buf2.begin())) {
            return false;
        }
    }

    return true;
}

inline void write_branch(std::ostream& stream, uint32_t depth, std::string const& msg) {
    for(uint32_t i = 0; i < depth; ++i) {
        stream << "|  ";
    }
    stream << "|- " << msg << std::endl;
}

inline void check_tree(std::filesystem::path const& path_a, std::filesystem::path const& path_b, uint32_t depth) {
    std::unordered_set<std::string> map_a;
    for(auto& path : std::filesystem::directory_iterator(path_a)) {
        if(path.is_symlink()) continue;
        map_a.insert(path.path().filename().string());
    }

    std::unordered_set<std::string> map_b;
    for(auto& path : std::filesystem::directory_iterator(path_b)) {
        if(path.is_symlink()) continue;
        map_b.insert(path.path().filename().string());
    }
    auto map_ab = map_a;
    map_ab.insert(map_b.begin(), map_b.end());

    auto category = [&](std::string const& name) -> uint32_t {
        bool a_is_dir = std::filesystem::is_directory(path_a / name);
        bool b_is_dir = std::filesystem::is_directory(path_b / name);

        if(!a_is_dir && !b_is_dir) {
            return 0;  // both files
        }
        if(a_is_dir != b_is_dir) {
            return 1;  // mixed (one file, one dir)
        }
        return 2;      // both dirs
    };

    std::vector<std::string> ordered_ab(map_ab.begin(), map_ab.end());
    std::sort(ordered_ab.begin(), ordered_ab.end(), [&](std::string const& a, std::string const& b) {
        auto ca = category(a);
        auto cb = category(b);
        if(ca != cb) {
            return ca < cb;
        }
        return a < b; // alphabetical
    });

    for(auto const& name : ordered_ab) {
        std::filesystem::path sub_path_a = path_a / name;
        std::filesystem::path sub_path_b = path_b / name;
        bool contains_a = map_a.contains(name);
        bool contains_b = map_b.contains(name);
        if(contains_a && contains_b) {
            bool is_dir_a = std::filesystem::is_directory(sub_path_a);
            bool is_dir_b = std::filesystem::is_directory(sub_path_b);
            if(is_dir_a && !is_dir_b) {
                write_branch(std::cout, depth, "dir \"" + name + "\" in \"" + path_a.string() + "\" is a file in \"" + path_b.string() + "\"");
            }
            else if(!is_dir_a && is_dir_b) {
                write_branch(std::cout, depth, "file \"" + name + "\" in \"" + path_a.string() + "\" is a dir in \"" + path_b.string() + "\"");
            }
            else if(is_dir_a && is_dir_b) {
                // traverse subdirectory
                write_branch(std::cout, depth, "dir \"" + name + "\"");
                check_tree(sub_path_a, sub_path_b, depth + 1);
            }
            else if(!files_are_identical(sub_path_a, sub_path_b)) {
                write_branch(std::cout, depth, "file \"" + name + "\" differs");
            }
        }
        else if(!contains_a) {
            bool is_dir_b = std::filesystem::is_directory(sub_path_b);
            write_branch(std::cout, depth, (is_dir_b ? "dir" : "file") + (" \"" + name) + "\" is missing in \"" + path_a.string() + "\"");
        }
        else if(!contains_b) {
            bool is_dir_a = std::filesystem::is_directory(sub_path_a);
            write_branch(std::cout, depth, (is_dir_a ? "dir" : "file") + (" \"" + name) + "\" is missing in \"" + path_b.string() + "\"");
        }
    }
}

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cerr << "Specify 2 directories to compare" << std::endl;
        return 1;
    }

    std::filesystem::path a(argv[1]);
    std::filesystem::path b(argv[2]);

    if(!std::filesystem::is_directory(a)) {
        std::cerr << "Path A must be a directory" << std::endl;
        return 1;
    }
    if(!std::filesystem::is_directory(b)) {
        std::cerr << "Path A must be a directory" << std::endl;
        return 1;
    }

    write_branch(std::cout, 0, "<root>");
    check_tree(a, b, 1);
}
