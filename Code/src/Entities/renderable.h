#pragma once

namespace kernel {

struct Pair {
  int x = 0;
  int y = 0;
};

class Renderable {
 public:
  virtual ~Renderable() = default;
  virtual char symbol() const = 0;
  virtual Pair position() const = 0;
};

}  // namespace kernel