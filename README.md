# resynth-api

This project unifies two popular repositories for image resynthesis under a single C api.

The original projects can be found here:
https://github.com/bootchk/resynthesizer
https://github.com/notwa/resynth 

This API interfaces with both implementations. Since different versions support different features, not everything is supported.


## Building

To build the project run the provided `build.sh` script.

You can select either `notwa`'s backend by setting `-D USE_RESYNTH_GIMP=OFF` or GIMP's official implementation by `bootchk` setting `-D USE_RESYNTH_GIMP=ON`.

```bash
sh build.sh
```

## Supported Operations
Currently, resynth-api supports textures tiling and image patch healing operations.


## Running

The project includes a small demo to showcase the API usage. Please find it in `./apps/resynthcli.c`
