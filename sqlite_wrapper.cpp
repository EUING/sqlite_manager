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
        sqlite3* sqlite = nullptr;
        int result = sqlite3_open(database_path.c_str(), &sqlite);
        if (SQLITE_OK != result) {
            if (sqlite) {
                sqlite3_close(sqlite);
            }
            return nullptr;
        }

        std::unique_ptr<SqliteWrapper> sqlite_wrapper(new SqliteWrapper);
        sqlite_wrapper->sqlite_ = sqlite;

        return sqlite_wrapper;

    }

    SqliteWrapper::~SqliteWrapper() {
        if (sqlite_) {
            sqlite3_close(sqlite_);
        }
    }

    SqlError SqliteWrapper::ExecuteUpdate(const std::string& query) {
        auto deleter = [](char* err_msg) {
            if (err_msg) {
                sqlite3_free(err_msg);
            }
        };

        char* err = nullptr;
        int result = sqlite3_exec(sqlite_, query.c_str(), nullptr, nullptr, &err);
        std::unique_ptr<char, decltype(deleter)> err_msg(err, deleter);
        last_error_ = err ? err : std::string();

        return static_cast<SqlError>(result);
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
        int result = sqlite3_exec(sqlite_, query.c_str(), callback, &v, &err);
        std::unique_ptr<char, decltype(deleter)> err_msg(err, deleter);

        if (SQLITE_OK != result) {
            return std::nullopt;
        }

        return v;
    }

    std::optional<int> SqliteWrapper::GetLastChangeRowCount() const {
        if (!sqlite_) {
            return std::nullopt;
        }
        
        return sqlite3_changes(sqlite_);
    }
}  // namespace utf8

namespace utf16 {
    std::unique_ptr<SqliteWrapper> SqliteWrapper::Create(const std::wstring& database_path) {
        sqlite3* sqlite = nullptr;
        int result = sqlite3_open16(database_path.c_str(), &sqlite);
        if (SQLITE_OK != result) {
            if (sqlite) {
                sqlite3_close(sqlite);
            }
            return nullptr;
        }        

        std::unique_ptr<SqliteWrapper> sqlite_wrapper(new SqliteWrapper);
        sqlite_wrapper->sqlite_ = sqlite;

        return sqlite_wrapper;
    }

    SqliteWrapper::~SqliteWrapper() {
        if (sqlite_) {
            sqlite3_close(sqlite_);
        }
    }

    SqlError SqliteWrapper::ExecuteUpdate(const std::wstring& query) {
        std::string utf_8 = query.empty() ? std::string() : CW2A(query.c_str(), CP_UTF8).m_psz;

        auto deleter = [](char* err_msg) {
            if (err_msg) {
                sqlite3_free(err_msg);
            }
        };

        char* err = nullptr;
        int result = sqlite3_exec(sqlite_, utf_8.c_str(), nullptr, nullptr, &err);
        std::unique_ptr<char, decltype(deleter)> err_msg(err, deleter);
        last_error_ = err ? CA2W(err, CP_UTF8).m_psz : std::wstring();
        
        return static_cast<SqlError>(result);
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
        int result = sqlite3_exec(sqlite_, utf_8.c_str(), callback, &v, &err);
        std::unique_ptr<char, decltype(deleter)> err_msg(err, deleter);

        if (SQLITE_OK != result) {
            return std::nullopt;
        }

        return v;
    }

    std::optional<int> SqliteWrapper::GetLastChangeRowCount() const {
        if (!sqlite_) {
            return std::nullopt;
        }

        return sqlite3_changes(sqlite_);
    }
}  // namespace utf16
}  // namespace sqlite_manager
