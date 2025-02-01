#ifndef JULES_EXT_VERSION_H
#define JULES_EXT_VERSION_H
#include "../details/adapter_common.h"
#include "../traits/core/traits.h"

namespace bitsery {

    namespace ext {

        template<size_t VERSION>
        class Version {
        public:

            template<typename Ser, typename T, typename Fnc>
            void serialize(Ser &ser, const T &v, Fnc &&fnc) const {
                details::writeSize(ser.adapter(), VERSION);
                fnc(ser, const_cast<T&>(v), VERSION);
            }

            template<typename Des, typename T, typename Fnc>
            void deserialize(Des &des, T &v, Fnc &&fnc) const {
                size_t version{};
                details::readSize(des.adapter(), version, 0u, std::false_type{});
                fnc(des, v, version);
            }

        };
    }

    namespace traits {
        template<typename T, size_t V>
        struct ExtensionTraits<ext::Version<V>, T> {
            using TValue = T;
            static constexpr bool SupportValueOverload = false;
            static constexpr bool SupportObjectOverload = false;
            static constexpr bool SupportLambdaOverload = true;
        };
    }

}

#endif //JULES_EXT_VERSION_H