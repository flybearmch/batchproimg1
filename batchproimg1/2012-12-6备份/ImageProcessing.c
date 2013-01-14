 
//**************************************************************************
//* WARNING: This file was automatically generated.  Any changes you make  *
//*          to this file will be lost if you generate the file again.     *
//**************************************************************************
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nivision.h>
#include <nimachinevision.h>
#include <windows.h>

// If you call Machine Vision functions in your script, add NIMachineVision.c to the project.

#define IVA_MAX_BUFFERS 10
#define IVA_STORE_RESULT_NAMES

#define VisionErrChk(Function) {if (!(Function)) {success = 0; goto Error;}}

typedef enum IVA_ResultType_Enum {IVA_NUMERIC, IVA_BOOLEAN, IVA_STRING} IVA_ResultType;

typedef union IVA_ResultValue_Struct    // A result in Vision Assistant can be of type double, BOOL or string.
{
    double numVal;
    BOOL   boolVal;
    char*  strVal;
} IVA_ResultValue;

typedef struct IVA_Result_Struct
{
#if defined (IVA_STORE_RESULT_NAMES)
    char resultName[256];           // Result name
#endif
    IVA_ResultType  type;           // Result type
    IVA_ResultValue resultVal;      // Result value
} IVA_Result;

typedef struct IVA_StepResultsStruct
{
#if defined (IVA_STORE_RESULT_NAMES)
    char stepName[256];             // Step name
#endif
    int         numResults;         // number of results created by the step
    IVA_Result* results;            // array of results
} IVA_StepResults;

typedef struct IVA_Data_Struct
{
    Image* buffers[IVA_MAX_BUFFERS];            // Vision Assistant Image Buffers
    IVA_StepResults* stepResults;              // Array of step results
    int numSteps;                               // Number of steps allocated in the stepResults array
    CoordinateSystem *baseCoordinateSystems;    // Base Coordinate Systems
    CoordinateSystem *MeasurementSystems;       // Measurement Coordinate Systems
    int numCoordSys;                            // Number of coordinate systems
} IVA_Data;



static IVA_Data* IVA_InitData(int numSteps, int numCoordSys);
static int IVA_DisposeData(IVA_Data* ivaData);
static int IVA_DisposeStepResults(IVA_Data* ivaData, int stepIndex);
static int IVA_Particle(Image* image,
                                 int connectivity,
                                 int pPixelMeasurements[],
                                 int numPixelMeasurements,
                                 int pCalibratedMeasurements[],
                                 int numCalibratedMeasurements,
                                 IVA_Data* ivaData,
                                 int stepIndex);

int IVA_ProcessImage(Image *image)
{
	int success = 1;
    IVA_Data *ivaData;
    int pKernel[25] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1};
    StructuringElement structElem;
    int pPixelMeasurements[81] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,35,36,
        37,38,39,41,42,43,45,46,48,49,50,51,53,54,55,56,58,59,60,61,
        62,63,64,65,66,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,
        83,84,85,86,87,88};
    int *pCalibratedMeasurements = 0;

    // Initializes internal data (buffers and array of points for caliper measurements)
    VisionErrChk(ivaData = IVA_InitData(4, 0));

    //-------------------------------------------------------------------//
    //                          Manual Threshold                         //
    //-------------------------------------------------------------------//

    // Thresholds the image.
    VisionErrChk(imaqThreshold(image, image, 0, 30, TRUE, 1));

    //-------------------------------------------------------------------//
    //                          Basic Morphology                         //
    //-------------------------------------------------------------------//

    // Sets the structuring element.
    structElem.matrixCols = 5;
    structElem.matrixRows = 5;
    structElem.hexa = FALSE;
    structElem.kernel = pKernel;

    // Applies a morphological transformation to the binary image.
    VisionErrChk(imaqMorphology(image, image, IMAQ_OPEN, &structElem));

    //-------------------------------------------------------------------//
    //             Advanced Morphology: Remove Border Objects            //
    //-------------------------------------------------------------------//

    // Eliminates particles touching the border of the image.
    VisionErrChk(imaqRejectBorder(image, image, TRUE));

	VisionErrChk(IVA_Particle(image, TRUE, pPixelMeasurements, 81, 
		pCalibratedMeasurements, 0, ivaData, 3));

    // Releases the memory allocated in the IVA_Data structure.
    IVA_DisposeData(ivaData);

Error:
	return success;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function Name: IVA_Particle
//
// Description  : Computes the number of particles detected in a binary image and
//                a 2D array of requested measurements about the particle.
//
// Parameters   : image                      -  Input image
//                connectivity               -  Set this parameter to 1 to use
//                                              connectivity-8 to determine
//                                              whether particles are touching.
//                                              Set this parameter to 0 to use
//                                              connectivity-4 to determine
//                                              whether particles are touching.
//                pixelMeasurements          -  Array of measuremnets parameters
//                numPixelMeasurements       -  Number of elements in the array
//                calibratedMeasurements     -  Array of measuremnets parameters
//                numCalibratedMeasurements  -  Number of elements in the array
//                ivaData                    -  Internal Data structure
//                stepIndex                  -  Step index (index at which to store
//                                              the results in the resuts array)
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
static int IVA_Particle(Image* image,
                                 int connectivity,
                                 int pPixelMeasurements[],
                                 int numPixelMeasurements,
                                 int pCalibratedMeasurements[],
                                 int numCalibratedMeasurements,
                                 IVA_Data* ivaData,
                                 int stepIndex)
{
    int success = 1;
    int numParticles;
    double* pixelMeasurements = NULL;
    double* calibratedMeasurements = NULL;
    unsigned int visionInfo;
    IVA_Result* particleResults;
    int i;
    int j;
    double centerOfMassX;
    double centerOfMassY;


    //-------------------------------------------------------------------//
    //                         Particle Analysis                         //
    //-------------------------------------------------------------------//

    // Counts the number of particles in the image.
    VisionErrChk(imaqCountParticles(image, connectivity, &numParticles));

    // Allocate the arrays for the measurements.
    pixelMeasurements = (double*)malloc(numParticles * numPixelMeasurements * sizeof(double));
    calibratedMeasurements = (double*)malloc(numParticles * numCalibratedMeasurements * sizeof(double));

    // Delete all the results of this step (from a previous iteration)
    IVA_DisposeStepResults(ivaData, stepIndex);

    // Check if the image is calibrated.
    VisionErrChk(imaqGetVisionInfoTypes(image, &visionInfo));

    // If the image is calibrated, we also need to log the calibrated position (x and y)
    ivaData->stepResults[stepIndex].numResults = (visionInfo & IMAQ_VISIONINFO_CALIBRATION ?
                                                  numParticles * 4 + 1 : numParticles * 2 + 1);
    ivaData->stepResults[stepIndex].results = malloc (sizeof(IVA_Result) * ivaData->stepResults[stepIndex].numResults);
    
    particleResults = ivaData->stepResults[stepIndex].results;

    #if defined (IVA_STORE_RESULT_NAMES)
        sprintf(particleResults->resultName, "Object #");
    #endif
    particleResults->type = IVA_NUMERIC;
    particleResults->resultVal.numVal = numParticles;
    particleResults++;
    
    for (i = 0 ; i < numParticles ; i++)
    {
        // Computes the requested pixel measurements about the particle.
        for (j = 0 ; j < numPixelMeasurements ; j++)
        {
            VisionErrChk(imaqMeasureParticle(image, i, FALSE, pPixelMeasurements[j], &pixelMeasurements[i*numPixelMeasurements + j]));
        }

        // Computes the requested calibrated measurements about the particle.
        for (j = 0 ; j < numCalibratedMeasurements ; j++)
        {
            VisionErrChk(imaqMeasureParticle(image, i, TRUE, pCalibratedMeasurements[j], &calibratedMeasurements[i*numCalibratedMeasurements + j]));
        }
        
        #if defined (IVA_STORE_RESULT_NAMES)
            sprintf(particleResults->resultName, "Particle %d.X Position (Pix.)", i + 1);
        #endif
        particleResults->type = IVA_NUMERIC;
        VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_CENTER_OF_MASS_X, &centerOfMassX));
        particleResults->resultVal.numVal = centerOfMassX;
        particleResults++;

        #if defined (IVA_STORE_RESULT_NAMES)
            sprintf(particleResults->resultName, "Particle %d.Y Position (Pix.)", i + 1);
        #endif
        particleResults->type = IVA_NUMERIC;
        VisionErrChk(imaqMeasureParticle(image, i, FALSE, IMAQ_MT_CENTER_OF_MASS_Y, &centerOfMassY));
        particleResults->resultVal.numVal = centerOfMassY;
        particleResults++;

        if (visionInfo & IMAQ_VISIONINFO_CALIBRATION)
        {
            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(particleResults->resultName, "Particle %d.X Position (Calibrated)", i + 1);
            #endif
            particleResults->type = IVA_NUMERIC;
            VisionErrChk(imaqMeasureParticle(image, i, TRUE, IMAQ_MT_CENTER_OF_MASS_X, &centerOfMassX));
            particleResults->resultVal.numVal = centerOfMassX;
            particleResults++;

            #if defined (IVA_STORE_RESULT_NAMES)
                sprintf(particleResults->resultName, "Particle %d.Y Position (Calibrated)", i + 1);
            #endif
            particleResults->type = IVA_NUMERIC;
            VisionErrChk(imaqMeasureParticle(image, i, TRUE, IMAQ_MT_CENTER_OF_MASS_Y, &centerOfMassY));
            particleResults->resultVal.numVal = centerOfMassY;
            particleResults++;
        }
    }

Error:
    free(pixelMeasurements);
    free(calibratedMeasurements);

    return success;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: IVA_InitData
//
// Description  : Initializes data for buffer management and results.
//
// Parameters   : # of steps
//                # of coordinate systems
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
static IVA_Data* IVA_InitData(int numSteps, int numCoordSys)
{
    int success = 1;
    IVA_Data* ivaData = NULL;
    int i;


    // Allocate the data structure.
    VisionErrChk(ivaData = (IVA_Data*)malloc(sizeof (IVA_Data)));

    // Initializes the image pointers to NULL.
    for (i = 0 ; i < IVA_MAX_BUFFERS ; i++)
        ivaData->buffers[i] = NULL;

    // Initializes the steo results array to numSteps elements.
    ivaData->numSteps = numSteps;

    ivaData->stepResults = (IVA_StepResults*)malloc(ivaData->numSteps * sizeof(IVA_StepResults));
    for (i = 0 ; i < numSteps ; i++)
    {
        #if defined (IVA_STORE_RESULT_NAMES)
            sprintf(ivaData->stepResults[i].stepName, "");
        #endif
        ivaData->stepResults[i].numResults = 0;
        ivaData->stepResults[i].results = NULL;
    }

    // Create the coordinate systems
    ivaData->baseCoordinateSystems = (CoordinateSystem*)malloc(sizeof(CoordinateSystem) * numCoordSys);
    ivaData->MeasurementSystems = (CoordinateSystem*)malloc(sizeof(CoordinateSystem) * numCoordSys);
    ivaData->numCoordSys = numCoordSys;

Error:
    return ivaData;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: IVA_DisposeData
//
// Description  : Releases the memory allocated in the IVA_Data structure
//
// Parameters   : ivaData  -  Internal data structure
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
static int IVA_DisposeData(IVA_Data* ivaData)
{
    int i;


    // Releases the memory allocated for the image buffers.
    for (i = 0 ; i < IVA_MAX_BUFFERS ; i++)
        imaqDispose(ivaData->buffers[i]);

    // Releases the memory allocated for the array of measurements.
    for (i = 0 ; i < ivaData->numSteps ; i++)
        IVA_DisposeStepResults(ivaData, i);

    free(ivaData->stepResults);
    ivaData->numSteps = 0;

    // Dispose of coordinate systems
    if (ivaData->numCoordSys)
    {
        free(ivaData->baseCoordinateSystems);
        free(ivaData->MeasurementSystems);
    }

    free(ivaData);

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: IVA_DisposeStepResults
//
// Description  : Dispose of the results of a specific step.
//
// Parameters   : ivaData    -  Internal data structure
//                stepIndex  -  step index
//
// Return Value : success
//
////////////////////////////////////////////////////////////////////////////////
static int IVA_DisposeStepResults(IVA_Data* ivaData, int stepIndex)
{
    int i;

    
    for (i = 0 ; i < ivaData->stepResults[stepIndex].numResults ; i++)
    {
        if (ivaData->stepResults[stepIndex].results[i].type == IVA_STRING)
            free(ivaData->stepResults[stepIndex].results[i].resultVal.strVal);
    }

    free(ivaData->stepResults[stepIndex].results);

    return TRUE;
}


