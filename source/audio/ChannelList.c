#include "ChannelList.h"
#include "HazeMacros.h"
#include "Result.h"
#include "audio/AudioEngine.h"
#include "audio/Channel.h"
#include "audio/ResultAudio.h"
#include <stdlib.h>
#include <string.h>

ChannelList *ChannelListNew(const AudioEngine *eng) {
  if (!eng) {
    return NULL;
  }

  ChannelList *clist = malloc(sizeof(ChannelList));
  if (!clist) {
    return NULL;
  }

  clist->quantity = 1;
  clist->channels = malloc(sizeof(Channel *));

  if (!clist->channels) {
    free(clist);
    return NULL;
  }

  clist->channels[0] = ChannelNew(eng, NULL);

  if (!clist->channels[0]) {
    free(clist->channels);
    free(clist);
    return NULL;
  }

  ChannelRename(clist->channels[0], "Master");

  return clist;
}


ResultAudio ChannelListAdd(ChannelList *clist, const AudioEngine *eng) {
  if (!clist || !eng) {
    return ResultAudioErr("invalid argument");
  }

  size_t index = clist->quantity;

  Channel **channels = realloc(
      clist->channels,
      sizeof(*clist->channels) * (index + 1)
  );

  if (!channels) {
    return ResultAudioErr("err alloc");
  }

  clist->channels = channels;

  Channel *channel = ChannelNew(eng, NULL);
  if (!channel) {
    return ResultAudioErr("err alloc");
  }

  char name[32];
  snprintf(name, sizeof(name), "Channel %zu", index + 1);

  ResultAudio renameResult = ChannelRename(channel, name);
  if (!ResultAudioIsOk(renameResult)) {
    ChannelFree(&channel);
    return ResultAudioErr(renameResult.msg);
  }

  clist->channels[index] = channel;
  clist->quantity++;

  return ResultAudioOk();
}


void ChannelListFree(ChannelList **chann) {
  PTR_FREE_ASSERT(chann);

  ChannelList *list = *chann;

  for (size_t i = 0; i < list->quantity; i++) {
    ChannelFree(&list->channels[i]);
  }

  free(list->channels);
  free(list);

  *chann = NULL;
}
