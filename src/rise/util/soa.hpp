#pragma once

#include <tuple>
#include <SG14/slot_map.h>

namespace rise {

    template<typename T>
    struct ref_wrap : public std::reference_wrapper<T> {
        operator T &() const noexcept { return this->get(); }

        ref_wrap(T &other_) : std::reference_wrapper<T>(other_) {}

        void operator=(T &&other_) { this->get() = other_; }
    };

    enum class DataLayout {
        SoA, //structure of arrays
        AoS //array of structures
    };

    template<typename TContainer>
    class Iterator;

    template<template<typename...> class Container, DataLayout TDataLayout, typename TItem>
    struct DataLayoutPolicy;

    template<template<typename...> class Container,
            template<typename...> class TItem, typename... Types>
    struct DataLayoutPolicy<Container, DataLayout::AoS, TItem<Types...>> {
        using type = Container<TItem<Types...>>;
        using value_type = TItem<Types...> &;

        constexpr static value_type get(type &c_, std::size_t position_) {
            return value_type(*static_cast<TItem<Types...> *>(&c_[position_]));
        }

        constexpr static void resize(type &c_, std::size_t size_) { c_.resize(size_); }

        template<typename TValue>
        constexpr static void push_back(type &c_, TValue &&val_) { c_.push_back(val_); }

        static constexpr std::size_t size(type &c_) { return c_.size(); }
    };


    template<template<typename...> class Container,
            template<typename...> class TItem, typename... Types>
    struct DataLayoutPolicy<Container, DataLayout::SoA, TItem<Types...>> {
        using Key = size_t;
        using type = std::tuple<Container<Types>...>;
        using value_type = TItem<ref_wrap<Types>...>;

        constexpr static value_type get(type &c_, Key position_) {
            return doGet(c_, position_,
                    std::make_integer_sequence<unsigned, sizeof...(Types)>()); // unrolling parameter pack
        }

        constexpr static void resize(type &c_, std::size_t size_) {
            doResize(c_, size_,
                    std::make_integer_sequence<unsigned, sizeof...(Types)>()); // unrolling parameter pack
        }

        template<typename TValue>
        constexpr static void push_back(type &c_, TValue &&val_) {
            doPushBack(c_, std::forward<TValue>(val_),
                    std::make_integer_sequence<unsigned, sizeof...(Types)>()); // unrolling parameter pack
        }

        static constexpr std::size_t size(type &c_) { return std::get<0>(c_).size(); }

    private:

        template<unsigned... Ids>
        constexpr static auto
        doGet(type &c_, Key position_, std::integer_sequence<unsigned, Ids...>) {
            return value_type{ref_wrap(std::get<Ids>(c_)[position_])...}; // guaranteed copy elision
        }

        template<unsigned... Ids>
        constexpr static void
        doResize(type &c_, unsigned size_, std::integer_sequence<unsigned, Ids...>) {
            ( std::get<Ids>(c_).resize(size_), ... ); //fold expressions
        }

        template<typename TValue, unsigned... Ids>
        constexpr static void
        doPushBack(type &c_, TValue &&val_, std::integer_sequence<unsigned, Ids...>) {
            ( std::get<Ids>(c_).push_back(
                    std::get<Ids>(std::forward<TValue>(val_))), ... ); // fold expressions
        }
    };

    template<typename T>
    struct DefaultSlotMap : stdext::slot_map<T> {
        auto push_back(T val) {
            return stdext::slot_map<T>::insert(val);
        }

    };

    template<template<typename...> class TItem, typename... Types>
    struct DataLayoutPolicy<DefaultSlotMap, DataLayout::SoA, TItem<Types...>> {
        using Key = std::pair<unsigned, unsigned>;
        using type = std::tuple<DefaultSlotMap<Types>...>;
        using value_type = TItem<ref_wrap<Types>...>;

        constexpr static value_type get(type &c_, size_t position_) {
            return doGet(c_, position_,
                    std::make_integer_sequence<unsigned, sizeof...(Types)>()); // unrolling parameter pack
        }

        constexpr static value_type at(type &c_, Key position_) {
            return doAt(c_, position_,
                    std::make_integer_sequence<unsigned, sizeof...(Types)>()); // unrolling parameter pack
        }

        constexpr static size_t find(type &c_, Key position_) {
            return doFind(c_, position_,
                    std::make_integer_sequence<unsigned, sizeof...(Types)>()); // unrolling parameter pack
        }

        constexpr static void resize(type &c_, std::size_t size_) {
            doResize(c_, size_,
                    std::make_integer_sequence<unsigned, sizeof...(Types)>()); // unrolling parameter pack
        }

        template<typename TValue>
        constexpr static auto push_back(type &c_, TValue &&val_) {
            return doPushBack(c_, std::forward<TValue>(val_),
                    std::make_integer_sequence<unsigned, sizeof...(Types)>()); // unrolling parameter pack
        }

        constexpr static auto erase(type &c_, Key position_) {
            return doErase(c_, position_, std::make_integer_sequence<unsigned, sizeof...(Types)>()); // unrolling parameter pack
        }

        static constexpr std::size_t size(type &c_) { return std::get<0>(c_).size(); }

    private:

        template<unsigned... Ids>
        constexpr static auto
        doGet(type &c_, size_t position_, std::integer_sequence<unsigned, Ids...>) {
            return value_type{ref_wrap(*std::get<Ids>(c_).find_unchecked({position_, 0}))...}; // guaranteed copy elision
        }

        template<unsigned... Ids>
        constexpr static auto
        doAt(type &c_, Key position_, std::integer_sequence<unsigned, Ids...>) {
            return value_type{ref_wrap(std::get<Ids>(c_)[position_])...}; // guaranteed copy elision
        }

        template<unsigned... Ids>
        constexpr static size_t
        doFind(type &c_, Key position_, std::integer_sequence<unsigned, Ids...>) {
            return std::get<0>(c_).find(position_) - std::get<0>(c_).begin(); // guaranteed copy elision
        }

        template<unsigned... Ids>
        constexpr static void
        doResize(type &c_, unsigned size_, std::integer_sequence<unsigned, Ids...>) {
            ( std::get<Ids>(c_).resize(size_), ... ); //fold expressions
        }

        template<typename TValue, unsigned... Ids>
        constexpr static auto
        doPushBack(type &c_, TValue &&val_, std::integer_sequence<unsigned, Ids...>) {
            return ( std::get<Ids>(c_).push_back(
                    std::get<Ids>(std::forward<TValue>(val_))), ... ); // fold expressions
        }

        template< unsigned... Ids>
        constexpr static auto
        doErase(type &c_, Key position_, std::integer_sequence<unsigned, Ids...>) {
            return ( std::get<Ids>(c_).erase(position_), ... ); // fold expressions
        }
    };

    template<template<typename> class TContainer, DataLayout TDataLayout, typename TItem>
    using policy_t = DataLayoutPolicy<TContainer, TDataLayout, TItem>;

    template<template<typename> class TContainer, DataLayout TDataLayout, typename TItem>
    struct BaseContainer {
        using policy_t = policy_t<TContainer, TDataLayout, TItem>;
        using iterator = Iterator<BaseContainer<TContainer, TDataLayout, TItem>>;
        using Key = typename policy_t::Key;
        using difference_type = std::ptrdiff_t;
        using value_type = typename policy_t::value_type;
        using reference = typename policy_t::value_type &;
        using size_type = std::size_t;
        auto static constexpr data_layout = TDataLayout;

        BaseContainer() {}

        BaseContainer(size_t size_) {
            resize(size_);
        }

        template<typename Fwd>
        auto push_back(Fwd &&val) {
            return policy_t::push_back(mValues, std::forward<Fwd>(val));
        }

        void erase() {
            return policy_t::erase(mValues);
        }

        std::size_t size() {
            return policy_t::size(mValues);
        }

        value_type operator[](std::size_t position_) {
            return policy_t::get(mValues, position_);
        }

        value_type at(Key position_) {
            return policy_t::at(mValues, position_);
        }

        size_t find(Key position_) {
            return policy_t::find(mValues, position_);
        }

        void resize(size_t size_) {
            policy_t::resize(mValues, size_);
        }

        iterator begin() { return iterator(this, 0); }

        iterator end() { return iterator(this, size()); }

    private:

        typename policy_t::type mValues;

    };

    template<typename TContainer>
    class Iterator {

    private:
        using container_t = TContainer;
    public:
        using policy_t = typename container_t::policy_t;
        using difference_type = std::ptrdiff_t;
        using value_type = typename policy_t::value_type;
        using reference = value_type &;
        //using pointer			= value_type*;
        using iterator_category = std::bidirectional_iterator_tag;

        template<typename TTContainer>
        Iterator(TTContainer *container_, std::size_t position_ = 0):
                mContainer(container_), mIterPosition(position_) {
        }

        Iterator &operator=(Iterator const &other_) {
            mIterPosition = other_.mIterPosition;
            // mContainer = other_.mContainer;
        }

        friend bool operator!=(Iterator const &lhs, Iterator const &rhs) {
            return lhs.mIterPosition != rhs.mIterPosition;
        }

        friend bool operator==(Iterator const &lhs, Iterator const &rhs) {
            return !operator!=(lhs, rhs);
        }

        explicit operator bool() const {
            return mIterPosition != std::numeric_limits<std::size_t>::infinity();
        }

        Iterator &operator=(
                std::nullptr_t const &) { mIterPosition = std::numeric_limits<std::size_t>::infinity(); }

        template<typename T>
        void operator+=(T size_) { mIterPosition += size_; }

        template<typename T>
        void operator-=(T size_) { mIterPosition -= size_; }

        void operator++() { return operator+=(1); }

        void operator--() { return operator-=(1); }

        value_type operator*() {
            return (*mContainer)[mIterPosition];
        }


    private:
        container_t *mContainer = nullptr;
        std::size_t mIterPosition = std::numeric_limits<std::size_t>::infinity();

    };

    template<typename T>
    using DefaultVector = std::vector<T, std::allocator<T>>;


    template<typename... Types>
    using SoaVector = BaseContainer<DefaultVector, DataLayout::SoA, std::tuple<Types...>>;

    template<typename... Types>
    using SoaSlotMap = BaseContainer<DefaultSlotMap, DataLayout::SoA, std::tuple<Types...>>;

    // id + version
    using Key = std::pair<unsigned, unsigned>;
    const static std::pair NullKey = {std::numeric_limits<unsigned>::max(), std::numeric_limits<unsigned>::max()};

    template<typename... Types>
    bool contains(SoaSlotMap<Types...> const& map, Key key) {
        auto id = map.find(key);
        return id != map.size();
    }


}
