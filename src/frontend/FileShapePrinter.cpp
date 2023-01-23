//
// Created by Piotr Kubala on 23/01/2023.
//

#include <fstream>

#include "FileShapePrinter.h"


void FileShapePrinter::store(const Shape &shape, Logger &logger) const {
    std::ofstream out(this->filename);
    ValidateOpenedDesc(out, this->filename, "to store " + this->writerFormat + " shape preview");
    out << this->printer->print(shape);
    logger.info() << this->writerFormat << " shape preview stored to " << this->filename << std::endl;
}
