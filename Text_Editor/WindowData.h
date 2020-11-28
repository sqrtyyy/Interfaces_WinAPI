//
// Created by Алексей on 30.10.2020.
//

#ifndef CLIONPROJECTS_WINDOWDATA_H
#define CLIONPROJECTS_WINDOWDATA_H

#include "Model.h"
#include "Viewer.h"
/**
 * @struct WindowData_t
 * @brief Struct which store information for instance of window.
 * @field viewerPtr Address of viewer.
 * @field modelPtr Address of model.
 */
typedef struct{
    Viewer* viewerPtr;
    Model* modelPtr;
}WindowData_t;



#endif //CLIONPROJECTS_WINDOWDATA_H
