#include "sqlite_wrapper.h"

#include <atlstr.h>

#include <optional>
#include <map>
#include <vector>
#include <string>
#include <memory>

#include "sqlite3.h"

namespace sqlite_manager {
namespace utf8 {
    std::unique_ptr<SqliteWrapper> SqliteWrapper::Create(const std::string& database_path) {
        auto deleter = [](sqlite3* sqlite3) {
            if (sqlite3) {
                sqlite3_close(sqlite3);
            }
        };

        sqlite3* sqlite = nullptr;
        int result = sqlite3_open(database_path.c_str(), &sqlite);
        std::unique_ptr<sqlite3, std::function<void(sqlite3*)>> sqlite_ptr(sqlite, deleter);

        if (SQLITE_OK != result) {
            return nullptr;
        }

        std::unique_ptr<SqliteWrapper> sqlite_wrapper(new SqliteWrapper);
        sqlite_wrapper->sqlite_ = std::move(sqlite_ptr);

        return sqlite_wrapper;

    }

    int SqliteWrapper::ExecuteUpdate(const std::string& query) {
        auto deleter = [](char* err_msg) {
            if (err_msg) {
                sqlite3_free(err_msg);
            }
        };

        char* err = nullptr;
        int result = sqlite3_exec(sqlite_.get(), query.c_str(), nullptr, nullptr, &err);
        std::unique_ptr<char, decltype(deleter)> err_msg(err, deleter);

        if (SQLITE_OK != result) {
            return -1;
        }

        return sqlite3_changes(sqlite_.get());
    }

    std::optional<std::vector<DataSet>> SqliteWrapper::ExecuteQuery(const std::string& query) {
        auto deleter = [](char* err_msg) {
            if (err_msg) {
                sqlite3_free(err_msg);
            }
        };

        auto callback = [](void* user_data, int argc, char** argv, char** col_name) -> int {
            std::vector<DataSet>* v = reinterpret_cast<std::vector<DataSet>*>(user_data);

            DataSet data;
            for (int i = 0; i < argc; ++i) {
                std::string utf_8_col_name = col_name[i] ? col_name[i] : std::string();
                std::string utf_8_arg = argv[i] ? argv[i] : std::string();

                data.insert(std::make_pair(utf_8_col_name, utf_8_arg));
            }

            v->push_back(data);

            return 0;
        };

        std::vector<DataSet> v;
        char* err = nullptr;
        int result = sqlite3_exec(sqlite_.get(), query.c_str(), callback, &v, &err);
        std::unique_ptr<char, decltype(deleter)> err_msg(err, deleter);

        if (SQLITE_OK != result) {
            return std::nullopt;
        }

        return v;
    }
}  // namespace utf8

namespace utf16 {
    std::unique_ptr<SqliteWrapper> SqliteWrapper::Create(const std::wstring& database_path) {
        auto deleter = [](sqlite3* sqlite3) {
            if (sqlite3) {
                sqlite3_close(sqlite3);
            }
        };

        sqlite3* sqlite = nullptr;
        int result = sqlite3_open16(database_path.c_str(), &sqlite);
        std::unique_ptr<sqlite3, std::function<void(sqlite3*)>> sqlite_ptr(sqlite, deleter);

        if (SQLITE_OK != result) {
            return nullptr;
        }

        std::unique_ptr<SqliteWrapper> sqlite_wrapper(new SqliteWrapper);
        sqlite_wrapper->sqlite_ = std::move(sqlite_ptr);

        return sqlite_wrapper;
    }

    int SqliteWrapper::ExecuteUpdate(const std::wstring& query) {
        std::string utf_8 = query.empty() ? std::string() : CW2A(query.c_str(), CP_UTF8).m_psz;

        auto deleter = [](char* err_msg) {
            if (err_msg) {
                sqlite3_free(err_msg);
            }
        };

        char* err = nullptr;
        int result = sqlite3_exec(sqlite_.get(), utf_8.c_str(), nullptr, nullptr, &err);
        std::unique_ptr<char, decltype(deleter)> err_msg(err, deleter);

        if (SQLITE_OK != result) {
            return -1;
        }

        return sqlite3_changes(sqlite_.get());
    }

    std::optional<std::vector<DataSet>> SqliteWrapper::ExecuteQuery(const std::wstring& query) {
        std::string utf_8 = query.empty() ? std::string() : CW2A(query.c_str(), CP_UTF8).m_psz;

        auto deleter = [](char* err_msg) {
            if (err_msg) {
                sqlite3_free(err_msg);
            }
        };

        auto callback = [](void* user_data, int argc, char** argv, char** col_name) -> int {
            std::vector<DataSet>* v = reinterpret_cast<std::vector<DataSet>*>(user_data);

            DataSet data;
            for (int i = 0; i < argc; ++i) {
                std::wstring utf_16_col_name = col_name[i] ? CA2W(col_name[i], CP_UTF8).m_psz : std::wstring();
                std::wstring utf_16_arg = argv[i] ? CA2W(argv[i], CP_UTF8).m_psz : std::wstring();

                data.insert(std::make_pair(utf_16_col_name, utf_16_arg));
            }

            v->push_back(data);

            return 0;
        };

        std::vector<DataSet> v;
        char* err = nullptr;
        int result = sqlite3_exec(sqlite_.get(), utf_8.c_str(), callback, &v, &err);
        std::unique_ptr<char, decltype(deleter)> err_msg(err, deleter);

        if (SQLITE_OK != result) {
            return std::nullopt;
        }

        return v;
    }
}  // namespace utf16
}  // namespace sqlite_manager
