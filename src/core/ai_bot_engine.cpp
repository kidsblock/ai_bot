#include "ai_bot_engine_impl.h"

namespace ai_bot {
Engine& Engine::GetInstance() {
  return EngineImpl::GetInstance();
}
}  // namespace ai_bot