//
// Created by pkua on 06.12.22.
//

#include "NodeLiteral.h"


namespace pyon::ast {
    std::shared_ptr<const NodeSigned> NodeSigned::create(long value) {
        return std::shared_ptr<const NodeSigned>(new NodeSigned(value));
    }

    std::shared_ptr<const NodeUnsigned> NodeUnsigned::create(unsigned long value) {
        return std::shared_ptr<const NodeUnsigned>(new NodeUnsigned(value));
    }

    std::shared_ptr<const NodeFloat> NodeFloat::create(double value) {
        return std::shared_ptr<const NodeFloat>(new NodeFloat(value));
    }

    std::shared_ptr<const NodeBoolean> NodeBoolean::create(bool value) {
        return std::shared_ptr<const NodeBoolean>(new NodeBoolean(value));
    }

    std::shared_ptr<const NodeString> NodeString::create(std::string value) {
        return std::shared_ptr<const NodeString>(new NodeString(std::move(value)));
    }

    std::shared_ptr<const NodeNone> NodeNone::create() {
        return std::shared_ptr<const NodeNone>(new NodeNone{});
    }
}