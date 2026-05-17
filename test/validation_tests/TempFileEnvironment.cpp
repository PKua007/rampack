//
// Created by Piotr Kubala on 17/05/2026.
//

#include <regex>

#include "TempFileEnvironment.h"

#include "utils/Exceptions.h"


TempFileEnvironment::TempFileEnvironment(const std::string &envName) {
    static const std::regex envNameRegex(R"([-[:alnum:]_]+)");
    Expects(std::regex_match(envName, envNameRegex));

    const auto rootPath = std::filesystem::path(__FILE__).remove_filename();
    this->envPath = rootPath / "tmp_files" / envName;
    Expects(!std::filesystem::exists(envPath));

    std::filesystem::create_directories(this->envPath);
}

TempFileEnvironment::~TempFileEnvironment() {
    if (std::filesystem::exists(this->envPath))
        std::filesystem::remove_all(this->envPath);
}

std::fstream TempFileEnvironment::openFile(const std::filesystem::path &filePath,
                                           std::ios_base::openmode openMode) const
{
    Expects(filePath.is_relative());

    if (filePath.has_parent_path()) {
        const auto fullParentPath = this->envPath / filePath.parent_path();
        if (!std::filesystem::exists(fullParentPath))
            std::filesystem::create_directories(fullParentPath);
    }

    std::fstream file(this->envPath / filePath, openMode);
    Assert(file.is_open());
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
