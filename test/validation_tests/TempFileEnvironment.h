//
// Created by Piotr Kubala on 17/05/2026.
//

#ifndef RAMPACK_TEMPFILEENVIRONMENT_H
#define RAMPACK_TEMPFILEENVIRONMENT_H

#include <filesystem>
#include <fstream>
#include <string>

class TempFileEnvironment {
private:
    std::filesystem::path envPath;

public:
    explicit TempFileEnvironment(const std::string &envName);
    TempFileEnvironment(const TempFileEnvironment &) = delete;
    TempFileEnvironment(TempFileEnvironment &&) noexcept = delete;
    TempFileEnvironment &operator=(const TempFileEnvironment &) = delete;
    TempFileEnvironment &operator=(TempFileEnvironment &&) noexcept = delete;
    ~TempFileEnvironment();

    [[nodiscard]] std::fstream openFile(const std::filesystem::path &filePath, std::ios_base::openmode openMode) const;
    void removeFile(const std::filesystem::path &filePath) const;
    void createDirectory(const std::filesystem::path &path) const;
    void removeDirectory(const std::filesystem::path &path) const;
};


#endif //RAMPACK_TEMPFILEENVIRONMENT_H
