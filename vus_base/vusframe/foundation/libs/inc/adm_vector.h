#ifndef VECTOR_H
#define VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ADM_VECTOR_ ADM_VECTOR_T;

ADM_VECTOR_T* adm_vector_new();
int adm_vector_get_size(const ADM_VECTOR_T* vec);
/*succuss:0 ; failed:non -0*/
int adm_vector_pushback(ADM_VECTOR_T* vec, const char* str);

/*succuss:0 ; failed:non -0*/
/*Removes the last element in the vector, effectively reducing the container size by one.*/
int adm_vector_popback(ADM_VECTOR_T* vec);

/*succuss:0 ; failed:non -0*/
/*Removes all elements from the vector (which are destroyed), leaving the container with a size of 0.*/
int adm_vector_clear(ADM_VECTOR_T* vec);

/*succuss:0 ; failed:non -0*/
/*Removes from the vector either a single element (position)*/
/*note: The value of index starts at 0*/
int adm_vector_erase(ADM_VECTOR_T* vec, const int index);

char* adm_iterator_get_value(const ADM_VECTOR_T* vec, const int index);
void adm_vector_delete(ADM_VECTOR_T* vec);

#ifdef __cplusplus
}
#endif

#endif  // VECTOR_H