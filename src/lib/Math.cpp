#include "Math.hpp"

#include "Logger.hpp"

namespace mks {

double Math::map(
    double n, double input_start, double input_end, double output_start, double output_end) {
  double range = 1.0 * (output_end - output_start) / (input_end - input_start);
  return output_start + range * (n - input_start);
}

}  // namespace mks