//  Copyright (c) 2014 Anton Bikineev
//  Copyright (c) 2014 Thomas Heller
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef HPX_SERIALIZATION_POLYMORPHIC_INTRUSIVE_FACTORY_HPP
#define HPX_SERIALIZATION_POLYMORPHIC_INTRUSIVE_FACTORY_HPP

#include <hpx/runtime/serialization/serialization_fwd.hpp>

#include <hpx/util/jenkins_hash.hpp>
#include <hpx/util/safe_lexical_cast.hpp>
#include <hpx/util/static.hpp>

#include <boost/preprocessor/stringize.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include <boost/atomic.hpp>
#include <boost/mpl/bool.hpp>

#include <hpx/config/warnings_prefix.hpp>

namespace hpx { namespace serialization { namespace detail
{
    class HPX_EXPORT polymorphic_intrusive_factory: boost::noncopyable
    {
    public:
        typedef void* (*ctor_type) ();
        typedef boost::unordered_map<std::string,
                ctor_type, hpx::util::jenkins_hash> ctor_map_type;

        static polymorphic_intrusive_factory& instance()
        {
            hpx::util::static_<polymorphic_intrusive_factory> factory;
            return factory.get();
        }

        void register_class(const std::string& name, ctor_type fun)
        {
            map_.emplace(name, fun);
        }

        template <class T>
        T* create(const std::string& name) const
        {
            return static_cast<T*>(map_.at(name)());
        }

        friend struct hpx::util::static_<polymorphic_intrusive_factory>;

        ctor_map_type map_;
    };

    template <class T>
    struct register_class_name
    {
        register_class_name()
        {
            T* t = 0; //dirty
            polymorphic_intrusive_factory::instance().
              register_class(
                t->T::hpx_serialization_get_name(), //non-virtual call
                &factory_function
              );
        }

        static void* factory_function()
        {
            return new T;
        }

        register_class_name& instantiate()
        {
            return *this;
        }

        static register_class_name instance;
    };

    template <class T>
    register_class_name<T> register_class_name<T>::instance;

}}}

#define HPX_SERIALIZATION_ADD_INTRUSIVE_MEMBERS_WITH_NAME(Class, Name)        \
  template <class> friend                                                     \
  struct ::hpx::serialization::detail::register_class_name;                   \
                                                                              \
  virtual std::string hpx_serialization_get_name() const                      \
  {                                                                           \
      hpx::serialization::detail::register_class_name<                        \
          Class>::instance.instantiate();                                     \
      return Name;                                                            \
  }                                                                           \
/**/

#define HPX_SERIALIZATION_POLYMORPHIC_WITH_NAME(Class, Name)                  \
  HPX_SERIALIZATION_ADD_INTRUSIVE_MEMBERS_WITH_NAME(Class, Name);             \
  virtual void load(hpx::serialization::input_archive& ar, unsigned n)        \
  {                                                                           \
      serialize<hpx::serialization::input_archive>(ar, n);                    \
  }                                                                           \
  virtual void save(hpx::serialization::output_archive& ar, unsigned n) const \
  {                                                                           \
      const_cast<Class*>(this)->                                              \
          serialize<hpx::serialization::output_archive>(ar, n);               \
  }                                                                           \
  HPX_SERIALIZATION_SPLIT_MEMBER();                                           \
/**/

#define HPX_SERIALIZATION_POLYMORPHIC_WITH_NAME_SPLITTED(Class, Name)         \
  HPX_SERIALIZATION_ADD_INTRUSIVE_MEMBERS_WITH_NAME(Class, Name);             \
  virtual void load(hpx::serialization::input_archive& ar, unsigned n)        \
  {                                                                           \
      load<hpx::serialization::input_archive>(ar, n);                         \
  }                                                                           \
  virtual void save(hpx::serialization::output_archive& ar, unsigned n) const \
  {                                                                           \
      save<hpx::serialization::output_archive>(ar, n);                        \
  }                                                                           \
/**/

#define HPX_SERIALIZATION_POLYMORPHIC_ABSTRACT(Class)                         \
  virtual std::string hpx_serialization_get_name() const = 0;                 \
  virtual void load(hpx::serialization::input_archive& ar, unsigned n)        \
  {                                                                           \
      serialize<hpx::serialization::input_archive>(ar, n);                    \
  }                                                                           \
  virtual void save(hpx::serialization::output_archive& ar, unsigned n) const \
  {                                                                           \
      const_cast<Class*>(this)->                                              \
          serialize<hpx::serialization::output_archive>(ar, n);               \
  }                                                                           \
  HPX_SERIALIZATION_SPLIT_MEMBER()                                            \
/**/

#define HPX_SERIALIZATION_POLYMORPHIC_ABSTRACT_SPLITTED(Class)                \
  virtual std::string hpx_serialization_get_name() const = 0;                 \
  virtual void load(hpx::serialization::input_archive& ar, unsigned n)        \
  {                                                                           \
      load<hpx::serialization::input_archive>(ar, n);                         \
  }                                                                           \
  virtual void save(hpx::serialization::output_archive& ar, unsigned n) const \
  {                                                                           \
      save<hpx::serialization::output_archive>(ar, n);                        \
  }                                                                           \
/**/

#define HPX_SERIALIZATION_POLYMORPHIC(Class)                                  \
  HPX_SERIALIZATION_POLYMORPHIC_WITH_NAME(Class, BOOST_PP_STRINGIZE(Class))   \
/**/

#define HPX_SERIALIZATION_POLYMORPHIC_SPLITTED(Class)                         \
  HPX_SERIALIZATION_POLYMORPHIC_WITH_NAME_SPLITTED(                           \
      Class, BOOST_PP_STRINGIZE(Class))                                       \
/**/

namespace hpx { namespace serialization { namespace detail {

    struct unique_suffix_for_template_name: boost::noncopyable
    {
        static unique_suffix_for_template_name& instance()
        {
            hpx::util::static_<unique_suffix_for_template_name> instance;
            return instance.get();
        }

        std::string get_new_suffix()
        {
            const uint32_t temp = ++suffix;
            return util::safe_lexical_cast<std::string>(temp);
        }

    private:
        unique_suffix_for_template_name():
            suffix(0U)
        {}

        friend struct hpx::util::static_<unique_suffix_for_template_name>;
        boost::atomic<boost::uint32_t> suffix;
    };

    template <class T>
    std::string get_unique_suffix_for_template_name()
    {
        static std::string suffix =
            unique_suffix_for_template_name::instance().get_new_suffix();
        return suffix;
    }

}}}

#include <hpx/config/warnings_suffix.hpp>

#define HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE_WITH_NAME(Class, Name)             \
  HPX_SERIALIZATION_POLYMORPHIC_WITH_NAME(Class, (Name +                          \
    ::hpx::serialization::detail::get_unique_suffix_for_template_name<Class>()))  \
/**/

#define HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE_WITH_NAME_SPLITTED(Class, Name)    \
  HPX_SERIALIZATION_POLYMORPHIC_WITH_NAME_SPLITTED(Class, (Name +                 \
    ::hpx::serialization::detail::get_unique_suffix_for_template_name<Class>()))  \
/**/

#define HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE(Class)                             \
  HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE_WITH_NAME(                               \
      Class, BOOST_PP_STRINGIZE(Class) BOOST_PP_STRINGIZE(__COUNTER__))           \
/**/

#define HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE_SPLITTED(Class)                    \
  HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE_WITH_NAME_SPLITTED(                      \
      Class, BOOST_PP_STRINGIZE(Class) BOOST_PP_STRINGIZE(__COUNTER__))           \
/**/

#endif
