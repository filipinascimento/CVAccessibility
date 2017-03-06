//
//  CVAccessibility.c
//  CVNetwork
//
//  Created by Filipi Nascimento Silva on 5/5/13.
//  Copyright (c) 2013 Filipi Nascimento Silva. All rights reserved.
//

#include "CVNetwork.h"
#include "CVNetworkAccessibility.h"
#include <getopt.h>
#define CVAccessibilityApplicationVersion "0.8b"


static void CVAccessibilityApplicationPrintProgress(CVOperationControl* thisControl) {
	fprintf(stderr,"\r# Progress: %4"CVIntegerScan"/%"CVIntegerScan,thisControl->currentProgress,thisControl->maxProgress);
}


static void CVAccessibilityApplicationPrintLiveStream(CVOperationControl* thisControl, CVIndex index, const char* format, ...) {
	FILE* outputFile = thisControl->defaultStreamFile;
	if(!outputFile){
		outputFile = stdout;
	}
	fprintf(outputFile,"%"CVIntegerScan"\t",index);
	va_list args;
    va_start( args, format);
    vfprintf(outputFile, format, args);
    va_end( args );
	fprintf(outputFile,"\n");
}

void CVAccessibilityApplicationPrintVersion (FILE* stream){
	fprintf(stream,"CVAccessibility v"CVAccessibilityApplicationVersion"\n");
}

void CVAccessibilityApplicationPrintUsage (FILE* stream, const char* programName){
	fprintf (stream, "Usage: %s [options] -i|<inputnetwork> [<outputfile>]\n", programName);
	fprintf (stream,
			 "Options:\n"
			 "  -l  --level h           Obtain accessibility for h steps.\n"
			 "                          (defaults to h=3)\n"
			 "  -r  --random-walk       Use random-walk dynamics. If not specified,\n"
			 "                          use self-avoiding dynamics.\n"
			 "  -b  --ball-measurement  Take the measurement over the ball instead\n"
			 "                          considering levels from 0 to h.\n"
			 "  -p  --path-diversity    Use path diversity instead of probabilities.\n"
			 "  -n  --normalize         Normalize accessibility with the number of\n"
			 "                          accessed node.\n"
			 "  -t  --output-probs      Also output the access probabilities for each\n"
			 "                          pair of nodes.\n"
			 "  -a  --output-accessed   Also output the number of accessed nodes\n"
			 "                          alongside acessibility.\n"
			 "  -s  --live-stream       Stream the output as results are obtained.\n"
			 "                          (note that the results may be out of order)\n"
			 "  -j  --parallel-jobs N   Maximum number of parallel jobs for multicore\n"
			 "                          calculation. (defaults to N=8)\n"
			 "  -h  --help              Display this usage information.\n"
			 "  -V  --version           Show version number and quit.\n"
			 "  -v  --verbose           Make the calculation more talkative.\n"
			 "  -q  --quiet             Don not show calculation progress.\n"
			 "  -S  --show-status       Always show calculation progress.\n"
			 "Input:\n"
			 "  <inputnetwork>          Path to the network file in .xnet format.\n"
			 "  -i  --input-stdin       Uses stdin as input instead of a file.\n"
			 "  <outputfile>            Path to output the results. (If not defined, \n"
			 "                          the software will output to stdout)\n"
			 "\n");
	CVAccessibilityApplicationPrintVersion(stream);
	fprintf (stream,
			 "Calculates accessibility of nodes for a network file.\n"
			 "\n"
			 "For more info visit:\n"
			 "<http://cyvision.ifsc.usp.br/Cyvision/Accessibility/>\n");
}

#define CVAccessibilityApplicationReportParameterError(programName, ...) do {fprintf(stderr,__VA_ARGS__);CVAccessibilityApplicationPrintUsage (stderr, programName);} while (0)

int CVAccessibilityApplication(int argc, char** argv){
	const char* programName = argv[0];
	
	static struct option options[] = {
		{"level",           required_argument, NULL, 'l'},
		{"random-walk",     no_argument,       NULL, 'r'},
		{"normalize",       no_argument,       NULL, 'n'},
		{"output-accessed", no_argument,       NULL, 'a'},
		{"output-probs",    no_argument,       NULL, 't'},
		{"live-stream",     no_argument,       NULL, 's'},
		{"ball-measurement",no_argument,       NULL, 'b'},
		{"path-diversity",  no_argument,       NULL, 'p'},
		{"parallel-jobs",   required_argument, NULL, 'j'},
		{"help",		    no_argument,       NULL, 'h'},
		{"version",         no_argument,       NULL, 'V'},
		{"verbose",         no_argument,       NULL, 'v'},
		{"quiet",           no_argument,       NULL, 'q'},
		{"show-status",     no_argument,       NULL, 'S'},
		{"input-stdin",     no_argument,       NULL, 'i'},
		{0, 0, 0, 0}
	};
	
	int optionCode;
	int optionIndex = 0;
	
	CVBool verbose = CVFalse;
	CVBool quiet = CVFalse;
	CVBool showHelp = CVFalse;
	CVBool showVersion = CVFalse;
	CVBool randomWalkDynamics = CVFalse;
	CVBool liveStream = CVFalse;
	CVBool ballMeasurement = CVFalse;
	CVBool usePathDiversity = CVFalse;
	CVBool normalize = CVFalse;
	CVBool outputAccessed = CVFalse;
	CVBool outputProbabilities = CVFalse;
	CVBool forceShowStatus = CVFalse;
	CVBool inputSTDIN = CVFalse;
	
	CVInteger level = 3;
	CVInteger maxParallelJobs = 0;
	
	CVSize filesCount = 0;
	FILE * verboseStream = NULL;
	
	const char* networkFilename = "<STDIN>";
	const char* outputFilename = "<STDOUT>";
	FILE* networkFile = stdin;
	FILE* outputFile = stdout;
	
	while ((optionCode=getopt_long (argc, argv, "l:rsbaptnj:hVvqSi",options, &optionIndex))>=0){
		switch (optionCode){
			case 'l':
				if(!sscanf(optarg, "%"CVIntegerScan,&level)||level<=0){
					CVAccessibilityApplicationReportParameterError(programName,"Error: Parameter \"%s\" for option --level (-l) is invalid. Use a integer > 0.\n",optarg);
					return EXIT_FAILURE;
				}
				break;
			case 'r':
				randomWalkDynamics = CVTrue;
				break;
			case 's':
				liveStream = CVTrue;
				break;
			case 'b':
				ballMeasurement = CVTrue;
				break;
			case 'n':
				normalize = CVTrue;
				break;
			case 'a':
				outputAccessed = CVTrue;
				break;
			case 'p':
				usePathDiversity = CVTrue;
				break;
			case 't':
				outputProbabilities = CVTrue;
				break;
			case 'j':
				if(!sscanf(optarg, "%"CVIntegerScan,&maxParallelJobs)||maxParallelJobs<=0){
					CVAccessibilityApplicationReportParameterError(programName,"Error: Parameter \"%s\" for option --parallel-jobs (-j) is invalid. Use a integer > 0.\n",optarg);
					return EXIT_FAILURE;
				}
				break;
			case 'h':
				showHelp=CVTrue;
				break;
			case 'V':
				showVersion = CVTrue;
				break;
			case 'v':
				verbose = CVTrue;
				break;
			case 'q':
				quiet = CVTrue;
				break;
			case 'S':
				forceShowStatus = CVTrue;
				break;
			case 'i':
				inputSTDIN = CVTrue;
				break;
			case '?':
				CVAccessibilityApplicationPrintVersion (stdout);
				return EXIT_FAILURE;
				break;
			default:
				return EXIT_FAILURE;
		}
	}
	
	if(showHelp){
		CVAccessibilityApplicationPrintUsage (stdout, programName);
		return EXIT_SUCCESS;
	}
	
	if(showVersion){
		CVAccessibilityApplicationPrintVersion (stdout);
		return EXIT_SUCCESS;
	}
	
	verboseStream = stderr;
	filesCount = argc-optind;
	
	if(inputSTDIN){
		if(filesCount>1){
			CVAccessibilityApplicationReportParameterError(programName,"Error: Found extra %"CVSizeScan" file parameters, please provide only the output file if --input-stdin is enabled.\n", filesCount- 1);
			return EXIT_FAILURE;
		}else if(filesCount>0){
			outputFilename = argv[optind];
			outputFile = fopen(outputFilename, "w");
		}
	}else{
		if(filesCount > 2){
			CVAccessibilityApplicationReportParameterError(programName,"Error: Found extra %"CVSizeScan" file parameters, please provide only one or two filenames.\n", filesCount - 2);
			return EXIT_FAILURE;
		}else if(filesCount<1){
			CVAccessibilityApplicationReportParameterError(programName,"Error: Missing input network file or -i(--input-stdin) parameter.\n");
			return EXIT_FAILURE;
		}else{
			networkFilename = argv[optind];
			networkFile = fopen(networkFilename, "r");
			if(filesCount>1){
				outputFilename = argv[optind+1];
				outputFile = fopen(outputFilename, "w");
			}
		}
	}
	
	if(quiet || outputFile==stdout){
		verboseStream = stderr;
	}else if(verbose){
		verboseStream = stdout;
	}
	
	if(liveStream && outputFile==stdout){
		quiet = CVTrue;
	}
	
	if(forceShowStatus){
		quiet = CVFalse;
	}
	
	if(!networkFile){
		fprintf(stderr, "Error: %s: No such file or directory, or user does not have access permissions to read it.\n",networkFilename);
		return EXIT_FAILURE;
	}
	
	if(!outputFile){
		fprintf(stderr, "Error: %s: No such file or directory, or user does not have write permissions.\n",outputFilename);
		return EXIT_FAILURE;
	}
	
	if(verbose){
		fprintf(verboseStream,"# Input Network File: %s\n",networkFilename);
		fprintf(verboseStream,"# Output File: %s\n",outputFilename);
		fprintf(verboseStream,"# Parsing network file.\n");
	}
	CVNetwork* theNetwork = CVNewNetworkFromXNETFile(networkFile);
	
	if(!theNetwork||theNetwork->verticesCount==0){
		fprintf(stderr, "Error: Network file \"%s\" is corrupted or malformed. \n",networkFilename);
		return EXIT_FAILURE;
	}
	
	CVFloatArray accessibility;
	CVFloatArrayInitWithCapacity(1, &accessibility);

	CVIntegerArray accessCount;
	CVIntegerArrayInitWithCapacity(1, &accessCount);
	
	CVOperationControl* operationControl = CVOperationControlCreate();
	
	if(maxParallelJobs>0){
		operationControl->maxParallelBlocks = maxParallelJobs;
	}
	
	if(!quiet){
		operationControl->updateCallback = CVAccessibilityApplicationPrintProgress;
	}
	
	if(liveStream){
		operationControl->streamCallback = CVAccessibilityApplicationPrintLiveStream;
		operationControl->defaultStreamFile = outputFile;
	}
	
	if(verbose){
		fprintf(verboseStream,"# Begining accessibility calculation with parameters:\n");
		fprintf(verboseStream,"#   Network: \n");
		fprintf(verboseStream,"#      Vertices: %"CVIntegerScan"\n",theNetwork->verticesCount);
		fprintf(verboseStream,"#      Edges: %"CVIntegerScan"\n",theNetwork->edgesCount);
		fprintf(verboseStream,"#      Type: ");
		fprintf(verboseStream,"%s",theNetwork->directed?"<Directed> ":"<Undirected> ");
		fprintf(verboseStream,"%s",theNetwork->vertexWeighted?"<Vertex Weighted> ":"");
		fprintf(verboseStream,"%s",theNetwork->edgeWeighted?"<Edge Weighted> ":"");
		fprintf(verboseStream,"\n");
		fprintf(verboseStream,"#   Level: %"CVIntegerScan"\n", level);
		fprintf(verboseStream,"#   Ball Measurement: %s\n", ballMeasurement?"yes":"no");
		fprintf(verboseStream,"#   Dynamics: %s\n", randomWalkDynamics?"Random-Walk":"Self-Avoiding");
		fprintf(verboseStream,"#   Live Stream: %s\n", liveStream?"yes":"no");
		fprintf(verboseStream,"#\n# Starting Calculations now...\n");
	}
	
	CVAccessProbabilities* accessProbabilities = NULL;
	
	if(outputProbabilities){
		accessProbabilities = calloc(theNetwork->verticesCount, sizeof(CVAccessProbabilities));
	}
	
	CVNetworkCalculateAccessibility(theNetwork, &accessibility, randomWalkDynamics?CVFalse:CVTrue, level, ballMeasurement,usePathDiversity,(normalize||outputAccessed)?&accessCount:NULL,accessProbabilities,operationControl);
	
	
	
	if(!quiet){
		fprintf(stderr,"\n");
	}
	if(verbose){
		fprintf(verboseStream,"# Finished accessibility calculation%s\n",liveStream?".":", now exporting to output...");
	}
	CVIndex i;
	if(!liveStream){
		for(i=0;i<accessibility.count;i++){
			CVFloat accessibilityValue =accessibility.data[i];
			if (normalize) {
				CVInteger accessed = accessCount.data[i];
				if(accessed>0){
					accessibilityValue = accessibilityValue/accessCount.data[i];
				}else{
					accessibilityValue = 0.0;
				}
			}
			
			fprintf(outputFile,"%"CVFloatScan,accessibilityValue);
			if (outputAccessed) {
				fprintf(outputFile,"\t%"CVIntegerScan,accessCount.data[i]);
			}
			if(outputProbabilities){
				fprintf(outputFile,"\t[");
				CVIndex accessIndex;
				CVBool first = CVTrue;
				for (accessIndex=0; accessIndex<accessProbabilities[i].count; accessIndex++) {
					if(first){
						first = CVFalse;
					}else{
						printf(", ");
					}
					fprintf(outputFile,"%"CVUIntegerScan,accessProbabilities[i].vertexIndices[accessIndex]);
				}
				
				fprintf(outputFile,"]\t(");
				first = CVTrue;
				for (accessIndex=0; accessIndex<accessProbabilities[i].count; accessIndex++) {
					if(first){
						first = CVFalse;
					}else{
						printf(", ");
					}
					fprintf(outputFile,"%"CVFloatScan,accessProbabilities[i].probabilities[accessIndex]);
				}
				
				fprintf(outputFile,")");
			}
			fprintf(outputFile,"\n");
		}
	}
	if(verbose){
		fprintf(verboseStream,"# Cleaning Up...\n");
	}
	CVNetworkDestroy(theNetwork);
	CVFloatArrayDestroy(&accessibility);
	if(outputProbabilities){
		for (i=0; i<theNetwork->verticesCount; i++) {
			CVDestroyAccessProbabilities(accessProbabilities+i);
		}
		free(accessProbabilities);
	}
	if(verbose){
		fprintf(verboseStream,"# Closing files...\n");
	}
	if(outputFile!=stdout){
		fclose(outputFile);
	}
	if(networkFile!=stdin){
		fclose(networkFile);
	}
	if(verbose){
		fprintf(verboseStream,"# Done!\n");
	}
	return EXIT_SUCCESS;
}


int main(int argc, char** argv){
	return CVAccessibilityApplication(argc,argv);
}
