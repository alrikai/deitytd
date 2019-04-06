/* TowerDispatcher.hpp -- part of the DietyTD Model subsystem implementation
 *
 * Copyright (C) 2015 Alrik Firl
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_MULTI_DISPATCH_HPP
#define TD_MULTI_DISPATCH_HPP

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

/*
//a bit of nastiness to allow for unique indexing for
//each class to tbe used with the dispatcher. Adapted
//from chapter 11 of Modern C++ Design
#define IMPLEMENT_INDEXABLE_CLASS(class_t)       \
    static int& get_class_static_idx()           \
    {                                            \
        static int index = -1;                   \
        return index;                            \
    }                                            \
    virtual int& get_class_idx()                 \
    {                                            \
        assert(typeid(*this) == typeid(class_t));\
        return get_class_static_idx();           \
    }
*/

// using the same manner or hash combining that boost does
template <class T>
inline void combine_hashes(std::size_t &seed, const T &hash) {
  std::hash<T> hasher;
  seed ^= hasher(hash) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// have a hash function for vector<std::type_index>, so that we can have support
// for variable-length combinations
template <class T> class dispatch_hash;
template <> class dispatch_hash<std::vector<std::type_index>> {
public:
  // need to determine if this is a good hash function still...
  std::size_t operator()(const std::vector<std::type_index> &key) const {
    assert(key.size() > 0);
    std::size_t hash_seed = 0;
    std::for_each(key.begin(), key.end(),
                  [&hash_seed](std::type_index key_val) {
                    // std::size_t val_hash =
                    // std::hash<std::type_index>()(key_val);
                    combine_hashes(hash_seed, key_val.hash_code());
                  });
    return hash_seed;
  }
};

// start with just double dispatch, then move up from there
template <typename return_t,
          typename callback_t, // = return_t (*)(base_t* ...),
          class... base_types>
class Dispatcher {
  using keys_type = std::vector<std::type_index>;

public:
  template <class... concrete_types> void add(callback_t td_fcn) {
    // make the key for the given concrete types
    keys_type type_ids{};
    generate_type_key<sizeof...(concrete_types),
                      concrete_types...>::append_type(type_ids);

    int type_counter = 0;
    std::for_each(
        type_ids.begin(), type_ids.end(), [&type_counter](std::type_index idx) {
          std::string typeidx_name = idx.name();
          std::cout << "type #" << type_counter++ << ": " << typeidx_name
                    << " --> hash code: " << idx.hash_code() << std::endl;
        });
    dispatch_hash<keys_type> hasher;
    std::cout << "Overall hash: " << hasher(type_ids) << std::endl;

    // check if the key already exists (would need to double check the hashing
    // function..)
    auto found_id = callbacks_.find(type_ids);
    if (found_id != callbacks_.end())
      std::cout << "NOTE: collision in the callbacks map -- this really "
                   "shouldnt happen"
                << std::endl;

    callbacks_.insert(std::make_pair(type_ids, td_fcn));
  }

  return_t invoke(base_types *... types) {
    // make the key for the given types
    keys_type type_ids{};
    generate_object_keys(type_ids, types...);

    int type_counter = 0;
    std::for_each(
        type_ids.begin(), type_ids.end(), [&type_counter](std::type_index idx) {
          std::string typeidx_name = idx.name();
          std::cout << "type #" << type_counter++ << ": " << typeidx_name
                    << " --> hash code: " << idx.hash_code() << std::endl;
        });
    dispatch_hash<keys_type> hasher;
    std::cout << "Overall hash: " << hasher(type_ids) << std::endl;

    // check if the key already exists (would need to double check the hashing
    // function..)
    auto found_id = callbacks_.find(type_ids);
    if (found_id != callbacks_.end())
      return callbacks_[type_ids](types...);
    else
      std::cout << "NOTE: no matching function for the given type" << std::endl;
    return return_t{};
  }

  // returns whether the input combination is a valid one
  bool is_combination(base_types *... types) {
    // make the key for the given types
    keys_type type_ids{};
    generate_object_keys(type_ids, types...);
    auto found_id = callbacks_.find(type_ids);
    return found_id != callbacks_.end();
  }

private:
  ////////////////////////////////////////////////////////////////
  // generates the type_index lists for the type lists (no objects)
  ////////////////////////////////////////////////////////////////
  template <int N, typename... T> struct generate_type_key;
  template <typename T0, typename... T> struct generate_type_key<1, T0, T...> {
    static void append_type(std::vector<std::type_index> &t_list) {
      t_list.emplace_back(std::type_index(typeid(T0)));
    }
  };
  template <int N, typename key_t, typename... key_types>
  struct generate_type_key<N, key_t, key_types...> {
    static void append_type(keys_type &type_ids) {
      type_ids.emplace_back(typeid(key_t));
      generate_type_key<N - 1, key_types...>::append_type(type_ids);
    }
  };

  ////////////////////////////////////////////////////////////////
  // generates the type_index lists for the object lists
  ////////////////////////////////////////////////////////////////
  void generate_object_keys(keys_type &type_ids) {}
  template <typename obj_t, typename... obj_types>
  void generate_object_keys(keys_type &type_ids, obj_t *t_obj,
                            obj_types... obj_list) {
    // NOTE: we are dereferencing the pointer here, otherwise it'll give the
    // POINTER type, not the object type
    type_ids.emplace_back(typeid(*t_obj));
    generate_object_keys(type_ids, obj_list...);
  }

  std::unordered_map<keys_type, callback_t, dispatch_hash<keys_type>>
      callbacks_;
};

/*
Until I figure out how to do this part correctly, we can just have explicit
overloads for the different number of template types.
--> to do this part correctly, I need to figure out how to take 2 parameter
packs, such that I get a pack of objects of the base type, then I need to cast
them to the types in the other pack, while using them as the parameters to for a
function call. Will likely involve making a compile-time index set, then
recursing over the two packs to do the casting. Not sure what I can do for the
function call part of it however -- unless I can bind the casted objects to a
function object, or maybe something involving tuples?
--> need to think about this one some more
The use of the non-type function parameter is also rather problematic, as we
can't use it anymore -- if we have a variadic type list of base types, that has
to be the last parameter, hence we cant have a dependant function template
parameter.
*/

template <typename return_t, class... T> class TowerDispatcher;

// 2 parameters
template <typename return_t, class base_t1, class base_t2>
class TowerDispatcher<return_t, base_t1, base_t2> {
public:
  template <class concrete_t1, class concrete_t2,
            return_t (*callback_fcn)(concrete_t1 *, concrete_t2 *)>
  void add() {
    struct trampoline {
      static return_t local_trampoline(base_t1 *base1, base_t2 *base2) {
        return callback_fcn(dynamic_cast<concrete_t1 *>(base1),
                            dynamic_cast<concrete_t2 *>(base2));
      }
    };
    return backend_dispatch_.template add<concrete_t1, concrete_t2>(
        &trampoline::local_trampoline);
  }

  return_t invoke(base_t1 *base1, base_t2 *base2) {
    return backend_dispatch_.invoke(base1, base2);
  }

  bool is_combination(base_t1 *base1, base_t2 *base2) {
    return backend_dispatch_.is_combination(base1, base2);
  }

private:
  typedef return_t (*callback_t)(base_t1 *, base_t2 *);
  Dispatcher<return_t, callback_t, base_t1, base_t2> backend_dispatch_;
};

// 3 parameters
template <typename return_t, class base_t1, class base_t2, class base_t3>
class TowerDispatcher<return_t, base_t1, base_t2, base_t3> {
public:
  template <class concrete_t1, class concrete_t2, class concrete_t3,
            return_t (*callback_fcn)(concrete_t1 *, concrete_t2 *,
                                     concrete_t3 *)>
  void add() {
    struct trampoline {
      static return_t local_trampoline(base_t1 *base1, base_t2 *base2,
                                       base_t3 *base3) {
        return callback_fcn(dynamic_cast<concrete_t1 *>(base1),
                            dynamic_cast<concrete_t2 *>(base2),
                            dynamic_cast<concrete_t3 *>(base3));
      }
    };
    return backend_dispatch_
        .template add<concrete_t1, concrete_t2, concrete_t3>(
            &trampoline::local_trampoline);
  }

  return_t invoke(base_t1 *base1, base_t2 *base2, base_t3 *base3) {
    return backend_dispatch_.invoke(base1, base2, base3);
  }

  bool is_combination(base_t1 *base1, base_t2 *base2, base_t3 *base3) {
    return backend_dispatch_.is_combination(base1, base2, base3);
  }

private:
  typedef return_t (*callback_t)(base_t1 *, base_t2 *, base_t3 *);
  Dispatcher<return_t, callback_t, base_t1, base_t2, base_t3> backend_dispatch_;
};

// 4 parameters
template <typename return_t, class base_t1, class base_t2, class base_t3,
          class base_t4>
class TowerDispatcher<return_t, base_t1, base_t2, base_t3, base_t4> {
public:
  template <class concrete_t1, class concrete_t2, class concrete_t3,
            class concrete_t4,
            return_t (*callback_fcn)(concrete_t1 *, concrete_t2 *,
                                     concrete_t3 *, concrete_t4 *)>
  void add() {
    struct trampoline {
      static return_t local_trampoline(base_t1 *base1, base_t2 *base2,
                                       base_t3 *base3, base_t4 *base4) {
        return callback_fcn(dynamic_cast<concrete_t1 *>(base1),
                            dynamic_cast<concrete_t2 *>(base2),
                            dynamic_cast<concrete_t3 *>(base3),
                            dynamic_cast<concrete_t4 *>(base4));
      }
    };
    return backend_dispatch_
        .template add<concrete_t1, concrete_t2, concrete_t3, concrete_t4>(
            &trampoline::local_trampoline);
  }

  return_t invoke(base_t1 *base1, base_t2 *base2, base_t3 *base3,
                  base_t4 *base4) {
    return backend_dispatch_.invoke(base1, base2, base3, base4);
  }

  bool is_combination(base_t1 *base1, base_t2 *base2, base_t3 *base3,
                      base_t4 *base4) {
    return backend_dispatch_.is_combination(base1, base2, base3, base4);
  }

private:
  typedef return_t (*callback_t)(base_t1 *, base_t2 *, base_t3 *, base_t4 *);
  Dispatcher<return_t, callback_t, base_t1, base_t2, base_t3, base_t4>
      backend_dispatch_;
};

/*
template <typename ... types>
struct typelist
{
   using types_tuple = typename std::tuple<types ...>;
   template<std::size_t N>
   using type_at = typename std::tuple_element<N, types_tuple>::type;
};

template <typename return_t, class ... base_types>
class RuneDispatcher
{
public:
    template <class ... concrete_types>
    void add()
    {
        struct trampoline
        {
            typedef return_t (*callback_fcn) (concrete_types* ...);
            static return_t local_trampoline(base_types* ... base_objs)
            {
                auto base_type_objs = std::make_tuple(base_objs ...);
                switch (sizeof ...(base_objs))
                {
                    case 2:
                        return callback_fcn(
                          dynamic_cast<typename typelist<concrete_types
...>::template type_at<0>*> (std::get<0>(base_type_objs)), dynamic_cast<typename
typelist<concrete_types ...>::template type_at<1>*>
(std::get<1>(base_type_objs))); break; case 3: return callback_fcn(
                          dynamic_cast<typename typelist<concrete_types
...>::template type_at<0>*> (std::get<0>(base_type_objs)), dynamic_cast<typename
typelist<concrete_types ...>::template type_at<1>*>
(std::get<1>(base_type_objs)), dynamic_cast<typename typelist<concrete_types
...>::template type_at<2>*> (std::get<2>(base_type_objs))); break; case 4:
                        return callback_fcn(
                          dynamic_cast<typename typelist<concrete_types
...>::template type_at<0>*> (std::get<0>(base_type_objs)), dynamic_cast<typename
typelist<concrete_types ...>::template type_at<1>*>
(std::get<1>(base_type_objs)), dynamic_cast<typename typelist<concrete_types
...>::template type_at<2>*> (std::get<2>(base_type_objs)), dynamic_cast<typename
typelist<concrete_types ...>::template type_at<3>*>
(std::get<3>(base_type_objs))); break; case 5: return callback_fcn(
                          dynamic_cast<typename typelist<concrete_types
...>::template type_at<0>*> (std::get<0>(base_type_objs)), dynamic_cast<typename
typelist<concrete_types ...>::template type_at<1>*>
(std::get<1>(base_type_objs)), dynamic_cast<typename typelist<concrete_types
...>::template type_at<2>*> (std::get<2>(base_type_objs)), dynamic_cast<typename
typelist<concrete_types ...>::template type_at<3>*>
(std::get<3>(base_type_objs)), dynamic_cast<typename typelist<concrete_types
...>::template type_at<4>*> (std::get<4>(base_type_objs))); break; default:
                        return callback_fcn(
                          dynamic_cast<typename typelist<concrete_types
...>::template type_at<0>*> (std::get<0>(base_type_objs)), dynamic_cast<typename
typelist<concrete_types ...>::template type_at<1>*>
(std::get<1>(base_type_objs)), dynamic_cast<typename typelist<concrete_types
...>::template type_at<2>*> (std::get<2>(base_type_objs)), dynamic_cast<typename
typelist<concrete_types ...>::template type_at<3>*>
(std::get<3>(base_type_objs)), dynamic_cast<typename typelist<concrete_types
...>::template type_at<4>*> (std::get<4>(base_type_objs)), dynamic_cast<typename
typelist<concrete_types ...>::template type_at<5>*>
(std::get<5>(base_type_objs)));
                }
            }
        };
        return backend_dispatch_.template add<concrete_types
...>(&trampoline::local_trampoline);
    }

    return_t invoke(base_types* ... base_objs)
    {
        return backend_dispatch_.invoke(base_objs ...);
    }
private:
    typedef return_t (*callback_t) (base_types* ...);
    Dispatcher <return_t, callback_t, base_types ...> backend_dispatch_;

};
*/

/*
//some limitations:
//  only works with functions, not functors
//  doesnt support symmetry
//  will have trouble with
//  can use static_cast rather than dynamic_cast, iff we dont use any virtual
inheritance
//  and no non-virtual diamond-multiple inheritance
template <typename return_t, class .. base_types>
class TowerDispatcher
{
public:
    template <typename callback_fcn, class ... concrete_types>
    void add()
    {
        struct trampoline
        {
            static return_t local_trampoline(base_types* ... base_objs)
            {
                //return the callback_fcn with casted parameters (casting from
base to concrete types) return callback_fcn( dynamic_cast<LHS_t*> (b_lhs),
                       dynamic_cast<RHS_t*> (b_rhs));
            }
        };

        return backend_dispatch_.template add<concrete_types
...>(&trampoline::local_trampoline);
    }

    return_t invoke(base_types* ... base_objs)
    {
        return backend_dispatch_.invoke(base_objs ...);
    }

private:
    typedef return_t (*callback_t) (base_types* ...);
    Dispatcher <return_t, callback_t, base_types* ...> backend_dispatch_;
};
*/

#endif
