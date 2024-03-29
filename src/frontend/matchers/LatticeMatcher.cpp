//
// Created by Piotr Kubala on 03/01/2023.
//

#include <utility>
#include <variant>
#include <iterator> // Must be included before ZipIterator!
#include <ZipIterator.hpp>

#include "LatticeMatcher.h"
#include "ArrangementMatcher.h"
#include "frontend/PackingFactory.h"
#include "core/lattice/UnitCell.h"
#include "core/lattice/UnitCellFactory.h"
#include "core/lattice/Lattice.h"
#include "core/lattice/LatticeTransformer.h"
#include "core/lattice/LatticePopulator.h"
#include "core/lattice/SerialPopulator.h"
#include "core/lattice/RandomPopulator.h"
#include "frontend/LatticeDimensionsOptimizer.h"
#include "core/lattice/CellOptimizationTransformer.h"
#include "core/lattice/ColumnarTransformer.h"
#include "core/lattice/FlipRandomizingTransformer.h"
#include "core/lattice/LayerRotationTransformer.h"
#include "core/lattice/LayerWiseCellOptimizationTransformer.h"
#include "core/lattice/RotationRandomizingTransformer.h"
#include "core/lattice/RotationRandomizingTransformer.h"

using namespace pyon::matcher;


namespace {
    using CellDimensions = std::variant<double, std::array<double, 3>, TriclinicBox>;

    class CellCreator {
    private:
        std::function<UnitCell(double)> doubleCreator;
        std::function<UnitCell(const std::array<double, 3>&)> arrayCreator;
        std::function<UnitCell(const TriclinicBox&)> boxCreator;

    public:
        template<typename Function>
        CellCreator(Function &&overloaded) : doubleCreator{overloaded}, arrayCreator{overloaded}, boxCreator{overloaded}
        { }

        UnitCell operator()(double a) const { return this->doubleCreator(a); }
        UnitCell operator()(const std::array<double, 3> &array) const { return this->arrayCreator(array); }
        UnitCell operator()(const TriclinicBox &box) const { return this->boxCreator(box); }
    };

    struct PopulatorData {
        std::shared_ptr<LatticePopulator> populator;
        std::size_t numShapes{};
    };

    class LatticePackingFactory : public PackingFactory {
    private:
        Lattice lattice;
        std::vector<std::shared_ptr<LatticeTransformer>> transformers;
        std::shared_ptr<LatticePopulator> populator;
        std::size_t numShapes{};

    public:
        LatticePackingFactory(Lattice lattice, std::vector<std::shared_ptr<LatticeTransformer>> transformers,
                              PopulatorData populatorData)
                : lattice{std::move(lattice)}, transformers{std::move(transformers)},
                  populator{std::move(populatorData.populator)}, numShapes{populatorData.numShapes}
        { }

        std::unique_ptr<Packing> createPacking(std::unique_ptr<BoundaryConditions> bc, const ShapeTraits &shapeTraits,
                                               std::size_t moveThreads, std::size_t scalingThreads) override
        {
            for (const auto &transformer : this->transformers)
                transformer->transform(this->lattice, shapeTraits);

            auto shapes = this->populator->populateLattice(this->lattice, this->numShapes);
            return std::make_unique<Packing>(this->lattice.getLatticeBox(), std::move(shapes), std::move(bc),
                                             shapeTraits.getInteraction(), moveThreads, scalingThreads);
        }
    };

    class AutoAxisOrderSerialPopulator : public LatticePopulator {
    private:
        [[nodiscard]] std::string optimizeAxisOrder(const Lattice &lattice) const {
            std::string axisOrder = "xyz";
            auto dim = lattice.getDimensions();
            auto zipped = Zip(dim, axisOrder);
            std::sort(zipped.begin(), zipped.end());
            std::reverse(axisOrder.begin(), axisOrder.end());
            return axisOrder;
        }

    public:
        [[nodiscard]] std::vector<Shape> populateLattice(const Lattice &lattice,
                                                         std::size_t numOfShapes) const override
        {
            SerialPopulator serialPopulator(this->optimizeAxisOrder(lattice));
            return serialPopulator.populateLattice(lattice, numOfShapes);
        }
    };

    class FullPopulator : public LatticePopulator {
    public:
        [[nodiscard]] std::vector<Shape> populateLattice(const Lattice &lattice,
                                                         [[maybe_unused]] std::size_t numOfShapes) const override
        {
            return lattice.generateMolecules();
        }
    };


    MatcherDataclass create_manual_lattice();
    MatcherDataclass create_automatic_lattice();
    MatcherDataclass create_automatic_cell_dim_lattice();

    MatcherAlternative create_cell();
    MatcherDataclass create_sc();
    MatcherDataclass create_bcc();
    MatcherDataclass create_fcc();
    MatcherDataclass create_hcp();
    MatcherDataclass create_hexagonal();
    MatcherDataclass create_custom();

    MatcherAlternative create_cell_dim();
    MatcherArray create_transformations();
    MatcherAlternative create_fill_partially();
    MatcherDataclass create_serial();
    MatcherDataclass create_random();

    MatcherDataclass create_optimize_cell();
    MatcherDataclass create_optimize_layers();
    MatcherDataclass create_columnar();
    MatcherDataclass create_randomize_flip();
    MatcherDataclass create_layer_rotate();
    MatcherDataclass create_randomize_rotations();

    std::vector<std::shared_ptr<LatticeTransformer>> do_create_transformations(const DictionaryData &kwargs);
    PopulatorData do_create_populator(const DictionaryData &kwargs);


    auto nCellsInteger = MatcherInt{}
            .positive()
            .mapTo([](long i) {
                auto iUl = static_cast<std::size_t>(i);
                return std::array<std::size_t, 3>{iUl, iUl, iUl};
            });
    auto nCellsArray = MatcherArray{}
            .elementsMatch(MatcherInt{}.positive().mapTo<std::size_t>())
            .size(3)
            .mapToStdArray<std::size_t, 3>();
    auto nCells = nCellsInteger | nCellsArray;

    auto axis = MatcherString{}
        .anyOf({"x", "y", "z"})
        .mapTo([](const std::string &axis) {
            switch (axis.front()) {
                case 'x':   return LatticeTraits::Axis::X;
                case 'y':   return LatticeTraits::Axis::Y;
                case 'z':   return LatticeTraits::Axis::Z;
                default:    AssertThrow(axis);
            }
        });

    auto axisOrder = MatcherString{}
        .containsOnlyCharacters("xyz")
        .uniqueCharacters()
        .length(3);


    MatcherDataclass create_manual_lattice() {
        auto kwargs = MatcherDictionary{}
            .valueAtKeyMatches("cell_dim", create_cell_dim())
            .valueAtKeyMatches("n_cells", nCells)
            .valueAtKeyMatches("transformations", create_transformations())
            .valueAtKeyMatches("fill_partially", create_fill_partially())
            .hasOnlyKeys({"cell_dim", "n_cells", "transformations", "fill_partially"})
            .hasKeys({"cell_dim", "n_cells"});

        return MatcherDataclass("lattice")
            .arguments({{"cell", create_cell()}})
            .variadicKeywordArguments(kwargs)
            .mapTo([](const DataclassData &lattice) -> std::shared_ptr<PackingFactory> {
                auto cellCreator = lattice["cell"].as<CellCreator>();
                const auto &kwargs = lattice.getVariadicKeywordArguments();
                auto cellDim = kwargs["cell_dim"].as<CellDimensions>();
                auto nCells = kwargs["n_cells"].as<std::array<std::size_t, 3>>();

                UnitCell cell = std::visit(cellCreator, cellDim);
                Lattice theLattice(cell, nCells);

                auto transformations = do_create_transformations(kwargs);
                auto populatorData = do_create_populator(kwargs);
                return std::make_shared<LatticePackingFactory>(theLattice, transformations, populatorData);
            });
    }

    MatcherDataclass create_automatic_lattice() {
        auto kwargs = MatcherDictionary{}
            .valueAtKeyMatches("box_dim", create_cell_dim())
            .valueAtKeyMatches("n_shapes", MatcherInt{}.positive().mapTo<std::size_t>())
            .valueAtKeyMatches("transformations", create_transformations())
            .hasOnlyKeys({"box_dim", "n_shapes", "transformations"})
            .hasKeys({"box_dim", "n_shapes"});

        return MatcherDataclass("lattice")
            .arguments({{"cell", create_cell()}})
            .variadicKeywordArguments(kwargs)
            .mapTo([](const DataclassData &lattice) -> std::shared_ptr<PackingFactory> {
                auto cellCreator = lattice["cell"].as<CellCreator>();
                const auto &kwargs = lattice.getVariadicKeywordArguments();
                auto boxDim = kwargs["box_dim"].as<CellDimensions>();
                auto nShapes = kwargs["n_shapes"].as<std::size_t>();

                auto box = std::visit([](auto &&args) { return TriclinicBox(args); }, boxDim);

                std::size_t cellSize = cellCreator(1.).size();
                auto latticeDim = LatticeDimensionsOptimizer::optimize(nShapes, cellSize, box);

                auto sides = box.getSides();
                std::transform(sides.begin(), sides.end(), latticeDim.begin(), sides.begin(), std::divides<>{});
                UnitCell cell = cellCreator(TriclinicBox(sides));
                Lattice theLattice(cell, latticeDim);

                auto transformations = do_create_transformations(kwargs);
                PopulatorData fullPopulator{std::make_shared<FullPopulator>(), 0};
                return std::make_shared<LatticePackingFactory>(theLattice, transformations, fullPopulator);
            });
    }

    MatcherDataclass create_automatic_cell_dim_lattice() {
        auto kwargs = MatcherDictionary{}
            .valueAtKeyMatches("n_cells", nCells)
            .valueAtKeyMatches("box_dim", create_cell_dim())
            .valueAtKeyMatches("transformations", create_transformations())
            .valueAtKeyMatches("fill_partially", create_fill_partially())
            .hasOnlyKeys({"n_cells", "box_dim", "transformations", "fill_partially"})
            .hasKeys({"n_cells", "box_dim"});

        return MatcherDataclass("lattice")
            .arguments({{"cell", create_cell()}})
            .variadicKeywordArguments(kwargs)
            .mapTo([](const DataclassData &lattice) -> std::shared_ptr<PackingFactory> {
                auto cellCreator = lattice["cell"].as<CellCreator>();
                const auto &kwargs = lattice.getVariadicKeywordArguments();
                auto nCells = kwargs["n_cells"].as<std::array<std::size_t, 3>>();
                auto boxDim = kwargs["box_dim"].as<CellDimensions>();

                auto box = std::visit([](auto &&args) { return TriclinicBox(args); }, boxDim);
                auto sides = box.getSides();
                std::transform(sides.begin(), sides.end(), nCells.begin(), sides.begin(), std::divides<>{});

                UnitCell cell = cellCreator(TriclinicBox(sides));
                Lattice theLattice(cell, nCells);

                auto transformations = do_create_transformations(kwargs);
                auto populatorData = do_create_populator(kwargs);
                return std::make_shared<LatticePackingFactory>(theLattice, transformations, populatorData);
            });
    }

    MatcherAlternative create_cell() {
        return create_sc() | create_bcc() | create_fcc() | create_hcp() | create_hexagonal() | create_custom();
    }

    MatcherDataclass create_sc() {
        return MatcherDataclass("sc")
            .mapTo([](const DataclassData &) -> CellCreator {
                return [](auto &&arg) { return UnitCellFactory::createScCell(arg); };
            });
    }

    MatcherDataclass create_bcc() {
        return MatcherDataclass("bcc")
            .mapTo([](const DataclassData &) -> CellCreator {
                return [](auto &&arg) { return UnitCellFactory::createBccCell(arg); };
            });
    }

    MatcherDataclass create_fcc() {
        return MatcherDataclass("fcc")
            .mapTo([](const DataclassData &) -> CellCreator {
                return [](auto &&arg) { return UnitCellFactory::createFccCell(arg); };
            });
    }

    MatcherDataclass create_hcp() {
        return MatcherDataclass("hcp")
            .arguments({{"axis", axis}})
            .mapTo([](const DataclassData &hcp) -> CellCreator {
                auto axis = hcp["axis"].as<LatticeTraits::Axis>();
                return [axis](auto &&arg) { return UnitCellFactory::createHcpCell(arg, axis); };
            });
    }

    MatcherDataclass create_hexagonal() {
        return MatcherDataclass("hexagonal")
            .arguments({{"axis", axis}})
            .mapTo([](const DataclassData &hexagonal) -> CellCreator {
                auto axis = hexagonal["axis"].as<LatticeTraits::Axis>();
                return [axis](auto &&arg) { return UnitCellFactory::createHexagonalCell(arg, axis); };
            });
    }

    MatcherDataclass create_custom() {
        auto pos = MatcherArray(MatcherFloat{}.mapTo<double>(), 3).mapToVector<3>();
        auto rot = MatcherArray(MatcherFloat{}.mapTo<double>(), 3)
            .mapTo([](const ArrayData &array) -> Matrix<3, 3> {
                auto angles = array.asStdArray<double, 3>();
                double factor = M_PI/180;
                return Matrix<3, 3>::rotation(factor*angles[0], factor*angles[1], factor*angles[2]);
            });

        auto shape = MatcherDataclass("shape")
            .arguments({{"pos", pos},
                        {"rot", rot, "[0, 0, 0]"}})
            .mapTo([](const DataclassData &shape) -> Shape {
                auto pos = shape["pos"].as<Vector<3>>();
                auto rot = shape["rot"].as<Matrix<3, 3>>();
                return {pos, rot};
            });

        auto shapes = MatcherArray{}
            .elementsMatch(shape)
            .nonEmpty()
            .mapToStdVector<Shape>();

        return MatcherDataclass("custom")
            .arguments({{"shapes", shapes}})
            .mapTo([](const DataclassData &custom) -> CellCreator {
                auto shapes = custom["shapes"].as<std::vector<Shape>>();
                return [shapes](auto &&args) { return UnitCell(TriclinicBox(args), shapes); };
            });
    }

    MatcherAlternative create_cell_dim() {
        auto cellDimFloat = MatcherFloat{}
            .positive()
            .mapTo([](double d) -> CellDimensions {
                return d;
            });
        auto cellDimArray = MatcherArray{}
            .elementsMatch(MatcherFloat{}.positive())
            .size(3)
            .mapTo([](const ArrayData &array) -> CellDimensions {
                return array.asStdArray<double, 3>();
            });
        auto cellDimBox = MatcherArray{}
            .elementsMatch(MatcherArray{}.elementsMatch(MatcherFloat{}).size(3))
            .size(3)
            .filter([](const ArrayData &array) {
                return std::abs(array.asMatrix<3, 3>().det()) > 1e-8;
            })
            .describe("box with non-zero volume")
            .mapTo([](const ArrayData &array) -> CellDimensions {
                return TriclinicBox(array.asMatrix<3, 3>().transpose());
            });
        return cellDimFloat | cellDimArray | cellDimBox;
    }

    MatcherArray create_transformations() {
        auto transformation = create_optimize_cell()
            | create_optimize_layers()
            | create_columnar()
            | create_randomize_flip()
            | create_layer_rotate()
            | create_randomize_rotations();

        return MatcherArray{}
            .elementsMatch(transformation)
            .mapToStdVector<std::shared_ptr<LatticeTransformer>>();
    }

    MatcherAlternative create_fill_partially() {
        return create_serial() | create_random();
    }

    MatcherDataclass create_serial() {
        auto axisOrderAuto = MatcherString("auto");

        return MatcherDataclass("serial")
            .arguments({{"n_shapes", MatcherInt{}.positive().mapTo<std::size_t>()},
                        {"axis_order", axisOrderAuto | axisOrder, R"("auto")"}})
            .mapTo([](const DataclassData &serial) -> PopulatorData {
                auto nShapes = serial["n_shapes"].as<std::size_t>();
                auto axisOrder = serial["axis_order"].as<std::string>();
                if (axisOrder == "auto")
                    return {std::make_shared<AutoAxisOrderSerialPopulator>(), nShapes};
                else
                    return {std::make_shared<SerialPopulator>(axisOrder), nShapes};
            });
    }

    MatcherDataclass create_random() {
        return MatcherDataclass("random")
            .arguments({{"n_shapes", MatcherInt{}.positive().mapTo<std::size_t>()},
                        {"seed", MatcherInt{}.mapTo<unsigned long>()}})
            .mapTo([](const DataclassData &random) -> PopulatorData {
                auto nShapes = random["n_shapes"].as<std::size_t>();
                auto seed = random["seed"].as<unsigned long>();
                return {std::make_shared<RandomPopulator>(seed), nShapes};
            });
    }

    MatcherDataclass create_optimize_cell() {
        return MatcherDataclass("optimize_cell")
            .arguments({{"spacing", MatcherFloat{}.positive()},
                        {"axis_order", axisOrder, R"("xyz")"}})
            .mapTo([](const DataclassData &optimizeCell) -> std::shared_ptr<LatticeTransformer> {
                auto spacing = optimizeCell["spacing"].as<double>();
                auto axisOrder = optimizeCell["axis_order"].as<std::string>();
                return std::make_shared<CellOptimizationTransformer>(axisOrder, spacing);
            });
    }

    MatcherDataclass create_optimize_layers() {
        return MatcherDataclass("optimize_layers")
            .arguments({{"spacing", MatcherFloat{}.positive()},
                        {"axis", axis}})
            .mapTo([](const DataclassData &optimizeLayers) -> std::shared_ptr<LatticeTransformer> {
                auto spacing = optimizeLayers["spacing"].as<double>();
                auto axis = optimizeLayers["axis"].as<LatticeTraits::Axis>();
                return std::make_shared<LayerWiseCellOptimizationTransformer>(axis, spacing);
            });
    }

    MatcherDataclass create_columnar() {
        return MatcherDataclass("columnar")
            .arguments({{"axis", axis},
                        {"seed", MatcherInt{}.mapTo<unsigned long>()}})
            .mapTo([](const DataclassData &columnar) -> std::shared_ptr<LatticeTransformer> {
                auto axis = columnar["axis"].as<LatticeTraits::Axis>();
                auto seed = columnar["seed"].as<unsigned long>();
                return std::make_shared<ColumnarTransformer>(axis, seed);
            });
    }

    MatcherDataclass create_randomize_flip() {
        return MatcherDataclass("randomize_flip")
            .arguments({{"seed", MatcherInt{}.mapTo<unsigned long>()}})
            .mapTo([](const DataclassData &randomizeFlip) -> std::shared_ptr<LatticeTransformer> {
                auto seed = randomizeFlip["seed"].as<unsigned long>();
                return std::make_shared<FlipRandomizingTransformer>(seed);
            });
    }

    MatcherDataclass create_layer_rotate() {
        return MatcherDataclass("layer_rotate")
            .arguments({{"layer_axis", axis},
                        {"rot_axis", axis},
                        {"rot_angle", MatcherFloat{}},
                        {"alternating", MatcherBoolean{}}})
            .mapTo([](const DataclassData &layerRotate) -> std::shared_ptr<LatticeTransformer> {
                auto layerAxis = layerRotate["layer_axis"].as<LatticeTraits::Axis>();
                auto rotAxis = layerRotate["rot_axis"].as<LatticeTraits::Axis>();
                auto rotAngle = M_PI * layerRotate["rot_angle"].as<double>() / 180;
                auto alternating = layerRotate["alternating"].as<bool>();
                return std::make_shared<LayerRotationTransformer>(layerAxis, rotAxis, rotAngle, alternating);
            });
    }

    MatcherDataclass create_randomize_rotations() {
        using Axis = RotationRandomizingTransformer::Axis;

        auto axisArray = MatcherArray(MatcherFloat{}, 3)
            .filter([](const ArrayData &arrayData) {
                return arrayData.asVector<3>().norm2() > 1e-20;
            })
            .describe("non-zero norm")
            .mapTo([](const ArrayData &arrayData) -> Axis {
                return arrayData.asVector<3>();
            });
        auto axisString = MatcherString{}
            .anyOf({"x", "y", "z", "primary", "secondary", "auxiliary"})
            .mapTo([](const std::string &axis) -> Axis {
                if (axis == "x")                return Vector<3>{1, 0, 0};
                else if (axis == "y")           return Vector<3>{0, 1, 0};
                else if (axis == "z")           return Vector<3>{0, 0, 1};
                else if (axis == "primary")     return ShapeGeometry::Axis::PRIMARY;
                else if (axis == "secondary")   return ShapeGeometry::Axis::SECONDARY;
                else if (axis == "auxiliary")   return ShapeGeometry::Axis::AUXILIARY;
                else                            AssertThrow(axis);
            });
        auto axisRandom = MatcherString("random")
            .mapTo([](const std::string &) -> Axis {
                return RotationRandomizingTransformer::RANDOM_AXIS;
            });
        auto rotAxis = axisArray | axisString | axisRandom;

        return MatcherDataclass("randomize_rotation")
            .arguments({{"seed", MatcherInt{}.mapTo<unsigned long>()},
                        {"axis", rotAxis, R"("random")"}})
            .mapTo([](const DataclassData &randomizeRotation) -> std::shared_ptr<LatticeTransformer> {
                auto seed = randomizeRotation["seed"].as<unsigned long>();
                auto axis = randomizeRotation["axis"].as<Axis>();
                return std::make_shared<RotationRandomizingTransformer>(axis, seed);
            });
    }

    std::vector<std::shared_ptr<LatticeTransformer>> do_create_transformations(const DictionaryData &kwargs) {
        if (kwargs.hasKey("transformations"))
            return kwargs["transformations"].as<std::vector<std::shared_ptr<LatticeTransformer>>>();
        else
            return {};
    }

    PopulatorData do_create_populator(const DictionaryData &kwargs) {
        if (kwargs.hasKey("fill_partially"))
            return kwargs["fill_partially"].as<PopulatorData>();
        else
            return {std::make_shared<FullPopulator>(), 0};
    }
}


MatcherAlternative LatticeMatcher::create() {
    return create_manual_lattice() | create_automatic_lattice() | create_automatic_cell_dim_lattice();
}
