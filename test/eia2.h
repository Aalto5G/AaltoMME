#ifndef EIA2_H
#define EIA2_H

#include <glib.h>

void eia2(const void *k,
          const void *count, const guint8 bearer, const guint8 direction,
          const void* msg, const gsize mLen,
          void *digest);


#endif /* EIA2_H */
