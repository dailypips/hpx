//  Copyright (c) 2011-2014 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPX_IOSTREAMS_SERVER_BUFFER_JUL_18_2014_0715PM)
#define HPX_IOSTREAMS_SERVER_BUFFER_JUL_18_2014_0715PM

#include <hpx/hpx_fwd.hpp>
#include <hpx/util/spinlock.hpp>
#include <hpx/util/move.hpp>

#include <vector>

#include <boost/shared_ptr.hpp>
#include <hpx/runtime/serialization/serialize.hpp>

namespace hpx { namespace iostreams { namespace detail
{
    struct buffer
    {
    private:
        typedef lcos::local::spinlock mutex_type;

    public:
        buffer()
          : data_(new std::vector<char>)
        {}

        buffer(buffer const& rhs)
          : data_(rhs.data_)
        {}

        buffer(buffer && rhs)
          : data_(rhs.data_)
        {
            rhs.data_.reset();
        }

        buffer& operator=(buffer const& rhs)
        {
            if (this != &rhs)
            {
                data_ = rhs.data_;
            }
            return *this;
        }

        buffer& operator=(buffer && rhs)
        {
            if (this != &rhs)
            {
                data_ = std::move(rhs.data_);
            }
            return *this;
        }

        bool empty() const
        {
            mutex_type::scoped_lock l(mtx_);
            return !data_.get() || data_->empty();
        }

        buffer init()
        {
            mutex_type::scoped_lock l(mtx_);

            buffer b;
            boost::swap(b.data_, data_);
            return b;
        }

        template <typename Char>
        std::streamsize write(Char const* s, std::streamsize n)
        {
            mutex_type::scoped_lock l(mtx_);
            std::copy(s, s + n, std::back_inserter(*data_));
            return n;
        }

        template <typename Mutex>
        void write(write_function_type const& f, Mutex& mtx)
        {
            typename mutex_type::scoped_lock l(mtx_);
            if (data_.get() && !data_->empty())
            {
                boost::shared_ptr<std::vector<char> > data(data_);
                data_.reset();
                l.unlock();

                typename Mutex::scoped_lock ll(mtx);
                f(*data);
            }
        }

    private:
        boost::shared_ptr<std::vector<char> > data_;

    private:
        friend class hpx::serialization::access;

        HPX_COMPONENT_EXPORT void save(
            serialization::output_archive& ar, unsigned) const;
        HPX_COMPONENT_EXPORT void load(
            serialization::input_archive& ar, unsigned);

        HPX_SERIALIZATION_SPLIT_MEMBER();

        mutable mutex_type mtx_;
    };
}}}

#endif
