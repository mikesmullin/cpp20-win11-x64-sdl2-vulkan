#include "Behavior.hpp"

namespace mks {

Behavior::Behavior() {
  mks::Logger::Infof("New Behavior");
}

Behavior::~Behavior() {
  mks::Logger::Infof("Behavior deleted");
}

void Behavior::BindScript(void* cls) {
  mks::Logger::Infof("Behavior.BindScript()");
}

}  // namespace mks