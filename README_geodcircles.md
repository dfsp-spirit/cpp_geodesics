# geodcircles

A command line program for fast and parallel computation of geodesic mean distances and geodesic circle stats on brain surface meshes.


## About

The `geodcircles` software is a command line program for computing descriptors related to geodesics on reconstructed human brain surfaces. It works on the output of FreeSurfer, i.e., on the SUBJECTS_DIR produced by running the FreeSurfer `recon_all` program on a directory of human brain scans (MRI images). The `geodcircles` program is written in C++ and uses OpenMP for parallelization.


## Required input files and downsampling FreeSurfer meshes

If you want to run on full-resolution meshes, all you need is the standard `recon-all` output for your subjects.

Due to the high mesh resolution and computational cost of running geodesics computation on huge meshes, it is common to compute the descriptors on down-sampled meshes. FreeSurfer refers to the full mesh resolution as ICO7, that is the resolution of fsaverage and the full native space meshes like `<subject>/surf/lh.pial`. The full resolution results in about 160k vertices per hemisphere. The mesh resolution of fsaverage6, known as ICO6, reduces the vertex count by a factor of about 4, to 40k vertices. One could go even lower (ICO5, about 10k  vertices) but that should not be needed if you have some computational resources available.

To use down-sampled meshes, you will have to generate them, and the related down-sampled labels if you want to restrict computations to the cortex (as opposed to the medial wall), using FreeSurfer command line tools `mri_surf2surf` and `mri_label2label`. I have bash scripts available for doing that in parallel for many subjects in my [freesurfer_parallel_scripts repo](https://github.com/dfsp-spirit/freesurfer_parallel_scripts/tree/main/tools) if you are interested. The script you want is called `parallel_downsample_label.bash`, but it uses other scripts from the same repo (`downsample_label.bash`), so it is safer to just clone the full repo.


### Installation and compiling

See instructions in the [README.md file](./README.md) in this repo.

## Usage

Run `./geodcircles` to see the usage help:

```
./geodcircles
== Compute mean geodesic distances and circle stats for FreeSurfer brain meshes ==.
Usage: ./geodcircles <subjects_file> [<subjects_dir> [<surface> [<do_circle_stats> [<keep_existing> [<circ_scale> [<cortex_label> [<hemi>]]]]]]]
  <subjects_file> : text file containing one subject identifier per line.
  <subjects_dir>  : directory containing the FreeSurfer recon-all output for the subjects. Defaults to current working directory.
  <surface>       : the surface file to load from the surf/ subdir of each subject, without hemi part. Defaults to 'pial'.
  <do_circle_stat>: flag whether to compute geodesic circle stats as well, must be 0 (off), 1 (on) or 2 (on with mean dists). Defaults to 2.
  <keep_existing> : flag whether to keep existing output files, must be 'no' (off: recompute and overwrite files. aliases: '0' and 'false' are also supported), or 'yes' (keep existing files, skip computation if exists. aliases '1' and 'true' are also supported). Defaults to 1.
  <circ_scale>    : int, the fraction of the total surface that the circles for the geodesic circle stats should have (in percent). Ignored if do_circle_stats is 0. Defaults to 5.
  <cortex_label>  : str, optional file name of a cortex label file, without the hemi prefix to load from the label/ subdir of each subject. If given, load label and ignore non-label vertices, typically the medial wall, during all computations. Defaults to the empty string, i.e., no cortex label file. E.g., 'cortex.label'. Can be set to 'none' to turn off.
  <hemi>          : str, which hemispheres to compute. One of 'lh', 'rh' or 'both'. Defaults to 'both'.
  <write_mgh>     : flag whether to write extra output files in MGH format (in addition to curv format), must be 'no' (off: only curv format) or 'yes' (on: write curv and MGH formats).  Aliases '1' / 'true', or '0' / 'false' are also supported. Defaults to 0.
NOTES:
 * Sorry for the current command line parsing state: you will have to supply all arguments if you want to change the last one.
 * We recommend to run this on simplified meshes to save computation time, e.g., by scaling the vertex count to that of fsaverage6. If you do that and use the cortex_label parameter, you will of course also need scaled cortex labels.
 * The output files will be written to the surf/ subdir of each subject. They are in FreeSurver curv format.
```

You can see the default values for all command line arguments at the end of the respective help text.

### Some example command lines

In the following examples, we change to your `SUBJECTS_DIR` with the `recon_all` output and run `geodcircles` in there. We assume that you:

* have cloned the cpp_geodesics repository to `~/cpp_geodesics` and built the applications. See the [README.md  file in this repo](./README.md) if you have not done that yet.
* that you have a text file with one subject identifier per line, named `subjects.txt`, in there. People who use FreeSurfer will know these files, and most likely already have one for their study.


#### Example 1

In this example, we want to run `geodcircles` with default settings in the current working directory, and use the mentioned `subjects.txt` file you provide.

Note: Here we assume that you have checked out this repo to the path ```~/cpp_geodesics/``` (which typically evaluates to something like ```/home/your_user/cpp_geodesics/```) and that you already compiled the `geodcircles` app in there, i.e., the executable file ```~/cpp_geodesics/geodcircles``` should exist on your system.

```shell
cd ~/data/study1/freesurfer_output/       # or whereever your data is
~/cpp_geodesics/geodcircles subjects.txt
```

This will run on the full resolution meshes, and may take quite a while. On a single core machine, this will take several hours per mesh (hemisphere). It will not use any cortex labels, so the computations will run on the full mesh, including the medial wall.

#### Example 2

In this example, we assume that you have created down-sampled meshes, as dicussed above in section `Required input files and downsampling FreeSurfer meshes`. We assume that for each subject, the ICO6-downsampled meshes are stored in the files `<subject>/surf/lh.pialsurface6` and `<subject>/surf/rh.pialsurface6`. We also assume that you want to exclude the medial wall during computations, and that you therefore also created down-sampled cortex labels for each subject, which are stored in the files `<subject>/label/lh.cortex6.label` and `<subject>/label/rh.cortex6.label`.

Again, we want to run `geodcircles` in the current working directory, and use the mentioned `subjects.txt` file you provide:

```shell
cd ~/data/study1/freesurfer_output/       # or whereever your data is
~/cpp_geodesics/geodcircles subjects.txt . pialsurface6 2 yes 5 cortex6.label
```

Note that we specified a dot ('`.`') as the working directory, as we want to use the current directory. You could also give the full path `~/cpp_geodesics/geodcircles`, of course, but why type more than needed? In detail, we used the following options:

* `subjects_file`=`subjects.txt`
* `subjects_dir`=`.`
* `surface`=`pialsurface6`
* `do_circle_stat`=`2` (on with mean dists, see help text above)
* `keep_existing`=`yes`  (if output file exists, skip mesh and keep existing results in file)
* `circ_scale`=`5`     (use 5 percent of area for circles)
* `cortex_label`=`cortex6.label`   (restrict vertices of mesh to those listed in the label file, delete all others for the computation. The results in the written surface overlay files will have NAN values for deleted vertices.)

We did not provide a value for the parameters `hemi` and `write_mgh` so the defaults were used. You can find out about the defaults from the help output that is printed when you run the application without any arguments.

## Information on the output files

The output files are written to the respective subject directory. The file names are constructed from the hemisphere, descriptor, and settings used. They are printed during the computation.

By default, output is written in FreeSurfer curv format. You can use command line arguments to also write in MGH format.

The output is in native space of the subjects, so you would need to map this to some standard space (typically fsaverage, or a downsampled version of it) for group comparison.


## Where to find information on installation, compiling, author, getting help, ...

Please see the main [README file](./README.md) of this repo for instructions on installation, getting help, etc. This file only gives a short usage overview for the `geodcircles` program.
