#include "../resynth.h"
#include "engine.h"


// API Functions
/* Image and Buffer Loading */
resynth_state_t
resynth_state_create_from_image(const char* filename, int desired_channels, int scale) {
    resynth_state_t s = calloc(1, sizeof(Resynth_state));
    int w, h, d;
    uint8_t *image = stbi_load(filename, &w, &h, &d, desired_channels);
    if (image == NULL) {
        fprintf(stderr, "invalid image: %s\n", filename);
        return NULL;
    }

    IMAGE_RESIZE(s->corpus, w, h, desired_channels);
    memcpy(s->corpus_array, image, w * h * desired_channels);

    s->input_bytes = desired_channels;
    {
        int data_w = 256, data_h = 256;
        if (scale > 0) data_w = scale * w, data_h = scale * h;
        if (scale < 0) data_w = -scale, data_h = -scale;
        IMAGE_RESIZE(s->data, data_w, data_h, s->input_bytes);
    }

    stbi_image_free(image);
    return s;
}

resynth_state_t
resynth_state_create_from_memory(uint8_t* pixels, size_t width, size_t height, size_t channels, int scale) {
    assert(pixels != NULL);
    assert(width > 0);
    assert(height > 0);
    assert(channels >= 3);

    resynth_state_t s = calloc(1, sizeof(Resynth_state));

    IMAGE_RESIZE(s->corpus, width, height, channels);
    memcpy(s->corpus_array, pixels, width * height * channels);

    s->input_bytes = channels;
    {
        int data_w = 256, data_h = 256;
        if (scale > 0) data_w = scale * width, data_h = scale * height;
        if (scale < 0) data_w = -scale, data_h = -scale;
        IMAGE_RESIZE(s->data, data_w, data_h, s->input_bytes);
    }

    return s;
}

resynth_state_t
resynth_state_create_from_memoryf(float* pixels, size_t width, size_t height, size_t channels, int scale) {
    size_t size = width * height * channels;
    uint8_t* pixels_u8 = calloc(size, sizeof(uint8_t));
    for (size_t i = 0; i < size; ++i) {
        /*pixels_u8[i] = (uint8_t)CLAMP(pixels[i] * 255, 0, 255);*/
        pixels_u8[i] = (uint8_t)fmin(255, fmax(0, (pixels[i] * 255)));
    }
    resynth_state_t state =  resynth_state_create_from_memory(pixels_u8, width, height, channels, scale);

    free(pixels_u8);
    return state;
}

/* Config */
resynth_parameters_t
resynth_parameters_create() {
    resynth_parameters_t parameters = calloc(1, sizeof(Parameters));
    parameters->v_tile = false;
    parameters->h_tile = false;
    parameters->magic = 192;         // 192 (3/4)
    parameters->autism = 32. / 256.; // 30. / 256.
    parameters->neighbors = 29;      // 30
    parameters->tries = 192;         // 200 (or 80 in the paper)
    parameters->random_seed = time(0);
    return parameters;
}

void
resynth_parameters_operation(resynth_parameters_t parameters, resynth_operation_t operation) {
    /* This version of resynth does not support healing */
    assert(operation == RESYNTH_OPERATION_TEXTURE);
}

void
resynth_parameters_mask(resynth_parameters_t parameters, uint8_t* pixels, size_t widht, size_t height) {
    /* This version of resynth does not support masking */
}


void
resynth_parameters_h_tile(resynth_parameters_t parameters, bool h_tile) {
    parameters->h_tile = h_tile;
}

void
resynth_parameters_v_tile(resynth_parameters_t parameters, bool v_tile) {
    parameters->v_tile = v_tile;
}

void
resynth_parameters_outlier_sensitivity(resynth_parameters_t parameters, double sensitivity) {
    parameters->autism = CLAMPV(sensitivity, 0., 1.);
}

void
resynth_parameters_neighbors(resynth_parameters_t parameters, int neighbors) {
    parameters->neighbors = CLAMPV(neighbors, 0, disc00[LEN(disc00) - 1]);
}

void
resynth_parameters_tries(resynth_parameters_t parameters, int tries) {
    parameters->tries = CLAMPV(tries, 0, 65536);
}

void
resynth_parameters_magic(resynth_parameters_t parameters, int magic) {
    parameters->magic = CLAMPV(magic, 0, 255);
}

void
resynth_parameters_random_seed(resynth_parameters_t parameters, unsigned long seed) {
    parameters->random_seed = seed;
}

/* Processing and Results */ 
resynth_result_t 
resynth_run(resynth_state_t state, resynth_parameters_t parameters) {
    assert(state != NULL);
    assert(parameters != NULL);

    rnd_pcg_seed(&pcg, parameters->random_seed);

    resynth_result_t result = calloc(1, sizeof(Resynth_result));
    resynth(state, *parameters);

    result->pixels = state->data_array;
    result->width = state->data.width;
    result->height = state->data.height;
    result->channels = state->data.depth;
    result->valid = true;
    return result;
}

bool 
resynth_result_valid(resynth_result_t result) {
    return result->valid;
}

uint8_t* 
resynth_result_pixels(resynth_result_t result) {
    return result->pixels;
}

float* 
resynth_result_pixelsf(resynth_result_t result) {
    if (result->pixelsf != NULL)
        return result->pixelsf;
    size_t size = result->width * result->height * result->channels;
    float* pixels_f32 = calloc(size, sizeof(float));

    for (size_t i = 0; i < size; ++i) {
        pixels_f32[i] = ((float)(result->pixels[i]) / 255.f);
    }
    result->pixelsf = pixels_f32;

    return result->pixelsf;
}

size_t
resynth_result_width(resynth_result_t result) {
    return result->width;
}

size_t
resynth_result_height(resynth_result_t result) {
    return result->height;
}

size_t
resynth_result_channels(resynth_result_t result) {
    return result->channels;
}


/* Memory Management */ 
void
resynth_free_state(resynth_state_t state) {
    state_free(state);
    free(state);
}

void
resynth_free_parameters(resynth_parameters_t parameters) {
    free(parameters);
}

void
resynth_free_result(resynth_result_t result) {
    if (result->pixelsf != NULL)
        free(result->pixelsf);
    free(result);
}
