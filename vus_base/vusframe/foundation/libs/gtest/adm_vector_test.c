#include "adm_vector.h"
#include <stdio.h>

int main()
{
    ADM_VECTOR_T* vec = adm_vector_new();
    adm_vector_pushback(vec, "qwwe");
    adm_vector_pushback(vec, "dff");
    adm_vector_pushback(vec, "fgbbh");
    adm_vector_pushback(vec, "etgg");
    adm_vector_pushback(vec, "err");

    for (int i = 0; i < adm_vector_get_size(vec); ++i) {
        printf("1 iter%d = %s\n", i, adm_iterator_get_value(vec, i));
    }
    adm_vector_erase(vec, 0);
    for (int i = 0; i < adm_vector_get_size(vec); ++i) {
        printf("2 iter%d = %s\n", i, adm_iterator_get_value(vec, i));
    }
    adm_vector_erase(vec, 4);
    for (int i = 0; i < adm_vector_get_size(vec); ++i) {
        printf("3 iter%d = %s\n", i, adm_iterator_get_value(vec, i));
    }

    adm_vector_popback(vec);
    adm_vector_popback(vec);
    adm_vector_popback(vec);
    for (int i = 0; i < adm_vector_get_size(vec); ++i) {
        printf("4 iter%d = %s\n", i, adm_iterator_get_value(vec, i));
    }

    adm_vector_clear(vec);
    printf("size = %d\n", adm_vector_get_size(vec));

    adm_vector_delete(vec);
    return 0;
}