#include "SampleList.h"
#include "HazeMacros.h"
#include "audio/AudioEngine.h"
#include "audio/ResultAudio.h"
#include "audio/Sample.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SampleList *SampleListNew(void) {
  SampleList *sList = malloc(sizeof *sList);
  if (!sList)
    return NULL;

  sList->samples = NULL;
  sList->len = 0;

  return sList;
}

void SampleListFree(SampleList **list) {
  PTR_FREE_ASSERT(list);

  for (uint64_t i = 0; i < (*list)->len; i++)
    SampleFree(&(*list)->samples[i]);

  free((*list)->samples);
  free(*list);

  *list = NULL;
}

ResultAudio SampleListImportByFile(SampleList *list, const AudioEngine *eng,
                                   const char *path) {
  if (!list || !path)
    return ResultAudioErr("invalid argument");

  uint64_t index = list->len;

  Sample *sample = SampleNew();
  if (!sample)
    return ResultAudioErr("failed to allocate sample");

  sample->id = index;

  ResultAudio result = SampleInitFromFile(sample, eng, path);
  if (!ResultAudioIsOk(result)) {
    SampleFree(&sample);
    return result;
  }

  const char *name = sample->sample_name;
  int occurrences = 0;

  for (uint64_t i = 0; i < index; i++) {
    Sample *existing = list->samples[i];

    if (existing && strcmp(existing->sample_name, name) == 0)
      occurrences++;
  }

  if (occurrences > 0) {
    size_t len = strlen(name);
    char *new_name = malloc(len + 32);

    if (!new_name) {
      SampleFree(&sample);
      return ResultAudioErr("failed to allocate sample name");
    }

    snprintf(new_name, len + 32, "%s #%d", name, occurrences);

    free(sample->sample_name);
    sample->sample_name = new_name;
  }

  Sample **new_samples =
      realloc(list->samples, sizeof(*list->samples) * (list->len + 1));

  if (!new_samples) {
    SampleFree(&sample);
    return ResultAudioErr("failed to resize sample list");
  }

  list->samples = new_samples;
  list->samples[index] = sample;
  list->len++;

  return ResultAudioOk();
}

uint64_t SampleListLen(SampleList *list) { return list ? list->len : 0; }

const char **SampleListStr(SampleList *list) {
  if (!list)
    return NULL;

  uint64_t len = SampleListLen(list);

  const char **strvec = malloc(sizeof(char *) * (len + 1));
  if (!strvec)
    return NULL;

  for (uint64_t i = 0; i < len; i++) {
    const char *name = list->samples[i]->sample_name;

    strvec[i] = strdup(name);

    if (!strvec[i]) {
      for (uint64_t j = 0; j < i; j++)
        free((void *)strvec[j]);

      free(strvec);
      return NULL;
    }
  }

  strvec[len] = NULL;

  return strvec;
}

ResultAudio SampleListDeleteSampleByName(
    SampleList *list,
    const char *name
) {
  /*
   * Validação básica dos argumentos.
   *
   * `list` precisa existir porque vamos acessar:
   *   - list->samples
   *   - list->len
   *
   * `name` também precisa existir porque será usado no strcmp().
   *
   * Retornamos erro sem executar nenhuma operação.
   */
  if (!list || !name)
    return ResultAudioErr("invalid argument");

  /*
   * Percorremos todos os samples atualmente armazenados.
   *
   * `list->len` representa a quantidade lógica de elementos.
   * Portanto, só precisamos examinar os índices [0, len).
   */
  for (uint64_t i = 0; i < list->len; i++) {
    Sample *sample = list->samples[i];

    /*
     * Há duas situações em que este elemento não pode ser
     * o sample procurado:
     *
     * 1. O ponteiro é NULL.
     * 2. O nome não coincide.
     *
     * Nesse caso, simplesmente continuamos para o próximo.
     */
    if (!sample || strcmp(sample->sample_name, name) != 0)
      continue;

    /*
     * Encontramos o sample.
     *
     * Primeiro liberamos o objeto apontado pelo elemento atual.
     *
     * Passar `&list->samples[i]` é importante porque SampleFree()
     * pode colocar o ponteiro em NULL depois de liberar a memória.
     */
    SampleFree(&list->samples[i]);

    /*
     * Agora precisamos compactar o vetor.
     *
     * Exemplo:
     *
     *   [A, B, C, D]
     *       ^
     *       i = 1
     *
     * Depois de remover B, queremos:
     *
     *   [A, C, D, NULL]
     *
     * Então cada elemento seguinte é movido uma posição para trás.
     *
     * O `j + 1 < list->len` garante que nunca acessaremos
     * uma posição além do último elemento válido.
     */
    for (uint64_t j = i; j + 1 < list->len; j++)
      list->samples[j] = list->samples[j + 1];

    /*
     * O último elemento agora ficou duplicado logicamente.
     *
     * Exemplo, depois da movimentação:
     *
     *   [A, C, D, D]
     *
     * A posição final já não pertence à lista, então limpamos
     * explicitamente esse ponteiro:
     *
     *   [A, C, D, NULL]
     */
    list->samples[list->len - 1] = NULL;

    /*
     * Reduzimos a quantidade lógica de elementos.
     *
     * Importante: não fazemos realloc aqui.
     *
     * A capacidade alocada do vetor continua a mesma.
     * Apenas `len` diminui.
     *
     * Isso evita uma realocação toda vez que um sample é removido.
     */
    list->len--;

    /*
     * A remoção foi concluída.
     *
     * Como o nome procurado deve corresponder a um elemento,
     * podemos terminar imediatamente a função.
     */
    return ResultAudioOk();
  }

  /*
   * Se chegamos aqui, percorremos toda a lista e nenhum sample
   * correspondeu ao nome informado.
   */
  return ResultAudioErr("sample not found");
}

Sample *SampleListGetSampleByName(SampleList *list, const char *name) {
  if (!list || !name)
    return NULL;

  for (uint64_t i = 0; i < list->len; i++) {
    Sample *sample = list->samples[i];

    if (sample && strcmp(sample->sample_name, name) == 0)
      return sample;
  }

  return NULL;
}
