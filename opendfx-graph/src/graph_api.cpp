#ifdef __cplusplus
#include <iostream>
#include <stdlib.h>
#include "graph.hpp"
#endif

#include "graph_api.h"

#ifdef __cplusplus
extern "C" {
#endif

GRAPH_HANDLE Graph_Create           (const char* name){
	std::string sname(name);
	std::cout << "Graph_Create called !!\n"; 
    opendfx::Graph *graph = new opendfx::Graph(sname, 0);
	return (GRAPH_HANDLE) graph;	
}

/*int          Graph_Distroy          (GRAPH_HANDLE gHandle){}

char*           Graph_getInfo           (GRAPH_HANDLE gHandle){}
ACCEL_HANDLE    Graph_addAccelByName    (GRAPH_HANDLE gHandle, const char* name){}
ACCEL_HANDLE    Graph_addAccelByHandle  (GRAPH_HANDLE gHandle, ACCEL_HANDLE aHandle){}
ACCEL_HANDLE    Graph_addInputNode      (GRAPH_HANDLE gHandle, const char* name){}
ACCEL_HANDLE    Graph_addOutputNode     (GRAPH_HANDLE gHandle, const char* name){}
BUFFER_HANDLE   Graph_addBufferByName   (GRAPH_HANDLE gHandle, const char* name){}
BUFFER_HANDLE   Graph_addBufferByHandle (GRAPH_HANDLE gHandle, BUFFER_HANDLE bHandle){}
LINK_HANDLE     Graph_addInputBuffer    (GRAPH_HANDLE gHandle, ACCEL_HANDLE aHandle, BUFFER_HANDLE  bHandle){}
LINK_HANDLE     Graph_addOutputBuffer   (GRAPH_HANDLE gHandle, ACCEL_HANDLE aHandle, BUFFER_HANDLE  bHandle){}
LINK_HANDLE     Graph_addLinkByHandle   (GRAPH_HANDLE gHandle, LINK_HANDLE  lHandle){}


int             Graph_delAccel          (GRAPH_HANDLE gHandle, ACCEL_HANDLE aHandle){}
int             Graph_delBuffer         (GRAPH_HANDLE gHandle, BUFFER_HANDLE bHandle){}
int             Graph_delLink           (GRAPH_HANDLE gHandle, LINK_HANDLE  lHandle){}
int             Graph_countAccel        (GRAPH_HANDLE gHandle){}
int             Graph_countBuffer       (GRAPH_HANDLE gHandle){}
int             Graph_countLink         (GRAPH_HANDLE gHandle){}
int             Graph_listAccels        (GRAPH_HANDLE gHandle){}
int             Graph_listBuffers       (GRAPH_HANDLE gHandle){}
int             Graph_listLinks         (GRAPH_HANDLE gHandle){}


char *          Graph_jsonAccels        (GRAPH_HANDLE gHandle){}
char *          Graph_jsonBuffers       (GRAPH_HANDLE gHandle){}
char *          Graph_jsonLinks         (GRAPH_HANDLE gHandle){}
char *          Graph_toJson            (GRAPH_HANDLE gHandle){}

char *          Graph_jsonAccelsDbg     (GRAPH_HANDLE gHandle){}
char *          Graph_jsonBuffersDbg    (GRAPH_HANDLE gHandle){}
char *          Graph_jsonLinksDbg      (GRAPH_HANDLE gHandle){}
char *          Graph_toJsonDbg         (GRAPH_HANDLE gHandle){}
*/

#ifdef __cplusplus
}
#endif

