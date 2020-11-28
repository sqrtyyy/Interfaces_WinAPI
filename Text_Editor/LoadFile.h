//
// Created by Алексей on 27.11.2020.
//

#ifndef CLIONPROJECTS_LOADFILE_H
#define CLIONPROJECTS_LOADFILE_H

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif
#ifdef __cplusplus
extern "C"
{
#endif
    DLL_EXPORT char* LoadFile(char const * fileName);

#ifdef __cplusplus
}
#endif


#endif //CLIONPROJECTS_LOADFILE_H
