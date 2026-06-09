#pragma once

namespace kernel {

struct Pair {
  int x;
  int y;
};

class Renderable {
 public:
  virtual ~Renderable() = default;

  virtual char Symbol() const = 0;
  virtual Pair Position() const = 0;
};

} 