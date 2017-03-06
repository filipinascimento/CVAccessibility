//
//  CVNetwork.c
//
//
//  Created by Filipi Nascimento Silva on 9/22/12.
//
//
#include "CVNetwork.h"

CVBool CVNetworkAddNewEdges(CVNetwork* network, CVIndex* fromIndices, CVIndex* toIndices, CVFloat* weights, CVSize count){
	CVIndex initialEdge = network->edgesCount;
	CVSize verticesCount = network->verticesCount;
	CVIndex i;

	CVNetworkGrowEdgesCount(network,count);

	for(i=0; i<count; i++){
		CVIndex toIndex   = toIndices[i];
		CVIndex fromIndex = fromIndices[i];
		if (toIndex >= verticesCount||fromIndex >= verticesCount) {
			return CVFalse;
		}
		network->edgeFromList[i+initialEdge]=fromIndex;
		network->edgeToList[i+initialEdge]=toIndex;

		CVNetworkGrowVertexSetEdgeForVertex(network,i+initialEdge,fromIndex,toIndex);

		if(network->edgeWeighted&&weights==NULL){
			network->edgesWeights[i+initialEdge]=1.0f;
		}else if(network->edgeWeighted){
			network->edgesWeights[i+initialEdge]=weights[i];
		}
		if(!network->directed){
			CVNetworkGrowVertexSetEdgeForVertex(network,i+initialEdge,toIndex,fromIndex);
		}else{
			CVNetworkGrowVertexSetInEdgeForVertex(network,i+initialEdge,toIndex,fromIndex);
		}
	}
	return CVTrue;
}



void CVNetworkPrint(const CVNetwork* network){
	printf("Vertices:" "%" CVSizeScan "\n", network->verticesCount);
	printf("Edges: " "%" CVSizeScan "\n", network->edgesCount);
	CVIndex fromVertex;
	for (fromVertex=0; fromVertex<=network->verticesCount; fromVertex++) {
		CVIndex i;
		CVSize toVerticesCount = network->vertexNumOfEdges[fromVertex];
		CVIndex* toVertices = network->vertexEdgesLists[fromVertex];
		printf("%"CVIndexScan"\t:",fromVertex);
		for (i=0; i<toVerticesCount; i++) {
			CVIndex toVertex = toVertices[i];
			printf("\t" "%"CVIndexScan,toVertex);
		}
		printf("\n");
	}
}

CVNetwork* CV_NewAllocationNetwork(CVSize verticesCount){
	CVNetwork* newNetwork = malloc(sizeof(CVNetwork));
	newNetwork->vertexNumOfEdges = calloc(verticesCount, sizeof(CVSize));
	newNetwork->vertexCapacityOfEdges = calloc(verticesCount, sizeof(CVSize));
	newNetwork->vertexEdgesLists = calloc(verticesCount, sizeof(CVSize*));
	newNetwork->vertexEdgesIndices = calloc(verticesCount, sizeof(CVSize*));

	newNetwork->vertexNumOfInEdges = calloc(verticesCount, sizeof(CVSize));
	newNetwork->vertexCapacityOfInEdges = calloc(verticesCount, sizeof(CVSize));
	newNetwork->vertexInEdgesLists = calloc(verticesCount, sizeof(CVSize*));
	newNetwork->vertexInEdgesIndices = calloc(verticesCount, sizeof(CVSize*));
	newNetwork->verticesWeights = calloc(verticesCount, sizeof(CVFloat));
	newNetwork->verticesEnabled = calloc(verticesCount, sizeof(CVBool));
	CVIndex i;
	for(i=0;i<verticesCount;i++){
		newNetwork->verticesWeights[i] = 1.0f;
		newNetwork->verticesEnabled[i] = CVTrue;
	}

	newNetwork->edgeFromList = NULL;
	newNetwork->edgeToList = NULL;

	newNetwork->edgesWeights = NULL;

	newNetwork->vertexCapacity = verticesCount;
	newNetwork->edgesCapacity = 0;

	newNetwork->edgesCount = 0;
	newNetwork->verticesCount = verticesCount;

	newNetwork->editable = CVFalse;
	newNetwork->directed = CVFalse;
	newNetwork->edgeWeighted = CVFalse;
	newNetwork->vertexWeighted = CVFalse;

	newNetwork->vertexNames = NULL;
	newNetwork->propertiesData = NULL;
	newNetwork->propertiesTypes = NULL;
	newNetwork->propertiesCount = 0;
	newNetwork->propertiesNames = NULL;
	
	return newNetwork;
}



void CV_NetworkDestroyProperties(CVNetwork* theNetwork){
	CVIndex i;
	for (i=0; i<theNetwork->propertiesCount; i++) {
		CVPropertyType type = theNetwork->propertiesTypes[i];
		
		if(type==CVStringPropertyType){
			CVIndex j;
			CVString* values = theNetwork->propertiesData[i];
			for (j=0; j<theNetwork->verticesCount; j++) {
				free(values[j]);
			}
		}
		free(theNetwork->propertiesData[i]);
		free(theNetwork->propertiesNames[i]);
	}
	free(theNetwork->propertiesData);
	free(theNetwork->propertiesNames);
	free(theNetwork->propertiesTypes);
	theNetwork->propertiesCount = 0;
	theNetwork->propertiesData= NULL;
	theNetwork->propertiesNames= NULL;
	theNetwork->propertiesTypes= NULL;
}


void CVNetworkAppendProperty(CVNetwork* theNetwork, CVString name, CVPropertyType type, void* data){
	theNetwork->propertiesCount++;
	CVIndex newIndex = theNetwork->propertiesCount-1;
	theNetwork->propertiesData  = realloc(theNetwork->propertiesData, sizeof(void*)*theNetwork->propertiesCount);
	theNetwork->propertiesNames = realloc(theNetwork->propertiesNames, sizeof(CVString)*theNetwork->propertiesCount);
	theNetwork->propertiesTypes = realloc(theNetwork->propertiesTypes, sizeof(CVPropertyType)*theNetwork->propertiesCount);
	
	theNetwork->propertiesTypes[newIndex] =type;
	theNetwork->propertiesNames[newIndex] = calloc(strlen(name)+1, sizeof(CVChar));
	strncpy(theNetwork->propertiesNames[newIndex], name, strlen(name));
	
	switch (type) {
		case CVNumberPropertyType:{
			CVFloat* values = calloc(theNetwork->verticesCount, sizeof(CVFloat));
			memcpy(values, data, sizeof(CVFloat)*theNetwork->verticesCount);
			theNetwork->propertiesData[newIndex] = values;
			break;
		}
		case CVStringPropertyType:{
			CVString* values = calloc(theNetwork->verticesCount, sizeof(CVString));
			CVString* stringValues = data;
			CVIndex i;
			for (i=0; i<theNetwork->verticesCount; i++) {
				values[i] = calloc(strlen(stringValues[i])+1, sizeof(CVChar));
				strncpy(values[i], stringValues[i], strlen(stringValues[i]));
			}
			theNetwork->propertiesData[newIndex] = values;
			break;
		}
		case CVVector2DPropertyType:{
			CVFloat* values = calloc(theNetwork->verticesCount*2, sizeof(CVFloat));
			memcpy(values, data, sizeof(CVFloat)*theNetwork->verticesCount*2);
			theNetwork->propertiesData[newIndex] = values;
			break;
		}
		case CVVector3DPropertyType:{
			CVFloat* values = calloc(theNetwork->verticesCount*3, sizeof(CVFloat));
			memcpy(values, data, sizeof(CVFloat)*theNetwork->verticesCount*3);
			theNetwork->propertiesData[newIndex] = values;
			break;
		}
		default:
			break;
	}
}


/*
 if((token = strsep(&lineSegment, " ")) != NULL){
 }
 */

CVNetwork* CVNewNetwork(CVSize verticesCount, CVBool edgeWeighted, CVBool directed){
	CVNetwork * theNetwork = NULL;
	theNetwork = CV_NewAllocationNetwork(verticesCount);
	theNetwork->vertexWeighted = CVFalse;
	theNetwork->edgeWeighted = edgeWeighted;
	theNetwork->directed = directed;
	return theNetwork;
}

CVNetwork* CVNewNetworkWithNetwork(CVNetwork* originalNetwork, CVBool edgeWeighted, CVBool directed){
	CVNetwork * theNetwork = NULL;
	theNetwork = CV_NewAllocationNetwork(originalNetwork->verticesCount);
	theNetwork->vertexWeighted = CVFalse;
	theNetwork->edgeWeighted = edgeWeighted;
	theNetwork->directed = directed;
	CVIndex i;
	for(i=0;i<originalNetwork->edgesCount;i++){
		CVIndex from,to;
		from = originalNetwork->edgeFromList[i];
		to = originalNetwork->edgeToList[i];
		CVFloat weight = 1.0f;
		if(originalNetwork->edgeWeighted){
			weight = originalNetwork->edgesWeights[i];
		}
		CVNetworkAddNewEdge(theNetwork, from, to, weight);
	}
	CVIndex propertyIndex;
	for(propertyIndex=0;propertyIndex<originalNetwork->propertiesCount;propertyIndex++){
		CVPropertyType type = originalNetwork->propertiesTypes[propertyIndex];
		void* data = originalNetwork->propertiesData[propertyIndex];
		CVString name = originalNetwork->propertiesNames[propertyIndex];
		CVNetworkAppendProperty(theNetwork, name, type, data);
	}
	
	return theNetwork;
}

void CVNetworkWriteToFile(CVNetwork* theNetwork, FILE* networkFile){
	fprintf(networkFile,"#vertices ""%"CVSizeScan" nonweighted\n",theNetwork->verticesCount);
	if(theNetwork->vertexNames){
		CVIndex i;
		for(i=0;i<theNetwork->verticesCount;i++){
			fprintf(networkFile,"\"%s\"\n",theNetwork->vertexNames[i]);
		}
	}
	fprintf(networkFile,"#edges %s %s\n",theNetwork->edgeWeighted?"weighted":"nonweighted",theNetwork->directed?"directed":"undirected");
	CVIndex edgeIndex;
	
	CVIndex* edgesFrom = theNetwork->edgeFromList;
	CVIndex* edgesTo = theNetwork->edgeToList;
	CVFloat* edgesWeights = theNetwork->edgesWeights;
	for (edgeIndex=0; edgeIndex<theNetwork->edgesCount; edgeIndex++) {
		CVIndex fromVertex = edgesFrom[edgeIndex];
		CVIndex toVertex = edgesTo[edgeIndex];
		if(theNetwork->edgeWeighted){
			CVFloat weight = edgesWeights[edgeIndex];
			fprintf(networkFile,"%"CVIndexScan" ""%"CVIndexScan" ""%"CVFloatScan"\n",fromVertex,toVertex,weight);
		}else{
			fprintf(networkFile,"%"CVIndexScan" ""%"CVIndexScan"\n",fromVertex,toVertex);
		}
	}
	CVIndex propertyIndex;
	for(propertyIndex=0;propertyIndex<theNetwork->propertiesCount;propertyIndex++){
		CVPropertyType type = theNetwork->propertiesTypes[propertyIndex];
		void* data = theNetwork->propertiesData[propertyIndex];
		CVString name = theNetwork->propertiesNames[propertyIndex];
		switch (type) {
			case CVNumberPropertyType:{
				CVFloat* floatData = data;
				fprintf(networkFile,"#v \"%s\" n\n",name);
				CVIndex i;
				for(i=0;i<theNetwork->verticesCount;i++){
					fprintf(networkFile,"%"CVFloatScan"\n",floatData[i]);
				}
				break;
			}
			case CVStringPropertyType:{
				CVString* stringData = data;
				fprintf(networkFile,"#v \"%s\" s\n",name);
				CVIndex i;
				for(i=0;i<theNetwork->verticesCount;i++){
					fprintf(networkFile,"\"%s\"\n",stringData[i]);
				}
				break;
			}
			case CVVector2DPropertyType:{
				CVFloat* floatData = data;
				fprintf(networkFile,"#v \"%s\" v2\n",name);
				CVIndex i;
				for(i=0;i<theNetwork->verticesCount;i++){
					fprintf(networkFile,"%"CVFloatScan" %"CVFloatScan"\n",floatData[i*2],floatData[i*2+1]);
				}
				break;
			}
			case CVVector3DPropertyType:{
				CVFloat* floatData = data;
				fprintf(networkFile,"#v \"%s\" v3\n",name);
				CVIndex i;
				for(i=0;i<theNetwork->verticesCount;i++){
					fprintf(networkFile,"%"CVFloatScan" %"CVFloatScan" %"CVFloatScan"\n",floatData[i*3],floatData[i*3+1],floatData[i*3+2]);
				}
				break;
			}
			default:{
			}
		}
	}
}



CVNetwork* CVNewNetworkFromXNETFile(FILE* networkFile){
	CVSize verticesCount = 0;

	CVBool isReadingVertices = CVFalse;
	CVBool isReadingEdges = CVFalse;
	CVBool isReadingProperty = CVFalse;

	CVNetwork * theNetwork = NULL;
	CVSize* fromIndices = NULL;
	CVSize* toIndices = NULL;
	CVFloat* edgesWeights = NULL;
	CVSize edgesCount = 0;
	CVSize edgesCapacity = 0;
	CVIndex currentVertex = 0;
	CVString propertyName = NULL;
	CVPropertyType propertyType;
	CVIndex propertyVertexIndex = 0;
	void* propertyData;
	CVString parsingError = NULL;

	CVIndex currentLine = 0;
#define __CVMAX_LINE_BUFFER_SIZE 255
	CVString lineBuffer = malloc((__CVMAX_LINE_BUFFER_SIZE+1)*sizeof(CVChar));
	while(fgets(lineBuffer, __CVMAX_LINE_BUFFER_SIZE, networkFile)&&!feof(networkFile)){
		CVString lineSegment = lineBuffer;
		currentLine++;
		if(lineSegment&&CVStringScanCharacters(&lineSegment, '#')){
			if(CVStringScan(&lineSegment, "vertices")){
				CVStringScanCharacters(&lineSegment, ' ');
				if(CVStringScanIndex(&lineSegment, &verticesCount)){
					CVStringScanCharacters(&lineSegment, ' ');
					//printf("VerticesCount: %ld\n", verticesCount);
					theNetwork = CV_NewAllocationNetwork(verticesCount);
				}
				if(CVStringScan(&lineSegment, "weighted")){
					theNetwork->vertexWeighted = CVTrue;
				}
				isReadingVertices=CVTrue;
				isReadingEdges=CVFalse;
				isReadingProperty = CVFalse;
				currentVertex = 0;
			}else if(CVStringScan(&lineSegment, "edges")){
				CVStringScanCharacters(&lineSegment, ' ');
				if(CVStringScan(&lineSegment, "weighted")){
					theNetwork->edgeWeighted = CVTrue;
				}
				if(CVStringScan(&lineSegment, "nonweighted")){
					theNetwork->edgeWeighted = CVFalse;
				}
				CVStringScanCharacters(&lineSegment, ' ');
				if(CVStringScan(&lineSegment, "directed")){
					theNetwork->directed = CVTrue;
				}
				if(CVStringScan(&lineSegment, "nonweighted")){
					theNetwork->edgeWeighted = CVFalse;
				}
				CVStringScanCharacters(&lineSegment, ' ');
				if(CVStringScan(&lineSegment, "weighted")){
					theNetwork->edgeWeighted = CVTrue;
				}
				isReadingVertices=CVFalse;
				isReadingEdges=CVTrue;
				isReadingProperty = CVFalse;
			}else if(CVStringScan(&lineSegment,"v")){
				CVStringScanCharacters(&lineSegment, ' ');
				CVStringScanCharacters(&lineSegment, '\"');
				free(propertyName);
				propertyName = CVNewStringScanningUpToCharacter(&lineSegment, '\"');
				propertyVertexIndex=0;
				CVStringScanCharacters(&lineSegment, '\"');
				CVStringScanCharacters(&lineSegment, ' ');
				if(CVStringScan(&lineSegment, "n")){
					propertyType=CVNumberPropertyType;
					isReadingProperty = CVTrue;
					propertyData = calloc(verticesCount,sizeof(CVFloat));
				}else if(CVStringScan(&lineSegment, "v2")){
					propertyType=CVVector2DPropertyType;
					isReadingProperty = CVTrue;
					propertyData = calloc(verticesCount*2,sizeof(CVFloat));
				}else if(CVStringScan(&lineSegment, "v3")){
					propertyType=CVVector3DPropertyType;
					isReadingProperty = CVTrue;
					propertyData = calloc(verticesCount*3,sizeof(CVFloat));
				}else if(CVStringScan(&lineSegment, "s")){
					propertyType=CVStringPropertyType;
					isReadingProperty = CVTrue;
					propertyData = calloc(verticesCount,sizeof(CVString));
				}else{
					isReadingProperty = CVFalse;
					printf("Unnalocationg %s\n",propertyName);
					free(propertyName);
					parsingError = "Error reading header for property.";
					break;
				}
			}else{
				isReadingVertices=CVFalse;
				isReadingEdges=CVFalse;
				isReadingProperty = CVFalse;
			}
		}else{
			if(isReadingVertices){
				if(currentVertex<verticesCount){
					if(!theNetwork->vertexNames){
						theNetwork->vertexNames = calloc(verticesCount, sizeof(CVString));
					}
					CVStringTrim(lineSegment, "\"\n \t");
					CVSize lineLength = strlen(lineSegment);
					theNetwork->vertexNames[currentVertex] = calloc(lineLength+1, sizeof(CVChar));
					strncpy(theNetwork->vertexNames[currentVertex], lineSegment, lineLength);
					currentVertex++;
				}else{
					isReadingVertices=CVFalse;
				}
			}else if(isReadingEdges){
				unsigned long _longFromIndex = 0;
				unsigned long _longToIndex = 0;
				CVFloat _doubleWeight = 1.0;
				if(sscanf(lineSegment, "%ld %ld " "%"CVFloatScan,&_longFromIndex,&_longToIndex,&_doubleWeight)>=2){
					edgesCount++;
					if(CVUnlikely(edgesCapacity < edgesCount)){
						edgesCapacity = 2 * edgesCount + 1;
						fromIndices = realloc(fromIndices, sizeof(CVIndex)*edgesCapacity);
						toIndices = realloc(toIndices, sizeof(CVIndex)*edgesCapacity);
						if(theNetwork->edgeWeighted){
							edgesWeights = realloc(edgesWeights, sizeof(CVFloat)*edgesCapacity);
						}
					}
					fromIndices[edgesCount-1]=_longFromIndex;
					toIndices[edgesCount-1]=_longToIndex;
					if(theNetwork->edgeWeighted){
						edgesWeights[edgesCount-1] = _doubleWeight;
					}
				}
			}else if(isReadingProperty){
				CVStringTrim(lineSegment, "\"\n \t");
				switch (propertyType) {
					case CVNumberPropertyType:{
						CVFloat currentValue = 0.0f;
						if(sscanf(lineSegment, "%"CVFloatScan,&currentValue)>0&&propertyVertexIndex<verticesCount){
							CVFloat* currentData = propertyData;
							currentData[propertyVertexIndex] = currentValue;
							propertyVertexIndex++;
							if(propertyVertexIndex==verticesCount){
								CVNetworkAppendProperty(theNetwork, propertyName, propertyType, currentData);
							}
						}
						break;
					}
					case CVStringPropertyType:{
						CVStringScanCharacters(&lineSegment, '\"');
						CVString currentString = CVNewStringScanningUpToCharacter(&lineSegment, '\"');
						
						CVString* currentData = propertyData;
						currentData[propertyVertexIndex] = currentString;
						propertyVertexIndex++;
						if(propertyVertexIndex==verticesCount){
							CVNetworkAppendProperty(theNetwork, propertyName, propertyType, currentData);
						}
						break;
					}
					case CVVector2DPropertyType:{
						CVFloat currentValue1 = 0.0f;
						CVFloat currentValue2 = 0.0f;
						if(sscanf(lineSegment, "%"CVFloatScan" %"CVFloatScan,&currentValue1,&currentValue2)>0&&propertyVertexIndex<verticesCount){
							CVFloat* currentData = propertyData;
							currentData[propertyVertexIndex*2] = currentValue1;
							currentData[propertyVertexIndex*2+1] = currentValue2;
							propertyVertexIndex++;
							if(propertyVertexIndex==verticesCount){
								CVNetworkAppendProperty(theNetwork, propertyName, propertyType, currentData);
							}
						}
						break;
					}
					case CVVector3DPropertyType:{
						CVFloat currentValue1 = 0.0f;
						CVFloat currentValue2 = 0.0f;
						CVFloat currentValue3 = 0.0f;
						if(sscanf(lineSegment, "%"CVFloatScan" %"CVFloatScan" %"CVFloatScan,&currentValue1,&currentValue2,&currentValue3)>0&&propertyVertexIndex<verticesCount){
							CVFloat* currentData = propertyData;
							currentData[propertyVertexIndex*3] = currentValue1;
							currentData[propertyVertexIndex*3+1] = currentValue2;
							currentData[propertyVertexIndex*3+2] = currentValue3;
							propertyVertexIndex++;
							if(propertyVertexIndex==verticesCount){
								CVNetworkAppendProperty(theNetwork, propertyName, propertyType, currentData);
							}
						}
						break;
					}
					default:
						break;
				}
			}

		}
	}
	
	if(parsingError==NULL){
		if(theNetwork && theNetwork->verticesCount>0){
			CVNetworkAddNewEdges(theNetwork, fromIndices,toIndices,edgesWeights, edgesCount);
		}
	}else{
		fprintf(stderr, "Parsing error occurred[at line %"CVIndexScan"]: %s\n", currentLine, parsingError);
		CVNetworkDestroy(theNetwork);
		theNetwork = NULL;
	}
	free(fromIndices);
	free(toIndices);
	free(edgesWeights);

	free(lineBuffer);
#undef __CVMAX_LINE_BUFFER_SIZE
	return theNetwork;
}

void CVNetworkDestroy(CVNetwork* network){
	CVIndex i;
	CV_NetworkDestroyProperties(network);
	for(i=0;i<network->vertexCapacity;i++){
		free(network->vertexEdgesLists[i]);
		free(network->vertexEdgesIndices[i]);

		if(network->directed){
			free(network->vertexInEdgesLists[i]);
			free(network->vertexInEdgesIndices[i]);
		}
	}

	free(network->vertexCapacityOfEdges);
	free(network->vertexNumOfEdges);
	free(network->vertexEdgesLists);
	free(network->vertexEdgesIndices);

	if(network->directed){
		free(network->vertexNumOfInEdges);
		free(network->vertexInEdgesLists);
		free(network->vertexInEdgesIndices);
		free(network->vertexCapacityOfInEdges);
	}

	free(network->edgeFromList);
	free(network->edgeToList);

	if(network->edgeWeighted)
		free(network->edgesWeights);

	free(network->verticesWeights);
	free(network->verticesEnabled);
}


CVNetwork* CVNewRegular2DNetwork(CVSize rows, CVSize columns, CVBool toroidal){
	CVSize verticesCount = rows*columns;
	CVSize maximumEdgesCount = verticesCount*2;
	CVSize edgesCount = 0;
	CVIndex* fromList = calloc(maximumEdgesCount, sizeof(CVIndex));
	CVIndex* toList = calloc(maximumEdgesCount, sizeof(CVIndex));
	CVFloat* positions = calloc(verticesCount*3, sizeof(CVFloat));
	CVIndex i,j;
	for (i=0; i<rows; i++) {
		for (j=0; j<columns; j++) {
			positions[(i*columns+j)*3+0]=(i-rows*0.5f)*200.0f/CVMAX(rows, columns);
			positions[(i*columns+j)*3+1]=(j-columns*0.5f)*200.0f/CVMAX(rows, columns);
			positions[(i*columns+j)*3+2]=0.0f;
			if(toroidal){
				fromList[edgesCount] = i*columns+j;
				toList[edgesCount] =(i)*columns+((j+1)%columns);
				edgesCount++;
				
				fromList[edgesCount] = i*columns+j;
				toList[edgesCount] =((i+1)%rows)*columns+(j);
				edgesCount++;
			}else{
				if(j+1<columns){
					fromList[edgesCount] = i*columns+j;
					toList[edgesCount] =(i)*columns+(j+1);
					edgesCount++;
				}
				if(i+1<rows){
					fromList[edgesCount] = i*columns+j;
					toList[edgesCount] =(i+1)*columns+(j);
					edgesCount++;
				}
			}
		}
	}
	CVNetwork* theNetwork = CVNewNetwork(verticesCount, CVFalse, CVFalse);
	CVNetworkAddNewEdges(theNetwork, fromList, toList, NULL, edgesCount);
	CVNetworkAppendProperty(theNetwork, "Position", CVVector3DPropertyType, positions);
	return theNetwork;
}


