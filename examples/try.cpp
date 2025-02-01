// include bitsery.h to get serialization and deserialization classes
#include <bitsery/bitsery.h>
// in ordered to serialize/deserialize data to buffer, include buffer adapter
#include <bitsery/adapter/buffer.h>
// bitsery itself doesn't is lightweight, and doesnt include any unnessessary
// files, traits helps library to know how to use types correctly, in this case
// we'll be using vector both, to serialize/deserialize data and to store use as
// a buffer.
#include <bitsery/traits/vector.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/core/traits.h>
#include <bitsery/details/adapter_common.h>
#include <bitsery/ext/growable.h>
#include <bitsery/ext/version.h>
#include <stdint.h>
#include <iostream>
#include <iomanip>

namespace ai_operation_manager
{
    struct AIOP
    {
        enum class ProtoVersion:size_t
        {
            V0,
            Vend,
        } proto_version{ProtoVersion::V0};
        uint16_t number{0};
        std::string name{"name"};
        std::string description{"description"};

        struct Model
        {
            struct Param
            {
                Param() = default;
                Param(uint8_t input_number_flag, uint8_t output_number_flag, uint8_t input_layout_nhwc, int8_t input_order_rgb, uint8_t input_c, uint16_t input_h, uint16_t input_w, float mean_r, float mean_g, float mean_b, float scale)
                : input_number_flag(input_number_flag),
                output_number_flag(output_number_flag),
                input_layout_nhwc(input_layout_nhwc),
                input_order_rgb(input_order_rgb),
                input_c(input_c),
                input_h(input_h),
                input_w(input_w),
                mean_r(mean_r),
                mean_g(mean_g),
                mean_b(mean_b),
                scale(scale)
                {}

                uint8_t input_number_flag;
                uint8_t output_number_flag;
                uint8_t input_layout_nhwc;
                uint8_t input_order_rgb;
                uint8_t input_c;
                uint16_t input_h;
                uint16_t input_w;
                float mean_r;
                float mean_g;
                float mean_b;
                float scale;
                
            private:
                friend bitsery::Access;
                template<typename S>
                void serialize(S& ser)
                {
                    ser.ext(*this, bitsery::ext::Growable{}, [](S& s, Param& p)
                    {
                        s.value1b(p.input_number_flag);
                        s.value1b(p.output_number_flag);
                        s.value1b(p.input_layout_nhwc);
                        s.value1b(p.input_order_rgb);
                        s.value1b(p.input_c);
                        s.value2b(p.input_h);
                        s.value2b(p.input_w);
                        s.value4b(p.mean_r);
                        s.value4b(p.mean_g);
                        s.value4b(p.mean_b);
                        s.value4b(p.scale);
                    });
                }
            }param;

            std::vector<uint8_t> data;

        private:
            friend bitsery::Access;
            template<typename S>
            void serialize(S& s)
            {
                s.ext(*this, bitsery::ext::Growable{}, [](S& s, Model& m)
                {
                    s.object(m.param);
                    s.container1b(m.data, 0x40000000u-1);
                });
            }
        } model {};
        
        
    private:
        friend bitsery::Access;
        template<typename S>
        void serialize(S& ser)
        {
            ser.ext(*this, bitsery::ext::Version<2u>{}, [](S& se, AIOP& aiop, size_t version)
            {
                se.ext(aiop, bitsery::ext::Growable{}, [](S& s, AIOP& o1)
                {
                    s.value2b(o1.number);
                    s.text1b(o1.name,100);
                    s.text1b(o1.description,1000);
                    s.object(o1.model);
                });
            });
        }

public:
        void dumpInfo()
        {
            std::cout << "number: " << number << std::endl;
            std::cout << "name: " << name << std::endl;
            std::cout << "description: " << description << std::endl;
            std::cout << "model.param.input_number_flag: " << static_cast<int>(model.param.input_number_flag) << std::endl;
            std::cout << "model.param.output_number_flag: " << static_cast<int>(model.param.output_number_flag) << std::endl;
            std::cout << "model.param.input_layout_nhwc: " << static_cast<int>(model.param.input_layout_nhwc) << std::endl;
            std::cout << "model.param.input_order_rgb: " << static_cast<int>(model.param.input_order_rgb) << std::endl;
            std::cout << "model.param.input_c: " << static_cast<int>(model.param.input_c) << std::endl;
            std::cout << "model.param.input_h: " << model.param.input_h << std::endl;
            std::cout << "model.param.input_w: " << model.param.input_w << std::endl;
            std::cout << "model.param.mean_r: " << model.param.mean_r << std::endl;
            std::cout << "model.param.mean_g: " << model.param.mean_g << std::endl;
            std::cout << "model.param.mean_b: " << model.param.mean_b << std::endl;
            std::cout << "model.param.scale: " << model.param.scale << std::endl;
            std::cout << "model.data.size: " << model.data.size() << std::endl;
            for (auto i = 0u; i < model.data.size(); i++)
            {
                std::cout << "model.data[" << i << "]: " << static_cast<int>(model.data[i]) << std::endl;
            }
            std::cout << std::endl;
        }
    };
}

// some helper types
using Buffer = std::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
using InputAdapter = bitsery::InputBufferAdapter<Buffer>;
using namespace ai_operation_manager;

int main()
{
    AIOP data;
    data.number = 1;
    data.name = "name1";
    data.description = "description";
    data.model.param = {1,1,1,1,3,224,224,127.5,127.5,127.5,127.5};
    data.model.data = {1,2,3,4};

    // create buffer to store data
    Buffer buffer;
    // use quick serialization function,
    // it will use default configuration to setup all the nesessary steps
    // and serialize data to container
    auto writtenSize = bitsery::quickSerialization<OutputAdapter>(buffer, data);


    AIOP data2;

    // same as serialization, but returns deserialization state as a pair
    // first = error code, second = is buffer was successfully read from begin to
    // the end.
    auto state = bitsery::quickDeserialization<InputAdapter>({ buffer.begin(), writtenSize }, data2);
    data2.dumpInfo();
    std::cout << "writtenSize: " << writtenSize << std::endl;
    for (size_t i = 0; i < writtenSize; i++)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i] << " ";
    }
    std::cout << std::endl;
    

    // assert(state.first == bitsery::ReaderError::NoError && state.second);
    // assert(data.fs == res.fs && data.i == res.i && data.e == res.e);

    return 0;
}
