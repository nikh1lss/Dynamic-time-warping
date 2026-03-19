#include "DTW.h"

void readSequence(FILE *seqFile, int *seqLen, long double **seq) {
    char *seqText = NULL;
    /* Read in text. */
    size_t allocated = 0;
    /* Exit if we read no characters or an error caught. */
    int success = getdelim(&seqText, &allocated, '\0', seqFile);

    if (success == -1) {
        /* Encountered an error. */
        perror("Encountered error reading dictionary file");
        exit(EXIT_FAILURE);
    } else {

        /* Assume file contains at least one character. */
        assert(success > 0);
    }

    /* Progress through string. */
    int progress = 0;
    /* Table string length. */
    int seqTextLength = strlen(seqText);

    int commaCount = 0;
    /* Count how many numbers are present. */
    for (int i = 0; i < seqTextLength; i++) {
        if (seqText[i] == ',') {
            commaCount++;
        }
    }
    long double *seqLocal =
        (long double *)malloc(sizeof(long double) * (commaCount + 1));
    assert(seqLocal);

    int seqAdded = 0;
    while (progress < seqTextLength) {
        int nextProgress;
        /* Read each value into the sequence. */
        assert(sscanf(seqText + progress, "%Lf , %n", &seqLocal[seqAdded],
                      &nextProgress) == 1);
        assert(nextProgress > 0);
        progress += nextProgress;
        seqAdded++;
    }
    assert(seqAdded == (commaCount + 1));
    *seq = seqLocal;
    *seqLen = seqAdded;
}

/*
    Reads the given dict file into a list of words
    and the given board file into a nxn board.
*/
struct problem *readProblemA(FILE *seqAFile, FILE *seqBFile) {
    struct problem *p = (struct problem *)malloc(sizeof(struct problem));
    assert(p);

    int seqALength = 0;
    long double *seqA = NULL;
    readSequence(seqAFile, &seqALength, &seqA);
    int seqBLength = 0;
    long double *seqB = NULL;
    readSequence(seqBFile, &seqBLength, &seqB);

    /* The length of the first sequence. */
    p->seqALength = seqALength;
    /* The first sequence. */
    p->sequenceA = seqA;

    /* The length of the second sequence. */
    p->seqBLength = seqBLength;
    /* The second sequence. */
    p->sequenceB = seqB;

    /* For Part D & F only. */
    p->windowSize = -1;
    p->maximumPathLength = -1;

    p->part = PART_A;

    return p;
}

struct problem *readProblemD(FILE *seqAFile, FILE *seqBFile, int windowSize) {
    /* Fill in Part A sections. */
    struct problem *p = readProblemA(seqAFile, seqBFile);

    p->part = PART_D;
    p->windowSize = windowSize;

    return p;
}

struct problem *readProblemF(FILE *seqAFile, FILE *seqBFile,
                             int maxPathLength) {
    /* Interpretation of inputs is same as Part A. */
    struct problem *p = readProblemA(seqAFile, seqBFile);

    p->part = PART_F;
    p->maximumPathLength = maxPathLength;

    return p;
}

/*
    Outputs the given solution to the given file. If colourMode is 1, the
    sentence in the problem is coloured with the given solution colours.
*/
void outputProblem(struct problem *problem, struct solution *solution,
                   FILE *outfileName) {
    assert(solution);
    fprintf(outfileName, "%.2Lf\n", solution->optimalValue);
    switch (problem->part) {
    case PART_A:
        assert(solution->matrix);
        for (int i = 1; i <= problem->seqALength; i++) {
            for (int j = 1; j <= problem->seqBLength; j++) {
                if (solution->matrix[i][j] == LDINFINITY) {
                    fprintf(outfileName, "    ");
                } else {
                    fprintf(outfileName, "%.2Lf", solution->matrix[i][j]);
                }
                if (j < (problem->seqBLength)) {
                    /* Intercalate with spaces. */
                    fprintf(outfileName, " ");
                }
            }
            fprintf(outfileName, "\n");
        }
        break;
    case PART_D:
    case PART_F:
        break;
    }
}

/*
    Frees the given solution and all memory allocated for it.
*/
void freeSolution(struct solution *solution, struct problem *problem) {
    if (solution) {
        if (solution->matrix) {
            for (int i = 0; i < problem->seqALength; i++) {
                free(solution->matrix[i]);
            }
            free(solution->matrix);
        }
        free(solution);
    }
}

/*
    Frees the given problem and all memory allocated for it.
*/
void freeProblem(struct problem *problem) {
    if (problem) {
        if (problem->sequenceA) {
            free(problem->sequenceA);
        }
        if (problem->sequenceB) {
            free(problem->sequenceB);
        }
        free(problem);
    }
}

/* Sets up a solution for the given problem
 * s->matrix is a seqAlength + 1 x seqBlength + 1 matrix initialised to 0
 */
struct solution *newSolution(struct problem *problem) {
    struct solution *s = (struct solution *)malloc(sizeof(struct solution));
    assert(s);
    if (problem->part == PART_F) {
        s->matrix = NULL;
    } else {
        s->matrix = (long double **)malloc(sizeof(long double *) *
                                           (problem->seqALength + 1));
        assert(s->matrix);
        for (int i = 0; i < (problem->seqALength + 1); i++) {
            s->matrix[i] = (long double *)malloc(sizeof(long double) *
                                                 (problem->seqBLength + 1));
            assert(s->matrix[i]);
            for (int j = 0; j < (problem->seqBLength + 1); j++) {
                s->matrix[i][j] = 0;
            }
        }
    }

    s->optimalValue = -1;

    return s;
}

/* Returns the minimum value between three inputs */
long double minimum(long double v1, long double v2, long double v3) {
    long double min = v1;
    if (v2 < min) {
        min = v2;
    }
    if (v3 < min) {
        min = v3;
    }
    return min;
}

/*
    Solves the given problem according to Part A's definition
    and places the solution output into a returned solution value.
*/
struct solution *solveProblemA(struct problem *p) {
    struct solution *s = newSolution(p);
    /* Fill in: Part A */

    /* Initialise the DTW matrix with LDINFINITY */
    for (int i = 1; i < p->seqALength + 1; i++) {
        s->matrix[i][0] = LDINFINITY;
    }
    for (int j = 1; j < p->seqBLength + 1; j++) {
        s->matrix[0][j] = LDINFINITY;
    }

    /* Populate the DTW matrix */
    for (int i = 1; i < p->seqALength + 1; i++) {
        for (int j = 1; j < p->seqBLength + 1; j++) {
            s->matrix[i][j] = fabsl(p->sequenceA[i - 1] - p->sequenceB[j - 1]) +
                              minimum(s->matrix[i - 1][j], s->matrix[i][j - 1],
                                      s->matrix[i - 1][j - 1]);
        }
    }
    s->optimalValue = s->matrix[p->seqALength][p->seqBLength];
    return s;
}

struct solution *solveProblemD(struct problem *p) {
    struct solution *s = newSolution(p);
    /* Fill in: Part D */
    int windowSize = p->windowSize;

    /* Initialize the DTW matrix with LDINFINITY */
    for (int i = 0; i < p->seqALength + 1; i++) {
        for (int j = 0; j < p->seqBLength + 1; j++) {
            s->matrix[i][j] = LDINFINITY;
        }
    }
    s->matrix[0][0] = 0.0;

    /* Populate the DTW matrix within the boundary constraints */
    for (int i = 1; i < p->seqALength + 1; i++) {
        for (int j = fmax(1, i - windowSize);
             j < fmin(p->seqBLength, i + windowSize) + 1; j++) {
            s->matrix[i][j] = fabsl(p->sequenceA[i - 1] - p->sequenceB[j - 1]) +
                              minimum(s->matrix[i - 1][j], s->matrix[i][j - 1],
                                      s->matrix[i - 1][j - 1]);
        }
    }

    /* Set the optimal value */
    s->optimalValue = s->matrix[p->seqALength][p->seqBLength];
    return s;
}

struct solution *solveProblemF(struct problem *p) {
    struct solution *s = newSolution(p);
    /* Fill in: Part F */
    int maxPathLength = p->maximumPathLength;

    /* Allocate 3D matrix for DTW calculation */
    long double ***dtwMatrix =
        (long double ***)malloc((p->seqALength + 1) * sizeof(long double **));
    assert(dtwMatrix);
    for (int i = 0; i < p->seqALength + 1; i++) {
        dtwMatrix[i] =
            (long double **)malloc((p->seqBLength + 1) * sizeof(long double *));
        assert(dtwMatrix[i]);
        for (int j = 0; j < p->seqBLength + 1; j++) {
            dtwMatrix[i][j] = (long double *)malloc((maxPathLength + 1) *
                                                    sizeof(long double));
            assert(dtwMatrix[i][j]);
            for (int k = 0; k < maxPathLength + 1; k++) {
                dtwMatrix[i][j][k] = LDINFINITY;
            }
        }
    }
    dtwMatrix[0][0][0] = 0.0;

    /* Populate the DTW matrix with total path length constraint */
    for (int k = 1; k < maxPathLength + 1; k++) {
        for (int i = 0; i < p->seqALength + 1; i++) {
            for (int j = 0; j < p->seqBLength + 1; j++) {
                if (i > 0) {
                    dtwMatrix[i][j][k] =
                        fmin(dtwMatrix[i][j][k],
                             dtwMatrix[i - 1][j][k - 1] +
                                 fabsl(p->sequenceA[i - 1] - p->sequenceB[j]));
                }
                if (j > 0) {
                    dtwMatrix[i][j][k] =
                        fmin(dtwMatrix[i][j][k],
                             dtwMatrix[i][j - 1][k - 1] +
                                 fabsl(p->sequenceA[i] - p->sequenceB[j - 1]));
                }
                if (i > 0 && j > 0) {
                    dtwMatrix[i][j][k] = fmin(
                        dtwMatrix[i][j][k],
                        dtwMatrix[i - 1][j - 1][k - 1] +
                            fabsl(p->sequenceA[i - 1] - p->sequenceB[j - 1]));
                }
            }
        }
    }

    /* Find the minimum cost among all feasible path lengths */
    s->optimalValue = LDINFINITY;
    for (int k = 0; k < maxPathLength + 1; k++) {
        s->optimalValue =
            fmin(s->optimalValue, dtwMatrix[p->seqALength][p->seqBLength][k]);
    }

    /* Free allocated memory for DTW matrix */
    for (int i = 0; i < p->seqALength + 1; i++) {
        for (int j = 0; j < p->seqBLength + 1; j++) {
            free(dtwMatrix[i][j]);
        }
        free(dtwMatrix[i]);
    }
    free(dtwMatrix);

    return s;
}
