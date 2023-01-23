//
// Created by Piotr Kubala on 23/01/2023.
//

#ifndef RAMPACK_FILESHAPEPRINTER_H
#define RAMPACK_FILESHAPEPRINTER_H

#include <string>
#include <map>

#include "core/ShapePrinter.h"
#include "utils/Logger.h"


class FileShapePrinter {
private:
    std::string filename;
    std::string writerFormat;
    std::shared_ptr<const ShapePrinter> printer;

public:
    FileShapePrinter(std::string filename, std::string printerDescription,
                     const std::shared_ptr<const ShapePrinter> &printer)
            : filename{std::move(filename)}, writerFormat{std::move(printerDescription)}, printer{printer}
    { }

    void store(const Shape &shape, Logger &logger) const;
    [[nodiscard]] const ShapePrinter &getPrinter() const { return *this->printer; }
    [[nodiscard]] const std::string &getFilename() const { return this->filename; }
};


#endif //RAMPACK_FILESHAPEPRINTER_H
