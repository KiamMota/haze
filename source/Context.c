#include "Context.h"
#include "HazeMacros.h"
#include "fs/InstanceRegistry.h"
#include "fs/Paths.h"
#include <stdlib.h>

Context *ContextNew(void) {
  Context *gi = malloc(sizeof(Context));
  if (!gi)
    return NULL;

  gi->_session = SessionNew(NULL);
  gi->_paths = PathsNew();
  gi->_instanceRegFile = InstanceRegistryNew(gi->_session, gi->_paths, 7192);

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

  free(*gi);
  *gi = NULL;
}


const Session* ContextGetSession(const Context* gi) {

  return gi->_session;
}
const Paths* ContextGetPaths(const Context* gi) {
  return gi->_paths;

}
const InstanceReg* ContextGetInstanceRegistry(const Context* gi) {
  return gi->_instanceRegFile;
}
