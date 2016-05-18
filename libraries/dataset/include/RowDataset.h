////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     RowDataset.h (dataset)
//  Authors:  Ofer Dekel
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RowDataset.h"
#include "SupervisedExample.h"
#include "IDataVector.h"

// utilities
#include "StlIterator.h"

// stl
#include <vector>
#include <random>
#include <ostream>

namespace dataset
{
    /// <summary> A row-wise dataset of `SupervisedExample<DataVectorType>` items. </summary>
    template<typename DataVectorType>
    class RowDataset 
    {
    public:
//        using ExampleType = SupervisedExample<DataVectorType>;
//        using Iterator = utilities::VectorIterator<ExampleType>;
        typedef SupervisedExample<DataVectorType> ExampleType;
        //        typedef utilities::VectorIterator<ExampleType> Iterator;
        typedef utilities::StlIterator<typename std::vector<ExampleType>::const_iterator> Iterator;

        RowDataset() = default;

        RowDataset(RowDataset&&) = default;

        explicit RowDataset(const RowDataset&) = default;

        /// <summary> Constructs an instance of RowDataset by making shallow copies of supervised examples. </summary>
        ///
        /// <param name="exampleIterator"> The example iterator. </param>
        RowDataset(Iterator exampleIterator);

        RowDataset<DataVectorType>& operator=(RowDataset&&) = default;

        RowDataset<DataVectorType>& operator=(const RowDataset&) = delete;

        /// <summary> Returns the number of examples in the dataset. </summary>
        ///
        /// <returns> The number of examples. </returns>
        uint64_t NumExamples() const { return _examples.size(); }

        /// <summary> Returns the maximal size of any example. </summary>
        ///
        /// <returns> The maximal size of any example. </returns>
        uint64_t GetMaxDataVectorSize() const { return _maxExampleSize; }

        /// <summary> Returns a reference to an example. </summary>
        ///
        /// <param name="index"> Zero-based index of the row. </param>
        ///
        /// <returns> Reference to the specified example. </returns>
        const ExampleType& GetExample(uint64_t index) const;

        /// <summary> Returns a reference to an example. </summary>
        ///
        /// <param name="index"> Zero-based index of the row. </param>
        ///
        /// <returns> Reference to the specified example. </returns>
        const ExampleType& operator[](uint64_t index) const;

        /// <summary> Returns an iterator that traverses the examples. </summary>
        ///
        /// <param name="firstExample"> Zero-based index of the first example to iterate over. </param>
        /// <param name="numExamples"> The number of examples to iterate over, a value of zero means all
        /// examples. </param>
        ///
        /// <returns> The iterator. </returns>
        Iterator GetIterator(uint64_t fromRowIndex = 0, uint64_t size = 0) const;

        /// <summary> Adds an example at the bottom of the matrix. </summary>
        ///
        /// <param name="example"> The example. </param>
        void AddExample(ExampleType&& example);

        /// <summary> Erases all of the examples in the RowDataset. </summary>
        void Reset();

        /// <summary> Randomly permutes the rows of the dataset. </summary>
        ///
        /// <param name="rng"> [in,out] The random number generator. </param>
        void RandomPermute(std::default_random_engine& rng);

        /// <summary>
        /// Permutes the dataset so that the first count examples are chosen uniformly, and the rest are arbitrary.
        /// </summary>
        ///
        /// <param name="rng"> [in,out] The random number generator. </param>
        /// <param name="count"> Number of examples to permute. </param>
        void RandomPermute(std::default_random_engine& rng, uint64_t count);

        /// <summary> Sorts the given row dataset by a certain key. </summary>
        ///
        /// <typeparam name="SortKeyType"> Type of the sort key. </typeparam>
        /// <param name="sortKey"> A function that takes const reference to ExampleType and returns a sort key. </param>
        /// <param name="fromRowIndex"> Zero-based index of the first row to sort. </param>
        /// <param name="size"> The number of examples to sort. </param>
        template<typename SortKeyType>
        void Sort(SortKeyType sortKey, uint64_t fromRowIndex = 0, uint64_t size = 0);

        /// <summary> Prints this object. </summary>
        ///
        /// <param name="os"> [in,out] Stream to write data to. </param>
        /// <param name="fromRowIndex"> Zero-based index of the first row to print. </param>
        /// <param name="size"> The number of rows to print, or 0 to print until the end. </param>
        void Print(std::ostream& os, uint64_t fromRowIndex = 0, uint64_t size = 0) const;

    private:
        uint64_t CorrectRangeSize(uint64_t fromRowIndex, uint64_t size) const;

        std::vector<ExampleType> _examples;
        uint64_t _maxExampleSize = 0;
    };

    typedef RowDataset<IDataVector> GenericRowDataset;
    typedef utilities::StlIterator<typename std::vector<dataset::SupervisedExample<dataset::IDataVector>>::const_iterator, dataset::SupervisedExample<dataset::IDataVector>> GenericRowIterator;

    template<typename DataVectorType>
    std::ostream& operator<<(std::ostream& os, RowDataset<DataVectorType>& dataset);
}

#include "../tcc/RowDataset.tcc"
