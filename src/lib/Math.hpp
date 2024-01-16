#pragma once

namespace mks {
class Math {
 public:
  static double round(double d);
  static double map(
      double n, double input_start, double input_end, double output_start, double output_end);
};
}  // namespace mks
