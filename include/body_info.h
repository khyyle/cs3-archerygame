#ifndef BODY_INFO_H
#define BODY_INFO_H

#include "body.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct body_info {
  const char *tag;
  int32_t hp;
} body_info_t;

#define BODY_TAG_PLAYER "player"
#define BODY_TAG_GROUND "ground"
#define BODY_TAG_ARROW "arrow"
#define BODY_TAG_PARTICLE "particle"
#define BODY_TAG_CRATE "crate"

static inline body_info_t *body_info_init(const char *tag, int32_t hp) {
  body_info_t *info = malloc(sizeof(*info));
  assert(info);
  info->tag = tag;
  info->hp = hp;
  return info;
}

static inline bool body_has_tag(body_t *body, const char *tag) {
  body_info_t *info = body_get_info(body);
  return info != NULL && strcmp(info->tag, tag) == 0;
}

#endif
