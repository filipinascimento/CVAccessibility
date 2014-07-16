//
//  CVNetworkAccessibility.c
//  CVNetwork
//
//  Created by Filipi Nascimento Silva on 8/27/13.
//  Copyright (c) 2013 Filipi Nascimento Silva. All rights reserved.
//

#include "CVNetworkAccessibility.h"
#include "CVNetworkAgent.h"


CVBool CVNetworkCalculateAccessibility_implementation(const CVNetwork* network, CVFloatArray* accessibility, CVBool selfAvoided, CVIndex level, CVBool ballMeasurement,CVBool usePathDiversity, CVIntegerArray* accessedCount, CVAccessProbabilities* accessProbabilities, CVOperationControl* operationControl){
	CVSize verticesCount = network->verticesCount;

	CVInteger* currentProgress = NULL;
	void (*updateCallback)(CVOperationControl*)  = NULL;
	void (*streamCallback)(CVOperationControl*, CVIndex index, const char* format,...)  = NULL;
	if(operationControl){
		operationControl->maxProgress = verticesCount;
		operationControl->currentProgress = 0;
		currentProgress = &(operationControl->currentProgress);
		updateCallback = operationControl->updateCallback;
		streamCallback = operationControl->streamCallback;
	}
	if(updateCallback){
		updateCallback(operationControl);
	}
	CVFloatArrayReallocToCapacity(verticesCount, accessibility);
	CVFloatArraySetCount(verticesCount, accessibility);
	CVInteger* accessedCountData = NULL;
	if(accessedCount){
		CVIntegerArrayReallocToCapacity(verticesCount, accessedCount);
		CVIntegerArraySetCount(verticesCount, accessedCount);
		accessedCountData = accessedCount->data;
		CVIndex i;
		for (i=0; i<verticesCount; i++) {
			if(network->verticesEnabled[i]){
				accessedCountData[i]  = 0;
			}
		}
	}
	CVIndex i;
	for (i=0; i<verticesCount; i++) {
		if(network->verticesEnabled[i]){
			accessibility->data[i]  = 0.0;
		}
	}

	CVFloat* accessibilityData = accessibility->data;

	CVIntegerArray frequencyIndicesList;
	CVIntegerArrayInitWithCapacity(16, &frequencyIndicesList);

	CVAgentPath* path = CVAgentPathCreate(4*level);
	CVIndex initialVertex;
	for (initialVertex=0; initialVertex<verticesCount; initialVertex++) {
		frequencyIndicesList.count=0;
		double probabilitiesWeightSum = 0.0f;
		CVNetworkAgent initialAgent;
		initialAgent.level=0;
		initialAgent.vertex=initialVertex;
		initialAgent.branchIndex=0;
		initialAgent.weight=1.0;
		CVFloat* transitionFrequencies = calloc(verticesCount, sizeof(CVFloat));

		CVAgentPathPushAgent(initialAgent, path);

		while(!CVAgentPathIsEmpty(path)){
			CVNetworkAgent* currentAgent = CVAgentPathLast(path);

			CVNetworkAgent nextAgent;
			nextAgent.level  = 0;
			nextAgent.vertex = 0;
			nextAgent.weight = 0.0f;

			CVFloat probability = 1.0;
			if(selfAvoided){
				currentAgent->branchIndex = CVNetworkPathSelfAvoidingNextBranch(network, currentAgent, path, (usePathDiversity)?NULL:&probability, &nextAgent);
			}else{
				currentAgent->branchIndex = CVNetworkPathNextBranch(network, currentAgent, path, (usePathDiversity)?NULL:&probability, &nextAgent);
			}
			nextAgent.weight*=probability;
			
			if(!ballMeasurement){
				if(nextAgent.level<=currentAgent->level){
					CVAgentPathPopAgent(path);
				}else{
					if(nextAgent.level >= level){
						probabilitiesWeightSum+=nextAgent.weight;
						if(!transitionFrequencies[nextAgent.vertex]){
							CVIntegerArrayAdd(nextAgent.vertex, &frequencyIndicesList);
						}
						transitionFrequencies[nextAgent.vertex]+=nextAgent.weight;
					}else{
						CVAgentPathPushAgent(nextAgent, path);
					}
				}
			}else{
				if(nextAgent.level<=currentAgent->level){
					CVAgentPathPopAgent(path);
				}else{
					probabilitiesWeightSum+=nextAgent.weight;
					if(!transitionFrequencies[nextAgent.vertex]){
						CVIntegerArrayAdd(nextAgent.vertex, &frequencyIndicesList);
					}
					transitionFrequencies[nextAgent.vertex]+=nextAgent.weight;
					if(nextAgent.level < level){
						CVAgentPathPushAgent(nextAgent, path);
					}
				}
			}
		}
		
		
		//Reduction
		if(probabilitiesWeightSum>0.0){
			double diversityValue = 0.0;
			CVIndex i;
			
			if(accessProbabilities){
				CVInitAccessProbabilities(accessProbabilities+initialVertex, frequencyIndicesList.count);
			}
			for (i=0;i<frequencyIndicesList.count;i++) {
				double probability = transitionFrequencies[frequencyIndicesList.data[i]]/probabilitiesWeightSum;
				diversityValue -= probability*log(probability);
				if(accessProbabilities){
					accessProbabilities[initialVertex].probabilities[i] = probability;
					accessProbabilities[initialVertex].vertexIndices[i] = frequencyIndicesList.data[i];
				}
				//	printf("\t%g-->%"CVSizeScan"\n",probability,frequencyIndicesList.data[i]);
			}
			//printf("---\n");
			accessibilityData[initialVertex] = (float)exp(diversityValue);
			if(accessedCountData){
				accessedCountData[initialVertex] = frequencyIndicesList.count;
			}
		}else{
			accessibilityData[initialVertex] = 0.0f;
			if(accessProbabilities){
				accessProbabilities[initialVertex].count = 0;
				accessProbabilities[initialVertex].probabilities = NULL;
				accessProbabilities[initialVertex].vertexIndices = NULL;
			}
		}
		free(transitionFrequencies);
		if(currentProgress){
			CVAtomicIncrementInteger(currentProgress);
			if(updateCallback){
				updateCallback(operationControl);
			}
			if(streamCallback){
				streamCallback(operationControl,initialVertex,"%"CVFloatScan,accessibilityData[initialVertex]);
			}
		}
	}
	CVIntegerArrayDestroy(&frequencyIndicesList);
	CVAgentPathDestroy(path);
	return CVTrue;
}

#if CV_ENABLE_PARALLELISM
CVBool CVNetworkCalculateAccessibility_parallel_implementation(const CVNetwork* network, CVFloatArray* accessibility, CVBool selfAvoided, CVIndex level, CVBool ballMeasurement,CVBool usePathDiversity, CVIntegerArray* accessedCount, CVAccessProbabilities* accessProbabilities, CVOperationControl* operationControl){
	CVSize verticesCount = network->verticesCount;
	CVSize unrolledLoops = kCVDefaultParallelBlocks;
	//__block BOOL stop = NO;
	CVInteger* currentProgress = NULL;

	CVFloatArrayReallocToCapacity(verticesCount, accessibility);
	CVFloatArraySetCount(verticesCount, accessibility);
	CVInteger* accessedCountData = NULL;
	if(accessedCount){
		CVIntegerArrayReallocToCapacity(verticesCount, accessedCount);
		CVIntegerArraySetCount(verticesCount, accessedCount);
		accessedCountData = accessedCount->data;
		CVIndex i;
		for (i=0; i<verticesCount; i++) {
			if(network->verticesEnabled[i]){
				accessedCountData[i]  = 0;
			}
		}
	}
	CVIndex i;
	for (i=0; i<verticesCount; i++) {
		if(network->verticesEnabled[i]){
			accessibility->data[i]  = 0.0;
		}
	}

	CVFloat* accessibilityData = accessibility->data;

	void (*updateCallback)(CVOperationControl*)  = NULL;
	void (*streamCallback)(CVOperationControl*, CVIndex index, const char* format,...)  = NULL;
	if(operationControl){
		operationControl->maxProgress = verticesCount;
		operationControl->currentProgress = 0;
		currentProgress = &(operationControl->currentProgress);
		if(operationControl->maxParallelBlocks>0){
			unrolledLoops = operationControl->maxParallelBlocks;
		}
		updateCallback = operationControl->updateCallback;
		streamCallback = operationControl->streamCallback;
	}
	if(updateCallback){
		updateCallback(operationControl);
	}
#if CV_USE_LIBDISPATCH
	dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
	dispatch_apply(unrolledLoops, queue, ^(size_t index) {
#elif CV_USE_OPENMP
		CVIndex index;
		if(unrolledLoops> omp_get_max_threads()){
			unrolledLoops = omp_get_max_threads();
		}
#pragma omp parallel for
		for(index=0;index<unrolledLoops;index++){
#endif
			CVSize unrolledOperations = 0;

			if(verticesCount%unrolledLoops==0){
				unrolledOperations= verticesCount/unrolledLoops;
			}else{
				unrolledOperations = verticesCount/unrolledLoops+1;
			}


			CVIntegerArray frequencyIndicesList;
			CVIntegerArrayInitWithCapacity(16, &frequencyIndicesList);

			CVAgentPath* path = CVAgentPathCreate(4*level);

			CVIndex initialVertex;
			CVSize maxIndex = CVMIN((index+1)*unrolledOperations, verticesCount);
			for(initialVertex=index*unrolledOperations;initialVertex<maxIndex;initialVertex++){
				frequencyIndicesList.count=0;
				double probabilitiesWeightSum = 0.0f;
				CVNetworkAgent initialAgent;
				initialAgent.level=0;
				initialAgent.vertex=initialVertex;
				initialAgent.branchIndex=0;
				initialAgent.weight=1.0;
				CVFloat* transitionFrequencies = calloc(verticesCount, sizeof(CVFloat));

				CVAgentPathPushAgent(initialAgent, path);

				while(!CVAgentPathIsEmpty(path)){
					CVNetworkAgent* currentAgent = CVAgentPathLast(path);
					CVNetworkAgent nextAgent;
					nextAgent.level  = 0;
					nextAgent.vertex = 0;
					nextAgent.weight = 0.0f;
					
					CVFloat probability = 1.0;
					if(selfAvoided){
						currentAgent->branchIndex = CVNetworkPathSelfAvoidingNextBranch(network, currentAgent, path, (usePathDiversity)?NULL:&probability, &nextAgent);
					}else{
						currentAgent->branchIndex = CVNetworkPathNextBranch(network, currentAgent, path, (usePathDiversity)?NULL:&probability, &nextAgent);
					}
					nextAgent.weight*=probability;
					
					if(!ballMeasurement){
						if(nextAgent.level<=currentAgent->level){
							CVAgentPathPopAgent(path);
						}else{
							if(nextAgent.level >= level){
								probabilitiesWeightSum+=nextAgent.weight;
								if(!transitionFrequencies[nextAgent.vertex]){
									CVIntegerArrayAdd(nextAgent.vertex, &frequencyIndicesList);
								}
								transitionFrequencies[nextAgent.vertex]+=nextAgent.weight;
							}else{
								CVAgentPathPushAgent(nextAgent, path);
							}
						}
					}else{
						if(nextAgent.level<=currentAgent->level){
							CVAgentPathPopAgent(path);
						}else{
							probabilitiesWeightSum+=nextAgent.weight;
							if(!transitionFrequencies[nextAgent.vertex]){
								CVIntegerArrayAdd(nextAgent.vertex, &frequencyIndicesList);
							}
							transitionFrequencies[nextAgent.vertex]+=nextAgent.weight;
							if(nextAgent.level < level){
								CVAgentPathPushAgent(nextAgent, path);
							}
						}
					}
					
					
				}
				//Reduction
				if(probabilitiesWeightSum>0.0){
					double diversityValue = 0.0;
					CVIndex i;
					
					if(accessProbabilities){
						CVInitAccessProbabilities(accessProbabilities+initialVertex, frequencyIndicesList.count);
					}
					for (i=0;i<frequencyIndicesList.count;i++) {
						double probability = transitionFrequencies[frequencyIndicesList.data[i]]/probabilitiesWeightSum;
						diversityValue -= probability*log(probability);
						if(accessProbabilities){
							accessProbabilities[initialVertex].probabilities[i] = probability;
							accessProbabilities[initialVertex].vertexIndices[i] = frequencyIndicesList.data[i];
						}
						//	printf("\t%g-->%"CVSizeScan"\n",probability,frequencyIndicesList.data[i]);
					}
					//printf("---\n");
					accessibilityData[initialVertex] = (float)exp(diversityValue);
					if(accessedCountData){
						accessedCountData[initialVertex] = frequencyIndicesList.count;
					}
				}else{
					accessibilityData[initialVertex] = 0.0f;
					if(accessProbabilities){
						accessProbabilities[initialVertex].count = 0;
						accessProbabilities[initialVertex].probabilities = NULL;
						accessProbabilities[initialVertex].vertexIndices = NULL;
					}
				}
				free(transitionFrequencies);
				if(currentProgress){
					CVAtomicIncrementInteger(currentProgress);
					if(updateCallback){
#if CV_USE_OPENMP
#pragma omp critical
						{
#endif
						updateCallback(operationControl);
#if CV_USE_OPENMP
						}
#endif
					}
					if(streamCallback){
#if CV_USE_OPENMP
#pragma omp critical
						{
#endif
						streamCallback(operationControl,initialVertex,"%"CVFloatScan,accessibilityData[initialVertex]);
#if CV_USE_OPENMP
						}
#endif
					}
				}
			}
			CVIntegerArrayDestroy(&frequencyIndicesList);
			CVAgentPathDestroy(path);
#if CV_USE_LIBDISPATCH
		});
#elif CV_USE_OPENMP
		}
#endif
	return CVTrue;
}
#endif //CV_ENABLE_PARALLELISM



CVBool CVNetworkCalculateAccessibility(const CVNetwork* network, CVFloatArray* accessibility, CVBool selfAvoided, CVIndex level,CVBool ballMeasurement,CVBool usePathDiversity, CVIntegerArray* accessedCount, CVAccessProbabilities* accessProbabilities, CVOperationControl* operationControl){
   CV_BenchmarkPrepare(CVNetworkCalculateAccessibility);
   CV_BenchmarkStart(CVNetworkCalculateAccessibility);

   CVBool returnValue;

#if CV_ENABLE_PARALLELISM
   CVInteger maxParallelBlocksCount = kCVDefaultParallelBlocks;
   CVSize problemSize = network->verticesCount * (selfAvoided?1:(CVSize)ipow(2, level-1));
   if(operationControl){
	   maxParallelBlocksCount = operationControl->maxParallelBlocks;
   }
   if(network&&problemSize>=128&&maxParallelBlocksCount>1){
	   returnValue = CVNetworkCalculateAccessibility_parallel_implementation(network, accessibility, selfAvoided, level,ballMeasurement,usePathDiversity,accessedCount, accessProbabilities, operationControl);
   }else{
	   returnValue = CVNetworkCalculateAccessibility_implementation(network, accessibility, selfAvoided, level,ballMeasurement,usePathDiversity, accessedCount, accessProbabilities, operationControl);
   }
#else
   returnValue = CVNetworkCalculateAccessibility_implementation(network, accessibility, selfAvoided, level,ballMeasurement,usePathDiversity, accessedCount, accessProbabilities, operationControl);
#endif //CV_ENABLE_PARALLELISM

   CV_BenchmarkStop(CVNetworkCalculateAccessibility);
   CV_BenchmarkPrint(CVNetworkCalculateAccessibility);
   return returnValue;
}

