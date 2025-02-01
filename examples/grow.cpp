#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
// include traits for types, that we'll be using
#include <bitsery/traits/array.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>
// include extension that will allow to have backward/forward compatibility
#include <bitsery/ext/growable.h>
#include <stdio.h>

namespace MyTypes {

// define data
enum Color : uint8_t
{
  Red,
  Green,
  Blue
};

struct Vec3
{
  float x, y, z;
};


template<typename S>
void
serialize(S& s, Vec3& o)
{
  s.value4b(o.x);
  s.value4b(o.y);
  s.value4b(o.z);
}

struct Monster
{
  std::string name{};
  Vec3 pos;
  Color color{};
  Monster() = default;

private:
  // define serialize function as private, and give access to bitsery
  friend bitsery::Access;
  template<typename S>
  void serialize(S& s)
  {
    // forward/backward compatibility for weapons
    s.ext(*this, bitsery::ext::Growable{}, [](S& s, Monster& o1) {
      s.text1b(o1.name, 20);
      s.object(o1.pos);
      s.value1b(o1.color);
    });
  }
};

struct Vec2
{
  float x, y;
};


template<typename S>
void
serialize(S& s, Vec2& o)
{
  s.value4b(o.x);
  s.value4b(o.y);
}

struct MonsterV2
{
  std::string name{};
  Vec3 pos;
  Color color{};
  Vec2 pos2;
  MonsterV2() = default;

private:
  // define serialize function as private, and give access to bitsery
  friend bitsery::Access;
  template<typename S>
  void serialize(S& s)
  {
    // forward/backward compatibility for weapons
    s.ext(*this, bitsery::ext::Growable{}, [](S& s, MonsterV2& o1) {
      s.text1b(o1.name, 20);
      s.object(o1.pos);
      s.value1b(o1.color);
      s.object(o1.pos2);
    });
  }
};

}

// use fixed-size buffer
using Buffer = std::array<uint8_t, 10000>;
using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
using InputAdapter = bitsery::InputBufferAdapter<Buffer>;

int
main()
{
  // set some random data
  MyTypes::Monster data{};
  data.name = "lew";
  data.pos = { 1.0f, 1.0f, 1.0f };
  data.color = MyTypes::Color::Red;

  // create buffer to store data to
  Buffer buffer{};
  auto writtenSize = bitsery::quickSerialization<OutputAdapter>(buffer, data);
    printf("written %zu : ",writtenSize);
    for (auto i = 0; i < writtenSize+1; ++i)
    {
      printf("%02x ", buffer[i]);
    }
    printf("\n");


  MyTypes::MonsterV2 res{};
  // deserialize
  auto state = bitsery::quickDeserialization<InputAdapter>(
    { buffer.begin(), writtenSize }, res);

    printf("res.name: %s\n", res.name.c_str());
    printf("res.pos.x: %f\n", res.pos.x);
    printf("res.pos.y: %f\n", res.pos.y);
    printf("res.pos.z: %f\n", res.pos.z);
    printf("res.pos2.x: %f\n", res.pos2.x);
    printf("res.pos2.y: %f\n", res.pos2.y);
    printf("res.color: %d\n", res.color);
    res.pos2.x = 2.0f;
    res.pos2.y = 2.0f;
    res.pos.x = 2.0f;
    res.pos.y = 2.0f;
    res.pos.z = 2.0f;
    res.color = MyTypes::Color::Red;

    
  writtenSize = bitsery::quickSerialization<OutputAdapter>(buffer, res);
    printf("written %zu : ",writtenSize);
    for (auto i = 0; i < writtenSize; ++i)
    {
      printf("%02x ", buffer[i]);
    }
    printf("\n");

    state = bitsery::quickDeserialization<InputAdapter>(
    { buffer.begin(), writtenSize }, data);
    printf("data.name: %s\n", data.name.c_str());
    printf("data.pos.x: %f\n", data.pos.x);
    printf("data.pos.y: %f\n", data.pos.y);
    printf("data.pos.z: %f\n", data.pos.z);
    printf("data.color: %d\n", data.color);


  assert(state.first == bitsery::ReaderError::NoError && state.second);
}
