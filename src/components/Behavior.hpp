#pragma once

#include "../lib/Logger.hpp"

namespace mks {

class Behavior {
 public:
  Behavior();
  ~Behavior();

  // attachments
  void* Animation;
  void* Audio;
  void* Camera;
  void* Collider2D;
  void* Light;
  void* Renderer;
  void* RigidBody2D;
  void* Transform;
  void* Scripts;
  void BindScript(void*);

 private:
};

}  // namespace mks