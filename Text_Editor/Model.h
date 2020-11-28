//
// Created by Алексей on 16.10.2020.
//

#ifndef CLIONPROJECTS_MODEL_H
#define CLIONPROJECTS_MODEL_H

#include <windows.h>

/**
 * @struct string_t
 * @brief String which doesn't ends by 0
 * @field length Length of string
 * @brief string Address of first element of string.
 */
typedef struct {
    UINT length;
    char* string;
}string_t;


/**
 * @struct Model
 * @brief Stores the text and data required to work with it.
 * @field strings - All text from file.
 * @field lengths - Array of start indexes of different lines separated by new lines.
 * @field maxLen - Maximum string's length.
 * @field linesNumber - number of lines in file.
 */
typedef struct{
    char* strings;
    UINT* lengths;
    UINT maxLen;
    UINT linesNumber;
}Model;

/**
 * Function inits Model
 * @param fileName The name of file which should be readed.
 * @param modelPtr Address of struct which will store text and associated with it additional information.
 * @return If modelPtr is filled successfully.
 */
BOOLEAN InitModel(char const * fileName, Model* modelPtr);
/**
 * @brief Frees the memory which was allocated for storing modelPtr data.
 * @param modelPtr
 */
void DeleteModel(Model* modelPtr);
/**
 * @brief Returns the string which is idx-th in order in text.
 * @param modelPtr Stores text's information.
 * @param idx Index of string.
 * @return String from text.
 */
string_t getString(Model const * modelPtr, size_t idx);



#endif //CLIONPROJECTS_MODEL_H
