//
// Created by Piotr Kubala on 17/05/2026.
//

#include <regex>
#include <sstream>

#include "TempFileEnvironment.h"

#include <iostream>

#include "utils/Exceptions.h"


TempFileEnvironment::TempFileEnvironment(const std::string &envName, const bool clearOnDestruction)
        : clearOnDestruction{clearOnDestruction}
{
    static const std::regex envNameRegex(R"([-[:alnum:]_]+)");
    Expects(std::regex_match(envName, envNameRegex));

    const auto rootPath = std::filesystem::path(__FILE__).remove_filename();
    this->envPath = rootPath / "tmp_files" / envName;
    if (this->clearOnDestruction && std::filesystem::exists(envPath))
        std::filesystem::remove_all(this->envPath);

    std::filesystem::create_directories(this->envPath);
}

TempFileEnvironment::~TempFileEnvironment() {
    if (this->clearOnDestruction && std::filesystem::exists(this->envPath))
        std::filesystem::remove_all(this->envPath);
}

std::fstream TempFileEnvironment::openFile(const std::filesystem::path &filePath,
                                           const std::ios_base::openmode openMode) const
{
    Expects(filePath.is_relative());

    if (filePath.has_parent_path()) {
        const auto fullParentPath = this->envPath / filePath.parent_path();
        if (!std::filesystem::exists(fullParentPath))
            std::filesystem::create_directories(fullParentPath);
    }

    const auto absoluteFilePath = this->envPath / filePath;
    std::fstream file(absoluteFilePath, openMode);
    AssertMsg(file.is_open(), absoluteFilePath);
    return file;
}

void TempFileEnvironment::removeFile(const std::filesystem::path &filePath) const {
    Expects(filePath.is_relative());
    std::filesystem::remove(this->envPath / filePath);
}

void TempFileEnvironment::createDirectory(const std::filesystem::path &path) const {
    Expects(path.is_relative());
    std::filesystem::create_directories(this->envPath / path);
}

void TempFileEnvironment::removeDirectory(const std::filesystem::path &path) const {
    Expects(path.is_relative());
    std::filesystem::remove_all(this->envPath / path);
}

std::string TempFileEnvironment::dumpFileContents(const std::filesystem::path &filePath) const {
    auto file = this->openFile(filePath, std::ios_base::in);
    std::ostringstream fileDump;
    fileDump << file.rdbuf();
    return fileDump.str();
}
