#include <concepts>
#include <cstddef>
#include <print>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>

template <typename T>
concept string_viewable = std::convertible_to<T, std::string_view>;

class string_twine
{
    /*
    indexes:
    0: twine
    1: const char *
    2: const string *
    3: string_view
    */

    using child = std::variant<const string_twine *, const char *, const std::string *, std::string_view>;
    child lhs_;
    child rhs_;
    size_t lhs_kind_ = std::variant_npos;
    size_t rhs_kind_ = std::variant_npos;

    explicit string_twine(size_t kind) : lhs_kind_(kind) {}

    explicit string_twine(child lhs, size_t lhs_kind, child rhs, size_t rhs_kind)
        : lhs_(lhs), rhs_(rhs), lhs_kind_(lhs_kind), rhs_kind_(rhs_kind)
    {
    }

    bool
    is_null() const
    {
        return lhs_kind_ == std::variant_npos;
    }

    bool
    is_empty() const
    {
        return is_empty_string(lhs_, lhs_kind_);
    }

    bool
    is_empty_string(const child &hs, size_t hs_kind) const
    {
        return hs_kind == 2 && std::get<2>(hs)->empty();
    }
    bool
    is_nullary() const
    {
        return is_null() || is_empty();
    }

    bool
    is_unary() const
    {
        return is_empty_string(rhs_, rhs_kind_) && !is_nullary();
    }

    bool
    is_single_string_view() const
    {
        if (!is_empty_string(rhs_, rhs_kind_)) {
            return false;
        }

        if (is_empty_string(lhs_, lhs_kind_)) {
            return true;
        }

        switch (lhs_kind_) {
        case 1:
        case 2:
        case 3:
            return true;
        default:
            return false;
        }
    }

    std::string_view
    get_single_string_view() const
    {
        if (is_empty_string(lhs_, lhs_kind_)) {
            return std::string_view();
        }
        switch (lhs_kind_) {
        case 1:
            return std::string_view(std::get<1>(lhs_));
        case 2:
            return std::string_view(*std::get<2>(lhs_));
        case 3:
            return std::get<3>(lhs_);
        default:
            return std::string_view();
        }
    }

    void
    printOneChild(std::stringstream &ss, child ptr, size_t kind) const
    {
        if (is_empty_string(ptr, kind)) {
            return;
        }
        switch (kind) {
        case std::variant_npos:
            break;
        case 0:
            std::get<0>(ptr)->print(ss);
            break;
        case 1:
            ss << std::get<1>(ptr);
            break;
        case 2:
            ss << *std::get<2>(ptr);
            break;
        case 3:
            ss << std::get<3>(ptr);
            break;
        }
    }

    void
    print(std::stringstream &ss) const
    {

        printOneChild(ss, lhs_, lhs_kind_);
        printOneChild(ss, rhs_, rhs_kind_);
    }

    std::string
    to_string(std::stringstream &ss) const
    {
        if (is_single_string_view()) {
            return std::string(get_single_string_view());
        }
        print(ss);
        return ss.str();
    }

  public:
    explicit string_twine(const string_twine &lhs, const string_twine &rhs)
        : lhs_(&lhs), rhs_(&rhs), lhs_kind_(0), rhs_kind_(0)
    {
    }

    string_twine() = default;
    string_twine(const string_twine &) = default;

    string_twine(std::nullptr_t) = delete;
    string_twine &operator=(const string_twine &) = delete;

    template <string_viewable T> string_twine(T val) : lhs_(val), lhs_kind_(lhs_.index()) {}

    string_twine
    concat(const string_twine &suffix) const
    {
        if (is_null() || suffix.is_null()) {
            return string_twine(std::variant_npos);
        }
        if (is_empty()) {
            return suffix;
        }
        if (suffix.is_empty()) {
            return *this;
        }
        child new_lhs, new_rhs;
        new_lhs = this;
        new_rhs = &suffix;
        size_t new_lhs_kind = 0, new_rhs_kind = 0;
        if (is_unary()) {
            new_lhs = lhs_;
            new_lhs_kind = lhs_kind_;
        }
        if (suffix.is_unary()) {
            new_rhs = suffix.lhs_;
            new_rhs_kind = suffix.lhs_kind_;
        }

        return string_twine(new_lhs, new_lhs_kind, new_rhs, new_rhs_kind);
    }
    std::string
    str() const
    {
        if (lhs_kind_ == 2 && is_empty_string(rhs_, rhs_kind_)) {
            return *std::get<const std::string *>(lhs_);
        }
        std::stringstream ss;
        return to_string(ss);
    }
};

string_twine
operator,(const string_twine &lhs, const string_twine &rhs)
{
    return lhs.concat(rhs);
}

string_twine
operator,(const char *lhs, const std::string_view &rhs)
{
    return string_twine(lhs, rhs);
}

string_twine
operator,(const std::string_view &lhs, const char *rhs)
{
    return string_twine(lhs, rhs);
}

int
main()
{
    std::string_view sv = "Hello", sv2 = "World";

    auto s = string_twine(sv).concat(" ").concat(sv2).str();
    std::println("{}", s);

    auto s2 = (string_twine(sv), " ", sv2).str();
    std::println("{}", s2);
}
