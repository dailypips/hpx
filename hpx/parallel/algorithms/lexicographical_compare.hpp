//  Copyright (c) 2014 Grant Mercer
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file parallel/algorithms/lexicographical_compare.hpp

#if !defined(HPX_PARALLEL_DETAIL_LEXI_COMPARE_DEC_30_2014_0312PM)
#define HPX_PARALLEL_DETAIL_LEXI_COMPARE_DEC_30_2014_0312PM

#include <hpx/hpx_fwd.hpp>
#include <hpx/parallel/execution_policy.hpp>
#include <hpx/parallel/algorithms/detail/algorithm_result.hpp>
#include <hpx/parallel/algorithms/detail/predicates.hpp>
#include <hpx/parallel/algorithms/detail/dispatch.hpp>
#include <hpx/parallel/algorithms/mismatch.hpp>
#include <hpx/parallel/algorithms/for_each.hpp>
#include <hpx/parallel/util/partitioner.hpp>
#include <hpx/parallel/util/loop.hpp>
#include <hpx/parallel/util/zip_iterator.hpp>

#include <algorithm>
#include <iterator>

#include <boost/static_assert.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_base_of.hpp>

namespace hpx { namespace parallel { HPX_INLINE_NAMESPACE(v1)
{    
    ///////////////////////////////////////////////////////////////////////////
    // lexicographical_compare
    namespace detail 
    {
        /// \cond NOINTERNAL
        struct lexicographical_compare : public detail::algorithm<lexicographical_compare, bool>
        {
            lexicographical_compare()
                : lexicographical_compare::algorithm("lexicographical_compare")
            {}

           template <typename ExPolicy, typename InIter1, typename InIter2,
                typename Pred>
           static bool
           sequential(ExPolicy const&, InIter1 first1, InIter1 last1, InIter2 first2,
                InIter2 last2, Pred && pred)
            {
                return std::lexicographical_compare(first1, last1, first2, last2, pred);
            }

            template <typename ExPolicy, typename InIter1, typename InIter2,
                typename Pred>
            static typename detail::algorithm_result<ExPolicy, bool>::type
            parallel(ExPolicy const& policy, InIter1 first1, InIter1 last1, InIter2 first2,
                InIter2 last2, Pred && pred)
            {
                typedef hpx::util::zip_iterator<InIter1, InIter2> zip_iterator;
                typedef typename zip_iterator::reference reference;            
 
                std::size_t count1 = std::distance(first1, last1);
                std::size_t count2 = std::distance(first2, last2);
                std::size_t count = count1 < count2 ? count1 : count2;     
        
                // An empty range is lexicographically less than any non-empty range
                if(count1 == 0 && count2 != 0)
                {
                    return detail::algorithm_result<ExPolicy, bool>::get(true);
                }

                if(count2 == 0 && count1 != 0)
                {
                    return detail::algorithm_result<ExPolicy, bool>::get(false);
                }
        
                util::cancellation_token<std::size_t> tok(count);

                return util::partitioner<ExPolicy, bool, void>::
                    call_with_index(
                        policy, hpx::util::make_zip_iterator(first1, first2),
                        count, [pred, tok](std::size_t base_idx, 
                        zip_iterator it, std::size_t part_count) mutable
                        {   
                            util::loop_idx_n(
                                base_idx, it, part_count, tok,
                                [&pred, &tok](reference t, std::size_t i)
                                {
                                    using hpx::util::get;
                                    if((pred(get<0>(t), get<1>(t)) ||
                                        pred(get<1>(t), get<0>(t))))
                                        tok.cancel(i);
                                });
                        },
                        [=, &last1, &last2]
                        (std::vector<hpx::future<void> > &&) mutable -> bool
                        {
                            std::size_t mismatched = tok.get_data();
                            
                            std::advance(first1, mismatched);
                            std::advance(first2, mismatched);
                            
                            if(first1 != last1 && first2 != last2)
                                return pred(*first1, *first2);
                            else
                                return first2 != last2;
                            
                        });
            }
        };
        /// \endcond
    }


    /// Checks if the first range [first1, last1) is lexicographically less than
    /// the second range [first2, last2). uses operator< to comapre elements.
    ///
    /// \note   Complexity: At most 2 * min(N1, N2) applications of the comparison
    ///         operation <, where \code N1 = std::distance(first1, last) \endcode
    ///         and \code N2 = std::distance(first2, last2) \endcode .
    ///
    /// \tparam ExPolicy    The type of the execution policy to use (deduced).
    ///                     It describes the manner in which the execution
    ///                     of the algorithm may be parallelized and the manner
    ///                     in which it executes the assignments.
    /// \tparam InIter1     The type of the source iterators used for the
    ///                     first range (deduced).
    ///                     This iterator type must meet the requirements of an
    ///                     input iterator.
    /// \tparam InIter2     The type of the source iterators used for the
    ///                     second range (deduced).
    ///                     This iterator type must meet the requirements of an
    ///                     input iterator.
    ///
    /// \param policy       The execution policy to use for the scheduling of
    ///                     the iterations.
    /// \param first1       Refers to the beginning of the sequence of elements
    ///                     of the first range the algorithm will be applied to.
    /// \param last1        Refers to the end of the sequence of elements of
    ///                     the first range the algorithm will be applied to.
    /// \param first2       Refers to the beginning of the sequence of elements
    ///                     of the second range the algorithm will be applied to.
    /// \param last2        Refers to the end of the sequence of elements of
    ///                     the second range the algorithm will be applied to.
    ///
    /// The comparison operations in the parallel \a lexicographical_compare 
    /// algorithm invoked with an execution policy object of type 
    /// \a sequential_execution_policy execute in sequential order in the 
    /// calling thread.
    ///
    /// The comparison operations in the parallel \a lexicographical_compare 
    /// algorithm invoked with an execution policy object of type \a parallel_execution_policy
    /// or \a parallel_task_execution_policy are permitted to execute in an unordered
    /// fashion in unspecified threads, and indeterminately sequenced
    /// within each thread.
    ///
    /// \note     Lexicographical comparision is an operation with the following properties
    ///             - Two ranges are compared element by element
    ///             - The first mismatching element defines which range is lexicographically
    ///               \a less or \a greater than the other
    ///             - If one range is a prefix of another, the shorter range is
    ///               lexicographically \a less than the other
    ///             - If two ranges have equivalent elements and are of the same length,
    ///               then the ranges are lexicographically \a equal
    ///             - An empty range is lexicographically \a less than any non-empty
    ///               range
    ///             - Two empty ranges are lexicographically \a equal
    ///
    /// \returns  The \a lexicographically_compare algorithm returns a 
    ///           \a hpx::future<bool> if the execution policy is of type
    ///           \a sequential_task_execution_policy or
    ///           \a parallel_task_execution_policy and
    ///           returns \a bool otherwise.
    ///           The \a lexicographically_compare algorithm returns true 
    ///           if the first range is lexicographically less, otherwise 
    ///           it returns false.
    ///
    template <typename ExPolicy, typename InIter1, typename InIter2>
    inline typename boost::enable_if<
        is_execution_policy<ExPolicy>,
        typename detail::algorithm_result<ExPolicy, bool>::type
    >::type
    lexicographical_compare(ExPolicy && policy, InIter1 first1, InIter1 last1,
        InIter2 first2, InIter2 last2)
    {
        typedef typename std::iterator_traits<InIter1>::iterator_category
            iterator_category1;
        typedef typename std::iterator_traits<InIter2>::iterator_category
            iterator_category2;

        BOOST_STATIC_ASSERT_MSG(
            (boost::is_base_of<
                std::input_iterator_tag, iterator_category1
            >::value),
            "Requires at least input iterator.");

        BOOST_STATIC_ASSERT_MSG(
            (boost::is_base_of<
                std::input_iterator_tag, iterator_category2
            >::value),
            "Requires at least input iterator.");

        typedef typename boost::mpl::or_<
            is_sequential_execution_policy<ExPolicy>,
            boost::is_same<std::input_iterator_tag, iterator_category1>,
            boost::is_same<std::input_iterator_tag, iterator_category2>
        >::type is_seq;

        return detail::lexicographical_compare().call(
            std::forward<ExPolicy>(policy), is_seq(),
            first1, last1, first2, last2, detail::less_than());
    }

    /// Checks if the first range [first1, last1) is lexicographically less than
    /// the second range [first2, last2). uses a provided predicate to comapre 
    /// elements.
    ///
    /// \note   Complexity: At most 2 * min(N1, N2) applications of the comparison
    ///         operation, where \code N1 = std::distance(first1, last) \endcode
    ///         and \code N2 = std::distance(first2, last2) \endcode .
    ///
    /// \tparam ExPolicy    The type of the execution policy to use (deduced).
    ///                     It describes the manner in which the execution
    ///                     of the algorithm may be parallelized and the manner
    ///                     in which it executes the assignments.
    /// \tparam InIter1     The type of the source iterators used for the
    ///                     first range (deduced).
    ///                     This iterator type must meet the requirements of an
    ///                     input iterator.
    /// \tparam InIter2     The type of the source iterators used for the
    ///                     second range (deduced).
    ///                     This iterator type must meet the requirements of an
    ///                     input iterator.
    /// \tparam Pred        comparison function object that returns true if the
    ///                     first argument is \a less than the second
    ///
    /// \param policy       The execution policy to use for the scheduling of
    ///                     the iterations.
    /// \param first1       Refers to the beginning of the sequence of elements
    ///                     of the first range the algorithm will be applied to.
    /// \param last1        Refers to the end of the sequence of elements of
    ///                     the first range the algorithm will be applied to.
    /// \param first2       Refers to the beginning of the sequence of elements
    ///                     of the second range the algorithm will be applied to.
    /// \param last2        Refers to the end of the sequence of elements of
    ///                     the second range the algorithm will be applied to.
    /// \param op           Refers to the comparision function that the first
    ///                     and second ranges will be applied to
    ///
    /// The comparison operations in the parallel \a lexicographical_compare 
    /// algorithm invoked with an execution policy object of type 
    /// \a sequential_execution_policy execute in sequential order in the 
    /// calling thread.
    ///
    /// The comparison operations in the parallel \a lexicographical_compare 
    /// algorithm invoked with an execution policy object of type \a parallel_execution_policy
    /// or \a parallel_task_execution_policy are permitted to execute in an unordered
    /// fashion in unspecified threads, and indeterminately sequenced
    /// within each thread.
    ///
    /// \note     Lexicographical comparision is an operation with the following properties
    ///             - Two ranges are compared element by element
    ///             - The first mismatching element defines which range is lexicographically
    ///               \a less or \a greater than the other
    ///             - If one range is a prefix of another, the shorter range is
    ///               lexicographically \a less than the other
    ///             - If two ranges have equivalent elements and are of the same length,
    ///               then the ranges are lexicographically \a equal
    ///             - An empty range is lexicographically \a less than any non-empty
    ///               range
    ///             - Two empty ranges are lexicographically \a equal
    ///
    /// \returns  The \a lexicographically_compare algorithm returns a 
    ///           \a hpx::future<bool> if the execution policy is of type
    ///           \a sequential_task_execution_policy or
    ///           \a parallel_task_execution_policy and
    ///           returns \a bool otherwise.
    ///           The \a lexicographically_compare algorithm returns true 
    ///           if the first range is lexicographically less, otherwise 
    ///           it returns false.
    /// range [first2, last2), it returns false.
    ///
    template <typename ExPolicy, typename InIter1, typename InIter2, typename Pred>
    inline typename boost::enable_if<
        is_execution_policy<ExPolicy>,
        typename detail::algorithm_result<ExPolicy, bool>::type
    >::type
    lexicographical_compare(ExPolicy && policy, InIter1 first1, InIter1 last1,
        InIter2 first2, InIter2 last2, Pred && pred)
    {
       typedef typename std::iterator_traits<InIter1>::iterator_category
            iterator_category1;
        typedef typename std::iterator_traits<InIter2>::iterator_category
            iterator_category2;

        BOOST_STATIC_ASSERT_MSG(
            (boost::is_base_of<
                std::input_iterator_tag, iterator_category1
            >::value),
            "Requires at least input iterator.");

        BOOST_STATIC_ASSERT_MSG(
            (boost::is_base_of<
                std::input_iterator_tag, iterator_category2
            >::value),
            "Requires at least input iterator.");

        typedef typename boost::mpl::or_<
            is_sequential_execution_policy<ExPolicy>,
            boost::is_same<std::input_iterator_tag, iterator_category1>,
            boost::is_same<std::input_iterator_tag, iterator_category2>
        >::type is_seq;

        return detail::lexicographical_compare().call(
            std::forward<ExPolicy>(policy), is_seq(),
            first1, last1, first2, last2, 
            std::forward<Pred>(pred));
    }
}}}

#endif
