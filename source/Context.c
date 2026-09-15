#include "Context.h"
#include "HazeMacros.h"
#include "audio/AudioEngine.h"
#include "fs/InstanceRegistry.h"
#include "fs/Paths.h"
#include "logc/log.h"
#include <stdlib.h>

Context *ContextNew(void) {
  Context *gi = malloc(sizeof(Context));
  if (!gi)
    return NULL;
  log_debug("CONTEXT Initializated.");

  gi->_audioEngine = AudioEngineNew();
  log_debug("CONTEXT Audio engine started.");

  gi->_session = SessionNew(NULL, gi->_audioEngine);
  log_debug("CONTEXT Session started.");

  gi->_paths = PathsNew();
  log_debug("CONTEXT Paths started.");
  gi->_instanceRegFile = InstanceRegistryNew(gi->_session, gi->_paths, 7192);
  log_debug("CONTEXT Registry started.");

  if (!gi->_session || !gi->_paths || !gi->_instanceRegFile) {
    ContextFree(&gi);
    return NULL;
  }

  return gi;
}

void ContextFree(Context **gi) {
  PTR_FREE_ASSERT(gi);

  if ((*gi)->_instanceRegFile)
    InstanceRegistryFree(&(*gi)->_instanceRegFile);

  if ((*gi)->_paths)
    PathsFree(&(*gi)->_paths);

  if ((*gi)->_session)
    SessionFree(&(*gi)->_session);

  if ((*gi)->_audioEngine) {
    AudioEngineFree(&(*gi)->_audioEngine);
  }

  free(*gi);
  *gi = NULL;
}

const Session *ContextGetSession(const Context *gi) { return gi->_session; }
const Paths *ContextGetPaths(const Context *gi) { return gi->_paths; }
const InstanceReg *ContextGetInstanceRegistry(const Context *gi) {
  return gi->_instanceRegFile;
}

const AudioEngine *ContextGetAudioEngine(const Context *ctx) {
  return ctx->_audioEngine;
}
