//
//  CVNetwork.h
//  
//
//  Created by FIlipi Nascimento Silva on 26/09/12.
//
//

/**
 * @file   CVNetwork.h
 * @Author Me (me@example.com)
 * @date   September, 2008
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef CVNetwork_CVNetwork_h
#define CVNetwork_CVNetwork_h

#include "CVCommons.h"

typedef enum{
	CVStringPropertyType = 0,
	CVNumberPropertyType = 1,
	CVVector2DPropertyType = 2,
	CVVector3DPropertyType = 3,
} CVPropertyType;

typedef struct{
	CVSize* vertexNumOfEdges;
	CVSize* vertexCapacityOfEdges;
	CVIndex** vertexEdgesLists;
	CVIndex** vertexEdgesIndices;
	
	CVIndex* vertexNumOfInEdges;
	CVIndex* vertexCapacityOfInEdges;
	CVIndex** vertexInEdgesLists;
	CVIndex** vertexInEdgesIndices;
	
	CVIndex* edgeFromList;
	CVIndex* edgeToList;
	
	CVFloat* edgesWeights;
	CVFloat* verticesWeights;
	CVBool* verticesEnabled;
	
	CVSize vertexCapacity;
	CVSize edgesCapacity;
	
	CVSize edgesCount;
	CVSize verticesCount;
	
	CVBool editable;
	CVBool directed;
	CVBool edgeWeighted;
	CVBool vertexWeighted;
	
	CVString* vertexNames;
    
	CVString* propertiesNames;
	void** propertiesData;
	CVPropertyType* propertiesTypes;
	CVSize propertiesCount;
	
} CVNetwork;

typedef struct{
	CVIndex vertex;
	CVIndex level;
	CVFloat weight;
	CVIndex branchIndex;
} CVNetworkAgent;

typedef struct{
	CVNetworkAgent* data;
	CVSize count;
	CVSize _capacity;
} CVAgentPath;


/**
 * @brief Example showing how to document a function with Doxygen.
 *
 * Description of what the function does. This part may refer to the parameters
 * of the function, like @p param1 or @p param2. A word of code can also be
 * inserted like @c this which is equivalent to <tt>this</tt> and can be useful
 * to say that the function returns a @c void or an @c int. If you want to have
 * more than one word in typewriter font, then just use @<tt@>.
 * We can also include text verbatim,
 * @verbatim like this@endverbatim
 * Sometimes it is also convenient to include an example of usage:
 * @code
 * BoxStruct *out = Box_The_Function_Name(param1, param2);
 * printf("something...\n");
 * @endcode
 * Or,
 * @code{.py}
 * pyval = python_func(arg1, arg2)
 * print pyval
 * @endcode
 * when the language is not the one used in the current source file (but
 * <b>be careful</b> as this may be supported only by recent versions
 * of Doxygen). By the way, <b>this is how you write bold text</b> or,
 * if it is just one word, then you can just do @b this.
 * @param param1 Description of the first parameter of the function.
 * @param param2 The second one, which follows @p param1.
 * @return Describe what the function returns.
 * @see Box_The_Second_Function
 * @see Box_The_Last_One
 * @see http://website/
 * @note Something to note.
 * @warning Warning.
 */
CVBool CVNetworkAddNewEdges(CVNetwork* network, CVIndex* fromIndices, CVIndex* toIndices, CVFloat* weights, CVSize count);
void CVNetworkDestroy(CVNetwork* network);
CVNetwork* CVNewNetworkFromXNETFile(FILE* networkFile);

CVNetwork* CVNewNetwork(CVSize verticesCount, CVBool edgeWeighted, CVBool directed);
void CVNetworkWriteToFile(CVNetwork* theNetwork, FILE* networkFile);

CVNetwork* CVNewNetworkWithNetwork(CVNetwork* originalNetwork, CVBool edgeWeighted, CVBool directed);

CVNetwork* CVNewRegular2DNetwork(CVSize rows, CVSize columns, CVBool toroidal);
void CVNetworkAppendProperty(CVNetwork* theNetwork, CVString name, CVPropertyType type, void* data);


CV_INLINE void CVNetworkGrowEdgesCount(CVNetwork* network,CVSize growSize){
	network->edgesCount+=growSize;
	if(CVUnlikely(network->edgesCapacity < network->edgesCount)){
		network->edgesCapacity = 2 * network->edgesCount;
		network->edgeFromList = realloc(network->edgeFromList, sizeof(CVIndex)*network->edgesCapacity);
		network->edgeToList = realloc(network->edgeToList, sizeof(CVIndex)*network->edgesCapacity);
		if(network->edgeWeighted)
			network->edgesWeights = realloc(network->edgesWeights, sizeof(CVFloat)*network->edgesCapacity);
	}
}
CV_INLINE void CVNetworkGrowEdgesCapacity(CVNetwork* network,CVSize capacityIncrease){
	CVSize newCapacity = network->edgesCount+capacityIncrease;
	if(CVUnlikely(network->edgesCapacity < newCapacity)){
		network->edgesCapacity = 2 * newCapacity;
		network->edgeFromList = realloc(network->edgeFromList, sizeof(CVIndex)*network->edgesCapacity);
		network->edgeToList = realloc(network->edgeToList, sizeof(CVIndex)*network->edgesCapacity);
		if(network->edgeWeighted)
			network->edgesWeights = realloc(network->edgesWeights, sizeof(CVFloat)*network->edgesCapacity);
	}
}
CV_INLINE void CVNetworkGrowVertexSetEdgeForVertex(CVNetwork* network,CVIndex edgeIndex,CVIndex vertexIndex,CVIndex toVertexIndex){
	network->vertexNumOfEdges[vertexIndex]++;
	if(CVUnlikely(network->vertexCapacityOfEdges[vertexIndex] < network->vertexNumOfEdges[vertexIndex])){
		network->vertexCapacityOfEdges[vertexIndex]=network->vertexNumOfEdges[vertexIndex] * 2;
		network->vertexEdgesLists[vertexIndex] = (CVIndex*) realloc(network->vertexEdgesLists[vertexIndex], sizeof(CVIndex)*network->vertexCapacityOfEdges[vertexIndex]);
		network->vertexEdgesIndices[vertexIndex] = (CVIndex*) realloc(network->vertexEdgesIndices[vertexIndex], sizeof(CVIndex)*network->vertexCapacityOfEdges[vertexIndex]);
	}
	network->vertexEdgesLists[vertexIndex][network->vertexNumOfEdges[vertexIndex]-1]=toVertexIndex;
	network->vertexEdgesIndices[vertexIndex][network->vertexNumOfEdges[vertexIndex]-1]=edgeIndex;
}

CV_INLINE void CVNetworkGrowVertexSetInEdgeForVertex(CVNetwork* network,CVIndex edgeIndex,CVIndex vertexIndex,CVIndex toVertexIndex){
	network->vertexNumOfInEdges[vertexIndex]++;
	if(CVUnlikely(network->vertexCapacityOfInEdges[vertexIndex] < network->vertexNumOfInEdges[vertexIndex])){
		network->vertexCapacityOfInEdges[vertexIndex]=network->vertexNumOfInEdges[vertexIndex] * 2;
		network->vertexInEdgesLists[vertexIndex] = (CVIndex*) realloc(network->vertexInEdgesLists[vertexIndex], sizeof(CVIndex)*network->vertexCapacityOfInEdges[vertexIndex]);
		network->vertexInEdgesIndices[vertexIndex] = (CVIndex*) realloc(network->vertexInEdgesIndices[vertexIndex], sizeof(CVIndex)*network->vertexCapacityOfInEdges[vertexIndex]);
	}
	network->vertexInEdgesLists[vertexIndex][network->vertexNumOfInEdges[vertexIndex]-1]=toVertexIndex;
	network->vertexInEdgesIndices[vertexIndex][network->vertexNumOfInEdges[vertexIndex]-1]=edgeIndex;
}


CV_INLINE CVBool CVNetworkAddNewEdge(CVNetwork* network, CVIndex fromIndex, CVIndex toIndex, CVFloat weight){
	CVIndex initialEdge = network->edgesCount;
	CVSize verticesCount = network->verticesCount;
	if (toIndex >= verticesCount||fromIndex >= verticesCount) {
		return CVFalse;
	}
	CVNetworkGrowEdgesCapacity(network,1);
	network->edgeFromList[initialEdge]=fromIndex;
	network->edgeToList[initialEdge]=toIndex;
	
	CVNetworkGrowVertexSetEdgeForVertex(network,initialEdge,fromIndex,toIndex);
	
	if(network->edgeWeighted&&weight>=0){
		network->edgesWeights[initialEdge]=weight;
	}else if(network->edgeWeighted){
		network->edgesWeights[initialEdge]=1.0f;
	}
	if(!network->directed){
		//printf("Index: %lu toIndex:%lu fromIndex:%lu\n",i+initialEdge,toIndex,fromIndex);
		CVNetworkGrowVertexSetEdgeForVertex(network,initialEdge,toIndex,fromIndex);
		//printf("OK\n");
	}else{
		CVNetworkGrowVertexSetInEdgeForVertex(network,initialEdge,toIndex,fromIndex);
	}
	network->edgesCount++;
	return CVTrue;
}

CV_INLINE CVBool CVNetworkAddNewEdgeAndIntegrateWeight(CVNetwork* network, CVIndex fromIndex, CVIndex toIndex, CVFloat weight){
	CVIndex initialEdge = network->edgesCount;
	CVSize verticesCount = network->verticesCount;
	if (toIndex >= verticesCount||fromIndex >= verticesCount) {
		return CVFalse;
	}
	
	CVBool edgeFound = CVFalse;
	CVIndex i;
	CVSize toVerticesCount = network->vertexNumOfEdges[fromIndex];
	CVIndex* toVertices = network->vertexEdgesLists[fromIndex];
	for (i=0; i<toVerticesCount; i++) {
		if(toVertices[i]==toIndex){
			edgeFound = CVTrue;
			break;
		}
	}
	
	if(edgeFound){
		if(network->edgeWeighted&&weight>0){
			network->edgesWeights[network->vertexEdgesIndices[fromIndex][i]]+=weight;
		}
	}else{
		CVNetworkGrowEdgesCapacity(network,1);
		network->edgeFromList[initialEdge]=fromIndex;
		network->edgeToList[initialEdge]=toIndex;
		
		CVNetworkGrowVertexSetEdgeForVertex(network,initialEdge,fromIndex,toIndex);
		
		if(network->edgeWeighted&&weight>=0){
			network->edgesWeights[initialEdge]=weight;
		}else if(network->edgeWeighted){
			network->edgesWeights[initialEdge]=1.0f;
		}
		if(!network->directed){
			//printf("Index: %lu toIndex:%lu fromIndex:%lu\n",i+initialEdge,toIndex,fromIndex);
			//FIXME: Directed Networks
			CVNetworkGrowVertexSetEdgeForVertex(network,initialEdge,toIndex,fromIndex);
			//printf("OK\n");
		}else{
			CVNetworkGrowVertexSetInEdgeForVertex(network,initialEdge,toIndex,fromIndex);
		}
		network->edgesCount++;
	}
	return CVTrue;
}

#endif


