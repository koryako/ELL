////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     ModelTransformer.cpp (model)
//  Authors:  Chuck Jacobs
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ModelTransformer.h"
#include "Node.h"
#include "InputNode.h"

// utilities
#include "Exception.h"

namespace emll
{
/// <summary> model namespace </summary>
namespace model
{
    //
    // TransformContext implementation
    //
    TransformContext::TransformContext()
    {
        _isNodeCompilableFunction = [](const Node& node) { return false; };
    }

    TransformContext::TransformContext(const std::function<bool(const Node&)>& isNodeCompilable)
        : _isNodeCompilableFunction(isNodeCompilable)
    {
    }

    bool TransformContext::IsNodeCompilable(const Node& node) const
    {
        return _isNodeCompilableFunction(node);
    }

    //
    // ModelTransformer implementation
    //
    Model ModelTransformer::CopyModel(const Model& oldModel, const TransformContext& context)
    {
        _context = context;
        _model = Model();
        _elementToElementMap.clear();
        oldModel.Visit([this](const Node& node) { node.InvokeCopy(*this); });
        _context = TransformContext();

        return _model;
    }

    Model ModelTransformer::RefineModel(const Model& oldModel, const TransformContext& context)
    {
        _context = context;
        _model = oldModel;

        int iterationCount = 0;

        // refine until all nodes are compilable according to context.IsNodeCompilable(), until
        // the model is fully refined, or until the maximum number of iterations is reached.
        do
        {
            Model currentModel = std::move(_model);
            _model = Model();

            auto currentElementToElementMap = std::move(_elementToElementMap);
            _elementToElementMap.clear();

            _isModelCompilable = true;

            // one refinement pass
            bool didRefineAny = false;
            currentModel.Visit([this, &didRefineAny](const Node& node) {
                bool didRefineNode = node.InvokeRefine(*this);
                didRefineAny |= didRefineNode;
            });

            // concatenate new port map onto existing port map
            if (currentElementToElementMap.size() > 0)
            {
                std::unordered_map<PortElementBase, PortElementBase> newElementToElementMap;
                for (const auto& entry : currentElementToElementMap)
                {
                    newElementToElementMap[entry.first] = _elementToElementMap[entry.second];
                }
                _elementToElementMap = newElementToElementMap;
            }

            // return if we didn't make any progress
            if (!didRefineAny)
            {
                break;
            }

            // die after too many iterations
            if (++iterationCount >= maxRefinementIterations)
            {
                std::string firstUncompilableNodeName;
                auto uncompilableNodes = FindUncompilableNodes(currentModel, context);
                if (uncompilableNodes.size() > 0)
                {
                    firstUncompilableNodeName = uncompilableNodes[0]->GetRuntimeTypeName();
                }
                throw utilities::LogicException(utilities::LogicExceptionErrors::illegalState, "More than " + std::to_string(maxRefinementIterations) + " refinement iterations, first uncompilable node: " + firstUncompilableNodeName);
            }
        } while (!_isModelCompilable);

        // clear out the context
        _context = TransformContext();
        return _model;
    }

    PortElementsBase ModelTransformer::TransformPortElements(const PortElementsBase& elements)
    {
        auto size = elements.Size();
        PortElementsBase result;
        result.Reserve(size);
        for (size_t index = 0; index < size; ++index)
        {
            auto oldElement = elements.GetElement(index);
            assert(_elementToElementMap.find(oldElement) != _elementToElementMap.end());
            if (_elementToElementMap.find(oldElement) == _elementToElementMap.end())
            {
                throw utilities::InputException(utilities::InputExceptionErrors::invalidArgument, "Could not find element in new model.");
            }
            auto newElement = _elementToElementMap[oldElement];
            auto newPort = newElement.ReferencedPort();
            result.Append({ *newPort, newElement.GetIndex() });
        }
        result.Consolidate();
        return result;
    }

    PortElementsBase ModelTransformer::GetCorrespondingOutputs(const OutputPortBase& port)
    {
        return GetCorrespondingOutputs(PortElementsBase(port));
    }

    PortElementsBase ModelTransformer::GetCorrespondingOutputs(const PortElementsBase& elements)
    {
        return TransformPortElements(elements);
    }

    InputNodeBase* ModelTransformer::GetCorrespondingInputNode(const InputNodeBase* inputNode)
    {
        return GetCorrespondingInputNodeAs(inputNode);
    }

    std::vector<const Node*> ModelTransformer::FindUncompilableNodes(const Model& model, const TransformContext& context) const
    {
        std::vector<const Node*> uncompilableNodes;

        auto iter = model.GetNodeIterator();
        while (iter.IsValid())
        {
            auto node = iter.Get();
            if (!context.IsNodeCompilable(*node))
            {
                uncompilableNodes.push_back(node);
            }
            iter.Next();
        }
        return uncompilableNodes;
    }
}
}