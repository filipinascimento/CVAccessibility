//
//  CVNetworkAccessibility.h
//  CVNetwork
//
//  Created by Filipi Nascimento Silva on 8/27/13.
//  Copyright (c) 2013 Filipi Nascimento Silva. All rights reserved.
//

#ifndef CVNetwork_CVNetworkAccessibility_h
#define CVNetwork_CVNetworkAccessibility_h

#include "CVNetwork.h"
#include "CVBasicArrays.h"
#include "CVSimpleQueue.h"


typedef struct _CVAccessProbabilities{
	CVIndex* vertexIndices;
	CVFloat* probabilities;
	CVSize count;
} CVAccessProbabilities;

CV_INLINE void CVInitAccessProbabilities(CVAccessProbabilities* CVAccessProbabilities,CVSize count){
	CVAccessProbabilities->count = count;
	CVAccessProbabilities->probabilities = calloc(count, sizeof(CVFloat));
	CVAccessProbabilities->vertexIndices = calloc(count, sizeof(CVSize));
}
CV_INLINE void CVDestroyAccessProbabilities(CVAccessProbabilities* CVAccessProbabilities){
	free(CVAccessProbabilities->probabilities);
	free(CVAccessProbabilities->vertexIndices);
}
CVBool CVNetworkCalculateAccessibility(const CVNetwork* network, CVFloatArray* accessibility, CVBool selfAvoided, CVIndex level,CVBool ballMeasurement,CVBool usePathDiversity, CVIntegerArray* accessedCount,CVAccessProbabilities* accessProbabilities , CVOperationControl* operationControl);



#endif
