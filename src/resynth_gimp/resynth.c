#include "../resynth.h"
#include "imageSynth.h"
#include <string.h>

// decide which features we want from stb_image.
// this should cover the most common formats.
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF
#include "stb_image.h"


struct _Resynth_state {
    ImageBuffer* imageBuffer;
    TImageFormat imageFormat;
};

struct _Parameters {
    TImageSynthParameters* parameters;
    ImageBuffer* mask;
    ImageBuffer* mask2;
    resynth_operation_t op;
};

struct _Resynth_result {
    ImageBuffer* imageBuffer;
    ImageBuffer* imageBufferf;
    TImageFormat imageFormat;
    bool valid;
};

/* Helper functions */
static void _resynth_progress_callback(int progress, void* userdataptr) {
    printf("%d\n", progress);
}

void
_resynth_create_default_masks(resynth_parameters_t parameters, resynth_state_t state) {
    free(parameters->mask);
    free(parameters->mask2);

    size_t width = state->imageBuffer->width;
    size_t height = state->imageBuffer->height;

    size_t mask_size = state->imageBuffer->width * state->imageBuffer->height;

    parameters->mask = calloc(1, sizeof(ImageBuffer));
    parameters->mask->width = width;
    parameters->mask->height = height;
    parameters->mask->rowBytes = width * sizeof(uint8_t);
    parameters->mask->data = calloc(mask_size, sizeof(uint8_t));
    memset(parameters->mask->data, 0xFF, mask_size * sizeof(uint8_t));

    parameters->mask2 = calloc(1, sizeof(ImageBuffer));
    parameters->mask2->width = width;
    parameters->mask2->height = height;
    parameters->mask2->rowBytes = width * sizeof(uint8_t);
    parameters->mask2->data = calloc(mask_size, sizeof(uint8_t));
    memset(parameters->mask2->data, 0xFF, mask_size * sizeof(uint8_t));
}

/* Image and Buffer Loading */
resynth_state_t
resynth_state_create_from_image(const char* filename, int scale) {
    resynth_state_t state = calloc(1, sizeof(Resynth_state));
    int w, h, d;
    uint8_t *image = stbi_load(filename, &w, &h, &d, 0);
    if (image == NULL) {
        fprintf(stderr, "invalid image: %s\n", filename);
        return NULL;
    }
    if (d < 3 || d > 4) {
        fprintf(stderr, "invalid channel count: %d\n", d); 
        stbi_image_free(image);
        return NULL;
    }

    if (d == 4) {
        for (int i = 0; i < w * h; ++i) {
            image[(i*4)+3] = 255;
        }
    }

    ImageBuffer* imageBuffer = calloc(1, sizeof(ImageBuffer));
    imageBuffer->data = calloc(w * h * d, sizeof(uint8_t));
    memcpy(imageBuffer->data, image, w*h*d*sizeof(uint8_t));
    imageBuffer->width = w;
    imageBuffer->height = h;
    imageBuffer->rowBytes = w * d * sizeof(uint8_t);

    state->imageBuffer = imageBuffer;
    if (d == 4) {
        state->imageFormat = T_RGBA;
    } else {
        state->imageFormat = T_RGB;
    }

    stbi_image_free(image);
    return state;
}

resynth_state_t
resynth_state_create_from_memory(uint8_t* pixels, size_t width, size_t height, size_t channels, int scale) {
    return NULL;
}

resynth_state_t
resynth_state_create_from_memoryf(float* pixels, size_t width, size_t height, size_t channels, int scale) {
    return NULL;
}

/* Config */
resynth_parameters_t
resynth_parameters_create() {
    resynth_parameters_t params = calloc(1, sizeof(Parameters));
    params->parameters = calloc(1, sizeof(TImageSynthParameters));
    setDefaultParams(params->parameters);
    params->op = RESYNTH_OPERATION_TEXTURE;
    params->mask = NULL;
    params->mask2 = NULL;

    return params;
}

void
resynth_parameters_operation(resynth_parameters_t parameters, resynth_operation_t operation) {
    parameters->op = operation;
    if (operation == RESYNTH_OPERATION_TEXTURE) {
        parameters->parameters->matchContextType = 0;
    } else {
        parameters->parameters->matchContextType = 2;
    }
}

void
resynth_parameters_mask(resynth_parameters_t parameters, uint8_t* pixels, size_t width, size_t height) {
    free(parameters->mask);
    free(parameters->mask2);

    parameters->mask = calloc(1, sizeof(ImageBuffer));
    parameters->mask->width = width;
    parameters->mask->height = height;
    parameters->mask->rowBytes = width * sizeof(uint8_t);
    parameters->mask->data = calloc(1, width * height * sizeof(uint8_t));
    memcpy(parameters->mask->data, pixels, width * height * sizeof(uint8_t));

    parameters->mask2 = calloc(1, sizeof(ImageBuffer));
    parameters->mask2->width = width;
    parameters->mask2->height = height;
    parameters->mask2->rowBytes = width * sizeof(uint8_t);
    parameters->mask2->data = calloc(1, width * height * sizeof(uint8_t));

    for (int i = 0; i < width*height; ++i) {
        parameters->mask2->data[i] = parameters->mask->data[i] ? 0x00 : 0XFF;
    }
}

void
resynth_parameters_h_tile(resynth_parameters_t parameters, bool h_tile) {
    parameters->parameters->isMakeSeamlesslyTileableHorizontally = h_tile;
}

void
resynth_parameters_v_tile(resynth_parameters_t parameters, bool v_tile) {
    parameters->parameters->isMakeSeamlesslyTileableVertically = v_tile;
}

void
resynth_parameters_outlier_sensitivity(resynth_parameters_t parameters, double sensitivity) {
    parameters->parameters->sensitivityToOutliers = sensitivity;
}

void
resynth_parameters_neighbors(resynth_parameters_t parameters, int neighbors) {
    parameters->parameters->patchSize = neighbors;
}

void
resynth_parameters_tries(resynth_parameters_t parameters, int tries) {
    parameters->parameters->maxProbeCount = tries;
}

void
resynth_parameters_magic(resynth_parameters_t parameters, int magic) {
    /* gimp version of resynth seems to have dropped configurable magic number */
}

void
resynth_parameters_random_seed(resynth_parameters_t parameters, unsigned long seed) {
    /* gimp version of resynth takes care of random seeds */
}


/* Processing and Results */ 
resynth_result_t 
resynth_run(resynth_state_t state, resynth_parameters_t parameters) {

    // Make sure we have a valid mask
    if (parameters->mask == NULL) {
        _resynth_create_default_masks(parameters, state);
    }


    // "Simple API" does the healing operation
    if (parameters->op == RESYNTH_OPERATION_HEAL) {
        printf("Running healing op\n");
        int cancel_flag = 0;
        imageSynth(state->imageBuffer,
                parameters->mask,
                state->imageFormat,
                parameters->parameters,
                &_resynth_progress_callback, NULL, &cancel_flag);
    }

    // "Full API" does everything else
    if (parameters->op == RESYNTH_OPERATION_TEXTURE) {
        printf("Running texture op\n");
        int cancel_flag = 0;
        TImageSynthError result = imageSynth2(state->imageBuffer,
                parameters->mask,
                parameters->mask2,
                state->imageFormat,
                parameters->parameters,
                &_resynth_progress_callback, NULL, &cancel_flag);

        if (result != IMAGE_SYNTH_SUCCESS) {
            printf("Error running texture op: err(%d)\n", result);
        }
    }

    // Copy result, which is stored in the state over to a dedicated result variable
    resynth_result_t result = calloc(1, sizeof(Resynth_result));
    result->imageBuffer = calloc(1, sizeof(ImageBuffer));
    memcpy(result->imageBuffer, state->imageBuffer, sizeof(ImageBuffer));
    result->imageBuffer->data = calloc((state->imageBuffer->rowBytes * state->imageBuffer->height) / sizeof(uint8_t), sizeof(uint8_t));
    memcpy(result->imageBuffer->data, state->imageBuffer->data, state->imageBuffer->rowBytes * state->imageBuffer->height);
    result->imageFormat = state->imageFormat;
    result->valid = true;

    return result;
}

bool 
resynth_result_valid(resynth_result_t result) {
    return result->valid;
}

uint8_t* 
resynth_result_pixels(resynth_result_t result) {
    return result->imageBuffer->data;
}

float* 
resynth_result_pixelsf(resynth_result_t result) {
    if (result->imageBufferf == NULL) {
        size_t pixelel_count = resynth_result_channels(result);
        assert(pixelel_count > 0);

        size_t pixelel_size = sizeof(float) * pixelel_count;
        result->imageBufferf = calloc(1, sizeof(ImageBuffer));
        result->imageBufferf->width = result->imageBuffer->width;
        result->imageBufferf->height = result->imageBuffer->height;
        result->imageBufferf->rowBytes = result->imageBufferf->width * pixelel_size;
        result->imageBufferf->data = calloc(result->imageBufferf->width * result->imageBufferf->height, pixelel_size);
        for (int i = 0; i < result->imageBuffer->height * result->imageBuffer->width * pixelel_count; ++i) {
            result->imageBufferf->data[i] = (float)result->imageBuffer->data[i] / 255.f;
        }
    }

    return (float*)result->imageBufferf->data;
}

size_t
resynth_result_width(resynth_result_t result) {
    return result->imageBuffer->width;
}

size_t
resynth_result_height(resynth_result_t result) {
    return result->imageBuffer->height;
}

size_t
resynth_result_channels(resynth_result_t result) {
    if (result->imageFormat == T_RGB)
        return 3;
    if (result->imageFormat == T_RGBA)
        return 4;
    if (result->imageFormat == T_Gray)
        return 1;
    if (result->imageFormat == T_GrayA)
        return 2;
    return 0;
}

/* Memory Management */ 
void
resynth_free_state(resynth_state_t state) {
    free(state->imageBuffer->data);
    free(state->imageBuffer);
    free(state);
}

void
resynth_free_parameters(resynth_parameters_t parameters) {
    free(parameters->parameters);
    free(parameters->mask);
    free(parameters->mask2);
    free(parameters);
}

void
resynth_free_result(resynth_result_t result) {
    free(result->imageBuffer->data);
    free(result->imageBuffer);
    if (result->imageBufferf != NULL) {
        free(result->imageBufferf->data);
        free(result->imageBufferf);
    }
    free(result);
}


