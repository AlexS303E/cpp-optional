#include <stdexcept>
#include <utility>
#include <type_traits>

class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;

    Optional(const T& value) : is_initialized_(true) {
        new (data_) T(value);
    }

    Optional(T&& value) : is_initialized_(true) {
        new (data_) T(std::move(value));
    }

    Optional(const Optional& other) : is_initialized_(other.is_initialized_) {
        if (other.is_initialized_) {
            new (data_) T(*other);
        }
    }

    Optional(Optional&& other) noexcept
        : is_initialized_(other.is_initialized_) {
        if (other.is_initialized_) {
            new (data_) T(std::move(*reinterpret_cast<T*>(other.data_)));
        }
    }

    ~Optional() {
        Reset();
    }

    bool HasValue() const {
        return is_initialized_;
    }

    T& Value()& {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return **this;
    }

    const T& Value() const& {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return **this;
    }

    T&& Value()&& {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return std::move(**this);
    }

    void Reset() {
        if (is_initialized_) {
            reinterpret_cast<T*>(data_)->~T();
            is_initialized_ = false;
        }
    }

    template <typename... Args>
    void Emplace(Args&&... args) {
        Reset();
        new (data_) T(std::forward<Args>(args)...);
        is_initialized_ = true;
    }

    T* operator->() {
        return reinterpret_cast<T*>(data_);
    }

    const T* operator->() const {
        return reinterpret_cast<const T*>(data_);
    }

    Optional& operator=(const T& value) {
        if (is_initialized_) {
            **this = value;
        }
        else {
            new (data_) T(value);
            is_initialized_ = true;
        }
        return *this;
    }

    Optional& operator=(T&& value) {
        if (is_initialized_) {
            **this = std::move(value);
        }
        else {
            new (data_) T(std::move(value));
            is_initialized_ = true;
        }
        return *this;
    }

    Optional& operator=(const Optional& rhs) {
        if (this != &rhs) {
            if (rhs.is_initialized_) {
                if (is_initialized_) {
                    **this = *rhs;
                }
                else {
                    new (data_) T(*rhs);
                    is_initialized_ = true;
                }
            }
            else {
                Reset();
            }
        }
        return *this;
    }

    Optional& operator=(Optional&& rhs) noexcept {
        if (this != &rhs) {
            if (rhs.is_initialized_) {
                if (is_initialized_) {
                    **this = std::move(*rhs);
                }
                else {
                    new (data_) T(std::move(*rhs));
                    is_initialized_ = true;
                }
            }
            else {
                Reset();
            }
        }
        return *this;
    }

    T& operator*()& {
        return *reinterpret_cast<T*>(data_);
    }

    const T& operator*() const& {
        return *reinterpret_cast<const T*>(data_);
    }

    T&& operator*()&& {
        return std::move(*reinterpret_cast<T*>(data_));
    }

private:
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
};
