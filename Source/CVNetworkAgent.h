//
//  CVNetworkAgent.h
//  CVNetwork
//
//  Created by Filipi Nascimento Silva on 8/27/13.
//  Copyright (c) 2013 Filipi Nascimento Silva. All rights reserved.
//

#ifndef CVNetwork_CVNetworkAgent_h
#define CVNetwork_CVNetworkAgent_h
#include "CVNetwork.h"


CV_INLINE CVAgentPath* CVAgentPathCreate(CVUInteger capacity){
	CVAgentPath* agentPath = calloc(1, sizeof(CVAgentPath));
	agentPath->count=0;
	agentPath->_capacity=capacity;
	if(capacity==0) {
		agentPath->data = NULL;
	}else{
		agentPath->data = calloc(agentPath->_capacity, sizeof(CVNetworkAgent));
	}
	return agentPath;
}

CV_INLINE void CVAgentPathDestroy(CVAgentPath* thePath){
	if(thePath!=NULL){
		if(thePath->data!=NULL){
			free(thePath->data);
		}
		free(thePath);
	}
}

CV_INLINE void CVAgentPathReallocToCapacity(CVUInteger newCapacity, CVAgentPath* thePath){
	if(thePath->data!=NULL){
		thePath->data = realloc(thePath->data,newCapacity*sizeof(CVNetworkAgent));
	}else{
		thePath->data = calloc(newCapacity, sizeof(CVNetworkAgent));
	}
	thePath->_capacity=newCapacity;
	if(thePath->_capacity<thePath->count) thePath->count = thePath->_capacity;
}

CV_INLINE void CVAgentPathPushAgent(CVNetworkAgent agent, CVAgentPath* thePath){
	if(thePath->_capacity < thePath->count+1){
		CVAgentPathReallocToCapacity(thePath->count*2+1, thePath);
	}
	thePath->data[thePath->count] = agent;
	thePath->count++;
}


CV_INLINE CVNetworkAgent CVAgentPathPopAgent(CVAgentPath* path){
	if(path->count>0){
		path->count--;
		return path->data[path->count];
	}else{
		CVNetworkAgent agent;
		agent.vertex = 0;
		agent.weight = NAN;
		agent.level = 0;
		return agent;
	}
}

CV_INLINE CVNetworkAgent* CVAgentPathLast(CVAgentPath* path){
	if(path->count>0){
		return path->data + (path->count - 1);
	}else{
		return NULL;
	}
}

CV_INLINE CVBool CVAgentPathIsEmpty(CVAgentPath* path){
	if(path->count>0){
		return CVFalse;
	}else{
		return CVTrue;
	}
}

CV_INLINE CVBool CVAgentPathContainsVertex(const CVAgentPath* path,CVIndex aVertex){
	CVIndex pathIndex;
	for (pathIndex=0; pathIndex< path->count - 1 ; pathIndex++) {
		if(path->data[pathIndex].vertex==aVertex){
			return CVTrue;
		}
	}
	return CVFalse;
}



CV_INLINE CVInteger CVNetworkPathSelfAvoidingNextBranch(const CVNetwork* network, const CVNetworkAgent* currentAgent, const CVAgentPath* path, CVFloat* choiceProbability, CVNetworkAgent* nextAgent){
	CVInteger currentLevel = currentAgent->level;
	CVIndex currentVertex = currentAgent->vertex;
	CVIndex branchIndex = currentAgent->branchIndex;
	CVFloat currentWeight = currentAgent->weight;
	CVFloat* edgesWeights = network->edgesWeights;
	CVSize vertexEdgesCount = network->vertexNumOfEdges[currentVertex];
	CVIndex* vertexEdgesList = network->vertexEdgesLists[currentVertex];
	CVIndex nextVertex = 0;
	
	
	//=CVFalse;
	CVFloat sumProbability = 0.0f;
	if(choiceProbability){
		CVIndex curBranch=0;;
		while(curBranch < vertexEdgesCount){
			CVFloat probability = 1.0;
			nextVertex = vertexEdgesList[curBranch++];
			if(network->edgeWeighted){
				probability = edgesWeights[network->vertexEdgesIndices[currentVertex][curBranch-1]];
			}else{
				probability = 1.0;
			}
			if(!CVAgentPathContainsVertex(path,nextVertex)){
				sumProbability+=probability;
			}
		}
		*choiceProbability = 1.0/sumProbability;
	}
	while(branchIndex < vertexEdgesCount){
		nextVertex = vertexEdgesList[branchIndex++];
		if(CVUnlikely(!network->verticesEnabled[nextVertex])){
			continue;
		}
		
		if(!CVAgentPathContainsVertex(path,nextVertex)){
			if(network->edgeWeighted){
				currentWeight *= edgesWeights[network->vertexEdgesIndices[currentVertex][branchIndex-1]];
			}
			nextAgent->vertex = nextVertex;
			nextAgent->level = currentLevel+1;
			nextAgent->weight = currentWeight;
			nextAgent->branchIndex=0;
			break;
		}
	}
	return branchIndex;
}

CV_INLINE CVInteger CVNetworkPathNextBranch(const CVNetwork* network, const CVNetworkAgent* currentAgent, const CVAgentPath* path, CVFloat* choiceProbability, CVNetworkAgent* nextAgent){
	CVInteger currentLevel = currentAgent->level;
	CVIndex currentVertex = currentAgent->vertex;
	CVIndex branchIndex = currentAgent->branchIndex;
	CVFloat currentWeight = currentAgent->weight;
	CVFloat* edgesWeights = network->edgesWeights;

	CVSize vertexEdgesCount = network->vertexNumOfEdges[currentVertex];
	CVIndex* vertexEdgesList = network->vertexEdgesLists[currentVertex];
	CVIndex nextVertex = 0;

	//=CVFalse;
	
	if(choiceProbability){
		CVFloat sumProbability = 0.0f;
		if(network->edgeWeighted){
			CVIndex curBranch=0;;
			while(curBranch < vertexEdgesCount){
				sumProbability+= edgesWeights[network->vertexEdgesIndices[currentVertex][curBranch-1]];
			}
		}else{
			sumProbability=vertexEdgesCount;
		}
	}
	while(branchIndex < vertexEdgesCount){
		nextVertex = vertexEdgesList[branchIndex++];
		if(CVUnlikely(!network->verticesEnabled[nextVertex])){
			break;
		}

		if(network->edgeWeighted){
			currentWeight *= network->edgesWeights[network->vertexEdgesIndices[currentVertex][branchIndex-1]];
		}
		nextAgent->vertex = nextVertex;
		nextAgent->level = currentLevel+1;
		nextAgent->weight = currentWeight;
		nextAgent->branchIndex=0;
		break;
	}

	return branchIndex;
}



#endif
